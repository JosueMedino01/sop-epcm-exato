#include "IteratedLocalSearch.h"
#include <iostream>
#include "../greedy-algorithm/GreedyAlgorithm.h"

IteratedLocalSearch::IteratedLocalSearch(int MXI, int K, double C, int SEED)
{
    this->MAX_NOT_IMPROVIMENT = MXI;
    this->K = K;
    this->C = C;
    this->SEED = SEED;
    srand(SEED); 
}


void IteratedLocalSearch::run(InstanceData data, int K, double C) 
{
    GreedyAlgorithm greedy; 
    this->bestSolution = greedy.kNeighborRandomInsertion(data, K, C);

    

    int i = 0;
    while (i < this->MAX_NOT_IMPROVIMENT)
    {

    }
   

}