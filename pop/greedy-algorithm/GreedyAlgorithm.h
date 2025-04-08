#ifndef GREEDYALGORITHM_H
#define GREEDYALGORITHM_H

#pragma once

#include "../utils/Structs.h"

class GreedyAlgorithm
{   
    private:
        double costBenefit (InstanceData data, int i, int j, double C);

    public:
        Customers kNeighborRandomInsertion(InstanceData data, int K, double C);

};

#endif