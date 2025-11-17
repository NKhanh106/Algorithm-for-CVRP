#include "GA_Core.h"
#include "../constructive/ClarkWright.h"
#include <random>
#include <ctime>
#include <iomanip>
#include <cassert>

using namespace std;

vector<int> extract_sequence(const vector<vector<int>>& routes, int depot) {
    vector<int> sequence;
    for (const auto& route : routes) {
        for (int customer : route) {
            if (customer != depot) {
                sequence.push_back(customer);
            }
        }
    }
    return sequence;
}

double get_route_cost(int start_idx, int end_idx, const vector<int>& seq, const CVRP& cvrp) {
    int current_load = 0;
    for (int k = start_idx; k <= end_idx; ++k) {
        auto it = cvrp.demands.find(seq[k]);
        assert(it != cvrp.demands.end());
        current_load += it->second;
        if(current_load > cvrp.capacity) {
            return 1e9;
        }
    }
    
    double cost = 0;
    int depot = cvrp.depot;
    
    cost += dis[depot][seq[start_idx]];
    
    for (int k = start_idx; k < end_idx; ++k) {
        cost += dis[seq[k]][seq[k+1]];
    }
    
    cost += dis[seq[end_idx]][depot];
    return cost;
}

pair<double, vector<vector<int>>> EvaluateIndividual(const vector<int>& seq, const CVRP& cvrp, int max_vehicles) {
    int n = seq.size();
    if (n == 0) return {0.0, {}};
    if (max_vehicles <= 0) return {1e9, {}};

    const int K = min(max_vehicles, n);

    vector<vector<double>> route_cost(n, vector<double>(n, 1e9));
    for (int j = 0; j < n; ++j) {
        int load = 0;
        for (int i = j; i < n; ++i) {
            auto it = cvrp.demands.find(seq[i]);
            assert(it != cvrp.demands.end());
            load += it->second;
            if (load > cvrp.capacity) break;
            route_cost[j][i] = get_route_cost(j, i, seq, cvrp);
        }
    }

    vector<vector<double>> f(n + 1, vector<double>(K + 1, 1e9));
    vector<vector<int>> p_idx(n + 1, vector<int>(K + 1, -1));

    f[0][0] = 0.0;

    for (int i = 1; i <= n; ++i) {
        for (int k = 1; k <= K; ++k) {
            double best = 1e9;
            int best_j = -1;
            for (int j = 0; j < i; ++j) {
                double rc = route_cost[j][i - 1];
                if (rc == 1e9) continue;
                double cand = f[j][k - 1] + rc;
                if (cand < best) {
                    best = cand;
                    best_j = j;
                }
            }
            f[i][k] = best;
            p_idx[i][k] = best_j;
        }
    }

    double best_cost = 1e9;
    int best_k = -1;
    for (int k = 1; k <= K; ++k) {
        if (f[n][k] < best_cost - 1e-12 || (abs(f[n][k] - best_cost) <= 1e-12 && k < best_k)) {
            best_cost = f[n][k];
            best_k = k;
        }
    }
    if (best_k < 0 || best_cost == 1e9) {
        return {1e9, {}};
    }

    vector<vector<int>> routes;
    int i = n, k = best_k;
    while (i > 0 && k > 0) {
        int j = p_idx[i][k];
        assert(j >= 0);
        vector<int> route;
        route.reserve(i - j + 2);
        route.push_back(cvrp.depot);
        for (int t = j; t <= i - 1; ++t) route.push_back(seq[t]);
        route.push_back(cvrp.depot);
        routes.push_back(move(route));
        i = j;
        --k;
    }
    reverse(routes.begin(), routes.end());

    return {best_cost, routes};
}

