import struct

def make_raw_frame(
    ts_us: int = 1000000,
    channel_id: int = 0,
    altitude_m: int = 5000,
    backscatter_raw: int = 750,
    snr_x10: int = 120,
    solar_flag: int = 0,
) -> bytes:
    from processor.frame_ingestor import _crc8, FRAME_FORMAT
    payload = struct.pack('<QBHhBB', ts_us, channel_id, altitude_m,
                          backscatter_raw, snr_x10, solar_flag)
    crc = _crc8(payload)
    return payload + bytes([crc])
