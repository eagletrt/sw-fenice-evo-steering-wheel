#!/bin/bash

sudo modprobe vcan
sudo ip link add dev vcan0 type vcan
sudo ifconfig vcan0 up
sudo ip link add dev vcan1 type vcan
sudo ifconfig vcan1 up

# This is the command to run the simulation of the CAN bus
# canplayer vcan0=primary vcan1=secondary -I candump.log -v
