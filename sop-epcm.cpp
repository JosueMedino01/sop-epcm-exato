#include <ilcplex/ilocplex.h>
#include <iostream>
#include "readInstances.h"
#include <queue>
#include <map>
#include <utility>

using namespace std;

const string PATH = "./instancias/quality/instances/berlin52FSTCII_q2_g4_p40_r20_s20_rs15.pop";
const int startNode = 0;
const int endNode = 0;
const double minPrize = 1000;


readInstances::DataOP getInstance(string path) {
    return readInstances::readFile(path);
}

IloInt checkTour(IloNumArray2 Xij, IloNumArray seen, IloNum tol)
{
	IloInt j, n	 = Xij.getSize();
	IloInt last	 = -1;
	IloInt length  = 0;
	IloInt current = 0;
	seen.clear();
	seen.add(n, 0.0);

	// Search for a subtour if sol[] is integer feasible

	while ( seen[current] == 0 )
	{
		length++;
		seen[current] = length;
		for (j = 0; j < current; j++)
		{
			if ( j != last  &&  Xij[current][j] >= 1.0-tol ) break;
		}
		if ( j == current )
		{
			for (j = current+1; j < n; j++) {
				if ( j != last  &&  Xij[j][current] >= 1.0-tol ) break;
			}
		}
		if ( j == n ) return (n+1);
		last = current;
		current = j;
	}

	return (length);
}

ILOLAZYCONSTRAINTCALLBACK2(SubtourEliminationCallback, IloArray<IloIntVarArray>, Xij, IloNum, tol) {
    IloInt n = Xij.getSize();
    IloEnv env = getEnv();
    IloNumArray2 sol(env, n);

    for (int i = 0; i < n; i++)
    {
        sol[i] = IloNumArray(env);
        getValues(sol[i], Xij[i]);
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
        return;
    }

    IloExpr subtour(env);
    for (int i = 0; i < n; i++)
    {
        if(seen[i])
        {
            for (int j = i + 1; j < n; j++)
            {
                if(seen[j]) {
                    subtour += Xij[i][j];
                }
            }
            
        }
    }
    
    add(subtour <= length - 1).end();
    subtour.end();
    seen.end();

    for (int i = 0; i < n; i++)
    {
        sol[i].end();
    }

    sol.end();
    return;
}



/* static std::map<std::pair<int, double>, double> hashMap; */

/* double evaluate(int i, double Pmin, const vector<int> &path, 
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
} */

/* struct EvaluateTourProb : IloCplex::LazyConstraintCallbackI {
    IloEnv& env;
    IloArray<IloIntVarArray>& Xij;
    readInstances::DataOP data;

    EvaluateTourProb(IloEnv& env, IloArray<IloIntVarArray> &Xij, readInstances::DataOP data) :
        LazyConstraintCallbackI{env}, env{env}, Xij{Xij}, data{data} {}

    [[nodiscard]] IloCplex::CallbackI* duplicateCallback() const override {
        return new(env) EvaluateTourProb{*this};
    }

    void main() override {
                vector<int> path;
        vector<bool> visited(data.nCustomers, false);
        int current = startNode;
        path.push_back(current);
        visited[current] = true;

        while (current != endNode) {
            bool found = false;
            for (int j = 0; j < data.nCustomers; j++) {
                if (current != j && !visited[j] && getValue(Xij[current][j]) == 1) {
                    path.push_back(j);
                    visited[j] = true;
                    current = j;
                    found = true;
                    break;
                }
            }
        }

        double prob = evaluate(path.size(), minPrize, path, data.probability,  data.prize);
      
        

        IloExpr lazyConstraint(env);
        
        add(lazyConstraint < 1);
        lazyConstraint.end();
    };
}; */




