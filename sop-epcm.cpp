#include <ilcplex/ilocplex.h>
#include <iostream>
#include "readInstances.h"
#include <queue>
#include <map>
#include <utility>

using namespace std;

const string PATH = "./instancias/quality/instances/berlin52FSTCII_q2_g4_p40_r20_s20_rs15.pop";
const int startNode = 0;
const int endNode = 14;
const double minPrize = 6000;

readInstances::DataOP getInstance(string path) {
    return readInstances::readFile(path);
}

IloInt checkTour(IloNumArray2 Xij, IloNumArray seen, IloNum tol)
{
	IloInt j = 0, n	 = Xij.getSize();
	IloInt last	 = -1;
	IloInt length  = 0;
	IloInt current = 0;
	seen.clear();
	seen.add(n, 0.0);

	// Search for a subtour if sol[] is integer feasible
	while (seen[current] == 0)
    {
        length++;
        seen[current] = length;
        IloInt next = -1;

        for (j = 0; j < n; j++)
        {
            if (Xij[current][j] >= 1.0 - tol) {
                if (j != last || (j == startNode && length > 1)) {
                    next = j;
                    break;
                }
            }
        }

        if (next == -1) {
            cout << "NOT CYCLE" << endl;
            return n + 1;
        }

        last = current;
        current = next;
    }

    return length;
}


ILOLAZYCONSTRAINTCALLBACK2(SubtourEliminationCallback, IloArray<IloIntVarArray>, Xij, IloNum, tol) {
    IloInt n = Xij.getSize();
    IloEnv env = getEnv();
    IloNumArray2 sol(env, n);

    for (int i = 0; i < n; i++)
    {
        sol[i] = IloNumArray(env, n);
        for (int j = 0; j < n; j++) {
            if(i != j){
                sol[i][j] = getValue(Xij[i][j]);
            }
        }
    }
   
    
    IloNumArray seen(env);
    IloInt length = checkTour(sol, seen, tol);

    if(length >= n) 
    {
        seen.end();
        for (int i = 0; i < n; i++)
        {
            sol[i].end();
        }

        sol.end();
        cout << "NOT CYCLE" << endl;
        return;
    }

    IloExpr subtour(env);
    for (int i = 0; i < n; i++) {
        if (seen[i]) {
            for (int j = 0; j < n; j++) {
                if (i != j && seen[j]) {
                    subtour += Xij[i][j];
                }
            }
        }
    }
   
    add((subtour <= length - 1)).end();

    subtour.end();
    seen.end();

    for (int i = 0; i < n; i++)
    {
        sol[i].end();
    }

    sol.end();
    return;
}



static std::map<std::pair<int, double>, double> hashMap;

double evaluate(int i, double Pmin, const vector<int> &path, 
    const vector<double> &probabilities, const vector<int> &prizes) 
{
    if (Pmin <= 0) 
    {
        return 1.0;
    } 
    else if (i == 0) 
    {
        return 0.0;
    }
    else if (hashMap.find({i, Pmin}) != hashMap.end())
    {
        return hashMap.find({i, Pmin})->second;
    }

    int node = path[i - 1];
    double prob_i = (probabilities[node] > 0) ? probabilities[node] : 0; 

    double prob = evaluate(i-1, Pmin - prizes[node], path, probabilities, prizes) * prob_i +  
            evaluate(i-1, Pmin, path, probabilities, prizes) * (1 - prob_i);

    hashMap[{i, Pmin}] = prob;

    return prob;
}

#include <fstream>

