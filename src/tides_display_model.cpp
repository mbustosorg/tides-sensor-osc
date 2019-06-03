/*
 
 Copyright (C) 2018 Mauricio Bustos (m@bustos.org)
 
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

#include "tides_display_model.h"

TidesDisplayModel::TidesDisplayModel() {
    
    char* OSC_HOST = std::getenv("OSC_HOST");
    char* OSC_PORT = std::getenv("OSC_PORT");
    
    if (OSC_HOST == NULL || OSC_PORT == NULL) {
        console->error("UNDEFINED ENVIRONMENT OSC_HOST AND OR OSC_PORT");
    }
    
    console->info("Starting sensor bridge to {}:{}", OSC_HOST, OSC_PORT);
    t = lo_address_new(OSC_HOST, OSC_PORT);
    console->info("Started sensor bridge to {}:{}", OSC_HOST, OSC_PORT);
}

// Keep track of state of sensors to coordinate multiple on at once
void TidesDisplayModel::received(int clientId, int value) {    
    
    int THRESHOLD = 512;
    int result = 0;
    clients->at(clientId) = value;
    int sum = 0;
    std::for_each(clients->begin(), clients->end(), [&] (int n) {
        if (n > THRESHOLD) sum++;
    });
    // 1-7 Individual
    // 8-10 Cross
    // 11-17 Individual Fire
    // 18-20 All Fire
    
    if (sum == 1) {
        result = lo_send(t, OSC_PATH, "i", clientId);
    } else if (sum == 2) {
        result = lo_send(t, OSC_PATH, "i", 8);
    } else if (sum == 3) {
        result = lo_send(t, OSC_PATH, "i", 9);
    } else if (sum == 4) {
        result = lo_send(t, OSC_PATH, "i", 10);
    }
    if (result == 1) {
        console->warn("Unable OSC message {} from {}", value, clientId);
    } else if (result == -1) {
        console->error("Unable to send OSC message {} from {}", value, clientId);   
    }
}
