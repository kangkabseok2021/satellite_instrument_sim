#!/usr/bin/env bash
set -euo pipefail

# Satellite Instrument Simulator — host environment setup
# Run once on the Docker host before: docker compose up

MIN_KERNEL_MAJOR=5
MIN_KERNEL_MINOR=4

echo "=== Satellite Instrument Simulator Setup ==="

# Check kernel version
KERNEL_VERSION=$(uname -r | cut -d. -f1,2)
MAJOR=$(echo "$KERNEL_VERSION" | cut -d. -f1)
MINOR=$(echo "$KERNEL_VERSION" | cut -d. -f2)

if [ "$MAJOR" -lt "$MIN_KERNEL_MAJOR" ] || \
   { [ "$MAJOR" -eq "$MIN_KERNEL_MAJOR" ] && [ "$MINOR" -lt "$MIN_KERNEL_MINOR" ]; }; then
    echo "ERROR: kernel $KERNEL_VERSION < $MIN_KERNEL_MAJOR.$MIN_KERNEL_MINOR (required for stable POSIX FIFO)"
    exit 1
fi
echo "Kernel $KERNEL_VERSION OK"

# Create data directory for shared named pipe
mkdir -p ./data
chmod 777 ./data

# Create named pipe if it doesn't exist
FIFO=./data/sim.fifo
if [ ! -p "$FIFO" ]; then
    mkfifo -m 0666 "$FIFO"
    echo "Created FIFO: $FIFO"
else
    echo "FIFO already exists: $FIFO"
fi

# Write environment file
cat > .env <<EOF
SIM_RATE=1000
SIM_SOLAR=
PROC_CO2_BASELINE=410.0
EOF

echo "Environment written to .env"
echo "=== Setup complete. Run: docker compose up ==="
