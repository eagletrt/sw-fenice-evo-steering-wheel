#!/bin/bash

simulate=true
canplayer vcan0=can0 -I candump.log -l i &
vcan0=$!

trap ctrl_c INT

function ctrl_c() {
	echo "Quitting"
	kill $vcan0
	kill $vcan1
	simulate=false
}

while [[ $simulate = true ]]; do
    sleep 1
    echo -n "."
done
