#pragma once
#include <string>
#include <cstdint>

class TelemetryPipe {
public:
    explicit TelemetryPipe(const std::string& path);
    ~TelemetryPipe();
    bool write_frame(const uint8_t data[16]);
    bool is_open() const { return fd_ >= 0; }
private:
    int fd_{-1};
};
