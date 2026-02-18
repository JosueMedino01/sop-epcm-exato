    #include <ilcplex/ilocplex.h>
    #include <iostream>
    #include <fstream>
    #include <sstream>
    #include <chrono>
    #include <ctime>
    #include "readInstances.h"
    #include <stack>
    #include <map>
    #include <utility>

    using namespace std;

    int START_NODE, END_NODE;
    double MIN_PRIZE, MIN_PROB;
    int TIME_LIMIT; // em segundos
    string PATH;
    string LOG_FILE;

    static std::map<std::pair<int, double>, double> hashMap;
    static std::ofstream logFileStream;

    /**
     * Responsável por modelar o problema e chamar o solver do CPLEX.
     * Após resolver o modelo, chama a função que escreve a resposta.
     */
    void solve(readInstances::DataOP &data, int startNode, int endNode, double minPrize, double minProb, int timeLimit);

    /**
     * Escreve uma mensagem no log file e no console
     */
    void writeLog(const string &message);

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
     * Função auxiliar para escrever logs
     */
    void writeLog(const string &message) {
        logFileStream << message << endl;
        logFileStream.flush();
        cout << message << endl;
    }

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
        stringstream ss;
        ss << "DEBUG - path: ";
        double cost = 0.0;
        for(int i = 0; i < path.size(); i++) {
            ss << path[i] << " ";
            if(i < path.size() -1)
                cost += data.cost[path[i]][path[i+1]];
        }
        ss << " | Cost: " << cost;
        writeLog(ss.str());
        stringstream ss2;
        ss2 << "path.size(): " << path.size();
        writeLog(ss2.str());
        stringstream ss3;
        ss3 << "edgeCount: " << edgeCount;
        writeLog(ss3.str());
        stringstream ss4;
        ss4 << "activedNodeValue: " << activedNodeValue;
        writeLog(ss4.str());
        if(activedNodeValue == edgeCount)
        {
            writeLog("verify prob tour");
            const double alfa = evaluate(path.size(), MIN_PRIZE, path, data.probability, data.prize);
            stringstream ss5;
            ss5 << "Prob: " << alfa;
            writeLog(ss5.str());


            if(alfa < MIN_PROB) 
            {
                //writeLog("add prob cut");
                IloExpr probCut(env);
                for (int k = 0; k < path.size() - 1; k++)
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
            writeLog("add subcicle tour");

            IloExpr subtour(env);
            for (int k = 0; k < path.size() - 1; k++)
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
            stringstream ss;
            ss << "Obj Fun: " << objectiveFuncValue;
            writeLog(ss.str());
            writeLog("Arestas (i, j) ativadas:");
            for (int i = 0; i < data.nCustomers; i++) {
                for (int j = 0; j < data.nCustomers; j++) {
                    if (i != j && cplex.getValue(Xij[i][j]) > 0) {
                        stringstream ss2;
                        ss2 << "(" << i  << ", " << j << ")";
                        writeLog(ss2.str());
                    }
                }   
            }

            double sumPrizes = 0;
            stringstream ss3;
            ss3 << "Nós visitados com seus Yi valores:";
            writeLog(ss3.str());
            for (int i = 0; i < data.nCustomers; i++) {
                if(cplex.getValue(Yi[i]) >= 1) {
                    sumPrizes += data.prize[i];
                    stringstream ss4;
                    ss4 << i << ": " << cplex.getValue(Yi[i]);
                    writeLog(ss4.str());
                } 
            }
            stringstream ss5;
            ss5 << "SUM PRIZE: " << sumPrizes;
            writeLog(ss5.str());

            int n = data.nCustomers;
            IloNumArray2 sol(cplex.getEnv(), n);

            for (int i = 0; i < n; i++)
            {
                sol[i] = IloNumArray(cplex.getEnv(), n);
                for (int j = 0; j < n; j++) {
                    if(i != j){
                        sol[i][j] = cplex.getValue(Xij[i][j]);
                    }
                }
            }

            auto [_, path] = buildSubTourByStartnode(sol, 0.5, START_NODE);
            stringstream ss6;
            ss6 << "Caminho: ";
            for (int node : path) {
                ss6 << node << " ";
            }
            writeLog(ss6.str());
            
            double successProb = evaluate(path.size(), MIN_PRIZE, path, data.probability, data.prize);
            stringstream ss7;
            ss7 << "SUM PROB: " << successProb;
            writeLog(ss7.str());
            stringstream ss8;
            ss8 << "CPLEX GAP: " << cplex.getMIPRelativeGap();
            writeLog(ss8.str());
        }
        else 
        {
            writeLog("Nao foi possivel encontrar uma solucao.");
        }
    }

    int main(int argc, char* argv[]) {
        if (argc < 7) {
            cerr << "Uso: " << argv[0] 
                << " <PATH> <startNode> <endNode> <minPrize> <minProb> <timeLimit>" << endl;
            cerr << "Exemplo: " << argv[0] 
                << " ./instancias/quality/instances/berlin52FSTCII_q2_g4_p40_r20_s20_rs15.pop 0 9 1200 0.0 1800" 
                << endl;
            cerr << "timeLimit em segundos (ex: 1800 para 30 minutos)" << endl;
            return 1;
        }

        PATH      = argv[1];
        START_NODE = atoi(argv[2]);
        END_NODE   = atoi(argv[3]);
        MIN_PRIZE  = atof(argv[4]);
        MIN_PROB   = atof(argv[5]);
        TIME_LIMIT = atoi(argv[6]);

        // Criar arquivo de log com timestamp
        time_t now = time(nullptr);
        tm* timeinfo = localtime(&now);
        char buffer[100];
        strftime(buffer, sizeof(buffer), "log_%Y%m%d_%H%M%S.txt", timeinfo);
        LOG_FILE = string(buffer);
        
        logFileStream.open(LOG_FILE, ios::app);
        if (!logFileStream.is_open()) {
            cerr << "Erro ao abrir arquivo de log: " << LOG_FILE << endl;
            return 1;
        }

        writeLog("========================================");
        writeLog("Iniciando execução do algoritmo SOP-EPCM");
        writeLog("========================================");
        stringstream ss;
        ss << "Arquivo: " << PATH;
        writeLog(ss.str());
        stringstream ss2;
        ss2 << "StartNode: " << START_NODE << "  EndNode: " << END_NODE;
        writeLog(ss2.str());
        stringstream ss3;
        ss3 << "MinPrize: " << MIN_PRIZE << "  MinProb: " << MIN_PROB;
        writeLog(ss3.str());
        stringstream ss4;
        ss4 << "Time Limit: " << TIME_LIMIT << " segundos (" << (TIME_LIMIT / 60.0) << " minutos)";
        writeLog(ss4.str());
        writeLog("========================================");

        readInstances::DataOP data = readInstances::readFile(PATH);

        solve(data, START_NODE, END_NODE, MIN_PRIZE, MIN_PROB, TIME_LIMIT);

        logFileStream.close();
        cout << "Logs salvos em: " << LOG_FILE << endl;

        return 0;
    };


    void solve(readInstances::DataOP &data, int startNode, int endNode, double minPrize, double minProb, int timeLimit) {
        IloEnv environment;
        IloModel model(environment, "Problema de Orientação Probabilístico com Premiação e Probabilidade Mínima");
        IloCplex cplex(model);
        IloNum tol = cplex.getParam(IloCplex::EpInt);
        
        // Configurar tempo limite em segundos
        cplex.setParam(IloCplex::Param::TimeLimit, timeLimit);

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