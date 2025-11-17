#ifndef ES_CORE_H
#define ES_CORE_H

#include <vector>
#include <limits>
#include "../CVRP.h"

struct ESIndividual {
    std::vector<int> customer_sequence;
    double strategy = 1.0; // số phép hoán vị kỳ vọng khi đột biến
    double fitness = std::numeric_limits<double>::infinity();
    std::vector<std::vector<int>> routes;
};

void InitializeESPopulation(CVRP& cvrp, int mu, std::vector<ESIndividual>& parents);
void EvolutionStrategy(CVRP& cvrp,
                       int mu,
                       int lambda,
                       int max_generations,
                       int patience,
                       bool plus_selection);

#endif

