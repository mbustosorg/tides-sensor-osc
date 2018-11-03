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

class TidesDisplayModel {

public:
    TidesDisplayModel();
    void newClient(int clientId);
    void clientDropped(int clientId);
    void received(int clientId, int value);
    
private:
    
    std::shared_ptr<spdlog::logger> console = spdlog::stdout_color_mt("console");
    lo_address t;
    vector<int> clients;
    TidesData tidesData = TidesData("./tides_data/tidelevels_9414863.csv");
    
};

#endif /* tides_display_model_hpp */