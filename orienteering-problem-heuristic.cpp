#include <iostream>
#include "readInstances.h"
#include <algorithm>

using namespace std;

double C = 0.001;
int K = 2, TABU_TENURE = 30, MAX_ITER = 1000;

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


void printData(Tour tour) {
    for (int i = 0; i < tour.tour.size(); i++)
    {
        cout << tour.tour[i] << " ";
    }
    cout << endl;
    cout << "Cost: " << tour.cost << endl;
    cout << "Prize: " << tour.prize << endl << endl;
}

double partialObjecFunc (readInstances::DataOP data, int i, int j) {
    return data.prize[j] - C * data.cost[i][j];
}

double objectiveFunction(Tour tour) {
    return tour.prize - C * tour.cost;
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
    for (int node: visited)
    {
        if(!node) {
            notVisited.push_back(node);
        }
    }
    
    
    return {feasibleTour, notVisited};
}

pair<Tour, pair<int, int>> two_opt (Tour tour, Tour bestTour, readInstances::DataOP data, vector<vector<int>> &tabuList) {
    double bestLocalCost = 0;
    Tour bestFeasibleLocalTour = extractFeasibleTour(tour, data);
    Tour bestTourFeasible = extractFeasibleTour(bestTour, data);

    pair<int, int> bestMove = {-1, -1};

    for (int i = 1  ; i < tour.tour.size(); i++) {
        for (int j = i + 1; j < tour.tour.size() - 1; j++) {
            Tour newFeasibleTour = tour;
            reverse(newFeasibleTour.tour.begin() + i, newFeasibleTour.tour.begin() + j + 1);

            double edgeRemoved  =  data.cost[tour.tour[i - 1]][tour.tour[i]] + data.cost[tour.tour[j]][tour.tour[j + 1]];
            double edgeAdded = data.cost[tour.tour[i - 1]][tour.tour[j]] + data.cost[tour.tour[i]][tour.tour[j + 1]];
            double newCost = tour.cost - edgeRemoved + edgeAdded;

            newFeasibleTour = extractFeasibleTour(newFeasibleTour, data);
           
            double ObjFuncNewFeasibleTour = objectiveFunction(newFeasibleTour);
            double ObjFuncBestFeasibleLocalTour = objectiveFunction(bestFeasibleLocalTour);

            double objBestFeasibleTour = objectiveFunction(bestTourFeasible);

            if (ObjFuncNewFeasibleTour > ObjFuncBestFeasibleLocalTour && (!isTabu(tour.tour[i], tour.tour[j], tabuList)  
                || ObjFuncBestFeasibleLocalTour > objBestFeasibleTour)) 
            {
                bestMove = {i, j};
                bestFeasibleLocalTour = newFeasibleTour;
                bestLocalCost = newCost;
                printData(bestFeasibleLocalTour);
            }
        }
    }

    
    if (bestMove.first != -1 && bestMove.second != -1) {
        reverse(tour.tour.begin() + bestMove.first, tour.tour.begin() + bestMove.second + 1);
        tour.cost = bestLocalCost;
    }

    return {tour, bestMove};
}



Tour tabuSearch(readInstances::DataOP data) 
{
    vector<vector<int>> tabuList(data.nCustomers, vector<int>(data.nCustomers, 0));
    
    pair<Tour, vector<int>> s0 = k_attractiveness_random_insertion(data);
    Tour currSolution = s0.first, bestSolution = s0.first;

    int iter = 0, bestIter = 0;

    while (iter - bestIter <= MAX_ITER) 
    {
        iter++;
        pair<Tour, pair<int, int>> tourAndMove = two_opt(currSolution, bestSolution, data, tabuList);
        currSolution = tourAndMove.first;

        decrementTabuList(tabuList);

        if(objectiveFunction(extractFeasibleTour(currSolution, data)) > objectiveFunction(extractFeasibleTour(bestSolution, data)) ) 
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



 
int main() 
{  
    readInstances::DataOP data = readInstances::readFile("./instancias/quality/instances/berlin52FSTCII_q2_g4_p40_r20_s20_rs15.pop");
    
    ofstream outFile("cost_matrix.txt");
    for (int i = 0; i < data.nCustomers; ++i) {
        for (int j = 0; j < data.nCustomers; ++j) {
            outFile << "(" << i << "," << j << ") = " << data.cost[i][j] << endl;
        }
    }
    outFile.close();

  

    Tour bestSolution = tabuSearch(data);


   /*  cout << "Best complete tour: "<< endl;
    printData(bestSolution); */
    
    /* Tour feasibleTour = extractFeasibleTour(bestSolution, data);
    cout << "Feasible tour: " << endl;
    printData(feasibleTour);
    cout << "Deadline" << data.deadline << endl; */
    
    return 0;
}