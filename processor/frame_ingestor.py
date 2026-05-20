import struct
from pydantic import BaseModel, field_validator

FRAME_FORMAT = '<QBHhBBB'  # 8+1+2+2+1+1+1 = 16 bytes, little-endian
FRAME_SIZE   = struct.calcsize(FRAME_FORMAT)
assert FRAME_SIZE == 16

def _crc8(data: bytes, init: int = 0xFF) -> int:
    crc = init
    for byte in data:
        crc ^= byte
        for _ in range(8):
            crc = ((crc << 1) & 0xFF) ^ 0x31 if (crc & 0x80) else (crc << 1) & 0xFF
    return crc

class TelemetryFrame(BaseModel):
    ts_us:           int
    channel_id:      int
    altitude_m:      int
    backscatter_raw: int
    snr_x10:         int
    solar_flag:      int

    @field_validator('channel_id')
    @classmethod
    def validate_channel(cls, v: int) -> int:
        if v not in (0, 1, 2, 3):
            raise ValueError(f'channel_id {v} not in 0-3')
        return v

    @field_validator('altitude_m')
    @classmethod
    def validate_altitude(cls, v: int) -> int:
        if not (0 <= v <= 65535):
            raise ValueError(f'altitude_m {v} out of range')
        return v

class CorruptFrameError(Exception):
    pass

def parse_frame(raw: bytes) -> TelemetryFrame:
    if len(raw) != FRAME_SIZE:
        raise CorruptFrameError(f'expected {FRAME_SIZE} bytes, got {len(raw)}')
    expected_crc = _crc8(raw[:15])
    if raw[15] != expected_crc:
        raise CorruptFrameError(
            f'CRC mismatch: got {raw[15]:#04x}, expected {expected_crc:#04x}')
    ts_us, ch, alt, back, snr, solar, _ = struct.unpack(FRAME_FORMAT, raw)
    return TelemetryFrame(
        ts_us=ts_us, channel_id=ch, altitude_m=alt,
        backscatter_raw=back, snr_x10=snr, solar_flag=solar,
    )

def read_frame(fd) -> TelemetryFrame:
    raw = fd.read(FRAME_SIZE)
    if len(raw) < FRAME_SIZE:
        raise EOFError('pipe closed')
    return parse_frame(raw)
