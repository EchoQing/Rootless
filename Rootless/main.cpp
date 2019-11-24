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

vector<vector<TSP::City>> connected_graph(TSP *tsp, int B);
void find_connected_dfs(int **graph, int *connected, int index, int n, int id);

int main1(int argc, char** argv) {
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
    
    TSP tsp(f, o);
    tsp.solution();
    
    int Bl = 0;
    int Bu = tsp.pathLength;
    while (Bl + 1 < Bu) {
        int B =  (Bl + Bu) / 2;
        vector<vector<TSP::City>> graph_lite = connected_graph(&tsp, B);
        int p = (int)graph_lite.size();
        if (p <= 1) {
            Bu = B;
            continue;
        }
        else if (p > 2 * __K__) {
            Bl = B;
            continue;
        }
    }
    return 0;
}

/// 根据 删除 B / 3 的边之后， 生成N 个联通图。
/// @param tsp tsp
/// @param B B
vector<vector<TSP::City>> connected_graph(TSP *tsp, int B)
{
    // 先删除掉所有大于 B / 3 的边
    int b = B / 3;
    int n = tsp->n;
    int **graph = new int*[n];
    for (int i = 0; i < n; i++) {
        graph[i] = new int[n];
        for (int j = 0; j <n; j++) {
            if (tsp->graph[i][j] <= b) {
                graph[i][j] = tsp->graph[i][j];
            } else {
                graph[i][j] = 0;
            }
        }
    }
    
    // 创建一个数组，记录分组p情况。
    int *connected = new int[n];
    for (int i = 0; i< n; i++) {
        connected[i] = 0;
    }
    
    // 使用深度搜索算法分组。
    int index = 1;
    for (int i = 0; i < n; i++) {
        if (connected[i] == 0) {
            connected[i] = index;
            find_connected_dfs(graph, connected, index, n, i);
            index ++;
        }
    }
    
    // 记录生成的各个小图
    vector<vector<TSP::City>> citites;
    for (int i = 1; i <= n; i++) {
        bool has = false;
        vector<TSP::City> city;
        for (int j = 0; j < n; j++) {
            if(connected[j] == i) {
                city.push_back(tsp->cities[j]);
                has = true;
            }
        }
        if (has) {
            citites.push_back(city);
        } else break;
    }
    
    // print log
//    for (int i = 0; i < n; i++) {
//        for (int j = 0; j < n; j++) {
//            cout << graph[i][j] << "  ";
//        }
//        cout << endl;
//    }
     
    // clear memory
    for (int i = 0; i < n; i++) {
//        cout << connected[i] << " - " ;
        delete [] graph[i];
    }
    delete [] graph;
    
    return citites;
}

/// 用深度搜索查找分组
/// @param graph 图
/// @param connected 当前分组情况
/// @param index 当前组号
/// @param n size
/// @param id 目前id
void find_connected_dfs(int **graph, int *connected, int index, int n, int id)
{
    graph[id][id] = -index;
    for (int i = 0; i < n; i++) {
        if (graph[id][i] > 0) {
            if (connected[i] == 0) {
                connected[i] = index;
                find_connected_dfs(graph, connected, index, n, i);
            }
            graph[id][i] = -index;
        }
    }
}

void testTsp(TSP tsp)
{
    
    int Bl = 0;
    int Bu = tsp.pathLength;
    while (Bl + 1 < Bu) {
        int B =  (Bl + Bu) / 2;
        
        cout << "B: " << B / 3 << endl;
        
        // 先切割成多个联通分量。
        vector<vector<int>> connectedCompleteds;
        vector<int> connect;
        for (vector<int>::iterator it = tsp.circuit.begin(); it != tsp.circuit.end(); ++it) {
            connect.push_back(*it);
            if (*it == tsp.circuit.back()) {
                if (tsp.graph[*it][tsp.circuit.front()] > (B / 3)) {
                    connectedCompleteds.push_back(connect);
                } else  if (connectedCompleteds.size() > 0) {
                    vector<int> firstConnect = connectedCompleteds.front();
                    firstConnect.insert(firstConnect.end(), connect.begin(), connect.end());
                    connectedCompleteds.erase(connectedCompleteds.begin());
                    connectedCompleteds.insert(connectedCompleteds.begin(), firstConnect);
                    cout<<endl;
                } else {
                    connectedCompleteds.push_back(connect);
                }
                break;
            }
            if (tsp.graph[*it][*(it+1)] > (B / 3)) {
                connectedCompleteds.push_back(connect);
                connect = vector<int>();
            }
        }
        
        int p = (int)connectedCompleteds.size();
        
        for (vector<vector<int>>::iterator it = connectedCompleteds.begin(); it != connectedCompleteds.end(); ++it) {
            vector<int> con = *it;
            cout << "-------" << con.size() << "-----" <<endl;
            if (con.size() == 1) {

                cout << con.front() << endl;
                continue;
            }
            for (vector<int>::iterator iit = con.begin(); iit != con.end() - 1; ++iit) {

                cout << *iit << " to " << *(iit+1) << " ";
                cout << tsp.graph[*iit][*(iit+1)] << endl;
            }
            cout << "\n" << endl;
        }
        
        if (p <= 1) {
            Bu = B;
            continue;
        }
        else if (p > 2 * __K__) {
            Bl = B;
            continue;
        }
    }
    
}

void print_reslut(vector<vector<int>> reslut, size_t box)
{
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
}

void print_c(vector<int> select)
{
    cout<< "[ ";
    for(vector<int>::iterator it=select.begin(); it != select.end(); it++) {
        cout << *it << ' ';
    }
    cout<< "] "<< endl;
}
/// 组合
/// @param data 源数据
/// @param step 步骤
/// @param select_data 中间值
/// @param target_num 取多少个
/// @param result 答案
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



/// 排列
/// @param arr 数组
/// @param box 盒子的个数。
/// @param selected 中间状态
/// @param result 最终的答案。
void permutation(vector<int> arr, int box, vector<vector<int>> selected, vector<vector<vector<int>>> *result)
{
    int count = (int)arr.size();
    if (box == 1) {
        selected.push_back(arr);
        result->push_back(selected);
//        print_reslut(reslut, 4);
        return;
    } else if (box == count) {
        for (vector<int>::iterator it = arr.begin(); it != arr.end(); ++it) {
            vector<int> res;
            res.push_back(*it);
            selected.push_back(res);
        }
        result->push_back(selected);
//        print_reslut(reslut, 4);
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
                select_copy.push_back(combin);
                for (vector<int>::iterator iit=combin.begin(); iit != combin.end(); iit++) {
                    data.erase(find(data.begin(), data.end(), *iit));
                }
                permutation(data, box - 1, select_copy, result);
                
            }
        }
    }
}

int main()
{
    int n = 4;
    vector<int> arr;
    for (int i = 0; i< n; i++) {
        arr.push_back(i);
    }
    
    vector<int> select;
    vector<vector<vector<int>>> aa;
    
//    for(vector<vector<int>>::iterator it=result.begin();it != result.end(); it++) {
//        print_c(*it);
//    }
    
    for (int i = 1 ; i <= n; i++) {
//        cout << "有" << i << "个盒子\n----------------" <<endl;
        vector<vector<int>> result;
        permutation(arr, i, result, &aa);
    }
    
    for (vector<vector<vector<int>>>::iterator it=aa.begin(); it != aa.end(); it++) {
        print_reslut(*it, n);
    }
    
    return 0;
}

