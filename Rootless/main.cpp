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
vector<vector<int>> cutting_tsp(TSP *tsp, int b);
int algorithm(TSP *tsp);
int algorithm1(TSP *tsp);
void run_algorithm(int **answer, int n);
vector<TSP::Map> connected_map(TSP *tsp, int B);

int algorithm(TSP *tsp)
{
    int Bl = 0;
    int Bu = tsp->pathLength;
    while (Bl + 1 < Bu) {
        int B =  (Bl + Bu) / 2;
        vector<vector<TSP::City>> graph_lite = connected_graph(tsp, B / 3.0);
        int p = (int)graph_lite.size();
        LOG(LOG_CUT, "当B = %d, 图的个数: %d",B ,p);
        LOG(LOG_CUT, "------------------------\n");
        if (p > 2 * __K__) {
            // 猜测值可能太小了。
            Bl = B;
            continue;
        } else {
            vector<vector<vector<int>>> permutation = get_permutation_combination(p);
            bool found = false;
            for (vector<vector<vector<int>>>::iterator it= permutation.begin(); it != permutation.end(); it++) {
                vector<vector<int>> combination = *it;
                if (combination.size() == 1) {
                    // 如果 组合是1个的时候，代码优化，直接就用目前生成好的tsp算法计算。
                    vector<vector<int>> connect_complete = cutting_tsp(tsp, 13.0 / 6.0 * B);
                    if (connect_complete.size() <= __K__) {
                        Bu = B;
                        found = true;
                        break;
                    }
                } else {
                    int route_count = 0;
                    for (vector<vector<int>>::iterator iit=combination.begin(); iit != combination.end(); iit++) {
                        vector<TSP::City> cities;
                        vector<int> com = *iit;
                        for (vector<int>::iterator iiit = com.begin(); iiit != com.end(); iiit++) {
                            vector<TSP::City> city = graph_lite.at(*iiit);
                            cities.insert(cities.end(), city.begin(), city.end());
                        }
                        TSP _tsp(cities);
                        _tsp.solution();
                        vector<vector<int>> connect_complete = cutting_tsp(&_tsp, 13.0 / 6.0 * B);
                        route_count += connect_complete.size();
                        if (route_count > __K__) {
                            break;
                        }
                    }
                    if (route_count <= __K__) {
                        Bu = B;
                        found = true;
                        break;
                    }
                }
            }
            if (!found)
                Bl = B;
            continue;
        }
    }
    return Bu;
}

/// 合并两个TSP::Map
/// @param map1 第一个map
/// @param map2 第二个map
/// @param city1 连接时，第一个map里面的城市坐标
/// @param city2 连接时，第二个map里面的城市坐标
/// @param distance 两个城市的距离.
TSP::Map merge_map(TSP::Map map1, TSP::Map map2, int city1, int city2, int distance)
{
    int count = map2.n + map1.n;
    int **graph = new int *[count];
    for (int i = 0; i < count; i++) {
        graph[i] = new int[count];
        for (int j = 0; j < count; j++) {
            if (i < map1.n && j < map1.n) {
                graph[i][j] = map1.graph[i][j];
            } else if (i >= map1.n && j >= map1.n) {
                graph[i][j] = map2.graph[i - map1.n][j - map1.n];
            } else if ((i == city1 && j == city2 + map1.n) || (j == city1 && i == city2 + map1.n)) {
                graph[i][j] = distance;
            }else {
                graph[i][j] = 0;
            }
        }
    }
    vector<TSP::City> cities;
    cities.insert(cities.end(), map1.cities.begin(), map1.cities.end());
    cities.insert(cities.end(), map2.cities.begin(), map2.cities.end());
    
    struct TSP::Map map = {cities, graph, count};
    return map;
}

