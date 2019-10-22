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

#ifndef EARTH_DATA_H
#define EARTH_DATA_H

#include <tuple>
#include <vector>
#include "spdlog/spdlog.h"

inline int ParseInt(const char* value) {
    return (int) std::strtol(value, nullptr, 10);
}

using namespace std;

class EarthData {
    
public:
    
    EarthData();
    void populateTidesData();
    void populateSunData();
    bool itsLightout();
    float tideHeight();
    long tideLevel();
    
    float min_tide = 1000.0;
    float max_tide = -1000.0;
    
    vector<float> tide_breaks;
    
private:
    
    std::shared_ptr<spdlog::logger> console = spdlog::get("console");
    time_t ParseISO8601(const std::string& input);
    vector<tuple<time_t, float>> tides;
    vector<tuple<time_t, time_t>> sunriseSunset;

};

#endif
