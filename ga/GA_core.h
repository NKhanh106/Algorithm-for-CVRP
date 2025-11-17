#ifndef GA_CORE_H
#define GA_CORE_H

#include <utility>
#include <vector>

#include "../CVRP.h"

// Hàm Utility cho GA
std::vector<int> extract_sequence(const std::vector<std::vector<int>>& routes, int depot);
double get_route_cost(int start_idx, int end_idx, const std::vector<int>& seq, const CVRP& cvrp);

// Hàm cốt lõi của GA
std::pair<double, std::vector<std::vector<int>>> EvaluateIndividual(const std::vector<int>& seq, const CVRP& cvrp, int max_vehicles);
void InitializePopulation(CVRP& cvrp, int population_size, std::vector<Individual>& population);
void GeneticAlgorithm(CVRP& cvrp, int population_size, int max_generations, int patience);

// Khai báo các toán tử di truyền cơ bản (sẽ được định nghĩa sau)
Individual SelectParent(const std::vector<Individual>& population);
std::vector<int> OrderCrossover(const std::vector<int>& parent1_seq, const std::vector<int>& parent2_seq);
void Mutate(std::vector<int>& sequence);

#endif