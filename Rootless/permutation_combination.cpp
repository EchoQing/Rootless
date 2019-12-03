//
//  permutation_combination.cpp
//  Rootless
//
//  Created by MacPu on 2019/11/25.
//  Copyright © 2019 MacPu. All rights reserved.
//

#include "permutation_combination.h"
#include <iostream>
#include <climits>
#include <vector>

void print_reslut(vector<vector<int>> reslut, size_t box)
{
#if permutation_log
    cout << "[ ";
    for (int i = 0; i < box; i++) {
        if (i < reslut.size()) {
            vector<int> res = reslut.at(i);
            cout << "{ ";
            for (vector<int>::iterator it = res.begin(); it != res.end(); ++it) {
                cout << *it << ", ";
            }
            cout << "} ";
        } else {
            cout << "-- ";
        }
        cout << ", ";
    }
    cout << "] " << endl;
#endif
}

void print_c(vector<int> select)
{
#if permutation_log

    cout<< "[ ";
    for(vector<int>::iterator it=select.begin(); it != select.end(); it++) {
        cout << *it << ' ';
    }
    cout<< "] "<< endl;
#endif
}

void combination(vector<int> data, int step, vector<int> select_data, int target_num, vector<vector<int>> *result)
{
    if (select_data.size() == target_num) {
        result->push_back(select_data);
        return;
    }
    if (step >= data.size()) {
        return;
    }
    select_data.push_back(data.at(step));
    combination(data, step + 1, select_data, target_num, result);
    select_data.pop_back();
    combination(data, step + 1, select_data, target_num, result);
}

void permutation(vector<int> arr, int box, vector<vector<int>> selected, vector<vector<vector<int>>> *result, int n)
{
    int count = (int)arr.size();
    if (box == 1) {
        selected.push_back(arr);
        result->push_back(selected);
        print_reslut(selected, n);
        return;
    } else if (box == count) {
        for (vector<int>::iterator it = arr.begin(); it != arr.end(); ++it) {
            vector<int> res;
            res.push_back(*it);
            selected.push_back(res);
        }
        result->push_back(selected);
        print_reslut(selected, n);
        return;
    } else {
        for(int i = 1; i <= count / box; i++) {
            vector<vector<int>> combination_res;
            vector<int> selected_combination;
            combination(arr, 0, selected_combination, i, &combination_res);
            for(vector<vector<int>>::iterator it=combination_res.begin(); it != combination_res.end(); it++) {
                vector<int> data = vector<int>(arr);
                vector<int> combin = *it;
                vector<vector<int>> select_copy = vector<vector<int>>(selected);
                // 这段纯属代码优化
                if (combin.size() == 1 && select_copy.size() > 0) {
                    vector<int> last = select_copy.back();
                    if (combin.front() <= last.front()) {
                        continue;
                    }
                }
                // 优化完。
                select_copy.push_back(combin);
                for (vector<int>::iterator iit=combin.begin(); iit != combin.end(); iit++) {
                    data.erase(find(data.begin(), data.end(), *iit));
                }
                permutation(data, box - 1, select_copy, result, n);
                
            }
        }
    }
}

vector<vector<vector<int>>> get_permutation_combination(int n)
{
    vector<int> data;
    for (int i = 0; i< n; i++) {
        data.push_back(i);
    }
    
    vector<int> select;
    vector<vector<vector<int>>> result;
    
    for (int i = 1 ; i <= n; i++) {
        vector<vector<int>> selected;
        permutation(data, i, selected, &result, n);
    }
    
    return result;
}
