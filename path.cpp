#include <iostream>
#include <vector>
#include <fstream>
#include <stack>
#include <set>
#include "readInstances.h"
using namespace std;

int main() {
    readInstances::DataOP data = readInstances::readFile("./instancias/quality/instances/berlin52FSTCII_q2_g4_p40_r20_s20_rs15.pop");

    ifstream arquivo("matriz_Xij.txt");

    if (!arquivo) {
        cerr << "Erro ao abrir o arquivo!" << endl;
        return 1;
    }

    int n;
    arquivo >> n; 
    vector<vector<int>> matriz(n, vector<int>(n));

    // Lendo a matriz do arquivo
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            arquivo >> matriz[i][j];
        }
    }

    arquivo.close();

    // Criando lista de adjacência
    vector<vector<int>> grafo(n);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (matriz[i][j] == 1) {
                grafo[i].push_back(j);
            }
        }
    }

    // Gerando um caminho a partir do vértice 0 usando DFS
    stack<int> pilha;
    set<int> visitados;
    vector<int> caminho;

    pilha.push(0);
    visitados.insert(0);

    while (!pilha.empty()) {
        int v = pilha.top();
        pilha.pop();
        caminho.push_back(v);

        for (int vizinho : grafo[v]) {
            if (visitados.find(vizinho) == visitados.end()) {
                pilha.push(vizinho);
                visitados.insert(vizinho);
            }
        }
    }

    // Exibindo o caminho encontrado
    cout << "Caminho gerado: ";
    double prize = 0, cost = 0;
    for (size_t i = 0; i < caminho.size(); i++) {
        cout << caminho[i];
        prize += data.prize[caminho[i]];
        if (i < caminho.size() - 1) {
            cout << " -> ";
            cost += data.cost[caminho[i]][caminho[i + 1]];
        }
    }
    cout << endl;
    cout << "Prize: " << prize << endl;
    cout << "Cost: " << cost << endl;
    cout << "Objective function: " << prize - 0.001*cost << endl;
    return 0;
}