/// 合并 多个 TSP::Map
/// @param tsp tsp
/// @param map_group map数组
TSP::Map merge_map_group(TSP *tsp, vector<TSP::Map> map_group)
{
    if (map_group.size() == 1) {
        return map_group.front();
    }
    
    // 先初始化一个二维数组，记录
    int **graph = new int *[map_group.size()];
    int **record_city = new int *[map_group.size()];
    vector<TSP::City> fake_city;
    for (int i = 0; i < map_group.size(); i++) {
        graph[i] = new int[map_group.size()];
        record_city[i] = new int[map_group.size()];
        struct TSP::City c = {i, 0, 0, 0};
        fake_city.push_back(c);
    }
    
    for (int i = 0; i < map_group.size(); i++) {
        graph[i][i] = 0;
        for (int j = i + 1; j < map_group.size(); j++) {
            TSP::Map map1 = map_group.at(i);
            TSP::Map map2 = map_group.at(j);
            vector<TSP::City> group1 = map1.cities;
            vector<TSP::City> group2 = map2.cities;
            
            int city1index = 0;
            int city2index = 0;
            int minni_route = 100000000;
            for (int m = 0; m < group1.size(); m ++) {
                for (int n = 0; n < group2.size(); n ++) {
                    TSP::City i = group1.at(m);
                    TSP::City j = group2.at(n);
                    if (tsp->graph[i.index][j.index] < minni_route) {
                        city1index = m; city2index = n;
                        minni_route = tsp->graph[i.index][j.index];
                    }
                }
            }
            graph[i][j] = minni_route;
            graph[j][i] = minni_route;
            record_city[i][j] = city1index;
            record_city[j][i] = city2index;
        }
    }
    
    TSP temp_tsp(fake_city, graph);
    temp_tsp.solution();
    TSP::Map map = map_group.at(temp_tsp.circuit.front());
    for (vector<int>::iterator it = temp_tsp.circuit.begin() + 1; it != temp_tsp.circuit.end(); it++) {
        int city1index = 0;
        int city2index = 0;
        if (*(it - 1) < *it) {
            city1index = record_city[*(it - 1)][*it];
            city2index = record_city[*it][*(it - 1)];
            city1index = map.n - map_group.at(*(it - 1)).n + city1index;
        } else {
            city1index = record_city[*it][*(it - 1)];
            city2index = record_city[*(it - 1)][*it];
            city1index = map.n - map_group.at(*it).n + city1index;
        }
        map = merge_map(map, map_group.at(*it), city1index, city2index, graph[*it][*(it - 1)]);
    }
    
#if LOG_MERGE
    for (int i = 0; i < map.n; i ++ ){
        for (int j = 0; j < map.n; j++) {
            cout << map.graph[i][j]<<' ';
        }
        cout<<endl;
    }
#endif
    
    return map;
}

/// 我们的h实验算法 ，使用边
/// @param tsp tsp
int algorithm5(TSP *tsp)
{
    int Bl = 0;
    int Bu = tsp->pathLength;
    while (Bl + 1 < Bu) {
        int B =  (Bl + Bu) / 2;
        vector<TSP::Map> maps = connected_map(tsp, B / 3.0);
//        vector<vector<TSP::City>> graph_lite = connected_graph(tsp, B / 3.0);
        int p = (int)maps.size();
        LOG(LOG_CUT, "当B = %d, 图的个数: %d",B ,p);
        LOG(LOG_CUT, "------------------------\n");
        if (p > 2 * __K__) {
            // 猜测值可能太小了。
            Bl = B;
            continue;
        } else {
            vector<vector<vector<int>>> permutation = get_permutation_combination(p);
            bool found = false;
            for (vector<vector<vector<int>>>::iterator it= permutation.begin(); it != permutation.end(); it++) {
                vector<vector<int>> combination = *it;
                if (combination.size() == 1) {
                    // 如果 组合是1个的时候，代码优化，直接就用目前生成好的tsp算法计算。
                    vector<vector<int>> connect_complete = cutting_tsp(tsp, 13.0 / 6.0 * B);
                    if (connect_complete.size() <= __K__) {
                        Bu = B;
                        found = true;
                        break;
                    }
                } else {
                    int route_count = 0;
                    for (vector<vector<int>>::iterator iit=combination.begin(); iit != combination.end(); iit++) {
                        vector<int> com = *iit;
                        vector<TSP::Map> map;
                        for (vector<int>::iterator iiit = com.begin(); iiit != com.end(); iiit++) {
                            map.push_back(maps.at(*iiit));
                        }
                        TSP::Map mergedMap = merge_map_group(tsp, map);
                        TSP _tsp(mergedMap.cities, mergedMap.graph);
                        _tsp.solution();
                        vector<vector<int>> connect_complete = cutting_tsp(&_tsp, 13.0 / 6.0 * B);
                        route_count += connect_complete.size();
                        if (route_count > __K__) {
                            break;
                        }
                    }
                    if (route_count <= __K__) {
                        Bu = B;
                        found = true;
                        break;
                    }
                }
            }
            if (!found)
                Bl = B;
            continue;
        }
    }
    return Bu;
}

