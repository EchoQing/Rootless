//
//  main.cpp
//  Rootless
//
//  Created by MacPu on 2019/11/2.
//  Copyright © 2019 MacPu. All rights reserved.
//

#include <iostream>
#include <climits>
#include "tsp.h"
#include "usage.h"
#include "twoOpt.h"
#include "MyThread.h"        // thread wrapper class
#include "defines.h"
#include "Clock.hpp"

int main(int argc, char** argv) {
    // Check that user entered filename on command line
    if (argc < 2)
    {
        usage();
        exit(-1);
    }
    
    srand(1);
    
    // Read file names from input
    string f, o;
    f = o = argv[1];
    o.append(".tour");
    
    vector<TSP::City>cities;
    ofstream outputStream;
    outputStream.open(f.c_str(), ios::out);
    for (int i = 0; i < NODE_NUMBER; ++i) {
        //for (vector<int>::iterator it = circuit.begin(); it != circuit.end()-1; ++it) {
        int x = rand() % GRAPH_RANGE;
        int y = rand() % GRAPH_RANGE;
        int hover_time = rand() % (MAX_HOVER_TIME - MIN_HOVER_TIME - 1) + MIN_HOVER_TIME;
//        struct TSP::City c = {x,y,hover_time};
//        cities.push_back(c);
        outputStream << i << ' ' << x << ' ' << y << ' ' << hover_time << endl;
    }
    //outputStream << *(circuit.end()-1);
    outputStream.close();
    // Create new tsp object
    
    Clock startTime;
    
    TSP tsp(f, o);
    
    tsp.solution();
    
    return 0;
}
