import struct
import pytest
from processor.frame_ingestor import parse_frame, CorruptFrameError, TelemetryFrame
from pydantic import ValidationError
from conftest import make_raw_frame

def test_valid_frame_parses():
    raw = make_raw_frame(altitude_m=3000, backscatter_raw=500)
    f = parse_frame(raw)
    assert f.altitude_m == 3000
    assert f.backscatter_raw == 500

def test_bad_crc_raises():
    raw = bytearray(make_raw_frame())
    raw[5] ^= 0xFF  # corrupt a data byte
    with pytest.raises(CorruptFrameError, match='CRC mismatch'):
        parse_frame(bytes(raw))

def test_wrong_length_raises():
    with pytest.raises(CorruptFrameError, match='expected 16'):
        parse_frame(b'\x00' * 10)

def test_all_channel_ids_accepted():
    for ch in (0, 1, 2, 3):
        raw = make_raw_frame(channel_id=ch)
        f = parse_frame(raw)
        assert f.channel_id == ch

def test_invalid_channel_raises():
    raw = bytearray(make_raw_frame())
    # channel_id is at byte offset 8 in the frame
    raw[8] = 5  # set to invalid value 5
    # Recompute CRC so it passes CRC check but fails Pydantic validation
    from processor.frame_ingestor import _crc8
    raw[15] = _crc8(bytes(raw[:15]))
    with pytest.raises(ValidationError):
        parse_frame(bytes(raw))
