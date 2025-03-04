#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <cmath>

using namespace std;

struct DataOP
{
    int nCustomers;
    vector<int> prize;
    vector<vector<int>> cost;
    int deadline;
    vector<double> probability;
};

double getCost(int x1, int y1, int x2, int y2) {
    return sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));;
}

DataOP readFile() {
    DataOP data;
    ifstream file("att48FSTCII_q1_g1_p2.pop");
    string line;

    if (!file.is_open()) {
        cerr << "Erro ao abrir o arquivo." << endl;
        return data;
    }

    while (getline(file, line)) {
        istringstream iss(line);
        string key;
        char colon;
        int value;

        iss >> key >> colon >> value;

        if (key == "DIMENSION") {
            data.nCustomers = value;
            data.cost.resize(data.nCustomers, vector<int>(data.nCustomers, 0));
            data.probability.resize(data.nCustomers, 0.0);
            data.prize.resize(data.nCustomers, 0);
        } 
        else if (key == "TMAX") {
            data.deadline = value;
        } 
        else if (key == "NODE_COORD_SECTION") {
            vector<pair<int, int>> coords(data.nCustomers);

            for (int i = 0; i < data.nCustomers; i++) {
                int id, x, y;
                file >> id >> x >> y;
                coords[i] = {x, y};
            }

            for (int i = 0; i < data.nCustomers; i++) {
                for (int j = 0; j < data.nCustomers; j++) {
                    data.cost[i][j] = getCost(coords[i].first, coords[i].second,
                                                coords[j].first, coords[j].second);
                }
            }
        } 
        else if(key == "EDGE_WEIGHT_SECTION") {
            for (int i = 0; i < data.nCustomers; i++) {
                for (int j = i + 1; j < data.nCustomers; j++) {
                    int cost;
                    file >> cost;
                    data.cost[i][j] = cost;
                    data.cost[j][i] = cost;
                }
            }
        }
        else if (key == "NODE_PRIZE_PROBABILITY_SECTION") {
            for (int i = 0; i < data.nCustomers; i++) {   
                int id, prize;
                double prob;  // Corrigido para double

                file >> id >> prize >> prob;
                data.probability[i] = prob;
                data.prize[i] = prize;
            }
        }
    }

    file.close();
    return data;
}

int main() {
    DataOP data = readFile();
    
    cout << "Numero de clientes: " << data.nCustomers << endl;
    cout << "Deadline: " << data.deadline << endl;

    cout << "COST: " << endl;
    for (int i = 0; i < data.nCustomers; i++) {   
        for (int j = 0; j < data.nCustomers; j++) {
            cout << data.cost[i][j] << " "; 
        }
        cout << endl;
    }

    cout << "NODE_PRIZE_PROBABILITY_SECTION: " << endl;
    for (int i = 0; i < data.nCustomers; i++) {   
        cout << i + 1 << ": " << data.prize[i] << " " << data.probability[i] << endl;
    }
}
