#! /usr/bin/env bash

# TODO check that the bin is compiled with the shifted linker script

cp ../build/fenice-evo-steering-wheel.bin wheel.bin
./bin2srec -a 0x8020000 -i wheel.bin -o wheel.srec

# Load the srec file to telemetry
scp ./wheel.srec control@control.local:/home/control/can-flashing/binaries

# Into the telemetry, run the flashing script
# ./can-flashing STEERING ../binaries/wheel.srec
