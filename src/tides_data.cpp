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
#include "fast-cpp-csv-parser/csv.h"
#include "tides_data.h"
#include "spdlog/spdlog.h"

using namespace std;

TidesData::TidesData() {
    
    string* filename = NULL;
    
    if(const char* env_p = std::getenv("OSC_TIDES_CSV")) {
        filename = new string(std::getenv("OSC_TIDES_CSV"));
    } else {
        filename = new string("./tides_data/tidelevels_9414863.csv");
    }
    time_t currentTime = time(NULL);
    console->info("Current Time: {}", currentTime);
    bool timeDetected = false;
    console->info("Reading tide data...");
    io::CSVReader<2> in(*filename);
    in.read_header(io::ignore_no_column, "DateTime", "Height");
    std::string dateTime; double height;
    while(in.read_row(dateTime, height)){
        time_t time = ParseISO8601(dateTime);
        if (time > currentTime && !timeDetected) {
            console->info("Current tide height = {} ft @ {}", height, dateTime.c_str());
            timeDetected = true;
        }
        timeseries.insert(timeseries.end(), tuple<time_t, float> (time, height));
    }
    console->info("Read {0:d} records", timeseries.size());
}

time_t TidesData::ParseISO8601(const string& input) {
    
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
