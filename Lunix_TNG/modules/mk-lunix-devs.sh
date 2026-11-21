#!/bin/bash


# Lunix:TNG nodes: 16 sensors, each has 3 nodes.
for sensor in $(seq 0 1 15); do
	echo "/dev/lunix$sensor-batt c 60 $[$sensor * 8 + 0]"
	echo "/dev/lunix$sensor-temp c 60 $[$sensor * 8 + 1]"
	echo "/dev/lunix$sensor-light c 60 $[$sensor * 8 + 2]"
done
