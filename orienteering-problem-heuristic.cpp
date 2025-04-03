#include <iostream>
#include "readInstances.h"
#include <algorithm>
#include <string.h>

using namespace std;

double C;
int K, TABU_TENURE, MAX_ITER_TABU, MAX_NOT_IMPROVIMENT;
int contador = 0;

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

void addTabu(int i, int j, vector<vector<int>> &tabuList, int iter) {
    tabuList[i][j] = iter;
    tabuList[j][i] = iter;
}

bool isTabu(int i, int j, vector<vector<int>> &tabuList, int iter) {  
    return iter - tabuList[i][j] < TABU_TENURE;
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

pair<Tour, pair<int, int>> exchangeMove(Tour tour, vector<int> &notVisited, vector<vector<int>> &tabuList, readInstances::DataOP data, int iter) {
    Tour bestTour = tour;
    vector<int> bestNotVisited = notVisited;
    pair<int, int> move = {-1, -1};
 
    for (size_t j = 1; j < tour.tour.size() - 1; j++) { 
        for (size_t k = 0; k < notVisited.size(); k++) { 

            contador = contador + 1;
            
            int removedNode = tour.tour[j];
            int addedNode = notVisited[k];

            double oldEdges = data.cost[tour.tour[j - 1]][tour.tour[j]] + data.cost[tour.tour[j]][tour.tour[j + 1]];
            double newEdges = data.cost[tour.tour[j - 1]][addedNode] + data.cost[addedNode][tour.tour[j + 1]];
            double newCost = tour.cost - oldEdges + newEdges;

            int newPrize = tour.prize + data.prize[addedNode] - data.prize[removedNode];

            if (newCost <= data.deadline && objectiveFunction(newPrize, newCost) > objectiveFunction(bestTour.prize, bestTour.cost) && !isTabu(removedNode, addedNode, tabuList, iter)) {
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

pair<Tour, pair<int, int>> insertChange(Tour tour, vector<int> &notVisited, vector<vector<int>> &tabuList, readInstances::DataOP data, int iter) {
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
                !isTabu(tour.tour[j], addedNode, tabuList, iter)) {
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


pair<Tour, pair<int, int>> two_opt (Tour tour, Tour bestTour, readInstances::DataOP data, vector<vector<int>> &tabuList, int iter) {
    double bestLocalCost = 0;
    pair<int, int> bestMove = {-1, -1};

    for (int i = 1  ; i < tour.tour.size(); i++) {
        for (int j = i + 1; j < tour.tour.size() - 1; j++) {
            double edgeRemoved  =  data.cost[tour.tour[i - 1]][tour.tour[i]] + data.cost[tour.tour[j]][tour.tour[j + 1]];
            double edgeAdded = data.cost[tour.tour[i - 1]][tour.tour[j]] + data.cost[tour.tour[i]][tour.tour[j + 1]];
            double newCost = tour.cost - edgeRemoved + edgeAdded;

        

            if (newCost < tour.cost && !isTabu(tour.tour[i], tour.tour[j], tabuList, iter))
            {
                bestMove = {i, j};
                bestLocalCost = newCost;
            }
        }
    }

    
    if (bestMove.first != -1 && bestMove.second != -1) {
        reverse(tour.tour.begin() + bestMove.first, tour.tour.begin() + bestMove.second + 1);
        tour.cost = bestLocalCost;
    }

    return {tour, bestMove};
}

pair<Tour, vector<int>> tabuSearch(readInstances::DataOP data, pair<Tour, vector<int>> customers) 
{  
    vector<vector<int>> tabuList(data.nCustomers, vector<int>(data.nCustomers, 0));
    

    Tour currSolution = customers.first, bestSolution = customers.first;
    vector<int> notVisited = customers.second;
   
    int iter = 0, notImprovement = 0;

    while (iter <= MAX_ITER_TABU) 
    {       
 
        pair<Tour, pair<int, int>> response = exchangeMove(currSolution, notVisited, tabuList, data, iter);
        pair<int, int> moved = response.second;
        currSolution = response.first;
      

        if (moved.first == -1)
        {
            pair<Tour, pair<int, int>> dataInsert = insertChange(currSolution, notVisited, tabuList, data, iter);
            moved = dataInsert.second;
            currSolution = dataInsert.first;
        }

        if(moved.first == -1) {
            pair<Tour, pair<int, int>> dataTwoOpt = two_opt(currSolution, bestSolution, data, tabuList, iter);
            moved = dataTwoOpt.second;
            currSolution = dataTwoOpt.first;
        }

        if (moved.first == -1) notImprovement++;
        else addTabu(moved.first, moved.second, tabuList, iter);

        if(notImprovement == 100) return {bestSolution, notVisited};
        
    
        if(objectiveFunction(currSolution.prize, currSolution.cost) > objectiveFunction(bestSolution.prize, bestSolution.cost)) 
        {  
            bestSolution = currSolution;
        }       
        iter++;
    }

    return {bestSolution, notVisited};
}


pair<Tour, vector<int>> doubleBridgeA (Tour feasibleTour, readInstances::DataOP data, vector<int> notVisited){
    int cuts[4], n = feasibleTour.tour.size();
    cout << "SIZE: " << n << endl;
    int partSize = n / 4;

    cuts[0] = 1 + rand() % partSize;      
    cuts[1] = partSize + rand() % partSize;
    cuts[2] = 2 * partSize + rand() % partSize;
    cuts[3] = 3 * partSize + rand() % (n - 3 * partSize - 1); 

    vector<int>& t = feasibleTour.tour;
    reverse(t.begin() + cuts[0], t.begin() + cuts[1]); 
    reverse(t.begin() + cuts[2], t.begin() + cuts[3]);

    double added = data.cost[cuts[0]][cuts[2]-1] + data.cost[cuts[0] - 1][cuts[2]] + 
                data.cost[cuts[1] - 1][cuts[3]] + data.cost[cuts[1]][cuts[3]-1];

    double deleted = data.cost[cuts[0]][cuts[0] - 1] +  data.cost[cuts[1]][cuts[1] - 1] + 
                data.cost[cuts[2]][cuts[2] - 1] +  data.cost[cuts[3]][cuts[3] - 1];

    feasibleTour.cost = feasibleTour.cost + added - deleted;     

    while (feasibleTour.cost + data.cost[feasibleTour.tour.back()][0] > data.deadline)
    { 
        int last = feasibleTour.tour.back();
        feasibleTour.tour.pop_back();
    
        feasibleTour.cost -= data.cost[feasibleTour.tour.back()][last];
        feasibleTour.prize -= data.prize[last];
        
        if(last != 0) 
            notVisited.push_back(last);
    }
    
    feasibleTour.cost += data.cost[feasibleTour.tour.back()][0];
   
    return {feasibleTour, notVisited};
}

pair<Tour, vector<int>> doubleBridge (Tour feasibleTour, readInstances::DataOP data, vector<int> notVisited){
    feasibleTour.tour.pop_back(); 
    feasibleTour.cost -= data.cost[feasibleTour.tour.back()][0];
    
    int n = feasibleTour.tour.size();
    if (n < 8) return {feasibleTour, notVisited};
    
    int partSize = max(1, (n - 4) / 4);
    
    int cuts[4];
    cuts[0] = 1 + rand() % partSize;
    cuts[1] = cuts[0] + 1 + rand() % partSize;
    cuts[2] = cuts[1] + 1 + rand() % partSize;
    cuts[3] = cuts[2] + 1 + rand() % (n - cuts[2] - 2);

    // Construir novo tour na ordem: A-D-C-B-E
    vector<int> newTour;
    // Parte A (0 até cuts[0])
    newTour.insert(newTour.end(), feasibleTour.tour.begin(), feasibleTour.tour.begin() + cuts[0]);
    // Parte D (cuts[3] até fim)
    newTour.insert(newTour.end(), feasibleTour.tour.begin() + cuts[3], feasibleTour.tour.end());
    // Parte C (cuts[2] até cuts[3])
    newTour.insert(newTour.end(), feasibleTour.tour.begin() + cuts[2], feasibleTour.tour.begin() + cuts[3]);
    // Parte B (cuts[1] até cuts[2])
    newTour.insert(newTour.end(), feasibleTour.tour.begin() + cuts[1], feasibleTour.tour.begin() + cuts[2]);
    // Parte E (cuts[0] até cuts[1])
    newTour.insert(newTour.end(), feasibleTour.tour.begin() + cuts[0], feasibleTour.tour.begin() + cuts[1]);

    // Recalcular o custo total (mais seguro que tentar ajustar)
    double newCost = 0;
    int newPrize = 0;
    for (int i = 1; i < newTour.size(); i++) {
        newCost += data.cost[newTour[i-1]][newTour[i]];
    }
    for (int i = 1; i < newTour.size()-1; i++) {
        newPrize += data.prize[newTour[i]];
    }

    feasibleTour.tour = newTour;
    feasibleTour.cost = newCost;
    feasibleTour.prize = newPrize;

    // Verificar deadline e remover nós se necessário
    while (feasibleTour.cost + data.cost[feasibleTour.tour.back()][0] > data.deadline && feasibleTour.tour.size() > 2) {
        int last = feasibleTour.tour.back();
        feasibleTour.tour.pop_back();
        feasibleTour.cost -= data.cost[feasibleTour.tour.back()][last];
        if(last != 0) {
            feasibleTour.prize -= data.prize[last];
            notVisited.push_back(last);
        }
    }

    feasibleTour.cost += data.cost[feasibleTour.tour.back()][0];
    feasibleTour.tour.push_back(0); // Adiciona o nó de origem novamente
    
    return {feasibleTour, notVisited};
}
pair<Tour, vector<int>> ILS(readInstances::DataOP data) {
    pair<Tour, vector<int>> customers = k_attractiveness_random_insertion(data);
    cout << "solucao inicial" << endl;
    printData(customers.first, customers.second);
  
    pair<Tour, vector<int>> bestSolution = tabuSearch(data, customers);

    int iter = 0;

    while (iter < MAX_NOT_IMPROVIMENT)
    {   
        pair<Tour, vector<int>> pertubSolutin = doubleBridge(bestSolution.first, data, bestSolution.second);
        pair<Tour, vector<int>> newSolution = tabuSearch(data, pertubSolutin);
        
        if(objectiveFunction(newSolution.first.prize, newSolution.first.cost) > objectiveFunction(bestSolution.first.prize, bestSolution.first.cost)) 
        {  
            bestSolution = newSolution;
            printData(bestSolution.first, bestSolution.second);
            cout << "Houve melhora solucao. Iteração: " << iter << endl;
            iter = 0;
           
        }

        iter++;
    }
    printData(bestSolution.first, bestSolution.second);
    return bestSolution;

}


 
int main(int argc, char *argv[]) 
{  
    string filename = "";
    int seed;

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
    readInstances::DataOP data = readInstances::readFile(filename);

    pair<Tour, vector<int>> customers = ILS(data);
    cout << "contator " << contador << endl;
    return 0;
}
