#ifndef EP_CORE_H
#define EP_CORE_H

#include <vector>
#include <limits>
#include "../CVRP.h"

struct EPIndividual {
    std::vector<int> customer_sequence;
    double mutation_rate = 0.1; // xác suất đột biến
    double fitness = std::numeric_limits<double>::infinity();
    std::vector<std::vector<int>> routes;
};

void InitializeEPPopulation(CVRP& cvrp, int population_size, std::vector<EPIndividual>& population);
void EvolutionaryProgramming(CVRP& cvrp,
                             int population_size,
                             int max_generations,
                             int patience,
                             int tournament_size);

#endif

