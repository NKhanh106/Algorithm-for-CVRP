#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <iomanip>
#include <chrono>
#include "ga/GA_Core.h"
#include "es/ES_Core.h"
#include "constructive/ClarkWright.h"

using namespace std;

void run_ga_and_output(CVRP& cvrp) {
    const int POPULATION_SIZE = 50;
    const int MAX_GENERATIONS = 500;
    const int PATIENCE = 150;

    cout << "Starting GA for " << cvrp.name << "\n";

    auto start_time = chrono::high_resolution_clock::now();
    GeneticAlgorithm(cvrp, POPULATION_SIZE, MAX_GENERATIONS, PATIENCE);
    auto end_time = chrono::high_resolution_clock::now();
    cvrp.ga_elapsed_ms = chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count();

    cout << "GA Finished. Final Best Fitness: " << fixed << setprecision(4) << cvrp.ga_best_fitness << "\n";
}

void run_es_and_output(CVRP& cvrp) {
    const int MU = 30;
    const int LAMBDA = 120;
    const int MAX_GENERATIONS = 400;
    const int PATIENCE = 120;
    const bool PLUS_SELECTION = true;

    cout << "Starting ES for " << cvrp.name << "\n";

    auto start_time = chrono::high_resolution_clock::now();
    EvolutionStrategy(cvrp, MU, LAMBDA, MAX_GENERATIONS, PATIENCE, PLUS_SELECTION);
    auto end_time = chrono::high_resolution_clock::now();
    cvrp.es_elapsed_ms = chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count();

    cout << "ES Finished. Final Best Fitness: " << fixed << setprecision(4) << cvrp.es_best_fitness << "\n";
}

int main() {
    string folder = "./Vrp-Set-A-input";
    string output = "./Vrp-Set-A-output";

    if (!filesystem::exists(output)) {
        filesystem::create_directory(output);
    }

    for (const auto& entry : filesystem::directory_iterator(folder)) {
        if (entry.path().extension() == ".vrp") {
            string filename = entry.path().string();
            cout << "\nProcessing file: " << filename << "\n";

            CVRP cvrp = readFile(filename);

            run_ga_and_output(cvrp);
            run_es_and_output(cvrp);
            
            //Ghi file ket qua
            write_solution_file(cvrp, output);

            // tóm tắt nhanh sau khi chạy xong file này
            cout << "Done: " << cvrp.name
                 << " | GA_Best=" << cvrp.ga_best_fitness
                 << " | ES_Best=" << cvrp.es_best_fitness
                 << " | GA_Routes=" << cvrp.ga_best_routes.size()
                 << " | ES_Routes=" << cvrp.es_best_routes.size()
                 << " | GA_Gens=" << cvrp.ga_generations_run
                 << " | ES_Gens=" << cvrp.es_generations_run
                 << " | GA_EarlyStop=" << (cvrp.ga_early_stopped ? "Yes" : "No")
                 << " | ES_EarlyStop=" << (cvrp.es_early_stopped ? "Yes" : "No")
                 << " | GA_Time(ms)=" << cvrp.ga_elapsed_ms
                 << " | ES_Time(ms)=" << cvrp.es_elapsed_ms
                 << "\n";
        }
    }

    cout << "\nAll input files processed. Results are in " << output << "\n";
    return 0;
}