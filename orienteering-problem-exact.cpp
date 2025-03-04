#include <ilcplex/ilocplex.h>
#include <iostream>
using namespace std;
int main() {
    int nCustomers = 6;
    int score[nCustomers] = {10, 20, 30, 40, 50, 60};
    int cost[nCustomers][nCustomers] = {
        {0, 10, 20, 30, 40, 50},
        {10, 0, 15, 25, 35, 45},
        {20, 15, 0, 14, 24, 34},
        {30, 25, 14, 0, 18, 28},
        {40, 35, 24, 18, 0, 10},
        {50, 45, 34, 28, 10, 0}
    };
    int deadline = 100;  

    IloEnv env;
   
    try
    {
        IloModel orienteeringProblem(env, "Orienteering Problem");
        IloCplex cplex(orienteeringProblem);

        /* Variáveis de decisão */
        IloArray<IloIntVarArray> Xij(env, nCustomers);
        for (int i = 0; i < nCustomers; i++) {
            Xij[i] = IloIntVarArray(env, nCustomers, 0, 1);
        }

        IloIntVarArray u(env, nCustomers, 1, nCustomers);

        /* Função objetivo */
        IloExpr objectiveFunction(env);
        for (int i = 0; i < nCustomers; i++)
        {
            for (int j = 0; j < nCustomers; j++)
            {
               objectiveFunction += Xij[i][j] * score[j]; 
            }
        }
        
        orienteeringProblem.add(IloMaximize(env, objectiveFunction));
        objectiveFunction.end();

        /* Restrições de Visitar o primeiro e ultimo cliente */
        IloExpr firstNodeSum(env);
        for (int i = 0; i < nCustomers; i++)
        {
            firstNodeSum += Xij[0][i];
        }
        orienteeringProblem.add(firstNodeSum == 1);

        IloExpr lastNodeSum(env);
        for (int i = 0; i < nCustomers; i++)
        {
            lastNodeSum += Xij[i][nCustomers-1];
        }
        orienteeringProblem.add(lastNodeSum == 1);

        /* Garantir conectividade e que é no mínimo visitado uma vez */

        /* for (int k = 1; k < nCustomers - 1; k++)
        {
            IloExpr leftSum(env);
            for (int i = 0; i < nCustomers - 2; i++)
            {
                leftSum += Xij[i][k];
            }

            IloExpr rightSum(env);
            for (int j = 1; j < nCustomers - 1; j++)
            {
                rightSum += Xij[k][j];
            }

            orienteeringProblem.add(leftSum == rightSum);
            orienteeringProblem.add(leftSum <= 1);

            leftSum.end();
            rightSum.end();
        } */
        for (int k = 1; k < nCustomers - 1; k++) {
            IloExpr inSum(env);
            IloExpr outSum(env);
            
            for (int i = 0; i < nCustomers; i++) {
                if (i != k) {
                    inSum += Xij[i][k];  // Somatório das entradas no nó k
                }
            }
        
            for (int j = 0; j < nCustomers; j++) {
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
        for (int i = 0; i < nCustomers; i++)
        {
            for (int j = 0; j < nCustomers; j++)
            {
                timeSum += Xij[i][j] * cost[i][j]; 
            }
        }

        orienteeringProblem.add(timeSum <= deadline);
        timeSum.end();

        /* Eliminar sub-tours */
        for (int i = 1; i < nCustomers - 1; i++)
        {
            orienteeringProblem.add(u[i] >= 1);
            orienteeringProblem.add(u[i] <= nCustomers - 1);
        }
        
        for (int i = 1; i < nCustomers; i++) {
            for (int j = 1; j < nCustomers; j++) {
                if (i != j) {
                    orienteeringProblem.add(u[i] - u[j] + 1 <= (nCustomers - 1) * (1 - Xij[i][j]));
                }
            }
        }

        /* Evitar lacos */

        for (int i = 0; i < nCustomers; i++)
        {
            orienteeringProblem.add(Xij[i][i] == 0);
        }

        cplex.solve();
        cout << "Solution status: " << cplex.getStatus() << endl;
        cout << "Objective value: " << cplex.getObjValue() << endl;
        for (int i = 0; i < nCustomers; i++)
        {
            for (int j = 0; j < nCustomers; j++)
            {
               if(cplex.getValue(Xij[i][j]) > 0) {
                   cout << "X[" << i << "][" << j << "] = " << cplex.getValue(Xij[i][j]) << endl;
               }
            }
            cout << endl;
        }

    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    


    env.end();
    return 0;
}