# Tides Sensor OSC Server

[![Build Status](https://travis-ci.org/mbustosorg/tides-sensor-osc.svg?branch=master)](https://travis-ci.org/mbustosorg/tides-sensor-osc)

## Dependencies

The application is dependent on the LIBLO Lightweight OSC library located at http://liblo.sourceforge.net/.  Installation instructions are at http://liblo.sourceforge.net/README.html.

## Building

The application uses ```make``` to build.  

```05:53:41 [~/Documents/development/git/tides-sensor-osc]$ make
$ make
$ g++ -c -o obj/tides_sensor_osc.o src/tides_sensor_osc.cpp -Isrc -std=c++11
$ g++ -c -o obj/tides_data.o src/tides_data.cpp -Isrc -std=c++11
$ g++ -o bin/tides_sensor_osc obj/tides_sensor_osc.o obj/tides_data.o -llo
```

## Configuration

```
$ set OSC_HOST={host ip address}
$ set OSC_PORT={host port}
$ set TIDES_SENSOR_PORT=1999 # Listen port for sensor devices
```

## Tides Data

The tide height data is retrieved from the NOAA.  There is a [Jupyter](http://jupyter.org/) notebook in ```./tides_data``` that demonstrates the downloading process.