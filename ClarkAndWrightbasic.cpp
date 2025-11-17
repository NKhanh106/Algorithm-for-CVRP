#include<iostream>
#include<fstream>
#include<string>
#include<vector>
#include<sstream>
#include<algorithm>
#include<map>
#include<cmath>
#include<filesystem>
#include<tuple>

using namespace std;

struct Toado{
    double x;
    double y;
};

struct CVRP{
    string name;
    string type;
    int optimal;
    int dimension;
    int trucks;
    string edge_weight_type;
    int capacity;
    vector<Toado> nodes;
    map<int, int> demands;
    int depot;
    double ClarkAndWrightoutput;
    double ClarkAndWright_2opt;
    double ClarkAndWright_3opt;
    vector<vector<int>> route;
    vector<vector<int>> route_2opt;
    vector<vector<int>> route_3opt;
};

struct Saving{
    int st;
    int en;
    double sav;
};

vector<CVRP> KetQua;

double KhoangCach(Toado a, Toado b){
    return sqrt((a.x - b.x) * (a.x - b .x) + (a.y - b.y) * (a.y - b.y));
}

bool cp(Saving a, Saving b){
    return (a.sav==b.sav)?((a.st==b.st)?((a.en==b.en)?(false):(a.en>b.en)):(a.st>b.st)):(a.sav>b.sav);
}

