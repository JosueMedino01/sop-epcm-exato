using namespace std;
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

struct Node {
    int id;
    double x, y;
    double prize;
    double probability;
};

vector<Node> nodes = {
    {1, 6734, 1453, 0, 1},
    {2, 2233, 1400, 4400, 0.5},
    {3, 5530, 1424, 3200, 0.5},
    {4, 401, 841, 3250, 0.5},
    {5, 3082, 1644, 3900, 0.5},
    {6, 7608, 4458, 4900, 0.5}
};

struct Neighbor {
    vector<Node> tour;
    int c1;
    int c2;
};
double D = 19000;
double C = 1;
int TABU_TENURE = 3;

double calculateDistance(Node node1, Node node2);
int getCost(vector<Node> tour);
int getPrize(vector<Node> tour);
int attractiveness (Node curr, Node candidate);

vector<Node> k_attractiveness_random_insertion();

void printData(vector<Node> tour);

void printTour(vector<Node> tour) {
    for (int i = 0; i < tour.size(); i++)
    {
        cout << tour[i].id << " -> ";
    }
    cout << endl;
}


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

int objectiveFunction(vector<Node> tour) {
    return getPrize(tour) - C * getCost(tour);
}

vector<Neighbor> two_opt (vector<Node> tour) {
    vector<Neighbor> neighbors;

    for (int i = 1  ; i < tour.size(); ++i) {
        for (int j = i + 1; j < tour.size() - 1; ++j) {
            Neighbor newNeighbor;
            newNeighbor.tour = tour;
            newNeighbor.c1 = i;
            newNeighbor.c2 = j; /* rever i e j pelos ids */

            reverse(newNeighbor.tour.begin() + i, newNeighbor.tour.begin() + j + 1);
        
            neighbors.push_back(newNeighbor);
        }
    }
    return neighbors;
}

vector<Node> bestNeighbor(vector<Neighbor> neighbors, vector<vector<int>> &tabuList, vector<Node> bestTour) {
    Neighbor localBest = neighbors[0];

    for (int i = 1; i < neighbors.size(); i++) {
        if (
            (objectiveFunction(neighbors[i].tour) > objectiveFunction(localBest.tour) && 
                !isTabu(neighbors[i].c1, neighbors[i].c2, tabuList)) || 
            (objectiveFunction(neighbors[i].tour) > objectiveFunction(bestTour) && 
                isTabu(neighbors[i].c1, neighbors[i].c2, tabuList)) 
        ) {
            localBest = neighbors[i];
            decrementTabuList(tabuList);
            addTabu(neighbors[i].c1, neighbors[i].c2, tabuList);
        }
    }   
    return localBest.tour;   
}



vector<Node> tabuList() {
    vector<Node> currSolution = k_attractiveness_random_insertion();
    vector<Node> betterSolution = currSolution;
    vector<vector<int>> tabuList(nodes.size(), vector<int>(nodes.size(), 0));
    int iter = 0, betteriter = 0, maxIter = 3;

    while (iter - betteriter <= maxIter) {
        iter++;
        vector<Neighbor> neighbors = two_opt(currSolution);
        currSolution = bestNeighbor(neighbors, tabuList, betterSolution);

        if(objectiveFunction(currSolution) > objectiveFunction(betterSolution)) {
            betterSolution = currSolution;
            betteriter = iter;
        }
    }

    return betterSolution;
}

int main () 
{   
    vector<Node> solution = tabuList();
    printData(solution);
  
    vector<Node> currSolution = k_attractiveness_insertion(); 
    printTour(currSolution);
    vector<Neighbor> neighbor = two_opt(currSolution);
    for (int i = 0; i < neighbor.size(); i++)
    {
        cout << "C1: " << neighbor[i].c1 << " C2: " << neighbor[i].c2 << endl;
        printTour(neighbor[i].tour);
    }
    
   
    
    return 0;
}

vector<Node> k_attractiveness_random_insertion() { 
    int n = nodes.size();
    vector<Node> tour; 
    tour.push_back(nodes[0]);
    
    vector<bool> visited(n, false);
    visited[0] = true;

    srand(time(0));
    int it = 0;
    while (
        getCost(tour) + calculateDistance(tour.back(), tour[0]) < D && 
        it < (n - tour.size()) 
    ) 
    {
        it++;
        Node last = tour.back();  
        vector<pair<int, Node>> neighbour;  

        for (int i = 0; i < n; i++) 
        {
            if (!visited[i]) 
            {
                int dist_curr_neighbor = calculateDistance(last, nodes[i]);
                int dist_neighbor_start = calculateDistance(nodes[i], tour[0]);
                int condition = getCost(tour) + dist_curr_neighbor + dist_neighbor_start;

                if (condition <= D) 
                {
                    int attraction = attractiveness(last, nodes[i]);
                    neighbour.emplace_back(attraction, nodes[i]);
                }
            }
        }

        sort(neighbour.begin(), neighbour.end(), 
        [](const pair<int, Node>& a, const pair<int, Node>& b) {
            return a.first > b.first; 
        });

        
        if (neighbour.size() > 0) {
            int k = min(3, (int)neighbour.size());
            Node nextNode = neighbour[rand() % k].second;

            tour.push_back(nextNode);
            visited[nextNode.id - 1] = true;
            it = 0;
        };
    }

    tour.push_back(nodes[0]);
    return tour;
}

int attractiveness (Node curr, Node candidate) {
    int value = candidate.prize - C * calculateDistance(curr, candidate);
    return value;
}

double calculateDistance(Node node1, Node node2) {
    return sqrt(pow(node1.x - node2.x, 2) + pow(node1.y - node2.y, 2));
}

int getCost(vector<Node> tour) {
    int cost = 0;
    for (int i = 0; i < tour.size(); i++)
    {
        if (i + 1 < tour.size()) {
            cost += calculateDistance(tour[i], tour[i + 1]);
        }
    }
    return cost;
}

int getPrize(vector<Node> tour) {
    int prize = 0;
    for (int i = 0; i < tour.size(); i++)
    {
        prize += tour[i].prize;
    }
    return prize;
}


void printData(vector<Node> tour) {
    int prize = 0, cost = 0;
    for (int i = 0; i < tour.size(); i++)
    {   
        if (i + 1 < tour.size()) {
            int cost_ij =  calculateDistance(tour[i], tour[i + 1]);
            cout << "De " << tour[i].id << " para " << tour[i + 1].id << 
            " - Custo: " << cost_ij << 
            " - Prêmio: " << tour[i+1].prize << 
            " - Atração: " << attractiveness(tour[i], tour[i+1]) << endl;
            cost += cost_ij;
        }
        prize += tour[i].prize;
    }

    cout << "Rota: ";
    for (int i = 0; i < tour.size(); i++)
    {   
        cout << tour[i].id << " -> ";
    }
    cout << endl;
    cout << "Coletado: " << prize << endl;
    cout << "Custo: " << cost << endl;
    cout << "u(tour): " << prize - C * cost << endl;
}
