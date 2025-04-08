#include <iostream>
#include <string.h>

#include "greedy-algorithm/GreedyAlgorithm.h"
#include "utils/Structs.h"
#include "utils/ReadInstance.h"
void printData(Tour tour, vector<int> notVisited) {
    ofstream outFile("solution_log.txt", ios::app); // Abre o arquivo em modo append

    if (!outFile) {
        cerr << "Erro ao abrir o arquivo para escrita!" << endl;
        return;
    }

    outFile << "Tour: ";
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
int main(int argc, char *argv[]) {
    string filename = "";
    int seed, K, TABU_TENURE, MAX_ITER_TABU, MAX_NOT_IMPROVIMENT;
    double C;

    for (int i = 0; i < argc; i++)
    {
        if (!strcmp(argv[i], "-f"))
        {
            filename = argv[i + 1];
        }
        else if (!strcmp(argv[i], "-C"))
        {
            C = atof(argv[i + 1]);
        }
        else if (!strcmp(argv[i], "-K"))
        {
            K = atoi(argv[i + 1]);
        }
        else if (!strcmp(argv[i], "-T"))
        {
            TABU_TENURE = atoi(argv[i + 1]);
        }
        else if (!strcmp(argv[i], "-maxIterTabu"))
        {
            MAX_ITER_TABU = atoi(argv[i + 1]);
        }
        else if (!strcmp(argv[i], "-maxNotImproviment"))
        {
            MAX_NOT_IMPROVIMENT = atoi(argv[i + 1]);
        } else if (!strcmp(argv[i], "-seed")) {
            seed = atoi(argv[i + 1]);
        }
        
    }

    srand(seed); 

    InstanceData data = readFile(filename);
    GreedyAlgorithm greedy;
    Customers customers = greedy.kAttractivenessRandomInsertion(data, K, C);
    printData(customers.feasibleTour, customers.notVisited);
    return 0;
}