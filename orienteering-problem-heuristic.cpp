#include <iostream>
#include "readInstances.h"
#include <algorithm>

using namespace std;

double C = 0.001;
int K = 2, TABU_TENURE = 15, MAX_ITER = 50;

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

double partialObjecFunc (readInstances::DataOP data, int i, int j) {
    return data.prize[j] - C * data.cost[i][j];
}

Tour k_attractiveness_random_insertion(readInstances::DataOP data) { 
    int n = data.nCustomers;
    Tour tour; 
    tour.tour.push_back(0);
    
    vector<bool> visited(n, false);
    visited[0] = true;

    srand(time(0));
    int it = 0;
    while (it < (n - tour.tour.size())) 
    {
        it++;
        int last = tour.tour.back();  
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

            tour.tour.push_back(nextCustormer);
            tour.cost += data.cost[last][nextCustormer];
            tour.prize += data.prize[nextCustormer];

            visited[nextCustormer] = true;
            it = 0;
        };
    }

    tour.cost += data.cost[tour.tour.back()][0];
    tour.tour.push_back(0);
    
    return tour;
}

pair<Tour, pair<int, int>> two_opt (Tour tour, Tour bestTour, readInstances::DataOP data, vector<vector<int>> &tabuList) {
    double bestCost = tour.cost;
    pair<int, int> bestMove = {-1, -1};

    for (int i = 1  ; i < tour.tour.size(); i++) {
        for (int j = i + 1; j < tour.tour.size() - 1; j++) {
            double edgeRemoved  =  data.cost[tour.tour[i - 1]][tour.tour[i]] + data.cost[tour.tour[j]][tour.tour[j + 1]];
            double edgeAdded = data.cost[tour.tour[i - 1]][tour.tour[j]] + data.cost[tour.tour[i]][tour.tour[j + 1]];
            double newCost = tour.cost - edgeRemoved + edgeAdded ;

            if (newCost < bestCost && (!isTabu(tour.tour[i], tour.tour[j], tabuList)  
                || newCost < bestTour.cost)) 
            {
                bestMove = {i, j};
                bestCost = newCost;
            }
        }
    }

    
    if (bestMove.first != -1 && bestMove.second != -1) {
        reverse(tour.tour.begin() + bestMove.first, tour.tour.begin() + bestMove.second + 1);
        tour.cost = bestCost;
    }

    return {tour, bestMove};
}

Tour selectFeasibleTour(Tour tour, readInstances::DataOP data) {
    while (tour.cost > data.deadline && tour.tour.size() > 2) {
        int worstCustomer = -1;
        double worstRatio = INFINITY; 

        for (int i = 1; i < tour.tour.size() - 1; i++) {
            int prev = tour.tour[i - 1];
            int curr = tour.tour[i];
            int next = tour.tour[i + 1];

            double costRemoved = data.cost[prev][curr] + data.cost[curr][next];
            double costAdded = data.cost[prev][next];
            double deltaCost = costAdded - costRemoved;

            double ratio = deltaCost / data.prize[curr] ;

            if (ratio < worstRatio) {
                worstRatio = ratio;
                worstCustomer = i;
            }
        }

        if (worstCustomer != -1) {
            int prev = tour.tour[worstCustomer - 1];
            int next = tour.tour[worstCustomer + 1];
            tour.cost -= data.cost[prev][tour.tour[worstCustomer]] + data.cost[tour.tour[worstCustomer]][next];
            tour.cost += data.cost[prev][next];
            tour.prize -= data.prize[tour.tour[worstCustomer]];
            tour.tour.erase(tour.tour.begin() + worstCustomer);
        }
    }
    return tour;
}

Tour tabuSearch(Tour s0, readInstances::DataOP data) 
{
    vector<vector<int>> tabuList(data.nCustomers, vector<int>(data.nCustomers, 0));

    Tour currSolution = s0;
    Tour bestSolution = s0;
    int iter = 0, bestIter = 0;

    while (iter - bestIter <= MAX_ITER) 
    {
        iter++;
        pair<Tour, pair<int, int>> tourAndMove =  two_opt(currSolution, bestSolution, data, tabuList);
        currSolution = tourAndMove.first;

        decrementTabuList(tabuList);

        if(currSolution.cost < bestSolution.cost) 
        {  
            addTabu(
                currSolution.tour[tourAndMove.second.first], 
                currSolution.tour[tourAndMove.second.second], 
                tabuList
            );
            bestSolution = currSolution;
            bestIter = iter;
        }

        
    }

    return bestSolution;
}

void printData(Tour tour) {
    for (int i = 0; i < tour.tour.size(); i++)
    {
        cout << tour.tour[i] << " ";
    }
    cout << endl;
    cout << "Cost: " << tour.cost << endl;
    cout << "Prize: " << tour.prize << endl << endl;
}

 
int main() 
{  
    readInstances::DataOP data = readInstances::readFile("./instancias/quality/instances/berlin52FSTCII_q2_g4_p05_r10_s10_rs5.pop");
    
    ofstream outFile("cost_matrix.txt");
    for (int i = 0; i < data.nCustomers; ++i) {
        for (int j = 0; j < data.nCustomers; ++j) {
            outFile << "(" << i << "," << j << ") = " << data.cost[i][j] << endl;
        }
    }
    outFile.close();

    Tour initialSolution = k_attractiveness_random_insertion(data);

    cout << "Initial tour complete: " << endl;
    printData(initialSolution);

    Tour bestSolution = tabuSearch(initialSolution, data);


    cout << "Best complete tour: "<< endl;
    printData(bestSolution);
    
    Tour feasibleTour = selectFeasibleTour(bestSolution, data);
    cout << "Feasible tour: " << endl;
    printData(feasibleTour);

    
    return 0;
}