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

#include <chrono>
#include <time.h>
#include <arpa/inet.h>
#include "tides_display_model.h"
#include "spdlog/spdlog.h"

auto logger = spdlog::stdout_color_mt("logger");

TidesDisplayModel model = TidesDisplayModel();
bool timingRestrictions = true;

void error(int num, const char *msg, const char *path) {
    if (path != NULL) logger->error("liblo server error {}: {}", num, msg);
    else logger->error("liblo server error {} in path {}: {}", num, path, msg);
    fflush(stdout);
}

bool systemOn() {
    // Enable system when between 0 and 10 GMT, 6PM to 8AM PST
    time_t theTime = time(NULL);
    struct tm *aTime = localtime(&theTime);
    if (aTime->tm_hour > 10) return true;
    else return false;
}

int sensor_handler(const char *path, const char *types, lo_arg ** argv,
                int argc, void *data, void *user_data)
{
    logger->info("{} <- id:{}, value:{}", path, argv[0]->i, argv[1]->i);
    model.received(argv[0]->i, argv[1]->i);
    return 0;
}

void check_timing()
{
    long tideLevel = model.tidesData.tideLevel();
    model.iterateState();
    if (model.tidesData.itsLightout()) {
        if (model.state != stopped && model.state != shuttingDown) {
            model.initiateShutdown();
        }
    } else {
        if (model.state == stopped) {
            model.initiateStartup();
        } else if (model.state == running && model.tideLevel != tideLevel) {
            logger->info("Tide level {}", tideLevel);
            model.setTideLevel(tideLevel);
        }
    }
}

lo_server_thread setupServer(const char* port) {
    
    lo_server_thread st = lo_server_thread_new(port, error);
    
    lo_server_thread_add_method(st, "/LEDPlay/player/foregroundRunIndex", "ii", sensor_handler, NULL);
    lo_server_thread_start(st);
    
    return st;
}

int main(int argc, char *argv[])
{
    char* timing = getenv("TIDES_SENSOR_TIMING_OFF");
    if (timing) timingRestrictions = false;
    
    char* port = getenv("TIDES_SENSOR_PORT");
    
    if (!port) port = (char*)"1999";
    logger->info("Port: {}", port);
    lo_server_thread st = setupServer(port);
    
    while (1) {
        usleep(50000);
        check_timing();
    }
    
    lo_server_thread_free(st);

    logger->info("EXITING");
    
    return 0;
}

