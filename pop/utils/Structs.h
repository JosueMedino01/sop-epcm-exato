#ifndef STRUCTS_H
#define STRUCTS_H

#include <string>
#include <vector>

using namespace std;

struct Tour
{
    vector<int> path;
    double cost = 0;
    int prize = 0;
};

struct Candidate {
    double attraction;
    int id;
};

struct Customers {
    Tour feasibleTour;
    vector<int> notVisited;
};

struct InstanceData {
    int size;
    int deadline;
    vector<double> probability;
    vector<int> prize;
    vector<vector<double>> cost;
};
#endif
