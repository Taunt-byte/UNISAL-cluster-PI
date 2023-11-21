#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cmath>
#include "mpi.h"

// Função principal
int main(int argc, char *argv[]) {
    MPI::Init(argc, argv);

    int id = MPI::COMM_WORLD.Get_rank();
    int p = MPI::COMM_WORLD.Get_size();
    int master = 0;

    int num_points_total = 1000000;  // Número total de pontos

    // Número de pontos a serem gerados por cada processo
    int num_points_per_process = num_points_total / p;

    int num_points_inside_circle = 0;

    // Semente diferente para cada processo
    srand(time(NULL) + id);

    for (int i = 0; i < num_points_per_process; ++i) {
        // Gera coordenadas x e y entre -1 e 1
        double x = (double)rand() / RAND_MAX * 2 - 1;
        double y = (double)rand() / RAND_MAX * 2 - 1;

        // Verifica se o ponto está dentro do círculo
        if (x*x + y*y <= 1) {
            num_points_inside_circle++;
        }
    }

    int total_inside_circle;

    // Redução para somar os resultados parciais de cada processo
    MPI::COMM_WORLD.Reduce(&num_points_inside_circle, &total_inside_circle, 1, MPI::INT, MPI::SUM, master);

    // Processo master calcula e exibe o valor estimado de π
    if (id == master) {
        double pi_estimate = 4.0 * total_inside_circle / num_points_total;
        std::cout << "Estimativa de Pi: " << std::setprecision(10) << pi_estimate << std::endl;
    }

    MPI::Finalize();

    return 0;
}
