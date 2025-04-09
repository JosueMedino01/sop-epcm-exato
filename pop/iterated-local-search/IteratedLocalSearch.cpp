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
    /* BuscaLocal(data, bestSolution) */
    
    this->shiftOneZero(data, this->bestSolution);
    this->shiftOneZero(data, this->bestSolution);
    this->shiftOneZero(data, this->bestSolution);
    this->shiftOneZero(data, this->bestSolution);
    this->shiftOneZero(data, this->bestSolution);

    this->swapOneOne(data, this->bestSolution);
   /*  this->swapOneOne(data, this->bestSolution);

    this->swapOneOne(data, this->bestSolution);

    this->swapOneOne(data, this->bestSolution);
    this->swapOneOne(data, this->bestSolution);
 */
    
   /*  int i = 0;
    while (i < this->MAX_NOT_IMPROVIMENT)
    {
        solvPertubed = Pertubacao(bestSolution)
        BuscaLocal(data, solvPertubed)
        Accept(solvPertubed, bestSolution)
        
    } */
   

}

double IteratedLocalSearch::objcFunc(double sumPrize, double sumCost) {
    return sumPrize - this->C * sumCost;
}

/* Move um cliente de não visitado para o caminho viável */
void  IteratedLocalSearch::shiftOneZero(InstanceData &data, Customers &customers) {
    int n = customers.feasibleTour.path.size();
    int bestNotVisitedIndex = -1, bestPositionIndex = -1;
    double bestCost = customers.feasibleTour.cost, bestPrize = customers.feasibleTour.prize; 
    /* double bestObjFunc =  this->objcFunc(customers.feasibleTour.prize, customers.feasibleTour.cost);  */

    for(int i = 1; i < n - 1; i++) {
        for (int j = 0; j < customers.notVisited.size(); j++)
        {   
            int addedNode = customers.notVisited[j];

            double dellEdges = data.cost[customers.feasibleTour.path[i - 1]][customers.feasibleTour.path[i]];
            double newEdges = data.cost[customers.feasibleTour.path[i - 1]][addedNode] 
                + data.cost[addedNode][customers.feasibleTour.path[i]]; 
            
            double newCost = customers.feasibleTour.cost - dellEdges + newEdges;
            int newPrize = customers.feasibleTour.prize + data.prize[addedNode];

            
            if( newCost <= data.deadline && this->objcFunc(newPrize, newCost) > this->objcFunc(bestPrize, bestCost)) {
                bestPositionIndex = i;
                bestNotVisitedIndex = j;
                bestCost = newCost;
                bestPrize = newPrize;
            }
        }
    }

    if (bestNotVisitedIndex != -1 && bestPositionIndex != -1) {
        int addeNode = customers.notVisited[bestNotVisitedIndex];
        customers.notVisited.erase(customers.notVisited.begin() + bestNotVisitedIndex);
        customers.feasibleTour.path.insert(customers.feasibleTour.path.begin() + bestPositionIndex, addeNode);

        customers.feasibleTour.cost = bestCost;
        customers.feasibleTour.prize = bestPrize;

    }
}

/* Troca entre um cliente do tour e um cliente não visitado */
void IteratedLocalSearch::swapOneOne(InstanceData &data, Customers &customers) {
    int n = customers.feasibleTour.path.size();
    int bestNotVisitedIndex = -1, bestPositionIndex = -1;
    double bestCost = customers.feasibleTour.cost, bestPrize = customers.feasibleTour.prize; 

    for(int i = 1; i < n - 1; i++) {
        for (int j = 0; j < customers.notVisited.size(); j++)
        {   
            int addedNode = customers.notVisited[j];
            int deletedNode = customers.feasibleTour.path[i];

            double dellEdges = data.cost[customers.feasibleTour.path[i - 1]][deletedNode] 
                + data.cost[deletedNode][customers.feasibleTour.path[i + 1]];

            double newEdges = data.cost[customers.feasibleTour.path[i - 1]][addedNode] 
                + data.cost[addedNode][customers.feasibleTour.path[i + 1]]; 
            
            double newCost = customers.feasibleTour.cost - dellEdges + newEdges;
            int newPrize = customers.feasibleTour.prize + data.prize[addedNode] - data.prize[deletedNode];

            cout << "Troca de " << addedNode << " por " << deletedNode << " newOF " <<  this->objcFunc(newPrize, newCost) << " Curr of" <<  this->objcFunc(bestPrize, bestCost);
            cout << " CUSTO : " << newCost<< endl ;
            if( newCost <= data.deadline && this->objcFunc(newPrize, newCost) > this->objcFunc(bestPrize, bestCost)) {
                bestPositionIndex = i;
                bestNotVisitedIndex = j;
                bestCost = newCost;
                bestPrize = newPrize;
                cout << "HOUVE MELHORA"<< endl;
            }
        }
    }

    if (bestNotVisitedIndex != -1 && bestPositionIndex != -1) {
        int addedNode = customers.notVisited[bestNotVisitedIndex];
        int deletedNode = customers.notVisited[bestNotVisitedIndex];

        customers.notVisited[bestNotVisitedIndex] = deletedNode;
        customers.feasibleTour.path[bestPositionIndex] = addedNode;

        customers.feasibleTour.cost = bestCost;
        customers.feasibleTour.prize = bestPrize;
    }
}


/* Customers neighboorhood(int k) {
    switch() {
        case: 1
          
            break;
    }
} */

void  IteratedLocalSearch::localSearch(InstanceData &data, Customers &solution) {
    int r = 3, k = 1;

    while(k <= r) {
        /* Encontre o melhor vizinho s’ ∈ N(k)(s) */

        if(true) {

        } 
        else {

        }
    }
}
/* https://iopscience.iop.org/article/10.1088/1742-6596/1320/1/012025/pdf */