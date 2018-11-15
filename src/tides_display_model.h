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

#ifndef tides_display_model_hpp
#define tides_display_model_hpp

#include <stdio.h>
#include <vector>
#include "tides_data.h"
#include "lo/lo.h"

const int TOWER_COUNT = 10;

class TidesDisplayModel {

public:
    TidesDisplayModel();
    void received(int clientId, int value);
    
    const char* OSC_PATH = "/LEDPlay/player/foregroundRunIndex";
    
private:
    
    std::shared_ptr<spdlog::logger> console = spdlog::get("console");
    lo_address t;
    vector<int>* clients = new vector<int>(TOWER_COUNT, 0);
    TidesData tidesData = TidesData();
    
};

#endif /* tides_display_model_h */
