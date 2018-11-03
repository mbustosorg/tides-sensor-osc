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
}

// Keep track of state of sensors to coordinate multiple on at once
void TidesDisplayModel::received(int clientId, int value) {    
    
    int THRESHOLD = 512;
    int result = 0;
    if (value > THRESHOLD) {
        result = lo_send(t, OSC_PATH, "i", clientId);
    }
    if (result == 1) {
        console->warn("Unable OSC message {} from {}", value, clientId);
    } else if (result == -1) {
        console->error("Unable to send OSC message {} from {}", value, clientId);
    }
}
