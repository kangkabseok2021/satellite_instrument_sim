#pragma once
#include <cstdint>
#include <array>

// 16-byte binary telemetry frame. Layout defined in ICD-001.
struct __attribute__((packed)) TelemetryFrame {
    uint64_t ts_us;           // microseconds since UNIX epoch
    uint8_t  channel_id;      // 0=355nm 1=532nm 2=1064nm 3=355nm-depol
    uint16_t altitude_m;      // 0-65535 m
    int16_t  backscatter_raw; // backscatter x 1000 (fixed-point)
    uint8_t  snr_x10;         // SNR x 10 (0-255 -> 0.0-25.5 dB)
    uint8_t  solar_flag;      // 0=clean 1=solar-contaminated
    uint8_t  crc8;            // CRC-8 over bytes 0-14 (poly 0x31, init 0xFF)
};
static_assert(sizeof(TelemetryFrame) == 16, "TelemetryFrame must be 16 bytes");

namespace frame {
    void     serialise(const TelemetryFrame& f, uint8_t out[16]);
    bool     deserialise(const uint8_t in[16], TelemetryFrame& out);
    uint8_t  crc8(const uint8_t* data, std::size_t len);
}
