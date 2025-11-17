#ifndef GA_OPERATORS_H
#define GA_OPERATORS_H

#include "GA_Core.h"

// Hàm toán tử di truyền (định nghĩa trong GA_Operators.cpp)
Individual SelectParent(const std::vector<Individual>& population);
std::vector<int> OrderCrossover(const std::vector<int>& parent1_seq, const std::vector<int>& parent2_seq);
void Mutate(std::vector<int>& sequence);

#endif // GA_OPERATORS_H