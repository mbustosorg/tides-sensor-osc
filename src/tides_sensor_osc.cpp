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

void monitorNetwork() {
    fd_set readfds;
    int sd, max_sd, new_socket;
    
    FD_ZERO(&readfds);
    FD_SET(master_socket, &readfds);
    max_sd = master_socket;
    for (int i = 0 ; i < MAX_CLIENTS ; i++) {
        sd = client_socket[i];
        if (sd > 0) FD_SET(sd , &readfds);
        if (sd > max_sd) max_sd = sd;
    }

    struct timeval tv = {0, FramePeriod};
    tv.tv_sec = 0;
    tv.tv_usec = FramePeriod - 1500;

    int selectResult = select(max_sd + 1, &readfds, NULL, NULL, &tv);
    
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
                    model.newClient(i);
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
                    model.clientDropped(i);
                } else if (received > 0L) {
                    try {
                        buffer[received] = 0;
                        int level = std::stoi((char*)buffer);
                        console->info("Received {} from {}:{}", level, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                        model.received(i, level);
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

int main(int argc, char *argv[])
{
    setupServer();

    while (1) {
        monitorNetwork();
    }
    console->info("EXITING");
    
    return 0;
}

