#!/bin/sh

process=tides_sensor_osc
makerun="/home/pi/tides-sensor-osc/bin/tides_sensor_osc"

if ps ax | grep -v grep | grep $process > /dev/null
then
    exit
else
    $makerun &
fi 
