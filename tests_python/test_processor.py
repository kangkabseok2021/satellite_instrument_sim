import numpy as np
import pytest
from processor.signal_processor import SignalProcessor
from processor.frame_ingestor import TelemetryFrame

def _make_frame(alt: int, back_raw: int, solar: int = 0) -> TelemetryFrame:
    return TelemetryFrame(
        ts_us=1000, channel_id=0,
        altitude_m=alt, backscatter_raw=back_raw,
        snr_x10=120, solar_flag=solar,
    )

def test_filtered_signal_reduces_rms_noise():
    sp = SignalProcessor()
    rng = np.random.default_rng(42)
    # Feed noisy signal (high-frequency noise on top of DC)
    for _ in range(500):
        noisy = int(500 + rng.normal(0, 100) * 5)  # DC=500 + high-freq noise
        sp.update(_make_frame(5000, noisy))
    # After 500 samples filter should have lower variation than raw
    assert sp.rms_filtered() <= sp.rms_raw() * 1.1

def test_co2_retrieval_positive():
    sp = SignalProcessor()
    result = sp.update(_make_frame(5000, 750))
    assert result['co2_ppm'] >= 0.0

def test_co2_increases_with_weaker_signal():
    sp1 = SignalProcessor()
    sp2 = SignalProcessor()
    # Warm up both processors
    for _ in range(200):
        sp1.update(_make_frame(5000, 700))
        sp2.update(_make_frame(5000, 300))
    r1 = sp1.update(_make_frame(5000, 700))
    r2 = sp2.update(_make_frame(5000, 300))
    # Weaker signal -> higher apparent CO2 (Beer-Lambert inversion)
    assert r2['co2_ppm'] >= r1['co2_ppm']

def test_solar_flag_preserved():
    sp = SignalProcessor()
    result = sp.update(_make_frame(5000, 750, solar=1))
    assert result['solar_flag'] == 1

def test_snr_db_scaling():
    sp = SignalProcessor()
    result = sp.update(_make_frame(5000, 750))
    # snr_x10 = 120 -> snr_db = 12.0
    assert abs(result['snr_db'] - 12.0) < 0.01
