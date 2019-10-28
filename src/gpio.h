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

#ifndef gpio_hpp
#define gpio_hpp

#include "spdlog/spdlog.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>

class GPIO {
    
public:
    
    GPIO();
    void setDirection(int pin, bool input);
    void setValue(int pin, bool high);
    
private:
    
    std::shared_ptr<spdlog::logger> logger = spdlog::get("logger");
    
};

#endif /* gpio_h */