void InitializePopulation(CVRP& cvrp, int population_size, vector<Individual>& population) {
    population.clear();

    // 1) RNG tốt hơn 
    mt19937 g(random_device{}());
    uniform_real_distribution<> alpha_dist(0.5, 1.5);

    // 2) Giới hạn số lần thử để tránh kẹt
    const int MAX_ATTEMPTS = population_size * 200;
    int attempts = 0;

    auto dup_seq = [&](const Individual& cand){
        for (const auto& ind : population) {
            if (ind.customer_sequence == cand.customer_sequence) return true;
            if (fabs(ind.fitness - cand.fitness) < 1e-9) return true;
        }
        return false;
    };

    while (population.size() < (size_t)population_size && attempts < MAX_ATTEMPTS) {
        attempts++;

        double alpha = alpha_dist(g);
        CVRP tmp = cvrp;
        ClarkAndWrightFunc(tmp, alpha);

        Individual x;
        x.customer_sequence = extract_sequence(tmp.route_3opt, tmp.depot);
        if (x.customer_sequence.empty()) continue; // bỏ cá thể rỗng

        x.fitness = tmp.ClarkAndWright_3opt;
        x.routes  = move(tmp.route_3opt);

        if (!dup_seq(x)) population.push_back(move(x));
    }

    // 3) Fallback: lấp đầy bằng hoán vị ngẫu nhiên nếu vẫn thiếu
    while (population.size() < (size_t)population_size) {
        vector<int> seq;
        seq.reserve(cvrp.dimension - 1);
        for (int id = 1; id <= cvrp.dimension; ++id) if (id != cvrp.depot) seq.push_back(id);
        if (seq.empty()) break;
        shuffle(seq.begin(), seq.end(), g);

        Individual rnd;
        rnd.customer_sequence = move(seq);
        int K = cvrp.max_vehicles;
        tie(rnd.fitness, rnd.routes) = EvaluateIndividual(rnd.customer_sequence, cvrp, K);
        if (!dup_seq(rnd)) population.push_back(move(rnd));
    }

    if (population.empty()) {
        cerr << "[FATAL] Population init failed\n";
        return;
    }

    sort(population.begin(), population.end(),
              [](const Individual& a, const Individual& b){ return a.fitness < b.fitness; });
}

void GeneticAlgorithm(CVRP& cvrp, int population_size, int max_generations, int patience) {
    vector<Individual> population;
    InitializePopulation(cvrp, population_size, population);

    double best_global_fitness = population[0].fitness;
    int stale_gens = 0;

    int  gen_ran = 0;
    int  best_global_gen = 0;
    bool early_stop = false;
    vector<vector<int>> best_routes = population[0].routes;

    for (int gen = 0; gen < max_generations; ++gen) {
        vector<Individual> new_population;

        Individual best_current = population[0];
        for (const auto& ind : population)
            if (ind.fitness < best_current.fitness) best_current = ind;
        new_population.push_back(best_current);

        while (new_population.size() < (size_t)population_size) {
            Individual parent1 = SelectParent(population);
            Individual parent2 = SelectParent(population);

            Individual offspring;
            offspring.customer_sequence = OrderCrossover(parent1.customer_sequence, parent2.customer_sequence);
            Mutate(offspring.customer_sequence);

            int K = cvrp.max_vehicles;
            tie(offspring.fitness, offspring.routes) = EvaluateIndividual(offspring.customer_sequence, cvrp, K);
            new_population.push_back(offspring);
        }

        population = move(new_population);
        sort(population.begin(), population.end(),
             [](const Individual& a, const Individual& b){ return a.fitness < b.fitness; });

        if (population[0].fitness + 1e-12 < best_global_fitness) {
            best_global_fitness = population[0].fitness;
            best_routes = population[0].routes;
            best_global_gen = gen + 1;
            stale_gens = 0;
        } else {
            ++stale_gens;
        }

        cout << "Gen " << gen + 1 << ": Best Fitness = "
             << fixed << setprecision(4) << population[0].fitness
             << " (Global Best: " << best_global_fitness << ")" << endl;

        gen_ran = gen + 1;

        if (stale_gens >= patience) {
            cout << "[EarlyStop] No improvement for " << stale_gens << " generations.\n";
            early_stop = true;
            break;
        }
    }

    cvrp.ga_best_fitness = best_global_fitness;
    cvrp.ga_best_routes = move(best_routes);
    cvrp.ga_generations_run = gen_ran;
    cvrp.ga_last_improve_gen = best_global_gen;
    cvrp.ga_patience_used = patience;
    cvrp.ga_early_stopped = early_stop;

}