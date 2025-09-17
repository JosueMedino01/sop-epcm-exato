#include <ilcplex/ilocplex.h>
#include <iostream>
#include "readInstances.h"
#include <stack>
#include <map>
#include <utility>

using namespace std;

int START_NODE, END_NODE;
double MIN_PRIZE, MIN_PROB;
string PATH;

static std::map<std::pair<int, double>, double> hashMap;

/**
 * Responsável por modelar o problema e chamar o solver do CPLEX.
 * Após resolver o modelo, chama a função que escreve a resposta.
 */
void solve(readInstances::DataOP &data, int startNode, int endNode, double minPrize, double minProb);


/**
 * Responsável por calcular a probabilidade de sucesso de um dado caminho.
 * Utiliza programação dinâmica com memoization para evitar cálculos repetidos.
 */
double evaluate(int i, double Pmin, const vector<int> &path, 
    const vector<double> &probabilities, const vector<int> &prizes);


/**
 * Define a função objetivo do modelo.
 */
void addObjectiveFunction(IloModel &model, IloArray<IloIntVarArray> &Xij, readInstances::DataOP &data, int n);

/**
 * Define as restrições relacionadas ao nó inicial do caminho.
 */
void addFirstNodeConstraints(IloModel &model, IloArray<IloIntVarArray> &Xij, IloIntVarArray &Yi, int startNode, int n);

/**
 * Define as restrições relacionadas ao nó final do caminho.
 */
void addLastNodeConstraints(IloModel &model, IloArray<IloIntVarArray> &Xij, IloIntVarArray &Yi, int endNode, int n);

/**
 * Define as restrições intermediárias do modelo.
 */
void addIntermediateConstraints(IloModel &model, IloArray<IloIntVarArray> &Xij, IloIntVarArray &Yi, int startNode, int endNode, int n);

/**
 * Define as restrições de premiação mínima do modelo.
 */
void addMinPrizeConstraints(IloModel &model, IloIntVarArray &Yi, readInstances::DataOP &data, double minPrize, int n);

void dfsRec(IloNumArray2 &adj, IloNum tol, vector<bool> &visited, int s, vector<int> &res);

vector<int> DFS(IloNumArray2 &adj, IloNum tol);

/**
 * Adiciona restricoes de subciclo caso tenha um ciclo inválido. 
 * Caso nao tenha ciclo inválido é verificado se o caminho possui uma probabilidade minima de atingir minPrize, 
 * caso negativo, é adicionado uma restricao por probabilidade inválida.
 */
ILOLAZYCONSTRAINTCALLBACK4(eliminationCallback, IloArray<IloIntVarArray>, Xij, IloIntVarArray &, Yi, IloNum, tol, readInstances::DataOP&, data) {
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

    vector<int> path = DFS(sol, tol);
    cout << "typed path" << endl;
    for (int i : path)
    {
        cout << i << "->";
    }
    
    
    if(path.front() != path.back())
    {
        cout <<  "add subcicle tour" << endl;
    }
    else 
    {
        cout <<  "verify prob tour" << endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 6) {
        cerr << "Uso: " << argv[0] 
             << " <PATH> <startNode> <endNode> <minPrize> <minProb>" << endl;
        cerr << "Exemplo: " << argv[0] 
             << " ./instancias/quality/instances/berlin52FSTCII_q2_g4_p40_r20_s20_rs15.pop 0 9 1200 0.0" 
             << endl;
        return 1;
    }

    PATH      = argv[1];
    START_NODE = atoi(argv[2]);
    END_NODE   = atoi(argv[3]);
    MIN_PRIZE  = atof(argv[4]);
    MIN_PROB   = atof(argv[5]);

    cout << "Arquivo: " << PATH << endl;
    cout << "StartNode: " << START_NODE << "  EndNode: " << END_NODE << endl;
    cout << "MinPrize: " << MIN_PRIZE << "  MinProb: " << MIN_PROB << endl;

    readInstances::DataOP data = readInstances::readFile(PATH);

    solve(data, START_NODE, END_NODE, MIN_PRIZE, MIN_PROB);

    return 0;
};


