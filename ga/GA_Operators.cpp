#include "GA_Operators.h"
#include <random>
#include <ctime>
#include <set>

using namespace std;

const int TOURNAMENT_SIZE = 3;
const double MUTATION_RATE = 0.2;

static mt19937 rng(random_device{}());

Individual SelectParent(const vector<Individual>& population) {
    int pop_size = population.size();
    Individual best_in_tournament = population[0];
    
    for (int i = 0; i < TOURNAMENT_SIZE; ++i) {
        int random_index = uniform_int_distribution<>(0, pop_size - 1)(rng);
        const Individual& challenger = population[random_index];
        if (i == 0 || challenger.fitness < best_in_tournament.fitness) {
            best_in_tournament = challenger;
        }
    }
    return best_in_tournament;
}

vector<int> OrderCrossover(const vector<int>& parent1_seq, const vector<int>& parent2_seq) {
    int n = parent1_seq.size();
    if (n <= 1) return parent1_seq;
    vector<int> offspring(n, 0);
    
    uniform_int_distribution<> distrib(0, n - 1);
    int start = distrib(rng);
    int end = distrib(rng);

    if (start > end) swap(start, end);

    for (int i = start; i <= end; ++i) {
        offspring[i] = parent1_seq[i];
    }

    set<int> copied_genes(parent1_seq.begin() + start, parent1_seq.begin() + end + 1);
    int offspring_idx = (end + 1) % n;
    
    for (int i = 0; i < n; ++i) {
        int gene_to_check = parent2_seq[(end + 1 + i) % n];
        
        if (copied_genes.find(gene_to_check) == copied_genes.end()) {
            offspring[offspring_idx] = gene_to_check;
            offspring_idx = (offspring_idx + 1) % n;
        }
    }

    return offspring;
}

void Mutate(vector<int>& sequence) {
    if (sequence.empty()) return;

    if (uniform_real_distribution<>(0.0, 1.0)(rng) < MUTATION_RATE) {
        int n = sequence.size();
        uniform_int_distribution<> distrib(0, n - 1);
        
        int idx1 = distrib(rng);
        int idx2 = distrib(rng);
        
        while (idx1 == idx2) {
            idx2 = distrib(rng);
        }
        
        swap(sequence[idx1], sequence[idx2]);
    }
}