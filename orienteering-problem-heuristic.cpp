#include <iostream>
#include "readInstances.h"
#include <algorithm>

using namespace std;

double C = 0.001;
int K = 2, TABU_TENURE = 10, MAX_ITER = 1000;

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


void printData(Tour tour, vector<int> notVisited) {
    cout << "Tour: " << endl;
    for (int i = 0; i < tour.tour.size(); i++)
    {
        cout << tour.tour[i] << " ";
    }
    cout<< endl<<  "Not viseted: " << endl;
    for (int i = 0; i < notVisited.size(); i++)
    {
        cout << notVisited[i] << " ";
    }
    cout << endl;
    cout << "Cost: " << tour.cost << endl;
    cout << "Prize: " << tour.prize << endl << endl;
}

double partialObjecFunc (readInstances::DataOP data, int i, int j) {
    return data.prize[j] - C * data.cost[i][j];
}

double objectiveFunction(double sumPrize, double sumCost) {
    return sumPrize - C * sumCost;
}

Tour extractFeasibleTour(Tour completeTour, readInstances::DataOP data) {
    Tour feasibleTour;
    feasibleTour.tour.push_back(0);
    feasibleTour.cost = 0;
    feasibleTour.prize = 0;

    int i = 1;
    while (i < data.nCustomers && 
        (feasibleTour.cost + data.cost[completeTour.tour[i - 1]][completeTour.tour[i]] + data.cost[completeTour.tour[i]][0]) <= data.deadline) 
    {
        feasibleTour.tour.push_back(completeTour.tour[i]);
        feasibleTour.cost += data.cost[completeTour.tour[i - 1]][completeTour.tour[i]];
        feasibleTour.prize += data.prize[completeTour.tour[i]];
        i++;
    }
    

    feasibleTour.cost += data.cost[feasibleTour.tour.back()][0];
    feasibleTour.tour.push_back(0);

    return feasibleTour;
}




pair<Tour, vector<int>> k_attractiveness_random_insertion(readInstances::DataOP data) { 
    int n = data.nCustomers;
    Tour feasibleTour; 
    feasibleTour.tour.push_back(0);
    
    vector<bool> visited(n, false);
    visited[0] = true;

    srand(time(0)); /* add parametro */

    while (feasibleTour.tour.size() < n && feasibleTour.cost + data.cost[feasibleTour.tour.back()][0] <= data.deadline) 
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
            int k = min(K, (int)candidateList.size() - 1);
            int nextCustormer = candidateList[k].id;

            feasibleTour.tour.push_back(nextCustormer);
            feasibleTour.cost += data.cost[last][nextCustormer];
            feasibleTour.prize += data.prize[nextCustormer];

            visited[nextCustormer] = true;
        };
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
            double newEdges = data.cost[tour.tour[j - 1]][addedNode];
            double newCost = tour.cost - oldEdges + newEdges;

            int newPrize = tour.prize + data.prize[addedNode];

            if (newCost <= data.deadline && !isTabu(addedNode, addedNode, tabuList)) {
                bestTour.tour = tour.tour;
                bestTour.tour.insert(bestTour.tour.begin() + j, addedNode);
                bestTour.cost = newCost;
                bestTour.prize = newPrize;

                
                bestNotVisited = notVisited;
                bestNotVisited.erase(bestNotVisited.begin() + k);

                move = {addedNode, addedNode};
            }
        }
    }

    notVisited = bestNotVisited;

    return {bestTour, move};
}


Tour tabuSearch(readInstances::DataOP data) 
{  
    vector<vector<int>> tabuList(data.nCustomers, vector<int>(data.nCustomers, 0));
    
    pair<Tour, vector<int>> customers = k_attractiveness_random_insertion(data);
    Tour currSolution = customers.first, bestSolution = customers.first;
    vector<int> notVisited = customers.second;
   
    cout << "Initial solution: " << endl;
    printData(currSolution, notVisited);
    int iter = 0, bestIter = 0;

    while (iter - bestIter <= MAX_ITER) 
    {   
        cout <<  "Iteração " << iter << "bestIter " << bestIter << " - "<<  iter - bestIter <<" SEM MELHORAR" << endl;
        iter++;
        pair<Tour, pair<int, int>> response = exchangeMove(currSolution, notVisited, tabuList, data);
        pair<int, int> moved = response.second;
        currSolution = response.first;
        
        cout << "moved " << moved.first << " " << moved.second << endl;
        printData(currSolution, notVisited);
      

        if (moved.first != -1)
        {
            pair<Tour, pair<int, int>> dataInsert = insertChange(currSolution, notVisited, tabuList, data);
            moved = dataInsert.second;
            currSolution = dataInsert.first;
            cout << "moved " << moved.first << " " << moved.second << endl;
            printData(currSolution, notVisited);
        }

        if (moved.first != -1) {
            cout << "Não ocorreu melhora" << endl;
            return bestSolution;
        }

        decrementTabuList(tabuList);

        addTabu(moved.first, moved.second, tabuList);
       
        
    
        if(objectiveFunction(currSolution.prize, currSolution.cost) > objectiveFunction(bestSolution.prize, bestSolution.cost)) 
        {  
            bestSolution = currSolution;
            bestIter = iter;
        }
    }

    return bestSolution;
}

Tour ILS(readInstances::DataOP data) {

    
}


 
int main() 
{  
    readInstances::DataOP data = readInstances::readFile("./instancias/quality/instances/berlin52FSTCII_q2_g4_p40_r20_s20_rs15.pop");
    
    Tour bestSolution = ILS(data);
    cout << "Best solution: " << endl;
    printData(bestSolution, {});


    return 0;
}