void ClarkAndWrightFunc(CVRP& cvrp){
    int n = cvrp.dimension;
    double dis[n + 1][n + 1] = {0};
    vector<vector<int>> route(n + 1);
    vector<Saving> saving;
    vector<int> nextnode(n + 1, 0), prenode(n + 1, 0), cycle(n + 1, 0);
    int numroute = n - 1;
    int dep = cvrp.depot;
    double ans = 0;
    vector<int> load(n + 1, 0);
    cout << fixed << setprecision(4);

    // Tinh ma tran khoang cach giua cac diem
    for(int i = 1;i <= n;i++){
        for(int j = 1;j <= n;j++){
            dis[i][j] = KhoangCach(cvrp.nodes[i], cvrp.nodes[j]);
        }
    }
    
    //Tinh gia tri toi uu giua cac cap diem
    for(int i = 1;i <= n;i++){
        for(int j = 1;j <= n;j++){
            if(i == j) continue;
            Saving a;
            a.st = i;
            a.en = j;
            a.sav = dis[dep][j] + dis[i][dep] - dis[i][j];
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
        load[i] = cvrp.demands[i];
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
    int type, dem;
    double ne, pre, epsilon = 0.0001;
    for(int i = 0;i < cvrp.route_3opt.size();i++){
        do{
            dis_3opt.clear();
            for(int j = 0;j < cvrp.route_3opt[i].size() - 1;j++){
                for(int k = j + 1;k < cvrp.route_3opt[i].size() - 1;k++){
                    for(int l = k + 1;l < cvrp.route_3opt[i].size() - 1;l++){
                        //cat chu trinh thanh 4 doan : depot -> j, j + 1 -> k, k + 1 -> l, l + 1 -> depot
                        pre = dis[cvrp.route_3opt[i][j]][cvrp.route_3opt[i][j + 1]] + dis[cvrp.route_3opt[i][k]][cvrp.route_3opt[i][k + 1]] + dis[cvrp.route_3opt[i][l]][cvrp.route_3opt[i][l + 1]];
                        cur = 0;
                        type = 0;

                        //depot -> j, k -> j + 1, k + 1 -> l, l + 1 -> depot : dao j + 1 -> k
                        ne = dis[cvrp.route_3opt[i][j]][cvrp.route_3opt[i][k]] + dis[cvrp.route_3opt[i][j + 1]][cvrp.route_3opt[i][k + 1]] + dis[cvrp.route_3opt[i][l]][cvrp.route_3opt[i][l + 1]];
                        if(ne < pre && cur < pre - ne){
                            cur = pre - ne;
                            type = 1;
                        }
                        
                        //depot -> j, j + 1 -> k, l -> k + 1, l + 1 -> depot : dao k + 1 -> l
                        ne = dis[cvrp.route_3opt[i][k]][cvrp.route_3opt[i][l]] + dis[cvrp.route_3opt[i][k + 1]][cvrp.route_3opt[i][l + 1]] + dis[cvrp.route_3opt[i][j]][cvrp.route_3opt[i][j + 1]];
                        if(ne < pre && cur < pre - ne){
                            cur = pre - ne;
                            type = 2;
                        }

                        //depot -> j, k -> j + 1, l -> k + 1, l + 1 -> depot : dao j + 1 -> k va k + 1 -> l
                        ne = dis[cvrp.route_3opt[i][j]][cvrp.route_3opt[i][k]] + dis[cvrp.route_3opt[i][j + 1]][cvrp.route_3opt[i][l]] + dis[cvrp.route_3opt[i][k + 1]][cvrp.route_3opt[i][l + 1]];
                        if(ne < pre && cur < pre - ne){
                            cur = pre - ne;
                            type = 3;
                        }

                        //depot -> j, k + 1 -> l, j + 1 -> k , l + 1 -> depot : dao vi tri j + 1 -> k va k + 1 -> l
                        ne = dis[cvrp.route_3opt[i][j]][cvrp.route_3opt[i][k + 1]] + dis[cvrp.route_3opt[i][l]][cvrp.route_3opt[i][j + 1]] + dis[cvrp.route_3opt[i][k]][cvrp.route_3opt[i][l + 1]];
                        if(ne < pre && cur < pre - ne){
                            cur = pre - ne;
                            type = 4;
                        }

                        //depot -> j, l -> k + 1, , j + 1 -> k,l + 1 -> depot : dao vi tri j + 1 -> k va k + 1 -> l va dao k + 1 -> l
                        ne = dis[cvrp.route_3opt[i][j]][cvrp.route_3opt[i][l]] + dis[cvrp.route_3opt[i][k + 1]][cvrp.route_3opt[i][j + 1]] + dis[cvrp.route_3opt[i][k]][cvrp.route_3opt[i][l + 1]];
                        if(ne < pre && cur < pre - ne){
                            cur = pre - ne;
                            type = 5;
                        }

                        //depot -> j, k + 1 -> l, k -> j + 1, l + 1 -> depot : dao vi tri j + 1 -> k va k + 1 -> l va dao j + 1 -> k
                        ne = dis[cvrp.route_3opt[i][j]][cvrp.route_3opt[i][k + 1]] + dis[cvrp.route_3opt[i][l]][cvrp.route_3opt[i][k]] + dis[cvrp.route_3opt[i][j + 1]][cvrp.route_3opt[i][l + 1]];
                        if(ne < pre && cur < pre - ne){
                            cur = pre - ne;
                            type = 6;
                        }

                        //depot -> j, l -> k + 1, k -> j + 1, l + 1 -> depot : dao vi tri j + 1 -> k va k + 1 -> l va dao ca 2
                        ne = dis[cvrp.route_3opt[i][j]][cvrp.route_3opt[i][l]] + dis[cvrp.route_3opt[i][k + 1]][cvrp.route_3opt[i][k]] + dis[cvrp.route_3opt[i][j + 1]][cvrp.route_3opt[i][l + 1]];
                        if(ne < pre && cur < pre - ne){
                            cur = pre - ne;
                            type = 7;
                        }
                       
                        //check xem co the cai thien khong
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
            //thuc hien sua lai chu trinh
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

CVRP readFile(const string& filename){
    CVRP cvrp;
    ifstream infile(filename);

    string line;
    while(getline(infile, line)){
        istringstream iss(line);
        string keyword, cur = "";
        char c = ':';
        iss >> keyword;
        
        if(keyword == "NAME"){
            iss >> c;
            iss >> cvrp.name;
        }
        else if(keyword == "COMMENT"){
            iss >> c;
            while(cur != "trucks:") iss >> cur;
            iss >> cur;
            cvrp.trucks = stoi(cur.substr(0, cur.size() - 1));
            while(cur != "value:") iss >> cur;
            iss >> cur;
            cvrp.optimal = stoi(cur.substr(0, cur.size() - 1));
        }
        else if(keyword == "TYPE"){
            iss >> c;
            iss >> cvrp.type;
        }
        else if(keyword == "DIMENSION"){
            iss >> c;
            iss >> cvrp.dimension;
        }
        else if(keyword == "EDGE_WEIGHT_TYPE"){
            iss >> c;
            iss >> cvrp.edge_weight_type;
        }
        else if(keyword == "CAPACITY"){
            iss >> c;
            iss >> cvrp.capacity;
        }
        else if(keyword == "NODE_COORD_SECTION"){
            cvrp.nodes.push_back({-1, -1});
            for(int i = 0, id;i < cvrp.dimension;i++){
                double x, y;
                getline(infile, line);
                istringstream iss1(line);
                iss1 >> id >> x >> y;
                cvrp.nodes.push_back({x, y});
            }
        }
        else if(keyword == "DEMAND_SECTION"){
            for(int i = 0, id, de;i < cvrp.dimension;i++){
                getline(infile, line);
                istringstream iss1(line);
                iss1 >> id >> de;
                cvrp.demands[id] = de;
            }
        }
        else if(keyword == "DEPOT_SECTION"){
            infile >> cvrp.depot;
            int end_marker;
            infile >> end_marker;
        }
        else if(keyword == "EOF") {
            break;
        }
    }

    infile.close();
    return cvrp;
}

int main(){
    string folder = "./Vrp-Set-A-input";
    for(const auto& entry : filesystem::directory_iterator(folder)){
        if(entry.path().extension() == ".vrp"){
            string filename = entry.path().string();
            cout << "Processing file: " << filename << "\n";
            CVRP cvrp = readFile(filename);
            ClarkAndWrightFunc(cvrp);
            KetQua.push_back(cvrp);
        }
    }

    string output = "./Vrp-Set-A-output";
    for(int i = 0;i < KetQua.size();i++){
        string filename = output + "/" + KetQua[i].name + ".sol";
        ofstream outfile(filename);
        outfile << fixed << setprecision(4);
        outfile << "Name: " << KetQua[i].name << "\n";
        outfile << "Dimension: " << KetQua[i].dimension << "\n";
        outfile << "Trucks: " << KetQua[i].trucks << "\n";
        outfile << "Capacity: " << KetQua[i].capacity << "\n";
        outfile << "Optimal: " << KetQua[i].optimal << "\n";
        outfile << "Clark and Wright answer: " << KetQua[i].ClarkAndWrightoutput << "\n";
        outfile << "Clark and Wright with 2-opt answer: " << KetQua[i].ClarkAndWright_2opt << "\n";
        outfile << "Clark and Wright with 3-opt answer: " << KetQua[i].ClarkAndWright_3opt << "\n";
        outfile << "\n";
        outfile << "Route Clark and Wright: \n";
        for(int j = 0;j < KetQua[i].route.size();j++){
            outfile << "Route " << j + 1 << ": " << KetQua[i].route[j][0];
            for(int k = 1;k < KetQua[i].route[j].size();k++){
                outfile  << " --> " << KetQua[i].route[j][k];
            }
            outfile << "\n";
        }
        outfile << "\n";
        outfile << "Route Clark and Wright with 2-opt: \n";
        for(int j = 0;j < KetQua[i].route_2opt.size();j++){
            outfile << "Route " << j + 1 << ": " << KetQua[i].route_2opt[j][0];
            for(int k = 1;k < KetQua[i].route_2opt[j].size();k++){
                outfile << " --> " << KetQua[i].route_2opt[j][k];
            }
            outfile << "\n";
        }
        outfile << "Impove: "  << (KetQua[i].ClarkAndWrightoutput - KetQua[i].ClarkAndWright_2opt) * 100 / KetQua[i].ClarkAndWrightoutput << "% \n\n";
        outfile << "Route Clark and Wright with 3-opt: \n";
        for(int j = 0;j < KetQua[i].route_3opt.size();j++){
            outfile << "Route " << j + 1 << ": " << KetQua[i].route_3opt[j][0];
            for(int k = 1;k < KetQua[i].route_3opt[j].size();k++){
                outfile << " --> " << KetQua[i].route_3opt[j][k];
            }
            outfile << "\n";
        }
        outfile << "Impove: "  << (KetQua[i].ClarkAndWrightoutput - KetQua[i].ClarkAndWright_3opt) * 100 / KetQua[i].ClarkAndWrightoutput << "% \n";
        outfile.close();
    }
}
