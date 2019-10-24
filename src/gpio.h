//
//  gpio.h
//  xcode
//
//  Created by Mauricio Bustos on 10/21/19.
//  Copyright © 2019 Mauricio Bustos. All rights reserved.
//

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
