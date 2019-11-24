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
#include "permutation_combination.h"

vector<vector<TSP::City>> connected_graph(TSP *tsp, int B);
void find_connected_dfs(int **graph, int *connected, int index, int n, int id);

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
        else if (p > 2 * __K__) { // 猜测值可能太小了。
            Bl = B;
            continue;
        } else {
            vector<vector<vector<int>>> permutation = get_permutation_combination(p);
            
            cout << permutation.size();
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

void test_tsp(TSP tsp)
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


int test_permutation_combination()
{
    int n = 4;
    vector<int> arr;
    for (int i = 0; i< n; i++) {
        arr.push_back(i);
    }
    
    vector<int> select;
    vector<vector<vector<int>>> aa;
    
    for (int i = 1 ; i <= n; i++) {
        cout << "有" << i << "个盒子\n----------------" <<endl;
        vector<vector<int>> result;
        permutation(arr, i, result, &aa);
    }
    
    for (vector<vector<vector<int>>>::iterator it=aa.begin(); it != aa.end(); it++) {
//        print_reslut(*it, n);
    }
    
    return 0;
}

