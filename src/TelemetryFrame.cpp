#include "TelemetryFrame.h"
#include <cstring>

uint8_t frame::crc8(const uint8_t* data, std::size_t len) {
    uint8_t crc = 0xFF;  // initial value per ICD-001
    for (std::size_t i = 0; i < len; ++i) {
        crc ^= data[i];
        for (int j = 0; j < 8; ++j) {
            crc = (crc & 0x80u)
                  ? static_cast<uint8_t>((crc << 1) ^ 0x31u)
                  : static_cast<uint8_t>(crc << 1);
        }
    }
    return crc;
}

void frame::serialise(const TelemetryFrame& f, uint8_t out[16]) {
    std::memcpy(out, &f, 16);
}

bool frame::deserialise(const uint8_t in[16], TelemetryFrame& out) {
    uint8_t expected = frame::crc8(in, 15);
    if (in[15] != expected) return false;
    std::memcpy(&out, in, 16);
    return true;
}
