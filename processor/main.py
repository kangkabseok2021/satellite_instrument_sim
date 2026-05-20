import os
import time
import threading
import uvicorn
from processor.frame_ingestor import read_frame, CorruptFrameError
from processor.signal_processor import SignalProcessor
from processor.insight_publisher import app, push_metric, set_fps

def ingest_loop(fifo_path: str) -> None:
    os.makedirs(os.path.dirname(fifo_path) or '.', exist_ok=True)
    try:
        os.mkfifo(fifo_path, 0o666)
    except FileExistsError:
        pass

    sp = SignalProcessor()
    t_start = time.monotonic()
    count = 0

    with open(fifo_path, 'rb') as f:
        while True:
            try:
                frame = read_frame(f)
                result = sp.update(frame)
                push_metric(result)
                count += 1
                elapsed = time.monotonic() - t_start
                if elapsed >= 1.0:
                    set_fps(count / elapsed)
                    count   = 0
                    t_start = time.monotonic()
            except CorruptFrameError:
                pass
            except EOFError:
                break

if __name__ == '__main__':
    fifo_path = os.environ.get('FIFO_PATH', '/data/sim.fifo')
    port      = int(os.environ.get('PROC_PORT', '8091'))

    t = threading.Thread(target=ingest_loop, args=(fifo_path,), daemon=True)
    t.start()

    uvicorn.run(app, host='0.0.0.0', port=port)
