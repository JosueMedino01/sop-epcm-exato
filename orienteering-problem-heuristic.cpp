#include <iostream>
#include "readInstances.h"
#include <algorithm>

using namespace std;

double C = 0.001;
int K = 52, TABU_TENURE = 50, MAX_ITER = 1000;

struct Tour
{
    vector<int> tour;
    double cost = 0;
    int prize = 0;
};

struct Candidate {
    int attraction;
    int id;
};

struct Customers {
    Tour featsibleTour;
    vector<int> notVisited;
};

void decrementTabuList(vector<vector<int>> &tabuList) {
    for (int i = 0; i < tabuList[0].size(); ++i) {
        for (int j = 0; j < tabuList[0].size(); ++j) {
            if (tabuList[i][j] > 0) {
                tabuList[i][j]--;
            }
        }
    }
}

void addTabu(int i, int j, vector<vector<int>> &tabuList) {
    tabuList[i][j] = TABU_TENURE;
    tabuList[j][i] = TABU_TENURE;
}

bool isTabu(int i, int j, vector<vector<int>> &tabuList) {  
    return tabuList[i][j] > 0;
}





double partialObjecFunc (readInstances::DataOP data, int i, int j) {
    return data.prize[j] - C * data.cost[i][j];
}

double objectiveFunction(double sumPrize, double sumCost) {
    return sumPrize - C * sumCost;
}

void printData(Tour tour, vector<int> notVisited) {
    ofstream outFile("solution_log.txt", ios::app); // Abre o arquivo em modo append

    if (!outFile) {
        cerr << "Erro ao abrir o arquivo para escrita!" << endl;
        return;
    }

    outFile << "Tour: ";
    for (int i = 0; i < tour.tour.size(); i++) {
        outFile << tour.tour[i] << " ";
    }
    
    outFile << "\nNot visited: ";
    for (int i = 0; i < notVisited.size(); i++) {
        outFile << notVisited[i] << " ";
    }

    outFile << "\nCost: " << tour.cost;
    outFile << "\nPrize: " << tour.prize;
    outFile << "\nObjetive Function: " << objectiveFunction(tour.prize, tour.cost) << "\n\n";

    outFile.close(); // Fecha o arquivo corretamente
}

pair<Tour, vector<int>> k_attractiveness_random_insertion(readInstances::DataOP data) { 
    int n = data.nCustomers;
    Tour feasibleTour; 
    feasibleTour.tour.push_back(0);
    
    vector<bool> visited(n, false);
    visited[0] = true;

    while (feasibleTour.tour.size() < n &&(feasibleTour.cost + data.cost[feasibleTour.tour.back()][0] <= data.deadline)) 
    {
        int last = feasibleTour.tour.back();  
        vector<Candidate> candidateList;  

        for (int i = 0; i < n; i++) 
        {
            if (!visited[i]) 
            {
                int attraction = partialObjecFunc(data, last, i);
                Candidate newCandidate = {attraction, i};
                candidateList.emplace_back(newCandidate);
            }
        }

        sort(candidateList.begin(), candidateList.end(), 
            [](const Candidate& a, const Candidate& b) {
                return a.attraction > b.attraction; 
            }
        );
        
        if (candidateList.size() > 0) {
            int k = min(K, (int)candidateList.size());
            if (k == 0) break;  // Avoid invalid access
            int randomNumber = rand() % k;
            int nextCustormer = candidateList[randomNumber].id;

            if (feasibleTour.cost + data.cost[last][nextCustormer] + data.cost[nextCustormer][0] > data.deadline) {
                break; // Impede que o novo nó ultrapasse o deadline
            }

            feasibleTour.tour.push_back(nextCustormer);
            feasibleTour.cost += data.cost[last][nextCustormer];
            feasibleTour.prize += data.prize[nextCustormer];

            visited[nextCustormer] = true;
        };
    }

    if(feasibleTour.cost > data.deadline) {
        int removed = feasibleTour.tour.back();
        feasibleTour.tour.pop_back();
        feasibleTour.cost -= data.cost[feasibleTour.tour.back()][removed];
        feasibleTour.prize -= data.prize[removed];

        visited[removed] = false;
    }

    feasibleTour.cost += data.cost[feasibleTour.tour.back()][0];
    feasibleTour.tour.push_back(0);

    vector<int> notVisited;
    for (int i = 0; i < visited.size(); i++) {
        if (!visited[i]) {
            notVisited.push_back(i);
        }
    }
    
    
    return {feasibleTour, notVisited};
}

pair<Tour, pair<int, int>> exchangeMove(Tour tour, vector<int> &notVisited, vector<vector<int>> &tabuList, readInstances::DataOP data) {
    Tour bestTour = tour;
    vector<int> bestNotVisited = notVisited;
    pair<int, int> move = {-1, -1};
 
    for (size_t j = 1; j < tour.tour.size() - 1; j++) { 
        for (size_t k = 0; k < notVisited.size(); k++) { 
            int removedNode = tour.tour[j];
            int addedNode = notVisited[k];

            double oldEdges = data.cost[tour.tour[j - 1]][tour.tour[j]] + data.cost[tour.tour[j]][tour.tour[j + 1]];
            double newEdges = data.cost[tour.tour[j - 1]][addedNode] + data.cost[addedNode][tour.tour[j + 1]];
            double newCost = tour.cost - oldEdges + newEdges;

            int newPrize = tour.prize + data.prize[addedNode] - data.prize[removedNode];

            if (newCost <= data.deadline && objectiveFunction(newPrize, newCost) > objectiveFunction(bestTour.prize, bestTour.cost) && !isTabu(removedNode, addedNode, tabuList)) {
                bestTour.tour = tour.tour;
                bestTour.tour[j] = addedNode; 
                bestTour.cost = newCost;
                bestTour.prize = newPrize;

                bestNotVisited = notVisited;
                bestNotVisited[k] = removedNode;

                move = {removedNode, addedNode};
            }
        }
    }

    notVisited = bestNotVisited;

    return {bestTour, move};
}

