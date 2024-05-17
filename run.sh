#!/bin/sh
cd /home/pi/tides-sensor-osc
sudo python3 -m tidessensorosc.supervisor --ip 192.168.0.1 --port 1999 --controller_ip 192.168.0.1 --disable_sound