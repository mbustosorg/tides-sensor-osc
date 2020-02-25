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

#ifndef tides_display_model_hpp
#define tides_display_model_hpp

#include <stdio.h>
#include <vector>
#include "earth_data.h"
#include "lo/lo.h"
#include "gpio.h"
#include "spdlog/spdlog.h"

const int TOWER_COUNT = 7;

enum model_state {
    startingUp,
    running,
    shuttingDown,
    stopped
};

class TidesDisplayModel {

public:
    TidesDisplayModel();
    void received(int clientId, int value);
    void setTideLevel(long level);
    void initiateStartup();
    void initiateShutdown();
    void iterateState();
    
    EarthData tidesData = EarthData();
    
    model_state state = stopped;
    long tideLevel = 0;

    const char* OSC_BG_RUN_PATH = "/LEDPlay/player/backgroundRunIndex";
    const char* OSC_BG_MODE_PATH = "/LEDPlay/player/backgroundMode";
    const char* OSC_FG_RUN_PATH = "/LEDPlay/player/foregroundRunIndex";
    const char* OSC_BG_SENSOR_PATH = "/LEDPlay/player/foregroundRunIndex";

private:
    
    GPIO gpio = GPIO();
    std::shared_ptr<spdlog::logger> logger = spdlog::get("logger");
    lo_address t;
    vector<int>* clients = new vector<int>(TOWER_COUNT + 1, 0);
    
    time_t transitionTime;
    
};

#endif /* tides_display_model_h */
