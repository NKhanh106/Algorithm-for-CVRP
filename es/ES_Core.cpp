#include "ES_Core.h"
#include "../ga/GA_Core.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>
#include <tuple>

using namespace std;

namespace {

mt19937& global_rng() {
    static thread_local mt19937 rng(random_device{}());
    return rng;
}

ESIndividual build_es_individual(const vector<int>& seq, double strategy, const CVRP& cvrp) {
    ESIndividual ind;
    ind.customer_sequence = seq;
    ind.strategy = max(0.5, strategy);
    tie(ind.fitness, ind.routes) = EvaluateIndividual(ind.customer_sequence, cvrp, cvrp.max_vehicles);
    return ind;
}

ESIndividual mutate_offspring(const ESIndividual& parent, const CVRP& cvrp, double tau) {
    auto& rng = global_rng();
    normal_distribution<double> gaussian(0.0, 1.0);
    uniform_real_distribution<double> prob(0.0, 1.0);

    ESIndividual child;
    child.customer_sequence = parent.customer_sequence;

    double mutated_strategy = parent.strategy * exp(tau * gaussian(rng));
    int max_swaps = max(1, static_cast<int>(child.customer_sequence.size()) - 1);
    mutated_strategy = min<double>(max_swaps, max(0.5, mutated_strategy));
    child.strategy = mutated_strategy;

    int n = static_cast<int>(child.customer_sequence.size());
    if (n >= 2) {
        uniform_int_distribution<int> distrib(0, n - 1);
        int num_swaps = max(1, min(max_swaps, static_cast<int>(round(mutated_strategy))));
        for (int s = 0; s < num_swaps; ++s) {
            int i = distrib(rng);
            int j = distrib(rng);
            while (j == i) j = distrib(rng);
            swap(child.customer_sequence[i], child.customer_sequence[j]);
        }

        if (n > 3 && prob(rng) < 0.30) {
            int i = distrib(rng);
            int j = distrib(rng);
            if (i > j) swap(i, j);
            reverse(child.customer_sequence.begin() + i, child.customer_sequence.begin() + j + 1);
        }

        if (n > 3 && prob(rng) < 0.20) {
            int from = distrib(rng);
            int to = distrib(rng);
            while (to == from) to = distrib(rng);
            int val = child.customer_sequence[from];
            child.customer_sequence.erase(child.customer_sequence.begin() + from);
            child.customer_sequence.insert(child.customer_sequence.begin() + to, val);
        }
    }

    tie(child.fitness, child.routes) = EvaluateIndividual(child.customer_sequence, cvrp, cvrp.max_vehicles);
    return child;
}

} // namespace

void InitializeESPopulation(CVRP& cvrp, int mu, vector<ESIndividual>& parents) {
    parents.clear();
    if (mu <= 0) return;

    vector<Individual> seeds;
    int seed_pop = max(mu, 20);
    InitializePopulation(cvrp, seed_pop, seeds);

    auto& rng = global_rng();
    double max_sigma = max(2.0, static_cast<double>(cvrp.dimension) / 8.0);
    uniform_real_distribution<double> sigma_dist(1.0, max_sigma);

    int take = min(mu, static_cast<int>(seeds.size()));
    parents.reserve(mu);
    for (int i = 0; i < take; ++i) {
        parents.push_back(build_es_individual(seeds[i].customer_sequence, sigma_dist(rng), cvrp));
    }

    while (static_cast<int>(parents.size()) < mu) {
        vector<int> seq;
        seq.reserve(cvrp.dimension - 1);
        for (int node = 1; node <= cvrp.dimension; ++node) {
            if (node != cvrp.depot) seq.push_back(node);
        }
        shuffle(seq.begin(), seq.end(), rng);
        parents.push_back(build_es_individual(seq, sigma_dist(rng), cvrp));
    }

    sort(parents.begin(), parents.end(),
         [](const ESIndividual& a, const ESIndividual& b){ return a.fitness < b.fitness; });
}

void EvolutionStrategy(CVRP& cvrp,
                       int mu,
                       int lambda,
                       int max_generations,
                       int patience,
                       bool plus_selection) {
    if (mu <= 0 || lambda <= 0) {
        cerr << "[ES] Invalid (mu, lambda) configuration.\n";
        return;
    }

    vector<ESIndividual> parents;
    InitializeESPopulation(cvrp, mu, parents);
    if (parents.empty()) {
        cerr << "[ES] Failed to initialize parents.\n";
        return;
    }

    auto& rng = global_rng();
    double tau = 1.0 / sqrt(2.0 * sqrt(static_cast<double>(max(2, cvrp.dimension - 1))));

    double best_global = parents.front().fitness;
    vector<vector<int>> best_routes = parents.front().routes;
    int best_gen = 0;
    int gen_ran = 0;
    int stale_gens = 0;
    bool early_stop = false;

    for (int gen = 0; gen < max_generations; ++gen) {
        vector<ESIndividual> offspring;
        offspring.reserve(lambda);
        if (parents.empty()) break;
        uniform_int_distribution<int> parent_pick(0, static_cast<int>(parents.size()) - 1);
        for (int i = 0; i < lambda; ++i) {
            const ESIndividual& parent = parents[parent_pick(rng)];
            offspring.push_back(mutate_offspring(parent, cvrp, tau));
        }

        vector<ESIndividual> pool;
        if (plus_selection) {
            pool.reserve(parents.size() + offspring.size());
            pool.insert(pool.end(), parents.begin(), parents.end());
            pool.insert(pool.end(), offspring.begin(), offspring.end());
        } else {
            pool = move(offspring);
        }

        sort(pool.begin(), pool.end(),
             [](const ESIndividual& a, const ESIndividual& b){ return a.fitness < b.fitness; });

        int survivors = min(mu, static_cast<int>(pool.size()));
        if (survivors == 0) break;
        parents.assign(pool.begin(), pool.begin() + survivors);

        const ESIndividual& current_best = parents.front();
        if (current_best.fitness + 1e-12 < best_global) {
            best_global = current_best.fitness;
            best_routes = current_best.routes;
            best_gen = gen + 1;
            stale_gens = 0;
        } else {
            ++stale_gens;
        }

        cout << "[ES] Gen " << gen + 1 << ": Best = " << current_best.fitness
             << " | Global Best = " << best_global << "\n";

        gen_ran = gen + 1;
        if (stale_gens >= patience) {
            cout << "[ES] Early stop after " << stale_gens << " stale generations.\n";
            early_stop = true;
            break;
        }
    }

    cvrp.es_best_fitness = best_global;
    cvrp.es_best_routes = move(best_routes);
    cvrp.es_generations_run = gen_ran;
    cvrp.es_last_improve_gen = best_gen;
    cvrp.es_patience_used = patience;
    cvrp.es_early_stopped = early_stop;
}

