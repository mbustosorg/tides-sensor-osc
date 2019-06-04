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

#include <chrono>
#include <time.h>
#include <arpa/inet.h>
#include "tides_display_model.h"

auto console = spdlog::stdout_color_mt("console");

TidesDisplayModel model = TidesDisplayModel();
bool timingRestrictions = true;

void error(int num, const char *msg, const char *path) {
    if (path != NULL) console->error("liblo server error {}: {}", num, msg);
    else console->error("liblo server error {} in path {}: {}", num, path, msg);
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
    console->info("{} <- id:{}, value:{}", path, argv[0]->i, argv[1]->i);
    if (model.tidesData.itsLightout() && timingRestrictions) {
        console->info("System is off");
    } else {
        model.received(argv[0]->i, argv[1]->i);
    }
    return 0;
}

lo_server_thread setupServer(const char* port) {
    
    lo_server_thread st = lo_server_thread_new(port, error);
    
    lo_server_thread_add_method(st, model.OSC_PATH, "ii", sensor_handler, NULL);
    lo_server_thread_start(st);
    
    return st;
}

int main(int argc, char *argv[])
{
    char* timing = getenv("TIDES_SENSOR_TIMING_OFF");
    if (timing) timingRestrictions = false;
    
    char* port = getenv("TIDES_SENSOR_PORT");
    
    if (!port) port = (char*)"1999";
    
    lo_server_thread st = setupServer(port);
    
    while (1) {
        usleep(100);
    }
    
    lo_server_thread_free(st);

    console->info("EXITING");
    
    return 0;
}

