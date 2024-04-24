//#include <iostream>
// 结构体用于存储IO操作信息
struct IOOperation {
    std::string volumeId;
    char type;
    unsigned long long lba;
    unsigned long size;
    unsigned long timestamp_in_ms;

    // 从字符串中提取IO操作信息（不使用std::istringstream）
    explicit IOOperation(const std::string& line) {
        size_t pos1 = line.find(',');
        if (pos1 == std::string::npos) {
            throw std::runtime_error("Invalid CSV format");
        }
        volumeId = line.substr(0, pos1);

        size_t pos2 = line.find(',', pos1 + 1);
        if (pos2 == std::string::npos) {
            throw std::runtime_error("Invalid CSV format");
        }
        type = line[pos1 + 1];

        pos1 = pos2 + 1;
        size_t pos3 = line.find(',', pos1);
        if (pos3 == std::string::npos) {
            throw std::runtime_error("Invalid CSV format");
        }
        lba = std::stoull(line.substr(pos1, pos3 - pos1));

        pos1 = pos3 + 1;
        size_t pos4 = line.find(',', pos1);
        if (pos4 == std::string::npos) {
            throw std::runtime_error("Invalid CSV format");
        }
        size = std::stoul(line.substr(pos1, pos4 - pos1));

        timestamp_in_ms = std::stoul(line.substr(pos4 + 1)); // 假设末尾没有其他逗号

    }
};