#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <sys/time.h>
#include <chrono>
#include <algorithm>
#include "common_struct.h"

typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds milliseconds;

class Analyzer {
public:
  void analyze_cluster(const std::string& tracesFile, const std::string& output_prefix) {
    output_file_ = output_prefix + '/' + "analyze_cluster.output";
    analyzerIO(tracesFile);
  }
  void analyze_volume(const std::string& tracesFile, const std::string& output_prefix) {
    std::size_t lastSlashPos = tracesFile.rfind('/');
    std::string fileName = tracesFile.substr(lastSlashPos + 1);
    std::size_t commaPos = fileName.find('.');
    if (commaPos != std::string::npos) {
      std::string currentId = fileName.substr(0, commaPos);
      output_file_ = output_prefix + '/' + currentId +".output";
      analyzerIO(tracesFile);
    }
  }
  void split_volumes(const std::string& tracesFile, const std::string& output_prefix, const std::string& selectedFile) {
    // 读取选中的卷id
    std::ifstream selectedStream(selectedFile);
    std::vector<std::string> volumeIds;
    std::string id;
    while (std::getline(selectedStream, id)) {
        // 移除可能存在的末尾'\r'字符
      if (!id.empty() && id.back() == '\r') {
        id.pop_back();
      }
      volumeIds.push_back(id);
    }
    selectedStream.close();

    // 使用map存储已打开的输出文件流
    std::map<std::string, std::ofstream> outputStreams;

    // 读取io_traces.csv并写入对应卷id的文件
    std::ifstream tracesStream(tracesFile);
    std::string line;
    myTimer(true, "split");
    while (std::getline(tracesStream, line)) {
        totReadBytes_ += line.length() + 1;
        std::size_t commaPos = line.find(',');
        if (commaPos != std::string::npos) {
            std::string currentId = line.substr(0, commaPos);
            auto it = std::find(volumeIds.begin(), volumeIds.end(), currentId);
            if (it != volumeIds.end()) {
                // 如果文件流尚未打开，则打开它
                if (outputStreams.find(currentId) == outputStreams.end()) {
                    std::string volumeFile = output_prefix + '/' + currentId + ".csv";
                    outputStreams[currentId].open(volumeFile);
                }
                // 使用存储在outputStreams中的文件流进行写入
                outputStreams[currentId] << line << std::endl;
            }
        }
        myTimer(false, "split");
    }
    tracesStream.close();

    // 关闭所有打开的输出文件流
    for (auto& pair : outputStreams) {
        pair.second.close();
    }
  }
private:
 struct IOStatistics {
    size_t iops = 0;
    unsigned long long totalSize = 0;
    size_t readIops = 0;
    size_t writeIops = 0;
    unsigned long long totalReadSize = 0;
    unsigned long long totalWriteSize = 0;
  };
  void myTimer(bool start, const char* event = "") {
    static Clock::time_point ts;
    static uint64_t cnt = 0;
    if (start) {
      ts = Clock::now();
      cnt = 0;
    } else {
      if (++cnt % 1000000 == 0) {
        Clock::time_point te = Clock::now();
        double duration2 = (std::chrono::duration_cast <std::chrono::milliseconds> (te - ts)).count() / 1024.0;
        fprintf(stderr, "Scan on %s: %lu requests, %lf seconds, read %.6lf GiB, speed %.6lf MiB/s\n",
            event, cnt, duration2, (double)totReadBytes_ / 1024.0 / 1024.0 / 1024.0,
            (double)totReadBytes_ / 1024.0 / 1024.0 / duration2);
      }
    }
  }
  void analyzerIO(const std::string& tracesFile) {
    // 计算IOPS，BandWidth
    // 读取io_traces.csv并写入对应卷id的文件
    std::ifstream tracesStream(tracesFile);

    // 打开结果文件
    std::ofstream outputFile(output_file_);
    if (!outputFile.is_open()) {
      throw std::runtime_error("Failed to open output file: " + output_file_);
    }

    std::string line;
    myTimer(true, "analyzerIO");
    unsigned int currentTimeSec = 0; // 当前时间（秒）
    std::map<char, IOStatistics> statsByType;
    while(std::getline(tracesStream, line)) {
      IOOperation ioOp(line);
      totReadBytes_ += line.length() + 1;
      // 计算当前时间秒数（这里简化为只基于最后一条记录的时间）
      unsigned int newTimeSec = ioOp.timestamp_in_ms / 1000;
      if (newTimeSec != currentTimeSec) {
        reportIOPSAndBandwidth(currentTimeSec, statsByType, outputFile);
        currentTimeSec = newTimeSec;
        statsByType.clear();
      }
      // 更新统计信息
      ++statsByType[ioOp.type].iops;
      statsByType[ioOp.type].totalSize += ioOp.size;
      if (ioOp.type == 'R') { // 处理读操作
        ++statsByType['R'].readIops;
        statsByType['R'].totalReadSize += ioOp.size;
      } else if (ioOp.type == 'W') { // 处理写操作
        ++statsByType['W'].writeIops;
        statsByType['W'].totalWriteSize += ioOp.size;
      }
      myTimer(false, "analyzerIO");
    }
    // 处理最后一段时间段的数据
    if (!statsByType.empty()) {
      reportIOPSAndBandwidth(currentTimeSec, statsByType, outputFile);
    }
    outputFile.close();
  }
  void reportIOPSAndBandwidth(unsigned int timeSec, const std::map<char, IOStatistics>& statsByType, std::ofstream& outputFile) {
    for (const auto& entry : statsByType) {
        const auto& type = entry.first;
        const auto& ioStat = entry.second;
        double bwMBps = static_cast<double>(ioStat.totalSize) / (timeSec * 1e6);
        double readBwMBps = 0.0, writeBwMBps = 0.0;
        if (type == 'R') {
            readBwMBps = static_cast<double>(ioStat.totalReadSize) / (timeSec * 1e6);
        } else if (type == 'W') {
            writeBwMBps = static_cast<double>(ioStat.totalWriteSize) / (timeSec * 1e6);
        }

        outputFile << "At time " << timeSec << " seconds, Type: " << type
                    << ", Total IOPS: " << ioStat.iops
                    << ", Read IOPS: " << ioStat.readIops
                    << ", Write IOPS: " << ioStat.writeIops
                    << ", Total Bandwidth: " << bwMBps << " MB/s"
                    << ", Read Bandwidth: " << readBwMBps << " MB/s"
                    << ", Write Bandwidth: " << writeBwMBps << " MB/s" << std::endl;
    }
  }
  uint64_t totReadBytes_ = 0;
  std::string output_file_;
};

int main(int argc, char *argv[]) {
    Analyzer analyzer;
    int arg3AsInt;
    try {
      arg3AsInt = std::stoi(argv[3]);
    } catch (const std::invalid_argument& ia) {
      //split_volumes("/home/yyqiao/papaer/alibaba_block_traces_2020/io_traces.csv", "/sds_data/ali_trace/", "/etc/ali_selected.txt");
      //analyzer.split_volumes(argv[1], argv[2]);
    }
    if (arg3AsInt == 1) {
      //analyze_volume("/sds_data/ali_trace/volume_trace/0.csv", "/sds_data/ali_trace/iops_bw", 1);
      analyzer.analyze_volume(argv[1], argv[2]);
    } else if (arg3AsInt == 2) {
      //analyze_cluster("/home/yyqiao/papaer/alibaba_block_traces_2020/io_traces.csv", "/sds_data/ali_trace/iops_bw", 2);
      analyzer.analyze_cluster(argv[1], argv[2]);
    } else {
      exit(1);
    }
    return 0;
}