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

#include <cstdlib>
#include <ctime>
#include <string>
#include <algorithm>
#include "fast-cpp-csv-parser/csv.h"
#include "earth_data.h"
#include "spdlog/spdlog.h"

using namespace std;

EarthData::EarthData() {
    populateTidesData();
    populateSunData();
}

void EarthData::populateTidesData() {

    string* filename = NULL;
    
    if(const char* env_p = std::getenv("OSC_TIDES_CSV")) {
        filename = new string(std::getenv("OSC_TIDES_CSV"));
    } else {
        filename = new string("./earth_data/tidelevels_9414863.csv");
    }
    time_t currentTime = time(NULL);
    console->info("Current Time: {}", currentTime);
    bool timeDetected = false;
    
    console->info("Detecting min/max heights...");
    io::CSVReader<2> in_minmax(*filename);
    in_minmax.read_header(io::ignore_no_column, "DateTime", "Height");
    std::string dateTime; float height = 0.0;
    while(in_minmax.read_row(dateTime, height)){
        min_tide = min(height, min_tide);
        max_tide = max(height, max_tide);
    }
    console->info("Min {} ft, Max {}", min_tide, max_tide);
    
    std::vector<float>::iterator it;
    for (int i = 0; i < 10; i++) {
        it = tide_breaks.begin();
        it = tide_breaks.insert (it, max_tide - (max_tide - min_tide) / 10.0 * i);
    }

    console->info("Reading tide data...");
    io::CSVReader<2> in(*filename);
    in.read_header(io::ignore_no_column, "DateTime", "Height");
    while(in.read_row(dateTime, height)){
        time_t time = ParseISO8601(dateTime);
        if (time > currentTime && !timeDetected) {
            console->info("Current tide height = {} ft @ {}", height, dateTime.c_str());
            timeDetected = true;
        }
        tides.insert(tides.end(), tuple<time_t, float> (time, height));
    }
    console->info("Read {0:d} tides records", tides.size());
}

void EarthData::populateSunData() {

    string* filename = NULL;
    
    if(const char* env_p = std::getenv("OSC_SUN_CSV")) {
        filename = new string(std::getenv("OSC_SUN_CSV"));
    } else {
        filename = new string("./earth_data/sunriseSunset.csv");
    }
    time_t currentTime = time(NULL);
    std::tm sunDataYear = *gmtime(&currentTime);
    sunDataYear.tm_year = 100;
    currentTime = timegm(&sunDataYear);
    bool timeDetected = false;
    console->info("Reading sunrise/sunset data...");
    io::CSVReader<2> in(*filename);
    in.read_header(io::ignore_no_column, "sunrise", "sunset");
    std::string sunrise; std::string sunset;
    time_t lastSunset = time(NULL);
    while(in.read_row(sunrise, sunset)){
        time_t sunriseTime = ParseISO8601(sunrise);
        time_t sunsetTime = ParseISO8601(sunset);
        if (!timeDetected) {
            if (sunsetTime > currentTime && sunriseTime < currentTime) {
                console->info("Daytime");
                timeDetected = true;
            } else if (lastSunset < currentTime && sunriseTime > currentTime) {
                console->info("Nighttime");
                timeDetected = true;
            }
        }
        lastSunset = sunsetTime;
        sunriseSunset.insert(sunriseSunset.end(), tuple<time_t, time_t> (sunriseTime, sunsetTime));
    }
    console->info("Read {0:d} sunrise/sunset records", sunriseSunset.size());
    itsLightout();
}

bool EarthData::itsLightout() {
    time_t currentTime = time(NULL);
    std::tm currentTimeStruct = *gmtime(&currentTime);
    currentTimeStruct.tm_year = 100;
    currentTime = timegm(&currentTimeStruct);
    vector<tuple<time_t, time_t>>::iterator it;
    for(it = sunriseSunset.begin(); it != sunriseSunset.end(); it++)    {
        tuple<time_t, time_t> record = *it;
        time_t sunset = std::get<1>(record) - 60 * 60;
        if (sunset > currentTime && currentTimeStruct.tm_hour < 12) {
            return false;
        }
    }
    return true;
}

float EarthData::tideHeight() {
    time_t currentTime = time(NULL);
    std::tm sunDataYear = *gmtime(&currentTime);
    currentTime = timegm(&sunDataYear);
    vector<tuple<time_t, float>>::iterator it;
    for(it = tides.begin(); it != tides.end(); it++)    {
        tuple<time_t, float> record = *it;
        if (std::get<0>(record) > currentTime) {
            return std::get<1>(record);
        }
    }
    return 0.0;
}

long EarthData::tideLevel() {
    float height = tideHeight();
    
    vector<float>::iterator it;
    for(it = tide_breaks.begin(); it != tide_breaks.end(); it++)    {
        float record = *it;
        if (record > height) {
            return it - tide_breaks.begin();
        }
    }
    return 0.0;

}

time_t EarthData::ParseISO8601(const string& input) {
    
    constexpr const size_t expectedLength = sizeof("1234-12-12 12:12:12");
    static_assert(expectedLength == 20, "Unexpected ISO 8601 date/time length");
    
    if (input.length() >= expectedLength) {
        return 0;
    }
    std::tm time = {0};
    time.tm_year = ParseInt(&input[0]) - 1900;
    time.tm_mon = ParseInt(&input[5]) - 1;
    time.tm_mday = ParseInt(&input[8]);
    time.tm_hour = ParseInt(&input[11]);
    time.tm_min = ParseInt(&input[14]);
    time.tm_sec = ParseInt(&input[17]);
    time.tm_isdst = 0;
    return timegm(&time);
}