int algorithm1(TSP *tsp)
{
    int Bl = 0;
    int Bu = tsp->pathLength;
    while (Bl + 1 < Bu) {
        int B =  (Bl + Bu) / 2;
        vector<vector<TSP::City>> graph_lite = connected_graph(tsp, B / 2.0);
        int p = (int)graph_lite.size();
        LOG(LOG_CUT, "当B = %d, 图的个数: %d",B ,p);
        LOG(LOG_CUT, "------------------------\n");
        if (p > 2 * __K__) {
            // 猜测值可能太小了。
            Bl = B;
            continue;
        } else {
            int route_count = 0;
            for (vector<vector<TSP::City>>::iterator it=graph_lite.begin(); it != graph_lite.end(); it++) {
                TSP _tsp(*it);
                _tsp.solution();
                vector<vector<int>> connect_complete = cutting_tsp(&_tsp, 5.0 / 2.0 * B);
                route_count += connect_complete.size();
                if (route_count > __K__) {
                    break;
                }
            }
            if (route_count <= __K__) {
                Bu = B;
            } else {
                Bl = B;
            }
        }
    }
    
    return Bu;
}

int algorithm2(TSP *tsp)
{
   
    int Bl = 0;
    int Bu = tsp->pathLength;
    while (Bl + 1 < Bu) {
        int B =  (Bl + Bu) / 2;
        vector<vector<TSP::City>> graph_lite = connected_graph(tsp, B / 3.0);
        int p = (int)graph_lite.size();
        LOG(LOG_CUT, "当B = %d, 图的个数: %d",B ,p);
        LOG(LOG_CUT, "------------------------\n");
        if (p > 2 * __K__) {
            // 猜测值可能太小了。
            Bl = B;
            continue;
        } else {
            int route_count = 0;
            for (vector<vector<TSP::City>>::iterator it=graph_lite.begin(); it != graph_lite.end(); it++) {
                TSP _tsp(*it);
                _tsp.solution();
                vector<vector<int>> connect_complete = cutting_tsp(&_tsp, 13.0 / 6.0 * B);
                route_count += connect_complete.size();
                if (route_count > __K__) {
                    break;
                }
            }
            if (route_count <= __K__) {
                Bu = B;
            } else {
                Bl = B;
            }

        }
    }
    
    return Bu;
}

int algorithm3(TSP *tsp)
{
    int Bl = 0;
    int Bu = tsp->pathLength;
    while (Bl + 1 < Bu) {
        int B =  (Bl + Bu) / 2;
        vector<TSP::Map> maps = connected_map(tsp, B / 2.0);
        int p = (int)maps.size();
        LOG(LOG_CUT, "当B = %d, 图的个数: %d",B ,p);
        LOG(LOG_CUT, "------------------------\n");
        if (p > 2 * __K__) {
            // 猜测值可能太小了。
            Bl = B;
            continue;
        } else {
            int route_count = 0;
            for (vector<TSP::Map>::iterator it=maps.begin(); it != maps.end(); it++) {
                TSP::Map map = *it;
                TSP _tsp(map.cities, map.graph);
                _tsp.solution();
                vector<vector<int>> connect_complete = cutting_tsp(&_tsp, 5.0 / 2.0 * B);
                route_count += connect_complete.size();
                if (route_count > __K__) {
                    break;
                }
            }
            if (route_count <= __K__) {
                Bu = B;
            } else {
                Bl = B;
            }
        }
    }
    
    return Bu;
}

int algorithm4(TSP *tsp)
{
    int Bl = 0;
    int Bu = tsp->pathLength;
    while (Bl + 1 < Bu) {
        int B =  (Bl + Bu) / 2;
        vector<TSP::Map> maps = connected_map(tsp, B / 3.0);
        int p = (int)maps.size();
        LOG(LOG_CUT, "当B = %d, 图的个数: %d",B ,p);
        LOG(LOG_CUT, "------------------------\n");
        if (p > 2 * __K__) {
            // 猜测值可能太小了。
            Bl = B;
            continue;
        } else {
            int route_count = 0;
            for (vector<TSP::Map>::iterator it=maps.begin(); it != maps.end(); it++) {
                TSP::Map map = *it;
                TSP _tsp(map.cities, map.graph);
                _tsp.solution();
                vector<vector<int>> connect_complete = cutting_tsp(&_tsp, 13.0 / 6.0 * B);
                route_count += connect_complete.size();
                if (route_count > __K__) {
                    break;
                }
            }
            if (route_count <= __K__) {
                Bu = B;
            } else {
                Bl = B;
            }
        }
    }
    
    return Bu;
}

