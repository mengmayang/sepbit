#include <fstream>
#include <string>
#include <vector>
#include <map>
#include<sys/time.h>
#include<chrono>
#include <algorithm>

typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds milliseconds;

class Analyzer {

public:
  void analyze_cluster(const std::string& tracesFile, const std::string& output_prefix) {}
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
  uint64_t totReadBytes_ = 0;
};

int main(int argc, char *argv[]) {
    Analyzer analyzer;
    if (argc == 3) {
      //analyze_cluster("/home/yyqiao/papaer/alibaba_block_traces_2020/io_traces.csv", "/sds_data/ali_trace/");
      analyzer.analyze_cluster(argv[1], argv[2]);
    } else {
      //split_volumes("/home/yyqiao/papaer/alibaba_block_traces_2020/io_traces.csv", "/sds_data/ali_trace/", "/etc/ali_selected.txt");
      analyzer.split_volumes(argv[1], argv[2], argv[3]);
    }
    return 0;
}