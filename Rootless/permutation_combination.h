//
//  permutation_combination.h
//  Rootless
//
//  Created by MacPu on 2019/11/25.
//  Copyright © 2019 MacPu. All rights reserved.
//

#ifndef permutation_combination_h
#define permutation_combination_h

#include <stdio.h>
#include <iostream>
#include <climits>
#include <vector>

#define permutation_log 0

using namespace std;

/// 组合
/// @param data 源数据
/// @param step 步骤
/// @param select_data 中间值
/// @param target_num 取多少个
/// @param result 答案
void combination(vector<int> data, int step, vector<int> select_data, int target_num, vector<vector<int>> *result);

/// 排列
/// @param arr 数组
/// @param box 盒子的个数。
/// @param selected 中间状态
/// @param result 最终的答案。
void permutation(vector<int> arr, int box, vector<vector<int>> selected, vector<vector<vector<int>>> *result, int n);

/// 获取排列组合
/// @param n 有多少个数。
vector<vector<vector<int>>> get_permutation_combination(int n);

#endif /* permutation_combination_h */