/// 根据 删除 B / 3 的边之后， 生成N 个联通图。
/// @param tsp tsp
/// @param B B
vector<vector<TSP::City>> connected_graph(TSP *tsp, int B)
{
    // 先删除掉所有大于 B / 3 的边
    int b = B;
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
    
    // clear memory
    for (int i = 0; i < n; i++) {
//        cout << connected[i] << " - " ;
        delete [] graph[i];
    }
    delete [] graph;
    
    return citites;
}

/// 根据 删除 B / 3 的边之后， 生成N 个联通图。
/// @param tsp tsp
/// @param B B
vector<TSP::Map> connected_map(TSP *tsp, int B)
{
    // 先删除掉所有大于 B / 3 的边
    int b = B;
    int n = tsp->n;
    int **graph = new int*[n]; // 用于计算切割图的。
    int **graph_copy = new int*[n]; // 用于记录
    for (int i = 0; i < n; i++) {
        graph[i] = new int[n];
        graph_copy[i] = new int[n];
        for (int j = 0; j <n; j++) {
            if (tsp->graph[i][j] <= b) {
                graph_copy[i][j] = graph[i][j] = tsp->graph[i][j];
            } else {
                graph_copy[i][j] = graph[i][j] = 0;
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
    vector<TSP::Map> maps;
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
            int count = (int)city.size();
            int **inner_graph = new int*[city.size()];
            for (int i = 0; i < count; i++) {
                inner_graph[i] = new int[n];
                for (int j = 0; j < count; j++) {
                    TSP::City city_i = city.at(i);
                    TSP::City city_j = city.at(j);
                    inner_graph[i][j] = graph_copy[city_i.index][city_j.index];
                }
            }
            struct TSP::Map map = {city, inner_graph, count};
            maps.push_back(map);
        } else break;
    }
    
    // clear memory
    for (int i = 0; i < n; i++) {
//        cout << connected[i] << " - " ;
        delete [] graph[i];
        delete [] graph_copy[i];
    }
    delete [] graph;
    delete [] graph_copy;
    
    return maps;
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

/// 切割tsp c生成的环
/// @param tsp tsp
/// @param b 需要l切割的路径长度。
vector<vector<int>> cutting_tsp(TSP *tsp, int b)
{
    // 先切割成多个联通分量。
    vector<vector<int>> connected_completeds;
    vector<int> router;
    int length = 0;
    for (vector<int>::iterator it = tsp->circuit.begin(); it != tsp->circuit.end(); ++it) {
        router.push_back(*it);
        if (*it == tsp->circuit.back()) {
            connected_completeds.push_back(router);
//            length += tsp->graph[tsp->circuit.front()][tsp->circuit.back()];
//            if (length > b) {
//                // 如果当前长度已经是大于 b 的,就把当前的路径加到连通图里
//                connected_completeds.push_back(router);
//            } else  if (connected_completeds.size() > 0) {
//                // 如果最后和最开始的两个点的距离是小于b 的,就添加到最开始的那个路径里面。
//                vector<int> firstConnect = connected_completeds.front();
//                firstConnect.insert(firstConnect.end(), router.begin(), router.end());
//                connected_completeds.erase(connected_completeds.begin());
//                connected_completeds.insert(connected_completeds.begin(), firstConnect);
//                cout<<endl;
//            } else {
//                // 如果 目前还没有切割好的连通图，就将整个加入连通图里。
//                connected_completeds.push_back(router);
//            }
            break;
        }
        length += tsp->graph[*it][*(it+1)];
        if (length > b) {
            // 如果当前节点到下一个节点的距离是大于b 的，就添加到连通图里。并创建新的路径。
            connected_completeds.push_back(router);
            router = vector<int>();
            length = 0;
        }
    }
    return connected_completeds;
}

/// 切割tsp c生成的环
/// @param tsp tsp
/// @param b 需要l切割的路径长度。
vector<vector<int>> __cutting_tsp(TSP tsp, int b)
{
    // 先切割成多个联通分量。
    vector<vector<int>> connected_completeds;
    vector<int> router;
    for (vector<int>::iterator it = tsp.circuit.begin(); it != tsp.circuit.end(); ++it) {
        router.push_back(*it);
        if (*it == tsp.circuit.back()) {
            // 如果已经循环到最后了
            if (tsp.graph[*it][tsp.circuit.front()] > b) {
                // 如果最后和最开始的两个点之间的距离也是大于 b 的,就把当前的路径加到连通图里
                connected_completeds.push_back(router);
            } else  if (connected_completeds.size() > 0) {
                // 如果最后和最开始的两个点的距离是小于b 的,就添加到最开始的那个路径里面。
                vector<int> firstConnect = connected_completeds.front();
                firstConnect.insert(firstConnect.end(), router.begin(), router.end());
                connected_completeds.erase(connected_completeds.begin());
                connected_completeds.insert(connected_completeds.begin(), firstConnect);
                cout<<endl;
            } else {
                // 如果 目前还没有切割好的连通图，就将整个加入连通图里。
                connected_completeds.push_back(router);
            }
            break;
        }
        if (tsp.graph[*it][*(it+1)] > b) {
            // 如果当前节点到下一个节点的距离是大于b 的，就添加到连通图里。并创建新的路径。
            connected_completeds.push_back(router);
            router = vector<int>();
        }
    }
    return connected_completeds;
}

////////////////////////////////////////////////////////////

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
        permutation(arr, i, result, &aa, n);
    }
    
    for (vector<vector<vector<int>>>::iterator it=aa.begin(); it != aa.end(); it++) {
//        print_reslut(*it, n);
    }
    
    return 0;
}

