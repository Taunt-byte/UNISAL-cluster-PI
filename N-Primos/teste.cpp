#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <ctime>
#include "mpi.h"
#include <array>
#include <memory>
#include <stdexcept>
#include <unistd.h>

// Função para obter o nome do host
std::string getHostName() {
    char hostname[1024];
    hostname[1023] = '\0';
    gethostname(hostname, 1023);
    return std::string(hostname);
}

// Função para mapear o nome do host para o nome do cluster
std::string getClusterName(const std::string& hostname) {
    if (hostname.find("clusterm") != std::string::npos) {
        return "ClusterM";
    } else if (hostname.find("no1") != std::string::npos) {
        return "No1";
    } else if (hostname.find("no2") != std::string::npos) {
        return "No2";
    }
    return "Cluster Desconhecido";
}

// Função principal
int main(int argc, char *argv[]) {
    MPI::Init(argc, argv);

    int id = MPI::COMM_WORLD.Get_rank();
    int p = MPI::COMM_WORLD.Get_size();
    int master = 0;

    std::string hostname = getHostName();
    std::string clusterName = getClusterName(hostname);

    // Mensagens informativas sobre o ambiente MPI
    if (id == master) {
        std::cout << "Processo Master no cluster: " << clusterName << std::endl;
    } else {
        std::cout << "Processo Worker " << id << " no cluster: " << clusterName << std::endl;
    }

    int n_lo = 1;
    int n_hi = 262144;
    int n_factor = 2;

    // Mais mensagens informativas sobre o ambiente MPI
    if (id == master) {
        std::cout << "Contando primos de " << n_lo << " até " << n_hi << std::endl;
        std::cout << "Rodando em " << p << " processos\n";
        std::cout << "N     Primos     Tempo\n";
    }

    int n = n_lo;
    while (n <= n_hi) {
        double wtime;
        if (id == master) {
            wtime = MPI::Wtime();
        }

        // Broadcast do valor de N para todos os processos
        MPI::COMM_WORLD.Bcast(&n, 1, MPI::INT, master);
        int primes_part = prime_number(n, id, p);

        int primes;
        // Redução para somar os resultados parciais de cada processo
        MPI::COMM_WORLD.Reduce(&primes_part, &primes, 1, MPI::INT, MPI::SUM, master);

        // Exibição dos resultados pelo processo master
        if (id == master) {
            wtime = MPI::Wtime() - wtime;
            std::cout << " " << std::setw(8) << n
                      << " " << std::setw(8) << primes
                      << " " << std::setw(14) << wtime << " segundos\n";
        }

        // Mensagem informativa sobre o término do processo worker
        if (id != master) {
            std::cout << "Processo " << id << " terminou de contar primos para N = " << n << std::endl;
        }

        n = n * n_factor;
    }

    MPI::Finalize();

    return 0;
}

// Função para contar números primos em uma faixa específica
int prime_number(int n, int id, int p) {
    int total = 0;
    for (int i = 2 + id; i <= n; i = i + p) {
        int prime = 1;
        for (int j = 2; j < i; j++) {
            if ((i % j) == 0) {
                prime = 0;
                break;
            }
        }
        total = total + prime;
    }
    return total;
}
