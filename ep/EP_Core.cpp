#include "EP_Core.h"
#include "../ga/GA_Core.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <random>
#include <tuple>

using namespace std;

namespace {

mt19937& global_rng() {
    static thread_local mt19937 rng(random_device{}());
    return rng;
}

EPIndividual build_ep_individual(const vector<int>& seq, double mutation_rate, const CVRP& cvrp) {
    EPIndividual ind;
    ind.customer_sequence = seq;
    ind.mutation_rate = max(0.05, min(0.5, mutation_rate));
    tie(ind.fitness, ind.routes) = EvaluateIndividual(ind.customer_sequence, cvrp, cvrp.max_vehicles);
    return ind;
}

EPIndividual mutate_offspring(const EPIndividual& parent, const CVRP& cvrp) {
    auto& rng = global_rng();
    uniform_real_distribution<double> prob(0.0, 1.0);
    normal_distribution<double> gaussian(0.0, 0.1);

    EPIndividual child;
    child.customer_sequence = parent.customer_sequence;
    
    // Adaptive mutation rate: thay đổi mutation rate dựa trên performance
    double mutated_rate = parent.mutation_rate * exp(gaussian(rng));
    mutated_rate = max(0.05, min(0.5, mutated_rate));
    child.mutation_rate = mutated_rate;

    int n = static_cast<int>(child.customer_sequence.size());
    if (n < 2) {
        tie(child.fitness, child.routes) = EvaluateIndividual(child.customer_sequence, cvrp, cvrp.max_vehicles);
        return child;
    }

    uniform_int_distribution<int> distrib(0, n - 1);
    
    // Áp dụng mutation với xác suất mutation_rate
    if (prob(rng) < mutated_rate) {
        // Swap mutation
        int i = distrib(rng);
        int j = distrib(rng);
        while (j == i) j = distrib(rng);
        swap(child.customer_sequence[i], child.customer_sequence[j]);
    }

    if (n > 3 && prob(rng) < mutated_rate * 0.8) {
        // Inversion mutation (reverse subsequence)
        int i = distrib(rng);
        int j = distrib(rng);
        if (i > j) swap(i, j);
        if (j - i >= 1) {
            reverse(child.customer_sequence.begin() + i, child.customer_sequence.begin() + j + 1);
        }
    }

    if (n > 3 && prob(rng) < mutated_rate * 0.6) {
        // Insertion mutation (move one element)
        int from = distrib(rng);
        int to = distrib(rng);
        while (to == from) to = distrib(rng);
        int val = child.customer_sequence[from];
        child.customer_sequence.erase(child.customer_sequence.begin() + from);
        child.customer_sequence.insert(child.customer_sequence.begin() + to, val);
    }

    if (n > 4 && prob(rng) < mutated_rate * 0.4) {
        // Scramble mutation (shuffle a subsequence)
        int i = distrib(rng);
        int j = distrib(rng);
        if (i > j) swap(i, j);
        if (j - i >= 2) {
            shuffle(child.customer_sequence.begin() + i, 
                   child.customer_sequence.begin() + j + 1, rng);
        }
    }

    tie(child.fitness, child.routes) = EvaluateIndividual(child.customer_sequence, cvrp, cvrp.max_vehicles);
    return child;
}

EPIndividual tournament_select(const vector<EPIndividual>& population, int tournament_size) {
    auto& rng = global_rng();
    int pop_size = static_cast<int>(population.size());
    if (pop_size == 0) {
        EPIndividual empty;
        return empty;
    }
    
    uniform_int_distribution<int> distrib(0, pop_size - 1);
    EPIndividual best = population[distrib(rng)];
    
    for (int i = 1; i < tournament_size; ++i) {
        const EPIndividual& challenger = population[distrib(rng)];
        if (challenger.fitness < best.fitness) {
            best = challenger;
        }
    }
    return best;
}

} // namespace

