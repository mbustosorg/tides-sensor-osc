/*
 
 Copyright (C) 2019 Mauricio Bustos (m@bustos.org)
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
 */

#include "gpio.h"

GPIO::GPIO() {
}

// Set direction for `pin' to `input'
void GPIO::setDirection(int pin, bool input) {
    int fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd == -1) {
        logger->error("Unable to open /sys/class/gpio/export");
    } else {
        if (write(fd, std::to_string(pin).c_str(), 2) != 2) {
            logger->error("Error writing to /sys/class/gpio/export");
            //exit(1);
        }
        close(fd);
        fd = open((std::string("/sys/class/gpio/gpio") + std::to_string(pin) + "/direction").c_str(), O_WRONLY);
        if (fd == -1) {
            logger->error("Unable to open /sys/class/gpio/gpio{}/direction", pin);
        } else {
            long result = 0;
            if (input) {
                result = write(fd, "in", 3);
            } else {
                result = write(fd, "out", 3);
            }
            if (result != 3) {
                logger->error("Error writing to /sys/class/gpio/gpio{}/direction", pin);
                //exit(1);
            }
            close(fd);
        }
    }
}

// Set value of `pin' to `high'
void GPIO::setValue(int pin, bool high) {
    int fd = open((std::string("/sys/class/gpio/gpio") + std::to_string(pin) + "/value").c_str(), O_WRONLY);
    if (fd == -1) {
        logger->error("Unable to open /sys/class/gpio/gpio{}/value", pin);
    } else {
        long result = 0;
        if (high) {
            result = write(fd, "1", 1);
        } else {
            result = write(fd, "0", 1);
        }
        if (result != 1) {
            logger->error("Error writing to /sys/class/gpio/gpio{}/value", pin);
            //exit(1);
        }
        close(fd);
        
    }
}
