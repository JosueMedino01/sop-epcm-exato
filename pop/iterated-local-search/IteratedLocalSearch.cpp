#include "IteratedLocalSearch.h"
#include <iostream>
#include <fstream>
#include <algorithm>
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
    /* Melhor Solução / Solução Inicial*/
    GreedyAlgorithm greedy; 
    this->bestSolution = greedy.kNeighborRandomInsertion(data, K, C); 
    this->printData(bestSolution.feasibleTour, bestSolution.notVisited, "SOLUCAO INICIAL");
    this->localSearch(data, bestSolution); 

    int i = 0;
    while (i < this-> MAX_NOT_IMPROVIMENT)
    {
        i++;

        /* Pertubacao */
        Customers disturbed =  doubleBridge(data, bestSolution);
        
        /* Busca Local */
        this->localSearch(data, disturbed); 

        /* Criterio de Aceitação */
        if (this->objcFunc(disturbed.feasibleTour.prize, disturbed.feasibleTour.cost) > this->objcFunc(bestSolution.feasibleTour.prize, bestSolution.feasibleTour.cost)) {
            this->bestSolution = disturbed;
            cout << "HOUVE MELHORA NA ITERACAO "<< i <<endl;
            i = 0;
        }
        
    }
}

double IteratedLocalSearch::objcFunc(double sumPrize, double sumCost) {
    return sumPrize - this->C * sumCost;
}

/* Move um cliente de não visitado para o caminho viável */
bool IteratedLocalSearch::shiftOneZero(InstanceData &data, Customers &customers) {
    int n = customers.feasibleTour.path.size();
    int bestNotVisitedIndex = -1, bestPositionIndex = -1;
    double bestCost = customers.feasibleTour.cost, bestPrize = customers.feasibleTour.prize; 

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
        return true;
    }

    return false;
}

/* Troca entre um cliente do tour e um cliente não visitado */
bool IteratedLocalSearch::swapOneOne(InstanceData &data, Customers &customers) {
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

            /* cout << "Troca de " << addedNode << " por " << deletedNode << " newOF " <<  this->objcFunc(newPrize, newCost) << " Curr of" <<  this->objcFunc(bestPrize, bestCost);
            cout << " CUSTO : " << newCost<< endl ; */
            if( newCost <= data.deadline && this->objcFunc(newPrize, newCost) > this->objcFunc(bestPrize, bestCost)) {
                bestPositionIndex = i;
                bestNotVisitedIndex = j;
                bestCost = newCost;
                bestPrize = newPrize;
               /*  cout << "HOUVE MELHORA"<< endl; */
            }
        }
    }

    if (bestNotVisitedIndex != -1 && bestPositionIndex != -1) {
        int addedNode = customers.notVisited[bestNotVisitedIndex];
        int deletedNode = customers.feasibleTour.path[bestPositionIndex];

        customers.notVisited[bestNotVisitedIndex] = deletedNode;
        customers.feasibleTour.path[bestPositionIndex] = addedNode;

        customers.feasibleTour.cost = bestCost;
        customers.feasibleTour.prize = bestPrize;
        return true;
    }

    return false;
}

bool IteratedLocalSearch::reinsertion(InstanceData &data, Customers &customers) {
    if (customers.feasibleTour.path.size() < 3) return false;

    int n = customers.feasibleTour.path.size();
    int bestRemovedIndex = -1, bestInsertionIndex = -1;
    double bestCost = customers.feasibleTour.cost; 

    for(int i = n - 2; i >= 2; i--) {
        int deletedNode = customers.feasibleTour.path[i];

        for (int j = 1; j < i; j++)
        {   
            if(i != j && i != j+1) {
                double dellEdges = data.cost[customers.feasibleTour.path[i - 1]][deletedNode] 
                    + data.cost[deletedNode][customers.feasibleTour.path[i + 1]]
                    + data.cost[customers.feasibleTour.path[j - 1]][customers.feasibleTour.path[j]];

                double newEdges = data.cost[customers.feasibleTour.path[j - 1]][customers.feasibleTour.path[i]] 
                    + data.cost[customers.feasibleTour.path[i]][customers.feasibleTour.path[j]] 
                    + data.cost[customers.feasibleTour.path[i-1]][customers.feasibleTour.path[i+1]]; 
                
                double newCost = customers.feasibleTour.cost - dellEdges + newEdges;

/*                 cout << "Remover o " << deletedNode << " e  add na pos. " << j << " novo custo: "<<  newCost << " custo atual: " << bestCost << endl;
 */                if( newCost <= data.deadline && (newCost <= bestCost)) {
                    bestRemovedIndex = i;
                    bestInsertionIndex = j;
                    bestCost = newCost;
                    
/*                     cout << "REINSERTION HOUVE MELHORA"<< endl;
 */                }
            }
        }
    }

    if (bestRemovedIndex != -1 && bestInsertionIndex != -1) {
        int bestRemovedNode = customers.feasibleTour.path[bestRemovedIndex];
        
        customers.feasibleTour.path.erase(customers.feasibleTour.path.begin() + bestRemovedIndex);

        if (bestRemovedIndex < bestInsertionIndex) {
            // Após remover bestRemovedIndex, os elementos após são deslocados para a esquerda.
            // A posição de inserção original (bestInsertionIndex) agora é bestInsertionIndex - 1.
            customers.feasibleTour.path.insert(customers.feasibleTour.path.begin() + (bestInsertionIndex - 1), bestRemovedNode);
        } else {
            customers.feasibleTour.path.insert(customers.feasibleTour.path.begin() + bestInsertionIndex, bestRemovedNode);
        }
        
        customers.feasibleTour.cost = bestCost;
        return true;
    }

    return false;
}

