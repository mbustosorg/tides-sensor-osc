# Tides Sensor OSC Server

[![Build Status](https://travis-ci.org/mbustosorg/tides-sensor-osc.svg?branch=master)](https://travis-ci.org/mbustosorg/tides-sensor-osc)

## Dependencies

The application is dependent on the LIBLO Lightweight OSC library located at http://liblo.sourceforge.net/.  Installation instructions are at http://liblo.sourceforge.net/README.html.  Ensure that LD_LIBRARY_PATH includes /usr/local/lib (not always set correctly on the RPi).

```$ export LD_LIBRARY_PATH=/usr/local/lib```

## Building

The application uses ```make``` to build.  

```05:53:41 [~/Documents/development/git/tides-sensor-osc]$ make
$ make
$ g++ -c -o obj/tides_sensor_osc.o src/tides_sensor_osc.cpp -Isrc -std=c++11
$ g++ -c -o obj/tides_data.o src/tides_data.cpp -Isrc -std=c++11
$ g++ -o bin/tides_sensor_osc obj/tides_sensor_osc.o obj/tides_data.o -llo -pthread
```

## Configuration

```
$ export OSC_HOST={host ip address}     # Usually localhost
$ export OSC_PORT={host port}           # Usually 1234
$ export TIDES_SENSOR_PORT=1999         # Listen port for sensor devices
$ export OSC_TIDES_CSV={}               # Location of tides csv file
$ export OSC_SUN_CSV={}                 # Location of sun csv file
$ export TIDES_SENSOR_TIMING_OFF=true   # Disable daytime shutoff
```

## Tides Data

The tide height data is retrieved from the NOAA.  There is a [Jupyter](http://jupyter.org/) notebook in ```./earth_data``` that demonstrates the downloading process.

## Sunrise / Sunset Data

The sunrise sunset data is retrieved from navy.mil.  There is a [Jupyter](http://jupyter.org/) notebook in ```./earth_data``` that demonstrates the process to convert the data properly.