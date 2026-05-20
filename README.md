# satellite_instrument_sim

A containerized satellite atmospheric lidar simulator (EarthCARE ATLID class). A C++17
engine models Beer-Lambert atmospheric backscatter and writes binary telemetry frames at
1 kHz over a POSIX named pipe. A Python service filters the signal and estimates CO₂
concentration. Both run in Docker Compose.

## Architecture

```
AtmosphericModel (Beer-Lambert + shot noise)
        │  TelemetryFrame @1kHz
        ▼
SpscQueue<TelemetryFrame, 4096>
        │  drain thread
        ▼
POSIX named pipe (/data/sim.fifo)
        │  struct.unpack('<QBHhBBB')
        ▼
FrameIngestor (CRC-8 validate, Pydantic)
        │
        ▼
SignalProcessor (Butterworth 4th-order LPF + Beer-Lambert CO₂ inversion)
        │
        ▼
FastAPI /api/metrics  /api/health  /api/frames/{n}
```

## Beer-Lambert Mathematics

The simulator models atmospheric lidar backscatter:

```
I(r) = I₀ · exp(−2·β_ext·r) · β_ext
```

- `r` = range (altitude, metres)
- `β_ext` = extinction coefficient: boundary layer (< 3 km): 1×10⁻⁴ m⁻¹; free troposphere: 1×10⁻⁵ m⁻¹
- The factor of 2 accounts for the two-way lidar path

CO₂ retrieval inverts Beer-Lambert:

```
N_CO₂ = −ln(I / I₀) / (σ_CO₂ · ΔL)
```

- `σ_CO₂ = 3.98×10⁻²³ m²·molecule⁻¹` (HITRAN database, 355 nm)
- `ΔL` = atmospheric path length (altitude, metres)

## Quickstart

```bash
bash scripts/setup.sh
docker compose up
curl http://localhost:8091/api/metrics
```

## Local C++ build

```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j4
ctest --test-dir build --output-on-failure -V
```

## Python tests

```bash
uv sync
uv run pytest tests_python/ -v
```

## CI

Jenkins pipeline: `Jenkinsfile` — 4 stages: Build C++ → Python Tests → Docker Build → Smoke Test.

## Documentation

- [`docs/SRS-001.md`](docs/SRS-001.md) — Software Requirements Specification (12 FR + 5 NFR)
- [`docs/ICD-001.md`](docs/ICD-001.md) — Interface Control Document (bit-level frame layout)