void run_algorithm(int **answer, int n)
{
    
    srand(n+1);
//    vector<TSP::City>cities;
//    int **maps = new int*[NODE_NUMBER];
//    for (int i = 0; i < NODE_NUMBER; ++i) {
//        maps[i] = new int[NODE_NUMBER];
//        for (int j = 0; j < NODE_NUMBER; j++) {
//            maps[i][j] = 0;
//        }
//        int x = i / 4 * 300 + (i - i / 4 * 4) / 2 * 100;
//        int y = i / 4 * 300 + (i - i / 4 * 4) % 2 * 100;
//        int hover_time = rand() % (MAX_HOVER_TIME - MIN_HOVER_TIME - 1) + MIN_HOVER_TIME;
//        struct TSP::City c = {i,x,y,0};
//        cout <<  i << "节点的信息：" <<  x << ","<<  y << ","<< hover_time << endl;
//        cities.push_back(c);
//    }
    
    
    vector<TSP::City>cities;
    for (int i = 0; i < NODE_NUMBER; ++i) {
        int x = rand() % GRAPH_RANGE;
        int y = rand() % GRAPH_RANGE;
        int hover_time = rand() % (MAX_HOVER_TIME - MIN_HOVER_TIME - 1) + MIN_HOVER_TIME;
        struct TSP::City c = {i,x,y,hover_time};
        cities.push_back(c);
    }
    
    TSP tsp(cities);
    tsp.solution();
    
    answer[n][0] = algorithm(&tsp);
    cout << "对照算法：" <<endl;
    answer[n][1] = algorithm1(&tsp);
    cout << "\n第三个算法:" <<endl;
    answer[n][2] = algorithm2(&tsp);
    cout << "\n修改过后的对比算法：" <<endl;
    answer[n][3] = algorithm3(&tsp);
    cout << "\n修改过后的第三个算法: " <<endl;
    answer[n][4] = algorithm4(&tsp);
    cout <<"\n修改过后Rootless算法: " << endl;
    answer[n][5] = algorithm5(&tsp);
//
    cout << "循环第"<<n+1<<"次："<<endl;
    cout << "Rootless算法最优值: " << answer[n][0] << endl
    << "修改过后Rootless算法:" << answer[n][5] << endl
    << "对照算法最优值: " << answer[n][1] << endl
    << "修改过后的对比算法：" << answer[n][3] << endl
    << "第三个算法: " << answer[n][2] << endl
    << "修改过后的第三个算法: " << answer[n][4] << endl
         << endl;
    
}

int main(int argc, char** argv) {
    // Check that user entered filename on command line
    if (argc < 2)
    {
        usage();
        exit(-1);
    }
    
    int **answer = new int *[LOOP_TIME];
    for (int i = 0; i < LOOP_TIME; i++) {
        answer[i] = new int [10];
        run_algorithm(answer, i);
    }
    
    for (int i = 1; i < LOOP_TIME; i++) {
        for (int j = 0; j < 7; j++) {
            answer[j][0] = answer[j][0] + answer[j][i];
        }
    }
    
    cout << "\n平均值：  " << endl;
    cout << "Rootless算法最优值: " << answer[0][0] << endl
    << "修改过后Rootless算法:" << answer[0][5] << endl
    << "对照算法最优值: " << answer[0][1] << endl
    << "修改过后的对比算法：" << answer[0][3] << endl
    << "第三个算法: " << answer[0][2] << endl
    << "修改过后的第三个算法: " << answer[0][4] << endl
         << endl;
    
    return 0;
}