pair<Tour, pair<int, int>> insertChange(Tour tour, vector<int> &notVisited, vector<vector<int>> &tabuList, readInstances::DataOP data) {
    Tour bestTour = tour;
    vector<int> bestNotVisited = notVisited;
    pair<int, int> move = {-1, -1};

    for (size_t j = 1; j < tour.tour.size() - 1; j++) {
        for (size_t k = 0; k < notVisited.size(); k++) {
            int addedNode = notVisited[k];

            double oldEdges = data.cost[tour.tour[j - 1]][tour.tour[j]];
            double newEdges = data.cost[tour.tour[j - 1]][addedNode] + data.cost[addedNode][tour.tour[j]]; 
            double newCost = tour.cost - oldEdges + newEdges;

            int newPrize = tour.prize + data.prize[addedNode];

            if (newCost <= data.deadline && 
                objectiveFunction(newPrize, newCost) > objectiveFunction(bestTour.prize, bestTour.cost) && 
                !isTabu(tour.tour[j], addedNode, tabuList)) {
                bestTour.tour = tour.tour;
                bestTour.tour.insert(bestTour.tour.begin() + j, addedNode);
                bestTour.cost = newCost;
                bestTour.prize = newPrize;

                
                bestNotVisited = notVisited;
                bestNotVisited.erase(bestNotVisited.begin() + k);

                move = {tour.tour[j], addedNode};
            }
        }
    }

    notVisited = bestNotVisited;

    return {bestTour, move};
}


pair<Tour, vector<int>> tabuSearch(readInstances::DataOP data, pair<Tour, vector<int>> customers) 
{  
    vector<vector<int>> tabuList(data.nCustomers, vector<int>(data.nCustomers, 0));
    

    Tour currSolution = customers.first, bestSolution = customers.first;
    vector<int> notVisited = customers.second;
   
    int iter = 0, bestIter = 0;

    while (iter - bestIter <= MAX_ITER) 
    {   
        iter++;
        pair<Tour, pair<int, int>> response = exchangeMove(currSolution, notVisited, tabuList, data);
        pair<int, int> moved = response.second;
        currSolution = response.first;
      

        if (moved.first == -1)
        {
            pair<Tour, pair<int, int>> dataInsert = insertChange(currSolution, notVisited, tabuList, data);
            moved = dataInsert.second;
            currSolution = dataInsert.first;
        }

        if (moved.first == -1) {
            cout << "Não ocorreu melhora" << endl;
            return {bestSolution, notVisited};
        }

        decrementTabuList(tabuList);

        addTabu(moved.first, moved.second, tabuList);
       
        
    
        if(objectiveFunction(currSolution.prize, currSolution.cost) > objectiveFunction(bestSolution.prize, bestSolution.cost)) 
        {  
            bestSolution = currSolution;
            bestIter = iter;
        }
    }

    return {bestSolution, notVisited};
}

pair<Tour, vector<int>> ILS(readInstances::DataOP data) {
    pair<Tour, vector<int>> customers = k_attractiveness_random_insertion(data);
    pair<Tour, vector<int>> bestSolution = tabuSearch(data, customers);

    int MAX_NOT_IMPROVIMENT = 1000, iter = 0;

    while (iter < MAX_NOT_IMPROVIMENT)
    {   
        pair<Tour, vector<int>> pertubSolutin = k_attractiveness_random_insertion(data);
        pair<Tour, vector<int>> newSolution = tabuSearch(data, pertubSolutin);

        if(objectiveFunction(newSolution.first.prize, newSolution.first.cost) > objectiveFunction(bestSolution.first.prize, bestSolution.first.cost)) 
        {  
            bestSolution = newSolution;
            iter = 0;
        }

        iter++;
    }

    return bestSolution;

}


 
int main() 
{  
    srand(time(0)); /* add parametro */
    readInstances::DataOP data = readInstances::readFile("./instancias/quality/instances/berlin52FSTCII_q2_g4_p40_r20_s20_rs15.pop");
    
    /* pair<Tour, vector<int>> bestSolution = ILS(data);
    cout << "Best solution: " << endl;
    printData(bestSolution.first, bestSolution.second); */

   /*  pair<Tour, vector<int>> customers = k_attractiveness_random_insertion(data);
    cout << "Solução inicial" << endl;
    printData(customers.first, customers.second);


    pair<Tour, vector<int>> localSearch = tabuSearch(data, customers);
    cout << "Best solution: " << endl;
    printData(localSearch.first, localSearch.second); */

    pair<Tour, vector<int>> customers = ILS(data);
    cout << "Best solution: " << endl;
    printData(customers.first, customers.second);
    return 0;
}
