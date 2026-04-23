#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <cmath>
#include <iomanip>

using namespace std;

struct Node {
    int id;
    int x, y;
    int prize;
    double prob;
};

int main(int argc, char* argv[]) {
    if (argc < 6) {
        cerr << "Uso: ./generator n prob_min prob_max seed output_file\n";
        return 1;
    }

    int n = stoi(argv[1]);
    double prob_min = stod(argv[2]);
    double prob_max = stod(argv[3]);
    int seed = stoi(argv[4]);
    string output_file = argv[5];

    // Parâmetros
    int coord_max = 1000;
    int prize_min = 200;
    int prize_max = 800;
    double alpha = 0.6;


    mt19937 rng(seed);

    uniform_int_distribution<int> coord_dist(0, coord_max);
    uniform_int_distribution<int> prize_dist(prize_min, prize_max);
    uniform_real_distribution<double> prob_dist(prob_min, prob_max);

    vector<Node> nodes;

    // Nó 1 (depósito)
    nodes.push_back({1, coord_dist(rng), coord_dist(rng), 0, 1.0});

    // Outros nós
    for (int i = 2; i <= n; i++) {
        Node node;
        node.id = i;
        node.x = coord_dist(rng);
        node.y = coord_dist(rng);
        node.prize = prize_dist(rng);
        node.prob = prob_dist(rng);

        nodes.push_back(node);
    }

    // Estatísticas
    double expected_prize = 0.0;
    double avg_prob = 0.0;
    int total_prize = 0;

    for (const auto& node : nodes) {
        expected_prize += node.prize * node.prob;
        avg_prob += node.prob;
        total_prize += node.prize;
    }

    avg_prob /= n;

    // Definir MIN_PRIZE
    int min_prize = static_cast<int>(alpha * expected_prize);

    // Definir MIN_PROB 
    double min_prob = 0.7;

    // Escrita
    ofstream out(output_file);

    out << "NAME : Artificial_POP_Instance\n";
    out << "COMMENT : Generated POP instance with " << n << " locations\n";
    out << "DIMENSION : " << n << "\n";
    out << "TPRIZE : " << total_prize << "\n";
    out << "MIN_PRIZE : " << min_prize << "\n";
    out << "MIN_PROB : " << fixed << setprecision(2) << min_prob << "\n";
    out << "EDGE_WEIGHT_TYPE : CEIL_2D\n";

    out << "NODE_COORD_SECTION\n";
    for (const auto& node : nodes) {
        out << node.id << " " << node.x << " " << node.y << "\n";
    }

    out << "NODE_PRIZE_PROBABILITY_SECTION\n";
    for (const auto& node : nodes) {
        out << node.id << " " << node.prize << " "
            << fixed << setprecision(2) << node.prob << "\n";
    }

    out.close();

    cout << "Instancia gerada: " << output_file << endl;

    return 0;
}