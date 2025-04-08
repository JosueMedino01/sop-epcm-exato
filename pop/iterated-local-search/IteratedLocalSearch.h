#ifndef ITERATEDLOCALSEARCH_H
#define ITERATEDLOCALSEARCH_H

#pragma once

#include "../utils/Structs.h"

class IteratedLocalSearch
{
    private:
        int MAX_NOT_IMPROVIMENT, K, SEED; 
        double C;
        
        
       
        /* void localSearch(InstanceData &data, Customers &solution, double C);
        void perturbation(InstanceData &data, Customers &solution, double C); */
    public:
        Customers bestSolution; 
        IteratedLocalSearch(int MXI, int K, double C, int SEED);
        
        void run(InstanceData data, int K, double C);
       
};

#endif