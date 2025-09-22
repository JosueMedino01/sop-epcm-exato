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


    pair<int, vector<int>> buildSubTourByStartnode(IloNumArray2 &adj, IloNum tol, int startNode);


    /**
     * Adiciona restricoes de subciclo caso tenha um ciclo inválido. 
     * Caso nao tenha ciclo inválido é verificado se o caminho possui uma probabilidade minima de atingir minPrize, 
     * caso negativo, é adicionado uma restricao por probabilidade inválida.
     */
    ILOLAZYCONSTRAINTCALLBACK4(eliminationCallback, IloArray<IloIntVarArray>, Xij, IloIntVarArray &, Yi, IloNum, tol, readInstances::DataOP&, data) {
        //cout << "CALL LAZY CONSTRAINT" << endl;
        IloInt n = Xij.getSize();
        IloEnv env = getEnv();
        IloNumArray2 sol(env, n);

        for (int i = 0; i < n; i++)
        {
            sol[i] = IloNumArray(env, n);
            for (int j = 0; j < n; j++) {
                if(i != j){
                    sol[i][j] = getValue(Xij[i][j]);
                    // if(sol[i][j] > 0.5)
                    //     cout << i << "->" << j << ": " << sol[i][j] << " | ";
                }
            }
            // cout << endl;
        }

        auto [edgeCount, path] = buildSubTourByStartnode(sol, tol, 0);

        const int activedNodeValue = path.size() - 1;
        cout << "DEBUG - path: ";
        for(int i = 0; i < path.size(); i++) {
            cout << path[i] << " ";
        }
        cout << endl;
        cout << "path.size(): " << path.size() << endl;
        cout << "edgeCount: " << edgeCount << endl; 
        cout << "activedNodeValue: " << activedNodeValue << endl;
        if(activedNodeValue == edgeCount)
        {
            cout <<  "verify prob tour" << endl;
            const double alfa = evaluate(path.size(), MIN_PRIZE, path, data.probability, data.prize);
            cout << "Prob: " << alfa << endl;


            if(alfa < MIN_PROB) 
            {
                //cout <<  "add prob cut" << endl;
                IloExpr probCut(env);
                for (int k = 0; k < path.size() ; k++)
                {
                    int i = path[k];
                    int j = path[k + 1];

                    if(k != 0 && i == path.front())
                        break;

                    // cout << "CUT ; add edge: " << i << "->" << j << endl;
                    probCut += Xij[i][j];
                }

                add((probCut <= activedNodeValue - 1)).end();
                probCut.end();
            }
        }
        else 
        {
            cout <<  "add subcicle tour" << endl;

            IloExpr subtour(env);
            for (int k = 0; k < path.size() ; k++)
            {
                // k = 0, 1, 2
                int i = path[k];
                int j = path[k + 1];

                if(k != 0 && i == path.front())
                    break;

                //cout << "CUT ; add edge: " << i << "->" << j << endl;
                subtour += Xij[i][j];
            }

            add((subtour <= activedNodeValue - 1));
            subtour.end();
        }    
    }
    
    void buildOutput(readInstances::DataOP &data, IloCplex &cplex, IloArray<IloIntVarArray> &Xij, IloIntVarArray &Yi) {
        const bool hasResult = cplex.getStatus() == IloAlgorithm::Optimal || cplex.getStatus() == IloAlgorithm::Feasible;

        if(hasResult) 
        {
            const int objectiveFuncValue = cplex.getObjValue();
            cout << "Obj Fun: " << objectiveFuncValue << endl; 
            cout << "Arestas (i, j) ativadas:" << endl;
            for (int i = 0; i < data.nCustomers; i++) {
                for (int j = 0; j < data.nCustomers; j++) {
                    if (i != j && cplex.getValue(Xij[i][j]) > 0) {
                        cout << "(" << i  << ", " << j << ")" << endl;
                    }
                }   
            }
            cout << endl;

            double sumPrizes = 0;
            for (int i = 0; i < data.nCustomers; i++) {
                if(cplex.getValue(Yi[i]) >= 1) {
                    sumPrizes += data.prize[i];
                    cout << i << ": " << cplex.getValue(Yi[i]) << endl;
                } 
            }
            cout << endl;
            cout << "SUM PRIZE: " << sumPrizes << endl;

            int n = data.nCustomers;
            IloNumArray2 sol(cplex.getEnv(), n);

            for (int i = 0; i < n; i++)
            {
                sol[i] = IloNumArray(cplex.getEnv(), n);
                for (int j = 0; j < n; j++) {
                    if(i != j){
                        sol[i][j] = cplex.getValue(Xij[i][j]);
                        // if(sol[i][j] > 0.5)
                        //     cout << i << "->" << j << ": " << sol[i][j] << " | ";
                    }
                }
                // cout << endl;
            }

            auto [_, path] = buildSubTourByStartnode(sol, 0.5, START_NODE);
            cout << "SUM PROB: " << evaluate(data.nCustomers, MIN_PRIZE, vector<int>{0}, data.probability, data.prize) << endl;
        }
        else 
        {
            cout << "Nao foi possivel encontrar uma solucao." << endl;
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
        //addMinPrizeConstraints(model, Yi, data, minPrize, n);
        
        IloCplex::Callback sec = cplex.use(eliminationCallback(environment, Xij, Yi, tol, data));
        cplex.solve();

        sec.end();
        buildOutput(data, cplex, Xij, Yi);
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

    pair<int, vector<int>> buildSubTourByStartnode(IloNumArray2 &adj, IloNum tol, int startNode)
    {   
        const int n = adj.getSize();
        vector<int> nextArr(n, -1);

        int edgeActivatedValue = 0;

        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                if(adj[i][j] > 1.0 - tol)
                {
                    nextArr[i] = j;
                    edgeActivatedValue++;
                    break;
                }
            }
        }

        vector<bool> visited(n, false);
        vector<int> path;
        int current = startNode;

        while (current != -1 && !visited[current]) 
        {
            visited[current] = true;
            path.push_back(current);
            current = nextArr[current];
        }

        if(current != -1)
            path.push_back(current);

        return { edgeActivatedValue, path };
    }