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
#include <arpa/inet.h>
#include "tides_display_model.h"

#define FRAME_RATE_INTERVAL_OUT (500)
const long long FramePeriod = 32000;

#define MAX_CLIENTS 10
int master_socket, addrlen, client_socket[30];
auto console = spdlog::stdout_color_mt("console");
struct sockaddr_in address;
uint8_t buffer[1025];
TidesDisplayModel model = TidesDisplayModel();

void error(int num, const char *msg, const char *path) {
    console->error("liblo server error {} in path {}: {}", num, path, msg);
    fflush(stdout);
}

int generic_handler(const char *path, const char *types, lo_arg ** argv, int argc, void *data, void *user_data) {
    
    int i;
    
    console->info("path: <{}>", path);
    for (i = 0; i < argc; i++) {
        console->info("arg {} '{}' ", i, types[i]);
        lo_arg_pp((lo_type)types[i], argv[i]);
    }
    fflush(stdout);
    return 1;
}

int sensor_handler(const char *path, const char *types, lo_arg ** argv,
                int argc, void *data, void *user_data)
{
    console->info("{} <- f:{}, i:{}", path, argv[0]->f, argv[1]->i);
    fflush(stdout);
    return 0;
}

lo_server_thread setupServer(const char* port) {
    
    lo_server_thread st = lo_server_thread_new(port, error);
    
    lo_server_thread_add_method(st, NULL, NULL, generic_handler, NULL);
    lo_server_thread_add_method(st, "/sensor", "ii", sensor_handler, NULL);
    
    lo_server_thread_start(st);
    
    return st;
}

int main(int argc, char *argv[])
{
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

