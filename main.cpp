#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <iomanip>
#include <chrono>
#include "ga/GA_Core.h"
#include "es/ES_Core.h"
#include "ep/EP_Core.h"
#include "aco/ACO_Core.h"
#include "constructive/ClarkWright.h"

using namespace std;

void run_ga_and_output(CVRP& cvrp) {
    // Adaptive parameters based on problem size
    int dimension = cvrp.dimension;
    int POPULATION_SIZE = max(30, min(100, static_cast<int>(dimension * 2)));  // 30-100, scale with dimension
    int MAX_GENERATIONS = max(300, min(800, static_cast<int>(dimension * 15))); // 300-800, more for larger problems
    int PATIENCE = max(100, min(200, MAX_GENERATIONS / 3));  // 1/3 of max generations

    cout << "Starting GA for " << cvrp.name 
         << " (Pop=" << POPULATION_SIZE << ", MaxGen=" << MAX_GENERATIONS 
         << ", Patience=" << PATIENCE << ")\n";

    auto start_time = chrono::high_resolution_clock::now();
    GeneticAlgorithm(cvrp, POPULATION_SIZE, MAX_GENERATIONS, PATIENCE);
    auto end_time = chrono::high_resolution_clock::now();
    cvrp.ga_elapsed_ms = chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count();

    cout << "GA Finished. Final Best Fitness: " << fixed << setprecision(4) << cvrp.ga_best_fitness << "\n";
}

void run_es_and_output(CVRP& cvrp) {
    // ES parameters: (μ + λ) with λ = 4*μ (common ratio)
    int dimension = cvrp.dimension;
    int MU = max(20, min(50, static_cast<int>(dimension * 1.5)));  // 20-50 parents
    int LAMBDA = MU * 4;  // 4:1 ratio (offspring:parents)
    int MAX_GENERATIONS = max(250, min(600, static_cast<int>(dimension * 12)));
    int PATIENCE = max(80, min(150, MAX_GENERATIONS / 3));
    const bool PLUS_SELECTION = true;  // (μ + λ) selection

    cout << "Starting ES for " << cvrp.name 
         << " (μ=" << MU << ", λ=" << LAMBDA << ", MaxGen=" << MAX_GENERATIONS 
         << ", Patience=" << PATIENCE << ")\n";

    auto start_time = chrono::high_resolution_clock::now();
    EvolutionStrategy(cvrp, MU, LAMBDA, MAX_GENERATIONS, PATIENCE, PLUS_SELECTION);
    auto end_time = chrono::high_resolution_clock::now();
    cvrp.es_elapsed_ms = chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count();

    cout << "ES Finished. Final Best Fitness: " << fixed << setprecision(4) << cvrp.es_best_fitness << "\n";
}

void run_ep_and_output(CVRP& cvrp) {
    // EP parameters: similar to GA but with tournament selection
    int dimension = cvrp.dimension;
    int POPULATION_SIZE = max(30, min(80, static_cast<int>(dimension * 1.8)));  // Slightly smaller than GA
    int MAX_GENERATIONS = max(300, min(700, static_cast<int>(dimension * 14)));
    int PATIENCE = max(100, min(180, MAX_GENERATIONS / 3));
    int TOURNAMENT_SIZE = 3;  // Standard tournament size

    cout << "Starting EP for " << cvrp.name 
         << " (Pop=" << POPULATION_SIZE << ", MaxGen=" << MAX_GENERATIONS 
         << ", Patience=" << PATIENCE << ", Tournament=" << TOURNAMENT_SIZE << ")\n";

    auto start_time = chrono::high_resolution_clock::now();
    EvolutionaryProgramming(cvrp, POPULATION_SIZE, MAX_GENERATIONS, PATIENCE, TOURNAMENT_SIZE);
    auto end_time = chrono::high_resolution_clock::now();
    cvrp.ep_elapsed_ms = chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count();

    cout << "EP Finished. Final Best Fitness: " << fixed << setprecision(4) << cvrp.ep_best_fitness << "\n";
}

void run_aco_and_output(CVRP& cvrp) {
    // ACO parameters optimized based on literature
    int dimension = cvrp.dimension;
    int NUM_ANTS = max(25, min(50, static_cast<int>(dimension * 1.2)));  // 25-50 ants
    int MAX_ITERATIONS = max(150, min(400, static_cast<int>(dimension * 8)));
    int PATIENCE = max(40, min(80, MAX_ITERATIONS / 4));
    
    // Optimized ACO parameters (from literature)
    const double ALPHA = 1.0;      // Pheromone importance (standard value)
    const double BETA = 3.0;        // Heuristic importance (increased for better exploitation)
    const double RHO = 0.05;        // Evaporation rate (lower = slower evaporation, better exploration)
    const double Q = 100.0;         // Pheromone deposit constant
    // TAU0 calculated based on nearest neighbor heuristic
    double nn_estimate = 0.0;
    if (dimension > 1) {
        // Rough estimate: average distance * dimension
        nn_estimate = dimension * 10.0;  // Rough approximation
    }
    const double TAU0 = (nn_estimate > 0) ? (1.0 / (dimension * nn_estimate)) : 0.1;

    cout << "Starting ACO for " << cvrp.name 
         << " (Ants=" << NUM_ANTS << ", MaxIter=" << MAX_ITERATIONS 
         << ", Patience=" << PATIENCE << ", α=" << ALPHA 
         << ", β=" << BETA << ", ρ=" << RHO << ")\n";

    auto start_time = chrono::high_resolution_clock::now();
    AntColonyOptimization(cvrp, NUM_ANTS, MAX_ITERATIONS, ALPHA, BETA, RHO, Q, TAU0, PATIENCE);
    auto end_time = chrono::high_resolution_clock::now();
    cvrp.aco_elapsed_ms = chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count();

    cout << "ACO Finished. Final Best Fitness: " << fixed << setprecision(4) << cvrp.aco_best_fitness << "\n";
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
            run_aco_and_output(cvrp);
            
            //Ghi file ket qua
            write_solution_file(cvrp, output);

            // So sánh kết quả 4 thuật toán
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
            cout << "ACO: Fitness=" << cvrp.aco_best_fitness
                 << " | Routes=" << cvrp.aco_best_routes.size()
                 << " | Iters=" << cvrp.aco_iterations_run
                 << " | Time(ms)=" << cvrp.aco_elapsed_ms << "\n";
            
            // Tìm best method
            double best_fitness = min({cvrp.ga_best_fitness, cvrp.es_best_fitness, 
                                      cvrp.ep_best_fitness, cvrp.aco_best_fitness});
            string best_method = "";
            if (abs(cvrp.ga_best_fitness - best_fitness) < 1e-9) best_method = "GA";
            if (abs(cvrp.es_best_fitness - best_fitness) < 1e-9) best_method += (best_method.empty() ? "" : "/") + string("ES");
            if (abs(cvrp.ep_best_fitness - best_fitness) < 1e-9) best_method += (best_method.empty() ? "" : "/") + string("EP");
            if (abs(cvrp.aco_best_fitness - best_fitness) < 1e-9) best_method += (best_method.empty() ? "" : "/") + string("ACO");
            cout << "Best Method(s): " << best_method << " with fitness " << best_fitness << "\n";
            cout << "Optimal ans: " << cvrp.optimal << "\n";
            cout << "==========================================\n\n";
        }
    }

    cout << "\nAll input files processed. Results are in " << output << "\n";
    return 0;
}