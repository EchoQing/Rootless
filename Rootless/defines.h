//
//  defines.h
//  Rootless
//
//  Created by MacPu on 2019/11/2.
//  Copyright © 2019 MacPu. All rights reserved.
//

#ifndef defines_h
#define defines_h

// Toggle printing debugging info to console
#define __DEBUG__ 0

// 默认速度
#define __V__ 10

// 节点的个数
#define NODE_NUMBER 30

// 最大和最小的停留时间。
#define MAX_HOVER_TIME 10
#define MIN_HOVER_TIME 1

// 生成的图范围
#define GRAPH_RANGE 1000

// 算最优路径的线程个数。
#define NUM_THREADS 10

// 
#define CPS CLOCKS_PER_SEC

// K 值
#define __K__ 3

#define DebugLog(...) \
if (__DEBUG__) {\
    printf(__VA_ARGS__); \
    printf("\n"); \
}


#endif /* defines_h */
