#include "ClarkWright.h"
#include <iomanip>
#include <vector>
#include <algorithm>
#include <map>
#include <cmath>
#include <tuple>
#include <iostream>

using namespace std;

void ClarkAndWrightFunc(CVRP& cvrp, double alpha){
    int n = cvrp.dimension;
    
    // Sử dụng ma trận toàn cục dis đã được khởi tạo

    vector<vector<int>> route(n + 1);
    vector<Saving> saving;
    vector<int> nextnode(n + 1, 0), prenode(n + 1, 0), cycle(n + 1, 0);
    int numroute = n - 1;
    int dep = cvrp.depot;
    double ans = 0;
    vector<int> load(n + 1, 0);
    
    // Tinh gia tri toi uu giua cac cap diem
    for(int i = 1;i <= n;i++){
        for(int j = 1;j <= n;j++){
            if(i == j) continue;
            if (i == cvrp.depot || j == cvrp.depot) continue;
            Saving a;
            a.st = i;
            a.en = j;
            a.sav = dis[dep][j] + dis[i][dep] - alpha * dis[i][j];
            saving.push_back(a);
        }
    }

    //Sap xep saving theo thu tu giam dan
    sort(saving.begin(), saving.end(), cp);
    
    //Khoi tao cac tuyen ban dau
    for(int i = 1;i < route.size();i++){
        if(i == dep) continue;
        route[i].push_back(dep);
        route[i].push_back(i);
        route[i].push_back(dep);
    }

    //Gan cac chu trinh av trong tai cho tung tuyen
    for(int i = 1;i <= n;i++){
        cycle[i] = i;
        load[i]    = (i == cvrp.depot ? 0 : cvrp.demands.at(i));
        nextnode[i] = prenode[i] = dep;
    }

    //Clark and Wright
    int st, en;
    double sav;
    for(int i = 0;i < saving.size();i++){
        st = saving[i].st;
        en = saving[i].en;
        sav = saving[i].sav;
        // kiem tra dieu kien
        if(nextnode[st] == dep && prenode[en] == dep && cycle[st] != cycle[en] && load[cycle[st]] + load[cycle[en]] <= cvrp.capacity && sav > 0){
            load[cycle[st]] += load[cycle[en]];
            load[cycle[en]] = 0;
            nextnode[st] = en;
            prenode[en] = st;
            numroute--;
            route[cycle[st]].pop_back();
            for(int j = 1;j < route[cycle[en]].size();j++){
                route[cycle[st]].push_back(route[cycle[en]][j]);
            }
            route[cycle[en]].clear();
            int x = cycle[en];
            for(int j = 1;j <= n;j++){
                if(cycle[j] == x) cycle[j] = cycle[st];
            }
        }
        if(numroute == cvrp.trucks) break;
    }

    //Lay ket qua chu trinh tu phuong phap Clark and Wright
    for(int i = 2;i < route.size();i++){
        if(route[i].size() == 0) continue;
        for(int j = 1;j < route[i].size();j++){
            ans += dis[route[i][j - 1]][route[i][j]];
        }
        cvrp.route.push_back(route[i]);
    }
    cvrp.ClarkAndWrightoutput = ans;

    //2-opt
    cvrp.route_2opt = cvrp.route;
    cvrp.ClarkAndWright_2opt = cvrp.ClarkAndWrightoutput;
    map<pair<int, int>, double> dis_2opt;
    double s1, s2, s3, s4, cur;
    for(int i = 0;i < cvrp.route_2opt.size();i++){
        do{
            dis_2opt.clear();
            for(int j = 1;j < cvrp.route_2opt[i].size() - 1;j++){
                for(int k = j + 1;k < cvrp.route_2opt[i].size() - 1;k++){
                    s1 = dis[cvrp.route_2opt[i][j - 1]][cvrp.route_2opt[i][j]];
                    s2 = dis[cvrp.route_2opt[i][k]][cvrp.route_2opt[i][k + 1]];
                    s3 = dis[cvrp.route_2opt[i][j - 1]][cvrp.route_2opt[i][k]];
                    s4 = dis[cvrp.route_2opt[i][j]][cvrp.route_2opt[i][k + 1]];
                    if(s1 + s2 > s3 + s4){
                        dis_2opt[{j, k}] = s1 + s2 - s3 - s4;
                    }
                }
            }
            st = en = 0;
            cur = 0;
            for(auto it : dis_2opt){
                if(it.second > cur){
                    cur = it.second;
                    st = it.first.first;
                    en = it.first.second;
                }
            }
            if(cur > 0){
                reverse(cvrp.route_2opt[i].begin() + st, cvrp.route_2opt[i].begin() + en + 1);
                cvrp.ClarkAndWright_2opt -= cur;
            }
        }while(dis_2opt.size() > 0);
    }

    //3-opt 
    cvrp.route_3opt = cvrp.route_2opt;
    cvrp.ClarkAndWright_3opt = cvrp.ClarkAndWright_2opt;
    map<tuple<int, int, int>, pair<int, double>> dis_3opt;
    int type;
    double ne, pre, epsilon = 0.0001;
    for(int i = 0;i < cvrp.route_3opt.size();i++){
        do{
            dis_3opt.clear();
            for(int j = 0;j < cvrp.route_3opt[i].size() - 1;j++){
                for(int k = j + 1;k < cvrp.route_3opt[i].size() - 1;k++){
                    for(int l = k + 1;l < cvrp.route_3opt[i].size() - 1;l++){
                        pre = dis[cvrp.route_3opt[i][j]][cvrp.route_3opt[i][j + 1]] + dis[cvrp.route_3opt[i][k]][cvrp.route_3opt[i][k + 1]] + dis[cvrp.route_3opt[i][l]][cvrp.route_3opt[i][l + 1]];
                        cur = 0;
                        type = 0;

                        ne = dis[cvrp.route_3opt[i][j]][cvrp.route_3opt[i][k]] + dis[cvrp.route_3opt[i][j + 1]][cvrp.route_3opt[i][k + 1]] + dis[cvrp.route_3opt[i][l]][cvrp.route_3opt[i][l + 1]];
                        if(ne < pre && cur < pre - ne){
                            cur = pre - ne;
                            type = 1;
                        }
                        
                        ne = dis[cvrp.route_3opt[i][k]][cvrp.route_3opt[i][l]] + dis[cvrp.route_3opt[i][k + 1]][cvrp.route_3opt[i][l + 1]] + dis[cvrp.route_3opt[i][j]][cvrp.route_3opt[i][j + 1]];
                        if(ne < pre && cur < pre - ne){
                            cur = pre - ne;
                            type = 2;
                        }

                        ne = dis[cvrp.route_3opt[i][j]][cvrp.route_3opt[i][k]] + dis[cvrp.route_3opt[i][j + 1]][cvrp.route_3opt[i][l]] + dis[cvrp.route_3opt[i][k + 1]][cvrp.route_3opt[i][l + 1]];
                        if(ne < pre && cur < pre - ne){
                            cur = pre - ne;
                            type = 3;
                        }

                        ne = dis[cvrp.route_3opt[i][j]][cvrp.route_3opt[i][k + 1]] + dis[cvrp.route_3opt[i][l]][cvrp.route_3opt[i][j + 1]] + dis[cvrp.route_3opt[i][k]][cvrp.route_3opt[i][l + 1]];
                        if(ne < pre && cur < pre - ne){
                            cur = pre - ne;
                            type = 4;
                        }

                        ne = dis[cvrp.route_3opt[i][j]][cvrp.route_3opt[i][l]] + dis[cvrp.route_3opt[i][k + 1]][cvrp.route_3opt[i][j + 1]] + dis[cvrp.route_3opt[i][k]][cvrp.route_3opt[i][l + 1]];
                        if(ne < pre && cur < pre - ne){
                            cur = pre - ne;
                            type = 5;
                        }

                        ne = dis[cvrp.route_3opt[i][j]][cvrp.route_3opt[i][k + 1]] + dis[cvrp.route_3opt[i][l]][cvrp.route_3opt[i][k]] + dis[cvrp.route_3opt[i][j + 1]][cvrp.route_3opt[i][l + 1]];
                        if(ne < pre && cur < pre - ne){
                            cur = pre - ne;
                            type = 6;
                        }

                        ne = dis[cvrp.route_3opt[i][j]][cvrp.route_3opt[i][l]] + dis[cvrp.route_3opt[i][k + 1]][cvrp.route_3opt[i][k]] + dis[cvrp.route_3opt[i][j + 1]][cvrp.route_3opt[i][l + 1]];
                        if(ne < pre && cur < pre - ne){
                            cur = pre - ne;
                            type = 7;
                        }
                        
                        if(cur > epsilon){
                            dis_3opt[{j, k, l}] = {type, cur};
                        }
                    }
                }
            }
            cur = 0;
            type = 0;
            int x = 0, y = 0, z = 0;
            for(auto it : dis_3opt){
                if(it.second.second > cur){
                    cur = it.second.second;
                    x = get<0>(it.first);
                    y = get<1>(it.first);
                    z = get<2>(it.first);
                    type = it.second.first;
                }
            }
            
            if(cur == 0) break;

            if(type == 1){
                reverse(cvrp.route_3opt[i].begin() + x + 1, cvrp.route_3opt[i].begin() + y + 1);
            }
            else if(type == 2){
                reverse(cvrp.route_3opt[i].begin() + y + 1, cvrp.route_3opt[i].begin() + z + 1);
            }
            else if(type == 3){
                reverse(cvrp.route_3opt[i].begin() + x + 1, cvrp.route_3opt[i].begin() + y + 1);
                reverse(cvrp.route_3opt[i].begin() + y + 1, cvrp.route_3opt[i].begin() + z + 1);
            }
            else if(type == 4){
                reverse(cvrp.route_3opt[i].begin() + x + 1, cvrp.route_3opt[i].begin() + z + 1);
                reverse(cvrp.route_3opt[i].begin() + x + 1, cvrp.route_3opt[i].begin() + x + 1 + z - y);
                reverse(cvrp.route_3opt[i].begin() + x + 1 + z - y, cvrp.route_3opt[i].begin() + z + 1);
            }
            else if(type == 5){
                reverse(cvrp.route_3opt[i].begin() + x + 1, cvrp.route_3opt[i].begin() + z + 1);
                reverse(cvrp.route_3opt[i].begin() + x + 1 + z - y, cvrp.route_3opt[i].begin() + z + 1);
            }
            else if(type == 6){
                reverse(cvrp.route_3opt[i].begin() + x + 1, cvrp.route_3opt[i].begin() + z + 1);
                reverse(cvrp.route_3opt[i].begin() + x + 1, cvrp.route_3opt[i].begin() + x + 1 + z - y);
            }
            else if(type == 7){
                reverse(cvrp.route_3opt[i].begin() + x + 1, cvrp.route_3opt[i].begin() + z + 1);
            }

            cvrp.ClarkAndWright_3opt -= cur;
        }while(dis_3opt.size() > 0);
    }

    return;
}