void InitializeEPPopulation(CVRP& cvrp, int population_size, vector<EPIndividual>& population) {
    population.clear();
    if (population_size <= 0) return;

    vector<Individual> seeds;
    int seed_pop = max(population_size, 20);
    InitializePopulation(cvrp, seed_pop, seeds);

    auto& rng = global_rng();
    uniform_real_distribution<double> rate_dist(0.05, 0.3);

    int take = min(population_size, static_cast<int>(seeds.size()));
    population.reserve(population_size);
    for (int i = 0; i < take; ++i) {
        population.push_back(build_ep_individual(seeds[i].customer_sequence, rate_dist(rng), cvrp));
    }

    while (static_cast<int>(population.size()) < population_size) {
        vector<int> seq;
        seq.reserve(cvrp.dimension - 1);
        for (int node = 1; node <= cvrp.dimension; ++node) {
            if (node != cvrp.depot) seq.push_back(node);
        }
        shuffle(seq.begin(), seq.end(), rng);
        population.push_back(build_ep_individual(seq, rate_dist(rng), cvrp));
    }

    sort(population.begin(), population.end(),
         [](const EPIndividual& a, const EPIndividual& b){ return a.fitness < b.fitness; });
}

void EvolutionaryProgramming(CVRP& cvrp,
                             int population_size,
                             int max_generations,
                             int patience,
                             int tournament_size) {
    if (population_size <= 0) {
        cerr << "[EP] Invalid population size.\n";
        return;
    }

    vector<EPIndividual> population;
    InitializeEPPopulation(cvrp, population_size, population);
    if (population.empty()) {
        cerr << "[EP] Failed to initialize population.\n";
        return;
    }

    double best_global = population.front().fitness;
    vector<vector<int>> best_routes = population.front().routes;
    int best_gen = 0;
    int gen_ran = 0;
    int stale_gens = 0;
    bool early_stop = false;

    for (int gen = 0; gen < max_generations; ++gen) {
        vector<EPIndividual> offspring;
        offspring.reserve(population_size);
        
        // Mỗi cá thể tạo ra 1 con thông qua mutation
        for (const auto& parent : population) {
            offspring.push_back(mutate_offspring(parent, cvrp));
        }

        // (μ + μ) selection: chọn từ cả parents và offspring
        vector<EPIndividual> pool;
        pool.reserve(population.size() + offspring.size());
        pool.insert(pool.end(), population.begin(), population.end());
        pool.insert(pool.end(), offspring.begin(), offspring.end());

        sort(pool.begin(), pool.end(),
             [](const EPIndividual& a, const EPIndividual& b){ return a.fitness < b.fitness; });

        // Tournament selection để chọn survivors
        vector<EPIndividual> new_population;
        new_population.reserve(population_size);
        for (int i = 0; i < population_size; ++i) {
            new_population.push_back(tournament_select(pool, tournament_size));
        }

        sort(new_population.begin(), new_population.end(),
             [](const EPIndividual& a, const EPIndividual& b){ return a.fitness < b.fitness; });

        population = move(new_population);

        const EPIndividual& current_best = population.front();
        if (current_best.fitness + 1e-12 < best_global) {
            best_global = current_best.fitness;
            best_routes = current_best.routes;
            best_gen = gen + 1;
            stale_gens = 0;
        } else {
            ++stale_gens;
        }

        cout << "[EP] Gen " << gen + 1 << ": Best = " << fixed << setprecision(4) << current_best.fitness
             << " | Global Best = " << best_global << "\n";

        gen_ran = gen + 1;
        if (stale_gens >= patience) {
            cout << "[EP] Early stop after " << stale_gens << " stale generations.\n";
            early_stop = true;
            break;
        }
    }

    cvrp.ep_best_fitness = best_global;
    cvrp.ep_best_routes = move(best_routes);
    cvrp.ep_generations_run = gen_ran;
    cvrp.ep_last_improve_gen = best_gen;
    cvrp.ep_patience_used = patience;
    cvrp.ep_early_stopped = early_stop;
}

