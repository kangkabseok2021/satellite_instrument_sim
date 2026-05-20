#include <gtest/gtest.h>
#include "TelemetryFrame.h"
#include <cstring>

TEST(TelemetryFrame, SerialisedSizeIs16Bytes) {
    EXPECT_EQ(sizeof(TelemetryFrame), 16u);
}

TEST(TelemetryFrame, Crc8RoundTrip) {
    TelemetryFrame f{};
    f.ts_us           = 1234567890ULL;
    f.channel_id      = 2;
    f.altitude_m      = 5000;
    f.backscatter_raw = 750;
    f.snr_x10         = 120;
    f.solar_flag      = 0;
    f.crc8            = frame::crc8(reinterpret_cast<const uint8_t*>(&f), 15);

    uint8_t buf[16];
    frame::serialise(f, buf);

    TelemetryFrame out{};
    EXPECT_TRUE(frame::deserialise(buf, out));
    EXPECT_EQ(out.ts_us,           f.ts_us);
    EXPECT_EQ(out.channel_id,      f.channel_id);
    EXPECT_EQ(out.altitude_m,      f.altitude_m);
    EXPECT_EQ(out.backscatter_raw, f.backscatter_raw);
}

TEST(TelemetryFrame, CorruptByteFailsCrc) {
    TelemetryFrame f{};
    f.altitude_m      = 1000;
    f.backscatter_raw = 500;
    f.crc8            = frame::crc8(reinterpret_cast<const uint8_t*>(&f), 15);

    uint8_t buf[16];
    frame::serialise(f, buf);
    buf[3] ^= 0xFF;  // corrupt one byte

    TelemetryFrame out{};
    EXPECT_FALSE(frame::deserialise(buf, out));
}

TEST(TelemetryFrame, KnownCrcNonTrivial) {
    uint8_t data[3] = {0x00, 0x00, 0x00};
    uint8_t crc = frame::crc8(data, 3);
    // With init=0xFF, poly=0x31, three zero bytes must produce non-zero result
    EXPECT_NE(crc, 0x00u);
    // And must be deterministic
    EXPECT_EQ(crc, frame::crc8(data, 3));
}
