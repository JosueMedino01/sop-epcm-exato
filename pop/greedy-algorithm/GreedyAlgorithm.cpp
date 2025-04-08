#include "GreedyAlgorithm.h"
#include <iostream>
#include <algorithm>
#include "../utils/Structs.h"

double GreedyAlgorithm::costBenefit (InstanceData data, int i, int j, double C) {
    return data.prize[j] - C * data.cost[i][j];
}

Customers GreedyAlgorithm::kNeighborRandomInsertion(InstanceData data, int K, double C) {
    int n = data.size;


    Tour feasibleTour; 
    feasibleTour.path.push_back(0);
    
    vector<bool> visited(n, false);
    visited[0] = true;

    while (feasibleTour.path.size() < n &&(feasibleTour.cost + data.cost[feasibleTour.path.back()][0] <= data.deadline)) 
    {
        int last = feasibleTour.path.back();  
        vector<Candidate> candidateList;  

        for (int i = 0; i < n; i++) 
        {
            if (!visited[i]) 
            {
                double attraction = costBenefit(data, last, i, C);
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

            feasibleTour.path.push_back(nextCustormer);
            feasibleTour.cost += data.cost[last][nextCustormer];
            feasibleTour.prize += data.prize[nextCustormer];

            visited[nextCustormer] = true;
        };
    }

    if(feasibleTour.cost > data.deadline) {
        int removed = feasibleTour.path.back();
        feasibleTour.path.pop_back();
        feasibleTour.cost -= data.cost[feasibleTour.path.back()][removed];
        feasibleTour.prize -= data.prize[removed];

        visited[removed] = false;
    }

    feasibleTour.cost += data.cost[feasibleTour.path.back()][0];
    feasibleTour.path.push_back(0);

    vector<int> notVisited;
    for (int i = 0; i < visited.size(); i++) {
        if (!visited[i]) {
            notVisited.push_back(i);
        }
    }
    
    
    return {feasibleTour, notVisited};
}
