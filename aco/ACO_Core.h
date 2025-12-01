#ifndef ACO_CORE_H
#define ACO_CORE_H

#include <vector>
#include <limits>
#include "../CVRP.h"

struct Ant {
    std::vector<std::vector<int>> routes;
    double total_cost = std::numeric_limits<double>::infinity();
    std::vector<bool> visited; // visited customers
};

void AntColonyOptimization(CVRP& cvrp,
                           int num_ants,
                           int max_iterations,
                           double alpha,        // pheromone importance
                           double beta,         // heuristic importance
                           double rho,          // evaporation rate
                           double q,            // pheromone deposit constant
                           double tau0,         // initial pheromone
                           int patience);

#endif

