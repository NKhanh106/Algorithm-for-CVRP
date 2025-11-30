#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <iomanip>
#include <chrono>
#include "ga/GA_Core.h"
#include "es/ES_Core.h"
#include "ep/EP_Core.h"
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

void run_ep_and_output(CVRP& cvrp) {
    const int POPULATION_SIZE = 50;
    const int MAX_GENERATIONS = 500;
    const int PATIENCE = 150;
    const int TOURNAMENT_SIZE = 3;

    cout << "Starting EP for " << cvrp.name << "\n";

    auto start_time = chrono::high_resolution_clock::now();
    EvolutionaryProgramming(cvrp, POPULATION_SIZE, MAX_GENERATIONS, PATIENCE, TOURNAMENT_SIZE);
    auto end_time = chrono::high_resolution_clock::now();
    cvrp.ep_elapsed_ms = chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count();

    cout << "EP Finished. Final Best Fitness: " << fixed << setprecision(4) << cvrp.ep_best_fitness << "\n";
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
            run_ep_and_output(cvrp);
            
            //Ghi file ket qua
            write_solution_file(cvrp, output);

            // So sánh kết quả 3 thuật toán
            cout << "\n========== Comparison for " << cvrp.name << " ==========\n";
            cout << "GA: Fitness=" << fixed << setprecision(4) << cvrp.ga_best_fitness
                 << " | Routes=" << cvrp.ga_best_routes.size()
                 << " | Gens=" << cvrp.ga_generations_run
                 << " | Time(ms)=" << cvrp.ga_elapsed_ms << "\n";
            cout << "ES: Fitness=" << cvrp.es_best_fitness
                 << " | Routes=" << cvrp.es_best_routes.size()
                 << " | Gens=" << cvrp.es_generations_run
                 << " | Time(ms)=" << cvrp.es_elapsed_ms << "\n";
            cout << "EP: Fitness=" << cvrp.ep_best_fitness
                 << " | Routes=" << cvrp.ep_best_routes.size()
                 << " | Gens=" << cvrp.ep_generations_run
                 << " | Time(ms)=" << cvrp.ep_elapsed_ms << "\n";
            
            // Tìm best method
            double best_fitness = min({cvrp.ga_best_fitness, cvrp.es_best_fitness, cvrp.ep_best_fitness});
            string best_method = "";
            if (abs(cvrp.ga_best_fitness - best_fitness) < 1e-9) best_method = "GA";
            if (abs(cvrp.es_best_fitness - best_fitness) < 1e-9) best_method += (best_method.empty() ? "" : "/") + string("ES");
            if (abs(cvrp.ep_best_fitness - best_fitness) < 1e-9) best_method += (best_method.empty() ? "" : "/") + string("EP");
            cout << "Best Method(s): " << best_method << " with fitness " << best_fitness << "\n";
            cout << "Optimal ans: " << cvrp.optimal << "\n";
            cout << "==========================================\n\n";
        }
    }

    cout << "\nAll input files processed. Results are in " << output << "\n";
    return 0;
}