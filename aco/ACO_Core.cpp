#include "ACO_Core.h"
#include "../ga/GA_Core.h"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>

using namespace std;

namespace {

mt19937& global_rng() {
    static thread_local mt19937 rng(random_device{}());
    return rng;
}

// Tính heuristic value: 1/distance (inverse distance)
double heuristic_value(int from, int to, const CVRP& cvrp) {
    if (from == to) return 0.0;
    double dist = dis[from][to];
    if (dist < 1e-9) return 1e9; // avoid division by zero
    return 1.0 / dist;
}

// Tính probability để chọn customer tiếp theo
double calculate_probability(int from, int to, 
                            const vector<vector<double>>& pheromone,
                            const CVRP& cvrp,
                            double alpha, double beta,
                            const vector<bool>& visited) {
    if (visited[to]) return 0.0;
    if (from == to) return 0.0;
    
    double tau = pheromone[from][to];
    double eta = heuristic_value(from, to, cvrp);
    
    return pow(tau, alpha) * pow(eta, beta);
}

// Chọn customer tiếp theo dựa trên probability
int select_next_customer(int current, 
                         const vector<vector<double>>& pheromone,
                         const CVRP& cvrp,
                         double alpha, double beta,
                         const vector<bool>& visited) {
    auto& rng = global_rng();
    vector<pair<int, double>> candidates;
    double sum_prob = 0.0;
    
    for (int i = 1; i <= cvrp.dimension; ++i) {
        if (i == cvrp.depot) continue;
        if (visited[i]) continue;
        
        double prob = calculate_probability(current, i, pheromone, cvrp, alpha, beta, visited);
        if (prob > 1e-9) {
            candidates.push_back({i, prob});
            sum_prob += prob;
        }
    }
    
    if (candidates.empty()) return -1;
    
    uniform_real_distribution<double> distrib(0.0, sum_prob);
    double random_val = distrib(rng);
    double cumulative = 0.0;
    
    for (const auto& cand : candidates) {
        cumulative += cand.second;
        if (random_val <= cumulative) {
            return cand.first;
        }
    }
    
    return candidates.back().first;
}

// Xây dựng solution cho một ant
Ant construct_solution(const vector<vector<double>>& pheromone,
                       const CVRP& cvrp,
                       double alpha, double beta) {
    Ant ant;
    ant.visited.assign(cvrp.dimension + 1, false);
    ant.visited[cvrp.depot] = true;
    ant.total_cost = 0.0;
    
    int unvisited_count = cvrp.dimension - 1; // trừ depot
    
    while (unvisited_count > 0) {
        vector<int> route;
        route.push_back(cvrp.depot);
        int current = cvrp.depot;
        int current_load = 0;
        bool route_updated = false;
        
        while (unvisited_count > 0) {
            int next = select_next_customer(current, pheromone, cvrp, alpha, beta, ant.visited);
            
            if (next == -1) {
                // Không còn customer nào có thể chọn, thử chọn customer nhỏ nhất còn lại
                bool found = false;
                for (int i = 1; i <= cvrp.dimension; ++i) {
                    if (i == cvrp.depot) continue;
                    if (ant.visited[i]) continue;
                    
                    auto it = cvrp.demands.find(i);
                    if (it == cvrp.demands.end()) continue;
                    
                    int demand = it->second;
                    if (current_load + demand <= cvrp.capacity) {
                        next = i;
                        found = true;
                        break;
                    }
                }
                if (!found) break; // Không thể thêm customer nào nữa
            }
            
            // Kiểm tra capacity constraint
            auto it = cvrp.demands.find(next);
            if (it == cvrp.demands.end()) break;
            
            int demand = it->second;
            if (current_load + demand > cvrp.capacity) {
                break; // Route đầy, bắt đầu route mới
            }
            
            route.push_back(next);
            ant.visited[next] = true;
            current_load += demand;
            current = next;
            unvisited_count--;
            route_updated = true;
        }
        
        if (route.size() > 1) {
            route.push_back(cvrp.depot);
            ant.routes.push_back(route);
            
            // Tính cost của route
            double route_cost = 0.0;
            for (size_t i = 0; i < route.size() - 1; ++i) {
                route_cost += dis[route[i]][route[i + 1]];
            }
            ant.total_cost += route_cost;
        } else if (!route_updated && unvisited_count > 0) {
            // Nếu không thể tạo route mới và vẫn còn customer chưa visit -> lỗi
            // Force visit customer còn lại (có thể vi phạm capacity nhưng đảm bảo feasibility)
            for (int i = 1; i <= cvrp.dimension; ++i) {
                if (i == cvrp.depot) continue;
                if (!ant.visited[i]) {
                    vector<int> forced_route = {cvrp.depot, i, cvrp.depot};
                    ant.routes.push_back(forced_route);
                    ant.visited[i] = true;
                    ant.total_cost += dis[cvrp.depot][i] + dis[i][cvrp.depot];
                    unvisited_count--;
                    break;
                }
            }
        }
    }
    
    return ant;
}

// Áp dụng 2-opt local search cho một route
void two_opt_improve(vector<int>& route, const CVRP& cvrp) {
    if (route.size() <= 4) return; // depot -> customer -> depot (tối thiểu)
    
    bool improved = true;
    while (improved) {
        improved = false;
        double best_cost = 0.0;
        for (size_t i = 1; i < route.size() - 1; ++i) {
            best_cost += dis[route[i]][route[i + 1]];
        }
        
        for (size_t i = 1; i < route.size() - 2; ++i) {
            for (size_t j = i + 1; j < route.size() - 1; ++j) {
                // Thử reverse segment từ i đến j
                double old_cost = dis[route[i-1]][route[i]] + dis[route[j]][route[j+1]];
                double new_cost = dis[route[i-1]][route[j]] + dis[route[i]][route[j+1]];
                
                if (new_cost < old_cost - 1e-9) {
                    reverse(route.begin() + i, route.begin() + j + 1);
                    improved = true;
                    break;
                }
            }
            if (improved) break;
        }
    }
}

// Áp dụng local search cho toàn bộ solution
void apply_local_search(Ant& ant, const CVRP& cvrp) {
    for (auto& route : ant.routes) {
        two_opt_improve(route, cvrp);
    }
    
    // Recalculate total cost
    ant.total_cost = 0.0;
    for (const auto& route : ant.routes) {
        for (size_t i = 0; i < route.size() - 1; ++i) {
            ant.total_cost += dis[route[i]][route[i + 1]];
        }
    }
}

// Cập nhật pheromone: evaporation + deposit
void update_pheromone(vector<vector<double>>& pheromone,
                      const vector<Ant>& ants,
                      double rho, double q,
                      int depot) {
    int n = pheromone.size() - 1;
    
    // Evaporation
    for (int i = 1; i <= n; ++i) {
        for (int j = 1; j <= n; ++j) {
            if (i != j) {
                pheromone[i][j] *= (1.0 - rho);
            }
        }
    }
    
    // Deposit pheromone từ best ants
    vector<Ant> sorted_ants = ants;
    sort(sorted_ants.begin(), sorted_ants.end(),
         [](const Ant& a, const Ant& b) { return a.total_cost < b.total_cost; });
    
    // Chỉ deposit từ top 50% ants
    int elite_count = max(1, static_cast<int>(sorted_ants.size() / 2));
    
    for (int idx = 0; idx < elite_count; ++idx) {
        const Ant& ant = sorted_ants[idx];
        double deposit = q / ant.total_cost; // More pheromone for better solutions
        
        for (const auto& route : ant.routes) {
            for (size_t i = 0; i < route.size() - 1; ++i) {
                int from = route[i];
                int to = route[i + 1];
                if (from != depot && to != depot) {
                    pheromone[from][to] += deposit;
                }
            }
        }
    }
}

} // namespace

