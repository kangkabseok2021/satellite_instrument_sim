#include "TelemetryPipe.h"
#include <fcntl.h>
#include <unistd.h>

TelemetryPipe::TelemetryPipe(const std::string& path) {
    fd_ = ::open(path.c_str(), O_WRONLY);
}

TelemetryPipe::~TelemetryPipe() {
    if (fd_ >= 0) ::close(fd_);
}

bool TelemetryPipe::write_frame(const uint8_t data[16]) {
    if (fd_ < 0) return false;
    ssize_t n = ::write(fd_, data, 16);
    return n == 16;
}
