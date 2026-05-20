import math
from collections import deque
import numpy as np
from scipy.signal import butter, sosfilt_zi, sosfilt
from processor.frame_ingestor import TelemetryFrame

# Butterworth LPF: 4th-order, 100 Hz cutoff, 1000 Hz sample rate
_SOS = butter(4, 100.0, btype='low', fs=1000.0, output='sos')

class SignalProcessor:
    # CO2 absorption cross-section at 355 nm (HITRAN database), m^2 molecule^-1
    SIGMA_CO2 = 3.98e-23
    # Reference backscatter from altitude > 25 km (clean-air baseline)
    I0_BASELINE = 800.0  # matches AtmosphericModel default I0 at high altitude

    def __init__(self, window: int = 1000):
        self._zi    = sosfilt_zi(_SOS) * 0.0  # zero initial conditions
        self._raw   = deque(maxlen=window)
        self._filt  = deque(maxlen=window)

    def update(self, frame: TelemetryFrame) -> dict:
        raw_val = frame.backscatter_raw / 1000.0  # undo fixed-point scaling

        # Apply IIR filter sample-by-sample
        y, self._zi = sosfilt(_SOS, [raw_val], zi=self._zi)
        filtered = float(y[0])

        self._raw.append(raw_val)
        self._filt.append(filtered)

        # Beer-Lambert CO2 retrieval (avoid log(0))
        signal = max(filtered, 1e-6)
        path_length = max(frame.altitude_m, 1.0)  # metres
        n_co2 = -math.log(signal / self.I0_BASELINE) / (self.SIGMA_CO2 * path_length)
        # Convert molecules/m^2 to ppm using total atmospheric column ~2.15e25 molecules/m^2
        co2_ppm = max(n_co2 / 2.15e25 * 1e6, 0.0)

        return {
            'altitude_m':  frame.altitude_m,
            'filtered':    filtered,
            'co2_ppm':     co2_ppm,
            'snr_db':      frame.snr_x10 / 10.0,
            'solar_flag':  frame.solar_flag,
        }

    def rms_raw(self) -> float:
        return float(np.sqrt(np.mean(np.array(self._raw)**2))) if self._raw else 0.0

    def rms_filtered(self) -> float:
        return float(np.sqrt(np.mean(np.array(self._filt)**2))) if self._filt else 0.0
