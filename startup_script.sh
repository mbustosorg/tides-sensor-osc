#!/bin/sh

export LD_LIBRARY_PATH=/usr/local/lib
export OSC_HOST={}
export OSC_PORT=1234
export OSC_TIDES_CSV={}
export TIDES_SENSOR_TIMING_OFF=true

process=tides_sensor_osc
makerun="/home/pi/tides-sensor-osc/bin/tides_sensor_osc"

if ps ax | grep -v grep | grep $process > /dev/null
then
    exit
else
    $makerun &
fi 