void writeResponse(readInstances::DataOP &data, IloCplex &cplex, IloArray<IloIntVarArray> &Xij, IloIntVarArray &Yi) {
    double sumPrizes = 0;
    if (cplex.getStatus() == IloAlgorithm::Optimal ||
    cplex.getStatus() == IloAlgorithm::Feasible) {
        double funcaoObjetivo = cplex.getObjValue(); 
        cout << "Valor da função objetivo: " << funcaoObjetivo << endl;

        cout << "Arestas (i, j) ativadas:" << endl;
        for (int i = 0; i < data.nCustomers; i++) {
            for (int j = 0; j < data.nCustomers; j++) {
                if (i != j && cplex.getValue(Xij[i][j]) > 0) {
                    cout << "(" << i + 1  << ", " << j + 1 << ")" << endl;
                }
            }   
        }
        cout << endl;
        for (int i = 0; i < data.nCustomers; i++) {
            if(cplex.getValue(Yi[i]) >= 1) {
                sumPrizes += data.prize[i];
                cout << i + 1 << ": " << cplex.getValue(Yi[i]) << endl;
            } 
        }
        cout << endl;

        /* // Escreve todos os Xij e seus valores em um arquivo
        std::ofstream outFile("xij_values.txt");
        if (outFile.is_open()) {
            for (int i = 0; i < data.nCustomers; i++) {
                for (int j = 0; j < data.nCustomers; j++) {
                    outFile << "X[" << i << "][" << j << "] = " << cplex.getValue(Xij[i][j]) << std::endl;
                }
            }
            outFile.close();
        } else {
            cerr << "Não foi possível abrir o arquivo para escrita dos Xij." << endl;
        } */
    } else {
        cerr << "Modelo não foi resolvido com sucesso. Status: " << cplex.getStatus() << endl;
    }

    cout << "Soma dos prêmios: " << sumPrizes << endl;
}


void solver(readInstances::DataOP &data) {
    IloEnv environment;
    IloModel POP_RC(environment, "Problema de Orientação Probabilístico com Premiação e Probabilidade Mínima");
    IloCplex cplex(POP_RC);

    const int n = data.nCustomers;

    /* VARIÁVEIS DE DECISÃO */
    IloArray<IloIntVarArray> Xij(environment, n);
    for (int i = 0; i < n; i++)
        Xij[i] = IloIntVarArray(environment, n, 0, 1);

    IloIntVarArray Yi(environment, n, 0, 1);

    /* FUNÇÃO OBJETIVO */
    IloExpr function(environment);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            function += Xij[i][j] * data.cost[i][j];
        
    POP_RC.add(IloMinimize(environment, function)); function.end();
   

    /* RESTRIÇÕES */

    // Define o primeiro cliente a ser visitado
    IloExpr edgeSumWithStartNode(environment);
    for (int j = 0; j < n; j++)
        if( startNode != j) edgeSumWithStartNode += Xij[startNode][j];

    POP_RC.add(edgeSumWithStartNode == 1); edgeSumWithStartNode.end();

    // Define o último cliente a ser visitado
    IloExpr edgeSumWithEndNode(environment);
    for (int i = 0; i < n; i++)
        if(endNode != i) edgeSumWithEndNode += Xij[i][endNode];
        
    POP_RC.add(edgeSumWithEndNode == 1); edgeSumWithEndNode.end();

    // Se sai aresta de i então i está na solução (Yi = 1)
    for(int i = 0; i < n; i++)
    {
        IloExpr edgeOutI(environment);
        IloExpr edgeOutJ(environment);

        for (int j = 0; j < n; j++)
        {
            if(i != j) {
                edgeOutI += Xij[i][j];
                edgeOutJ += Xij[j][i];
            }
        }

        POP_RC.add(edgeOutI == Yi[i]);
        POP_RC.add(edgeOutJ == Yi[i]);

        edgeOutI.end();
        edgeOutJ.end();
    }

    // Premiação mínima
    IloExpr prizeSum(environment);
    for (int i = 0; i < n; i++)
        prizeSum += Yi[i] * data.prize[i];
    
    POP_RC.add(prizeSum >= minPrize); 
    prizeSum.end();

    /* Eliminação de Subciclos */
    cplex.extract(POP_RC);
    IloNum tol = cplex.getParam(IloCplex::EpInt);
    cplex.use(SubtourEliminationCallback(environment, Xij, tol));

    cplex.solve();
    writeResponse(data, cplex, Xij, Yi);
}

int main() {
    readInstances::DataOP data = getInstance(PATH);
    solver(data);
    return 0;
}