/* bool dfsCheck(int node, vector<int> adj[], int vis[], int pathVis[]){ 
    vis[node] = 1;
    pathVis[node] = 1;

    for (auto it : adj[node]) 
    {
        if(!vis[it]) {
            if(dfsCheck(it, adj, vis, pathVis) == true)
                return true;
        }
        else if(pathVis[it]) {
            return true;
        }
    }

    pathVis[node] = 0;
    return false;
}


bool isCyclic(int V, vector<int> adj[]) {
    int vis[V] = {0};
    int pathVis[V] = {};

    for (int i = 0; i < V; i++)
    {
        if(!vis[i]) {
            if(dfsCheck(i, adj, vis, pathVis) == true) return true;
        }
    }
    
    return false;
}
 */
#include <fstream>

void writeResponse(readInstances::DataOP &data, IloCplex &cplex, IloArray<IloIntVarArray> &Xij) {
    double sumPrizes = 0;
    if (cplex.solve()) {
        double funcaoObjetivo = cplex.getObjValue(); 
        cout << "Valor da função objetivo: " << funcaoObjetivo << endl;

        cout << "Arestas (i, j) ativadas:" << endl;
        for (int i = 0; i < data.nCustomers; i++) {
            for (int j = 0; j < data.nCustomers; j++) {
                if (i != j && cplex.getValue(Xij[i][j]) > 0.5) {
                    cout << "(" << i + 1  << ", " << j + 1 << ") ";
                    sumPrizes += data.prize[j];
                }
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
    /* IloIntVarArray U(environment, n, 1, n); */

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

    // Eliminação de subciclo
    /* for (int i = 1; i < n; i++)
    {
        POP_RC.add(U[i] >= 1);
        POP_RC.add(U[i] <= n - 1);
    }
    
    for (int i = 1; i < n; i++) {
        for (int j = 1; j < n; j++) {
            if (i != j) {
                POP_RC.add(U[i] - U[j] + 1 <= (n - 1) * (1 - Xij[i][j]));
            }
        }
    } */

    /* Garantir conectividade e que é no mínimo visitado uma vez */
    /* for (int k = 1; k < n - 1; k++) {
        IloExpr inSum(environment);
        IloExpr outSum(environment);
        
        for (int i = 0; i < n; i++) {
            if (i != k) {
                inSum += Xij[i][k];  
            }
        }
    
        for (int j = 0; j < n; j++) {
            if (j != k) {
                outSum += Xij[k][j];
            }
        }
    
        // Garante que cada nó tenha no máximo uma entrada e uma saída
        POP_RC.add(inSum <= 1);
        POP_RC.add(outSum <= 1);
        POP_RC.add(inSum == outSum); // Conectividade
    
        inSum.end();
        outSum.end();
    } */
        



    /* Eliminação de Subciclos */
   /*  IloNum tol = cplex.getParam(IloCplex::EpInt);
    IloCplex::Callback sec = cplex.use( SubtourEliminationCallback(environment, Xij, tol)); */

    // Premiação mínima
    IloExpr prizeSum(environment);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            prizeSum += Xij[i][j] * data.prize[j];
    
    POP_RC.add(prizeSum >= minPrize); prizeSum.end();

    // Sem laços
    /* for (int i = 0; i < n; i++) {
        POP_RC.add(Xij[i][i] == 0);
    } */


    cplex.solve();
    writeResponse(data, cplex, Xij);
}

int main() {
    readInstances::DataOP data = getInstance(PATH);
    solver(data);
    return 0;
}


/* class SubtourEliminationCallback : public IloCplex::LazyConstraintCallbackI {
    IloArray<IloIntVarArray> Xij;
    int n;

public:
    SubtourEliminationCallback(IloEnv env, IloArray<IloIntVarArray>& _Xij, int _n)
        : IloCplex::LazyConstraintCallbackI(env), Xij(_Xij), n(_n) {}

    void main() override {
        IloExpr lazyConstraint(getEnv());
        add(lazyConstraint < 1);
        lazyConstraint.end();
        return; 
    }

    

    IloCplex::CallbackI* duplicateCallback() const override {
        return new (getEnv()) SubtourEliminationCallback(*this);
    }
};
 */