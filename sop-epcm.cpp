#include <ilcplex/ilocplex.h>
#include <iostream>
#include "readInstances.h"
#include <queue>
#include <map>
#include <utility>

using namespace std;

const string PATH = "./instancias/quality/instances/berlin52FSTCII_q2_g4_p40_r20_s20_rs15.pop";
const int startNode = 0;
const int endNode = 9;
const double minPrize = 1200;
const double minProb = 0;

readInstances::DataOP getInstance(string path) {
    return readInstances::readFile(path);
}

//vector<int> buildPathByMatriz(IloNumArray2 matriz, IloNum tol, int startNode);
vector<int> buildPathFromSeen(IloNumArray2 Xij, IloNumArray seen, int startNode);
double evaluate(int i, double Pmin, const vector<int> &path, 
    const vector<double> &probabilities, const vector<int> &prizes);



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

        if (next == -1 || seen[next] > 0) {
            // caminho terminou ou bateu em nó já visitado
            return length;
        }


        last = current;
        current = next;
    }

    return length;
}


ILOLAZYCONSTRAINTCALLBACK4(SubtourEliminationCallback, IloArray<IloIntVarArray>, Xij, IloIntVarArray &, Yi, IloNum, tol, readInstances::DataOP&, data) {
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
        vector<int> path = buildPathFromSeen(sol, seen, startNode);
        double prob = evaluate(path.size(), minPrize, path, data.probability, data.prize);
        cout << "Probabilidade de sucesso: " << prob << endl;
        cout << "Caminho: " << path.size() <<endl;

        // if(prob < minProb) {
        //     vector<int> S;
        //     for (int i = 0; i < n; i++)
        //     {
        //         if(seen[i] > 0) {
        //             S.push_back(i);
        //         }
        //     }

        //     if(!S.empty()) {
        //         IloExpr cut(env);
        //         for (int i  : S) cut += Yi[i];
        //         add((cut <= (int)S.size() - 1)).end();
        //         cut.end();
        //         cout << "Corte adicionado. Probabilidade: " << prob << endl;
        //     }
            
        // }


        seen.end();
        for (int i = 0; i < n; i++)
        {
            sol[i].end();
        }

        sol.end();
        
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


vector<int> buildPathFromSeen(IloNumArray2 Xij, IloNumArray seen, int startNode) {
    vector<int> path;
    int curr = startNode;

    while (true) {
        path.push_back(curr);
        int next = -1;
        for (int j = 0; j < Xij.getSize(); j++) {
            if (Xij[curr][j] >= 1.0 - 1e-6 && seen[j] > 0 && j != curr) {
                next = j;
                break;
            }
        }
        if (next == startNode || next == -1) {
            // path.push_back(startNode);
            break;
        }
        curr = next;
    }

    return path;
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

    // CORRECTED CONSTRAINTS FOR PATH FROM startNode TO endNode

// Start node constraints
IloExpr startOut(environment);
for (int j = 0; j < n; j++)
    if(j != startNode) startOut += Xij[startNode][j];
POP_RC.add(startOut == 1); // Exactly one outgoing edge from start
POP_RC.add(Yi[startNode] == 1); // Start node is always visited
startOut.end();

// End node constraints  
IloExpr endIn(environment);
for (int i = 0; i < n; i++)
    if(i != endNode) endIn += Xij[i][endNode];
POP_RC.add(endIn == 1); // Exactly one incoming edge to end
POP_RC.add(Yi[endNode] == 1); // End node is always visited
endIn.end();

// Intermediate nodes (excluding start and end)
for(int i = 0; i < n; i++)
{
    if(i != startNode && i != endNode) {
        IloExpr edgeOutI(environment);
        IloExpr edgeInI(environment);
        
        for (int j = 0; j < n; j++)
        {
            if(i != j) {
                edgeOutI += Xij[i][j];
                edgeInI += Xij[j][i];
            }
        }
        
        // If visited, must have exactly 1 in and 1 out edge
        POP_RC.add(edgeOutI == Yi[i]);
        POP_RC.add(edgeInI == Yi[i]);
        // Flow conservation is automatically satisfied since both equal Yi[i]
        
        edgeOutI.end();
        edgeInI.end();
    }
}

    // Premiação mínima
    IloExpr prizeSum(environment);
    for (int i = 0; i < n; i++)
        prizeSum += Yi[i] * data.prize[i];
    
    POP_RC.add(prizeSum >= minPrize); 
    prizeSum.end();

    /* Eliminação de Subciclos */
    /* cplex.extract(POP_RC); */
    IloNum tol = cplex.getParam(IloCplex::EpInt);
    cplex.use(SubtourEliminationCallback(environment, Xij, Yi, tol, data));

    cplex.solve();
    writeResponse(data, cplex, Xij, Yi);
}

int main() {
    
    readInstances::DataOP data = getInstance(PATH);
    solver(data);
    return 0;
}