void solve(readInstances::DataOP &data, int startNode, int endNode, double minPrize, double minProb) {
    IloEnv environment;
    IloModel model(environment, "Problema de Orientação Probabilístico com Premiação e Probabilidade Mínima");
    IloCplex cplex(model);
    IloNum tol = cplex.getParam(IloCplex::EpInt);

    const int n = data.nCustomers;

    // Definição das variáveis de decisão
    IloArray<IloIntVarArray> Xij(environment, n);
    for (int i = 0; i < n; i++)
    {
        Xij[i] = IloIntVarArray(environment, n, 0, 1);
    }
        
    IloIntVarArray Yi(environment, n, 0, 1);

    // Função objetivo: minimizar o custo total
    addObjectiveFunction(model, Xij, data, n);

    // Restrições
    addFirstNodeConstraints(model, Xij, Yi, startNode, n);
    addLastNodeConstraints(model, Xij, Yi, endNode, n);
    addIntermediateConstraints(model, Xij, Yi, startNode, endNode, n);
    addMinPrizeConstraints(model, Yi, data, minPrize, n);

    
    cplex.use(eliminationCallback(environment, Xij, Yi, tol, data));
    cplex.solve();

}

void addObjectiveFunction(IloModel &model, IloArray<IloIntVarArray> &Xij, readInstances::DataOP &data, int n) {
    IloExpr function(model.getEnv());
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            function += Xij[i][j] * data.cost[i][j];

    model.add(IloMinimize(model.getEnv(), function));
    function.end();
}

void addFirstNodeConstraints(IloModel &model, IloArray<IloIntVarArray> &Xij, IloIntVarArray &Yi, int startNode, int n) {
    IloExpr startOut(model.getEnv());
    for (int j = 0; j < n; j++)
    {
        if(j != startNode) 
            startOut += Xij[startNode][j];
    }
        

    model.add(startOut == 1); 
    model.add(Yi[startNode] == 1);
    startOut.end();
}

void addLastNodeConstraints(IloModel &model, IloArray<IloIntVarArray> &Xij, IloIntVarArray &Yi, int endNode, int n) {
    IloExpr endIn(model.getEnv());
    for (int i = 0; i < n; i++)
    {
        if(i != endNode) 
        endIn += Xij[i][endNode];
    }
    model.add(endIn == 1);
    model.add(Yi[endNode] == 1); 

    endIn.end();
}

void addIntermediateConstraints(IloModel &model, IloArray<IloIntVarArray> &Xij, IloIntVarArray &Yi, int startNode, int endNode, int n) {
    for(int i = 0; i < n; i++)
    {
        if(i != startNode && i != endNode) {
            IloExpr edgeOutI(model.getEnv());
            IloExpr edgeInI(model.getEnv());
            
            for (int j = 0; j < n; j++)
            {
                if(i != j) {
                    edgeOutI += Xij[i][j];
                    edgeInI += Xij[j][i];
                }
            }
            
            model.add(edgeOutI == Yi[i]);
            model.add(edgeInI == Yi[i]);
            
            edgeOutI.end();
            edgeInI.end();
        }
    }
}

void addMinPrizeConstraints(IloModel &model, IloIntVarArray &Yi, readInstances::DataOP &data, double minPrize, int n) {
    IloExpr prizeSum(model.getEnv());
    for (int i = 0; i < n; i++)
    {
        prizeSum += Yi[i] * data.prize[i];
    }
    
    model.add(prizeSum >= minPrize); 
    prizeSum.end();
}

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


// Recursive function for DFS traversal
void dfsRec(IloNumArray2 &adj, IloNum tol, vector<bool> &visited, int s, vector<int> &res)
{
    visited[s] = true;
    res.push_back(s);

    // Recursively visit all adjacent vertices
    // that are not visited yet
    for (int i = 0; i < adj[s].getSize(); i++)
        if (adj[s][i] >= 1.0 - tol && visited[i] == false)
            dfsRec(adj, tol, visited, i, res);
}

// Main DFS function that initializes the visited array
// and call DFSRec
vector<int> DFS(IloNumArray2 &adj, IloNum tol)
{   
    const int n = adj.getSize();
    vector<bool> visited(n, false);
    vector<int> path;

    for (int u = 0; u < n; u++)
    {
        if(!visited[u])
        {
            dfsRec(adj, tol, visited, u, path);
        }
    }

    return path;
}