bool IteratedLocalSearch::twoOpt(InstanceData &data, Customers &customers) {
    int n = customers.feasibleTour.path.size();
    double bestCost = customers.feasibleTour.cost;
    int bestIndexI = -1, bestIndexJ = -1;

    for (int i = 1  ; i < n; i++) {
        for (int j = i + 1; j < n - 1; j++) {
            double edgeRemoved  =  data.cost[customers.feasibleTour.path[i - 1]][customers.feasibleTour.path[i]] + data.cost[customers.feasibleTour.path[j]][customers.feasibleTour.path[j + 1]];
            double edgeAdded = data.cost[customers.feasibleTour.path[i - 1]][customers.feasibleTour.path[j]] + data.cost[customers.feasibleTour.path[i]][customers.feasibleTour.path[j + 1]];
            double newCost = customers.feasibleTour.cost - edgeRemoved + edgeAdded;

            if (newCost < bestCost)
            {
                bestCost = newCost;
                bestIndexI = i;
                bestIndexJ = j;
            }
        }
    }

    
    if (bestIndexI != -1 && bestIndexJ != -1) {
        reverse(customers.feasibleTour.path.begin() + bestIndexI, customers.feasibleTour.path.begin() + bestIndexJ+ 1);
        customers.feasibleTour.cost = bestCost;
        return true;
    }

    return false;
}


void  IteratedLocalSearch::localSearch(InstanceData &data, Customers &customers) {
    int r = 4, k = 1;

    while(k <= r) {
        bool hasImprovement = false;
       
        switch(k) {
            case 1:
                hasImprovement = this->shiftOneZero(data, customers);
                if (hasImprovement) printData(customers.feasibleTour, customers.notVisited, "shiftOneZero");

                break;
            case 2:
                hasImprovement = this->swapOneOne(data, customers);
                if (hasImprovement) printData(customers.feasibleTour, customers.notVisited, "swapOneOne");
                break;
            case 3: 
                hasImprovement = this->reinsertion(data, customers);
                if (hasImprovement) printData(customers.feasibleTour, customers.notVisited, "reinsertion");
                break;
            case 4: 
                hasImprovement = this->twoOpt(data, customers);
                if (hasImprovement) printData(customers.feasibleTour, customers.notVisited, "twoOpt");
            break;
        }

        k = hasImprovement ? 1 : k + 1;
    }
}
/* https://iopscience.iop.org/article/10.1088/1742-6596/1320/1/012025/pdf */


Customers IteratedLocalSearch::doubleBridge(InstanceData &data, Customers customers) {
    customers.feasibleTour.path.pop_back(); 
    customers.feasibleTour.cost -= data.cost[customers.feasibleTour.path.back()][0];
    
    int n = customers.feasibleTour.path.size();
    if (n < 8) return {customers.feasibleTour, customers.notVisited};
    
    int partSize = max(1, (n - 4) / 4);
    
    int cuts[4];
    cuts[0] = 1 + rand() % partSize;
    cuts[1] = cuts[0] + 1 + rand() % partSize;
    cuts[2] = cuts[1] + 1 + rand() % partSize;
    cuts[3] = cuts[2] + 1 + rand() % (n - cuts[2] - 2);


    vector<int> newTour;
    newTour.insert(newTour.end(), customers.feasibleTour.path.begin(), customers.feasibleTour.path.begin() + cuts[0]);
    newTour.insert(newTour.end(), customers.feasibleTour.path.begin() + cuts[3], customers.feasibleTour.path.end());
    newTour.insert(newTour.end(), customers.feasibleTour.path.begin() + cuts[2], customers.feasibleTour.path.begin() + cuts[3]);
    newTour.insert(newTour.end(), customers.feasibleTour.path.begin() + cuts[1], customers.feasibleTour.path.begin() + cuts[2]);
    newTour.insert(newTour.end(), customers.feasibleTour.path.begin() + cuts[0], customers.feasibleTour.path.begin() + cuts[1]);

    // Custo -> forma errada, mas provisória
    double newCost = 0;

    for (int i = 1; i < newTour.size(); i++) {
        newCost += data.cost[newTour[i-1]][newTour[i]];
    }


    customers.feasibleTour.path = newTour;
    customers.feasibleTour.cost = newCost;

    // Verificar deadline e remove clientes se necessário
    while (customers.feasibleTour.cost + data.cost[customers.feasibleTour.path.back()][0] > data.deadline && customers.feasibleTour.path.size() > 2) {
        int last = customers.feasibleTour.path.back();
        customers.feasibleTour.path.pop_back();
        customers.feasibleTour.cost -= data.cost[customers.feasibleTour.path.back()][last];
        if(last != 0) {
            customers.feasibleTour.prize -= data.prize[last];
            customers.notVisited.push_back(last);
        }
    }

    customers.feasibleTour.cost += data.cost[customers.feasibleTour.path.back()][0];
    customers.feasibleTour.path.push_back(0); 

    return {customers.feasibleTour, customers.notVisited};
}

void IteratedLocalSearch::printData(Tour tour, vector<int> notVisited, string source) {
    ofstream outFile("solution_log.txt", ios::app); // Abre o arquivo em modo append

    if (!outFile) {
        cerr << "Erro ao abrir o arquivo para escrita!" << endl;
        return;
    }
    outFile << "\nOrigem: " << source;
    outFile << "\nTour: ";
    for (int i = 0; i < tour.path.size(); i++) {
        outFile << tour.path[i] << " ";
    }
    
    outFile << "\nNot visited: ";
    for (int i = 0; i < notVisited.size(); i++) {
        outFile << notVisited[i] << " ";
    }

    outFile << "\nCost: " << tour.cost;
    outFile << "\nPrize: " << tour.prize;
    

    outFile.close(); // Fecha o arquivo corretamente
}