//
//  gpio.h
//  xcode
//
//  Created by Mauricio Bustos on 10/21/19.
//  Copyright © 2019 Mauricio Bustos. All rights reserved.
//

#ifndef gpio_hpp
#define gpio_hpp

class GPIO {
    
public:
    
    GPIO();
    void setDirection(int pin, bool input);
    void setValue(int pin, bool high);
    
};

#endif /* gpio_h */