void AntColonyOptimization(CVRP& cvrp,
                           int num_ants,
                           int max_iterations,
                           double alpha,
                           double beta,
                           double rho,
                           double q,
                           double tau0,
                           int patience) {
    if (num_ants <= 0 || max_iterations <= 0) {
        cerr << "[ACO] Invalid parameters.\n";
        return;
    }
    
    int n = cvrp.dimension;
    
    // Khởi tạo pheromone matrix
    vector<vector<double>> pheromone(n + 1, vector<double>(n + 1, tau0));
    for (int i = 1; i <= n; ++i) {
        pheromone[i][i] = 0.0; // No self-loops
    }
    
    double best_global_cost = numeric_limits<double>::infinity();
    vector<vector<int>> best_routes;
    int best_iter = 0;
    int iter_ran = 0;
    int stale_iters = 0;
    bool early_stop = false;
    
    for (int iter = 0; iter < max_iterations; ++iter) {
        vector<Ant> ants;
        ants.reserve(num_ants);
        
        // Mỗi ant xây dựng solution
        for (int a = 0; a < num_ants; ++a) {
            Ant ant = construct_solution(pheromone, cvrp, alpha, beta);
            
            // Áp dụng local search (2-opt)
            apply_local_search(ant, cvrp);
            
            ants.push_back(move(ant));
        }
        
        // Tìm best ant trong iteration này
        Ant best_ant = ants[0];
        for (const auto& ant : ants) {
            if (ant.total_cost < best_ant.total_cost) {
                best_ant = ant;
            }
        }
        
        // Cập nhật pheromone
        update_pheromone(pheromone, ants, rho, q, cvrp.depot);
        
        // Cập nhật global best
        if (best_ant.total_cost + 1e-12 < best_global_cost) {
            best_global_cost = best_ant.total_cost;
            best_routes = best_ant.routes;
            best_iter = iter + 1;
            stale_iters = 0;
        } else {
            ++stale_iters;
        }
        
        cout << "[ACO] Gen " << iter + 1 << ": Best = " << fixed << setprecision(4) 
             << best_ant.total_cost << " | Global Best = " << best_global_cost << "\n";
        
        iter_ran = iter + 1;
        if (stale_iters >= patience) {
            cout << "[ACO] Early stop after " << stale_iters << " stale iterations.\n";
            early_stop = true;
            break;
        }
    }
    
    cvrp.aco_best_fitness = best_global_cost;
    cvrp.aco_best_routes = move(best_routes);
    cvrp.aco_iterations_run = iter_ran;
    cvrp.aco_last_improve_iter = best_iter;
    cvrp.aco_patience_used = patience;
    cvrp.aco_early_stopped = early_stop;
}

