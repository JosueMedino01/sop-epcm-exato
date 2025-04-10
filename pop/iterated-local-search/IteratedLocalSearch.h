#ifndef ITERATEDLOCALSEARCH_H
#define ITERATEDLOCALSEARCH_H

#pragma once

#include "../utils/Structs.h"

class IteratedLocalSearch
{
    private:
        int MAX_NOT_IMPROVIMENT, K, SEED; 
        double C;
        double objcFunc(double sumPrize, double sumCost);
        void printData(Tour tour, vector<int> notVisited, string source);

        bool shiftOneZero(InstanceData &data, Customers &customers);
        bool swapOneOne(InstanceData &data, Customers &customers);
        bool reinsertion(InstanceData &data, Customers &customers);
        bool twoOpt(InstanceData &data, Customers &customers);
        
        void localSearch(InstanceData &data, Customers &customers);
        Customers doubleBridge(InstanceData &data, Customers customers);
    public:
        Customers bestSolution; 
        IteratedLocalSearch(int MXI, int K, double C, int SEED);
        
        void run(InstanceData data, int K, double C);
       
};

#endif