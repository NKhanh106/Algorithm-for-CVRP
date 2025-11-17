#include "CVRP.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>

using namespace std;

// 1. ĐỊNH NGHĨA BIẾN TOÀN CỤC (Giải quyết lỗi undefined reference cho dist_matrix và KetQua)
double dis[101][101]; 
vector<CVRP> KetQua;

// 2. ĐỊNH NGHĨA HÀM TIỆN ÍCH (Giải quyết lỗi undefined reference cho cp và KhoangCach)
double KhoangCach(Toado a, Toado b){
    return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

bool cp(Saving a, Saving b){
    return (a.sav == b.sav) ? ((a.st == b.st) ? (a.en > b.en) : (a.st > b.st)) : (a.sav > b.sav);
}

void InitializeDistanceMatrix(CVRP& cvrp) {
    int n = cvrp.dimension;
    for (int i = 1; i <= n; ++i) {
        for (int j = 1; j <= n; ++j) {
            dis[i][j] = KhoangCach(cvrp.nodes[i], cvrp.nodes[j]);
        }
    }
}

// 3. ĐỊNH NGHĨA HÀM I/O (Giải quyết lỗi undefined reference cho readFile)
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
    
    // Khởi tạo ma trận khoảng cách sau khi đọc xong
    InitializeDistanceMatrix(cvrp);
    
    return cvrp;
}

void write_solution_file(const CVRP& ins, const string& output_folder) {
    string filename = output_folder + "/" + ins.name + ".sol";
    ofstream outfile(filename);
    outfile << fixed << setprecision(4);

    // --- Thông tin chung ---
    outfile << "Name: " << ins.name << "\n";
    outfile << "Dimension: " << ins.dimension << "\n";
    outfile << "Trucks: " << ins.trucks << "\n";
    outfile << "Capacity: " << ins.capacity << "\n";
    outfile << "Optimal: " << ins.optimal << "\n";

    // --- Kết quả GA (Các trường ga_* đã được cập nhật trong GeneticAlgorithm) ---
    outfile << "GA Best Fitness: " << ins.ga_best_fitness << "\n";
    outfile << "GA Vehicles Used: " << ins.ga_best_routes.size() << "\n";
    outfile << "GA Generations Run: " << ins.ga_generations_run << "\n";
    outfile << "GA Last Improve Gen: " << ins.ga_last_improve_gen << "\n";
    outfile << "GA Early Stopped: " << (ins.ga_early_stopped ? "Yes" : "No") << "\n";
    outfile << "GA Patience (gens): " << ins.ga_patience_used << "\n";
    outfile << "GA Elapsed (ms): " << ins.ga_elapsed_ms << "\n";

    // --- Chi tiết Tuyến đường GA Tốt nhất ---
    outfile << "\nRoute GA Final:\n";
    for (size_t j = 0; j < ins.ga_best_routes.size(); ++j) {
        outfile << "Route #" << j + 1 << ": ";
        const auto& r = ins.ga_best_routes[j];
        if (!r.empty()) {
            outfile << r[0];
            for (size_t k = 1; k < r.size(); ++k) outfile << " --> " << r[k];
        }
        outfile << "\n";
    }

    outfile.close();
    cout << "Wrote: " << filename << "\n";
}