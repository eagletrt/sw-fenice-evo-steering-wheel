#! /usr/bin/env bash

cp ../build/fenice-evo-steering-wheel.bin wheel.bin
./bin2srec -a 0x8020000 -i ./wheel.bin -o ./wheel.srec

# Load the srec file to telemetry
# scp fenice-evo-steering-wheel.bin control@control:/home/control/can-flashing/binaries

# Into the telemetry, run the flashing script
# ./can-flashing STEERING ../binaries/wheel.srec
