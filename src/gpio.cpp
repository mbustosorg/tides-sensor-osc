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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

GPIO::GPIO() {
}

// Set direction for `pin' to `input'
void GPIO::setDirection(int pin, bool input) {

    int fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd == -1) {
        perror("Unable to open /sys/class/gpio/export");
    } else {
        if (write(fd, "26", 2) != 2) {
            perror("Error writing to /sys/class/gpio/export");
            exit(1);
        }
        close(fd);
        fd = open("/sys/class/gpio/gpio26/direction", O_WRONLY);
        if (fd == -1) {
            perror("Unable to open /sys/class/gpio/gpio26/direction");
        } else {
            if (write(fd, "out", 3) != 3) {
                perror("Error writing to /sys/class/gpio/gpio26/direction");
                exit(1);
            }
            close(fd);
        }
    }
}

// Set value of `pin' to `high'
void GPIO::setValue(int pin, bool high) {
    int fd = open("/sys/class/gpio/gpio26/value", O_WRONLY);
    if (fd == -1) {
        perror("Unable to open /sys/class/gpio/gpio26/value");
    } else {
        if (write(fd, "1", 1) != 1) {
            perror("Error writing to /sys/class/gpio/gpio26/value");
            exit(1);
        }
        close(fd);
    }
}
