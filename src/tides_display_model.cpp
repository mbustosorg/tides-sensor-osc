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

#include "tides_display_model.h"

TidesDisplayModel::TidesDisplayModel() {
    
    //std::string hostString = "tides2.local";
    //const char* OSC_HOST = hostString.c_str();
    //std::string portString = "1234";
    //const char* OSC_PORT = portString.c_str();
    
    char* OSC_HOST = std::getenv("OSC_HOST");
    char* OSC_PORT = std::getenv("OSC_PORT");
    
    if (OSC_HOST == NULL || OSC_PORT == NULL) {
        spdlog::get("logger")->error("UNDEFINED ENVIRONMENT OSC_HOST AND OR OSC_PORT");
    }
    
    spdlog::get("logger")->info("Starting sensor bridge to {}:{}", OSC_HOST, OSC_PORT);
    t = lo_address_new(OSC_HOST, OSC_PORT);
    spdlog::get("logger")->info("Started sensor bridge to {}:{}", OSC_HOST, OSC_PORT);
    
    gpio.setDirection(26, false);
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
        result = lo_send(t, OSC_BG_RUN_PATH, "i", clientId);
    } else if (sum == 2) {
        result = lo_send(t, OSC_BG_RUN_PATH, "i", 8);
    } else if (sum == 3) {
        result = lo_send(t, OSC_BG_RUN_PATH, "i", 9);
    } else if (sum == 4) {
        result = lo_send(t, OSC_BG_RUN_PATH, "i", 10);
    }
    if (result == 1) {
        logger->warn("Unable OSC message {} from {}", value, clientId);
    } else if (result == -1) {
        logger->error("Unable to send OSC message {} from {}", value, clientId);
    }
}

// Set the tide display level to `level'
void TidesDisplayModel::setTideLevel(long level) {
    tideLevel = level;
    int result = 0;
    result = lo_send(t, OSC_BG_RUN_PATH, "i", tideLevel);
}

// Initiate startup sequence
void TidesDisplayModel::initiateStartup() {
    logger->info("Initiating startup sequence");
    transitionTime = time(NULL);
    state = startingUp;
    int result = 0;
    result = lo_send(t, OSC_BG_MODE_PATH, "i", 1);
    result = lo_send(t, OSC_BG_RUN_PATH, "i", 99);
    
    gpio.setValue(26, true);
}

// Initiate shutdown sequence
void TidesDisplayModel::initiateShutdown() {
    logger->info("Initiating shutdown sequence");
    transitionTime = time(NULL);
    state = shuttingDown;
    int result = 0;
    result = lo_send(t, OSC_BG_RUN_PATH, "i", 99);

}

// Iterate transition
void TidesDisplayModel::iterateState() {
    if (state == startingUp) {
        if (time(NULL) - transitionTime > 10) {
            logger->info("Transitioning to running");
            state = running;
        }
    } else if (state == shuttingDown) {
        if (time(NULL) - transitionTime > 10) {
            logger->info("Transitioning to stopped");
            state = stopped;            
            lo_send(t, OSC_BG_MODE_PATH, "i", 0);
            gpio.setValue(26, false);
        }
    }
}
