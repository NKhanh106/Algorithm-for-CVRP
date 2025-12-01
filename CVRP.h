#ifndef CVRP_H
#define CVRP_H

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <tuple>
#include <limits>

using namespace std;

struct Toado{
    double x;
    double y;
};

struct Saving{
    int st;
    int en;
    double sav;
};

struct CVRP{
    string name;
    string type;
    int optimal;
    int dimension;
    int trucks;
    int max_vehicles = 1000000;
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

    double ga_best_fitness = numeric_limits<double>::infinity();
    vector<vector<int>> ga_best_routes;

    int ga_generations_run = 0;
    int ga_last_improve_gen = 0;
    int ga_patience_used = 0;
    bool ga_early_stopped = false;
    long long ga_elapsed_ms = 0;

    double es_best_fitness = numeric_limits<double>::infinity();
    vector<vector<int>> es_best_routes;
    int es_generations_run = 0;
    int es_last_improve_gen = 0;
    int es_patience_used = 0;
    bool es_early_stopped = false;
    long long es_elapsed_ms = 0;

    double ep_best_fitness = numeric_limits<double>::infinity();
    vector<vector<int>> ep_best_routes;
    int ep_generations_run = 0;
    int ep_last_improve_gen = 0;
    int ep_patience_used = 0;
    bool ep_early_stopped = false;
    long long ep_elapsed_ms = 0;

    double aco_best_fitness = numeric_limits<double>::infinity();
    vector<vector<int>> aco_best_routes;
    int aco_iterations_run = 0;
    int aco_last_improve_iter = 0;
    int aco_patience_used = 0;
    bool aco_early_stopped = false;
    long long aco_elapsed_ms = 0;
};


//GA
struct Individual {
    vector<int> customer_sequence; 
    double fitness;                 
    vector<vector<int>> routes;    
};

extern double dis[101][101];

extern vector<CVRP> KetQua;

double KhoangCach(Toado a, Toado b);

bool cp(Saving a, Saving b);

CVRP readFile(const string& filename);

void write_solution_file(const CVRP& ins, const string& output_folder);

void InitializeDistanceMatrix(CVRP& cvrp);

#endif