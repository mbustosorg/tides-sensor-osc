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
#include "lo/lo.h"
#include "tides_data.h"

static int frameCount = 0;
#define FRAME_RATE_INTERVAL_OUT (500)
#define TIMING_OUT (false)
const long long FramePeriod = 32000;

#define MAX_CLIENTS 10
int master_socket, addrlen, client_socket[30];
auto console = spdlog::stdout_color_mt("console");
struct sockaddr_in address;
uint8_t buffer[1025];

bool timingOutput(int frameCount) {
    return TIMING_OUT && frameCount % FRAME_RATE_INTERVAL_OUT == 0;
}

void setupServer() {
    int opt = 1;
    
    const char* envPort = getenv("TIDES_SENSOR_PORT");
    int port = 0;
    if (envPort) port = atoi(envPort);
    else port = 1999;
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_socket[i] = 0;
    }
    if ((master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0) {
        console->warn("socket failed");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) {
        console->warn("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    if (::bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0) {
        console->warn("bind failed");
        exit(EXIT_FAILURE);
    }
    console->info("Listening for sensor clients on port {0:d}", port);
    if (listen(master_socket, 3) < 0) {
        console->warn("could not set listen");
        exit(EXIT_FAILURE);
    }
    addrlen = sizeof(address);
    console->info("Waiting for connections ...");
}

void sendOscCommand(lo_address* target, int source, int level) {
    int THRESHOLD = 512;
    int result = 0;
    switch(source) {
        case 0:
            console->info("Run pattern 1");
            if (level < THRESHOLD) {
                result = lo_send(*target, "/LEDPlay/player/backgroundRunIndex/", "i", 1);
            } else {
                result = lo_send(*target, "/LEDPlay/player/backgroundRunIndex/", "i", 2);
            }
            break;
        case 1:
            console->info("Run pattern 1");
            if (level < THRESHOLD) {
                result = lo_send(*target, "/LEDPlay/player/backgroundRunIndex/", "i", 1);
            } else {
                result = lo_send(*target, "/LEDPlay/player/backgroundRunIndex/", "i", 3);
            }
            break;
    }
    if (result == 0) {
        console->info("Send OSC message {} from {}", level, source);
    } else if (result == 1) {
        console->warn("Send no OSC message {} from {}", level, source);
    } else if (result == -1) {
        console->error("Unable to send OSC message {} from {}", level, source);
    }
}

int main(int argc, char *argv[])
{
    TidesData tidesData("./tides_data/tidelevels_9414863.csv");
    
    fd_set readfds;
    int valread, sd, max_sd, new_socket;
    
    char* OSC_HOST = std::getenv("OSC_HOST");
    char* OSC_PORT = std::getenv("OSC_PORT");
    
    if (OSC_HOST == NULL || OSC_PORT == NULL) {
        console->error("UNDEFINED ENVIRONMENT OSC_HOST AND OR OSC_PORT");
        return -1;
    }
    
    setupServer();
    
    console->info("Starting sensor bridge to {}:{}", OSC_HOST, OSC_PORT);
    lo_address t = lo_address_new(OSC_HOST, OSC_PORT);

    struct timeval tv = {0, FramePeriod};
    
    auto frameRateMonitorStart = chrono::high_resolution_clock::now();
    
    while (1) {
        auto frameStart = chrono::high_resolution_clock::now();
        
        frameCount++;
        if (timingOutput(frameCount)) {
            auto frameRateMonitorStop = std::chrono::high_resolution_clock::now();
            long long updateLength = std::chrono::duration_cast<std::chrono::microseconds>(frameRateMonitorStop - frameRateMonitorStart).count();
            console->info("Average Frame Time (ms): {:03.2f}", float(updateLength / FRAME_RATE_INTERVAL_OUT));
            frameRateMonitorStart = std::chrono::high_resolution_clock::now();
        }
        
        // Do Somthing Here
        
        FD_ZERO(&readfds);
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;
        for (int i = 0 ; i < MAX_CLIENTS ; i++) {
            sd = client_socket[i];
            if (sd > 0) FD_SET(sd , &readfds);
            if (sd > max_sd) max_sd = sd;
        }
        
        auto frameStop = chrono::high_resolution_clock::now();
        long long updateLength = chrono::duration_cast<std::chrono::microseconds>(frameStop - frameStart).count();
        
        tv.tv_sec = 0;
        tv.tv_usec = FramePeriod - 1500 - updateLength;
        
        if (timingOutput(frameCount)) {
            console->info("start: {0:d}", frameStart.time_since_epoch().count());
        }
        
        int selectResult = select(max_sd + 1, &readfds, NULL, NULL, &tv);
        
        if (timingOutput(frameCount)) {
            console->info("stop: {0:d}", frameStop.time_since_epoch().count());
        }
        
        if (selectResult > 0) {
            if (FD_ISSET(master_socket, &readfds)) {
                if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
                    console->warn("Could not accept new client");
                    exit(EXIT_FAILURE);
                }
                console->info("New connection, socket fd: {}, ip: {}, port, {}", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    if (client_socket[i] == 0) {
                        client_socket[i] = new_socket;
                        console->info("Adding to list of sockets as {}", i);
                        break;
                    }
                }
            }
            for (int i = 0; i < MAX_CLIENTS; i++) {
                sd = client_socket[i];
                if (FD_ISSET(sd , &readfds)) {
                    long received = read(sd , buffer, 1024);
                    if (received == 0L) {
                        getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);
                        console->info("Host {}:{} disconnected", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                        close(sd);
                        client_socket[i] = 0;
                    } else if (received > 0L) {
                        try {
                            buffer[received] = 0;
                            int level = std::stoi((char*)buffer);
                            console->info("Received {} from {}:{}", level, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                            sendOscCommand(&t, i, level);
                        }
                        catch(std::exception const & e)
                        {
                            console->error("Received invalid value from client: {}", buffer);
                        }
                    }
                }
            }
        }
    }
    console->info("EXITING");
    
    return 0;
}

