//
//  Clock.hpp
//  Rootless
//
//  Created by MacPu on 2019/11/5.
//  Copyright © 2019 MacPu. All rights reserved.
//

#ifndef Clock_hpp
#define Clock_hpp

#include <stdio.h>
#include <iostream>

class Clock {
    struct timespec _time;
public:
    Clock() {
        clock_gettime(CLOCK_MONOTONIC, &_time);
    };
    ~Clock() {
    };
    
    double operator -(const Clock& clock) {
        double duration;
        duration = (_time.tv_sec - clock._time.tv_sec);
        duration += (_time.tv_nsec - clock._time.tv_nsec) / 1000000000.0;
        return duration;
    }
};

#endif /* Clock_hpp */
