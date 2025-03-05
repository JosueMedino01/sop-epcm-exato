#include <ilcplex/ilocplex.h>
#include <iostream>
#include "readInstances.h"

using namespace std;
 
int main() {  
    readInstances::DataOP data = readInstances::readFile("./instancias/quality/instances/berlin52FSTCII_q2_g4_p05.pop");

    IloEnv env;
   
    try
    {
        IloModel orienteeringProblem(env, "Orienteering Problem");
        IloCplex cplex(orienteeringProblem);

        /* Variáveis de decisão */
        IloArray<IloIntVarArray> Xij(env, data.nCustomers);
        for (int i = 0; i < data.nCustomers; i++) {
            Xij[i] = IloIntVarArray(env, data.nCustomers, 0, 1);
        }

        IloIntVarArray u(env, data.nCustomers, 1, data.nCustomers);

        /* Função objetivo */
        IloExpr objectiveFunction(env);
        for (int i = 0; i < data.nCustomers; i++)
        {
            for (int j = 0; j < data.nCustomers; j++)
            {
               objectiveFunction += Xij[i][j] * data.prize[j]; 
            }
        }
        
        orienteeringProblem.add(IloMaximize(env, objectiveFunction));
        objectiveFunction.end();

        /* Restrições de Visitar o primeiro e ultimo cliente */
        IloExpr firstNodeSum(env);
        for (int i = 0; i < data.nCustomers; i++)
        {
            firstNodeSum += Xij[0][i];
        }
        orienteeringProblem.add(firstNodeSum == 1);

        IloExpr lastNodeSum(env);
        for (int i = 0; i < data.nCustomers; i++)
        {
            lastNodeSum += Xij[i][data.nCustomers-1];
        }
        orienteeringProblem.add(lastNodeSum == 1);

        /* Garantir conectividade e que é no mínimo visitado uma vez */
        for (int k = 1; k < data.nCustomers - 1; k++) {
            IloExpr inSum(env);
            IloExpr outSum(env);
            
            for (int i = 0; i < data.nCustomers; i++) {
                if (i != k) {
                    inSum += Xij[i][k];  // Somatório das entradas no nó k
                }
            }
        
            for (int j = 0; j < data.nCustomers; j++) {
                if (j != k) {
                    outSum += Xij[k][j]; // Somatório das saídas do nó k
                }
            }
        
            // Garante que cada nó tenha no máximo uma entrada e uma saída
            orienteeringProblem.add(inSum <= 1);
            orienteeringProblem.add(outSum <= 1);
            orienteeringProblem.add(inSum == outSum); // Conectividade
        
            inSum.end();
            outSum.end();
        }
        
        /* Tempo máximo */
        
        IloExpr timeSum(env);
        for (int i = 0; i < data.nCustomers; i++)
        {
            for (int j = 0; j < data.nCustomers; j++)
            {
                timeSum += Xij[i][j] * data.cost[i][j]; 
            }
        }

        orienteeringProblem.add(timeSum <= data.deadline);
        timeSum.end();

        /* Eliminar sub-tours */
        for (int i = 1; i < data.nCustomers - 1; i++)
        {
            orienteeringProblem.add(u[i] >= 1);
            orienteeringProblem.add(u[i] <= data.nCustomers - 1);
        }
        
        for (int i = 1; i < data.nCustomers; i++) {
            for (int j = 1; j < data.nCustomers; j++) {
                if (i != j) {
                    orienteeringProblem.add(u[i] - u[j] + 1 <= (data.nCustomers - 1) * (1 - Xij[i][j]));
                }
            }
        }

        /* Evitar lacos */

        for (int i = 0; i < data.nCustomers; i++)
        {
            orienteeringProblem.add(Xij[i][i] == 0);
        }

        cplex.solve();
        cout << "Solution status: " << cplex.getStatus() << endl;
        cout << "Objective value: " << cplex.getObjValue() << endl;
        
        int currCustomer = 0, i = 0;
        string path = "0";
        while (i < data.nCustomers)
        {
           if(cplex.getValue(Xij[currCustomer][i]) == 1)
           {
               path += " -> " + to_string(i);
               currCustomer = i;
               i = -1;
           }
           i++;
        }

        cout << "Path: " << path << endl;
        

    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    


    env.end();
    return 0;
}