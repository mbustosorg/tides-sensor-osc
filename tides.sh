#!/bin/bash
cd /home/pi/LEDPlay
if 1; then
	./LEDPlay tides
else
	i=0
	while :
	do
		i=$[i + 1]
		./LEDPlay tides
	done
fi
