// 结构体用于存储IO操作信息
struct IOOperation {
    std::string volumeId;
    std::string type;
    unsigned long long lba;
    unsigned long size;
    unsigned long timestamp_in_ms; // 假设这是毫秒级的时间戳

    // 从字符串中提取IO操作信息
    explicit IOOperation(const std::string& line) {
        std::istringstream iss(line);
        if (!(iss >> volumeId >> type >> lba >> size >> timestamp_in_ms)) {
            throw std::runtime_error("Invalid CSV format");
        }
    }
};