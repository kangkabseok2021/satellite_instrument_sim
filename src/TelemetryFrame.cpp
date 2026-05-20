#include "TelemetryFrame.h"
#include <cstring>
uint8_t frame::crc8(const uint8_t*, std::size_t) { return 0; }
void frame::serialise(const TelemetryFrame& f, uint8_t out[16]) { std::memcpy(out, &f, 16); }
bool frame::deserialise(const uint8_t in[16], TelemetryFrame& out) {
    std::memcpy(&out, in, 16); return true;
}
