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

#include "lo/lo.h"
#include "spdlog/spdlog.h"

int main(int argc, char *argv[])
{

  auto console = spdlog::stdout_color_mt("console");

  char* OSC_HOST = std::getenv("OSC_HOST");
  char* OSC_PORT = std::getenv("OSC_PORT");

  if (OSC_HOST == NULL || OSC_PORT == NULL) {
    console->error("UNDEFINED ENVIRONMENT OSC_HOST AND OR OSC_PORT");
    return -1;
  }

  console->info("Starting sensor bridge to {}:{}", OSC_HOST, OSC_PORT);
  
  lo_address t = lo_address_new(OSC_HOST, OSC_PORT);

  console->info("Sending test data");
  lo_send(t, "/LEDPlay/player/backgroundRunIndex/", "i", 1);
  sleep(5000);
  lo_send(t, "/LEDPlay/player/backgroundRunIndex/", "i", 2);
  sleep(5000);
  lo_send(t, "/LEDPlay/player/backgroundRunIndex/", "i", 3);
  sleep(5000);
  lo_send(t, "/LEDPlay/player/backgroundRunIndex/", "i", 4);
  sleep(5000);

  return 0;
}

