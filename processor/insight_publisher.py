from collections import deque
from typing import Any
import threading
from fastapi import FastAPI

app   = FastAPI(title="Satellite Instrument Processor")
_lock = threading.Lock()
_metrics_window: deque = deque(maxlen=10)
_status = {'frames_processed': 0, 'fps': 0.0, 'running': True}

def push_metric(m: dict) -> None:
    with _lock:
        _metrics_window.append(m)
        _status['frames_processed'] += 1

def set_fps(fps: float) -> None:
    with _lock:
        _status['fps'] = fps

@app.get('/api/health')
def health() -> dict:
    with _lock:
        return {'status': 'ok', 'fps': _status['fps'],
                'frames_processed': _status['frames_processed']}

@app.get('/api/metrics')
def metrics() -> dict:
    with _lock:
        if not _metrics_window:
            return {'co2_ppm': None, 'snr_db': None, 'solar_contamination': None}
        recent = list(_metrics_window)
    co2_avg    = sum(m['co2_ppm']    for m in recent) / len(recent)
    snr_avg    = sum(m['snr_db']     for m in recent) / len(recent)
    solar_frac = sum(1 for m in recent if m['solar_flag']) / len(recent)
    return {'co2_ppm': co2_avg, 'snr_db': snr_avg,
            'solar_contamination_fraction': solar_frac}

@app.get('/api/frames/{n}')
def frames(n: int) -> list[Any]:
    with _lock:
        recent = list(_metrics_window)
    return recent[-n:] if n <= len(recent) else recent
