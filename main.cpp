#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <iomanip>
#include <chrono>
#include "ga/GA_Core.h"
#include "constructive/ClarkWright.h"

using namespace std;

void run_ga_and_output(CVRP& cvrp) {
    const int POPULATION_SIZE = 50;
    const int MAX_GENERATIONS = 500;
    const int PATIENCE = 150;

    cout << "Starting GA for " << cvrp.name << "\n";
    
    GeneticAlgorithm(cvrp, POPULATION_SIZE, MAX_GENERATIONS, PATIENCE);
    
    cout << "GA Finished. Final Best Fitness: " << fixed << setprecision(4) << cvrp.ga_best_fitness << "\n";
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

            //Start tgian
            auto start_time = chrono::high_resolution_clock::now();

            CVRP cvrp = readFile(filename);

            // chạy GA (bên trong sẽ in "Gen i | Best=... | GlobalBest=... | Stale=...")
            run_ga_and_output(cvrp);

            //End tgian
            auto end_time = chrono::high_resolution_clock::now();

            //Tinh tgian
            auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time);
            cvrp.ga_elapsed_ms = duration.count();
            
            //Ghi file ket qua
            write_solution_file(cvrp, output);

            // tóm tắt nhanh sau khi chạy xong file này
            cout << "Done: " << cvrp.name
                 << " | GA_Best=" << cvrp.ga_best_fitness
                 << " | Routes=" << cvrp.ga_best_routes.size()
                 << " | Gens=" << cvrp.ga_generations_run
                 << " | EarlyStop=" << (cvrp.ga_early_stopped ? "Yes" : "No")
                 << " | Time(ms)=" << cvrp.ga_elapsed_ms
                 << "\n";
        }
    }

    cout << "\nAll input files processed. Results are in " << output << "\n";
    return 0;
}