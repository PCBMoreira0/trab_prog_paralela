#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <math.h>

#define TAMANHO 500000

int primo(int n) {
    int i;
    for (i = 3; i < (int)(sqrt(n) + 1); i += 2)
        if (n % i == 0) return 0;
    return 1;
}

int main(int argc, char *argv[]) {
    double t_inicial, t_final;
    int cont = 0, total = 0, i, n, meu_ranque, num_procs, inicio, dest, raiz = 0, tag = 1, stop = 0;
    MPI_Status estado;

    n = strtol(argv[1], (char **)NULL, 10);
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &meu_ranque);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    t_inicial = MPI_Wtime();

    if (meu_ranque == 0) {
        MPI_Request req_recv[num_procs - 1];
        int resultados[num_procs - 1];

        for (dest = 1, inicio = 3; dest < num_procs; dest++, inicio += TAMANHO) {
            int t = (inicio > n) ? 50 : 1;
            MPI_Irecv(&resultados[dest - 1], 1, MPI_INT, dest, MPI_ANY_TAG, MPI_COMM_WORLD, &req_recv[dest - 1]);
            MPI_Send(&inicio, 1, MPI_INT, dest, t, MPI_COMM_WORLD);
        }

        while (stop < (num_procs - 1)) {
            int index;
            MPI_Waitany(num_procs - 1, req_recv, &index, &estado);
            total += resultados[index];
            dest = estado.MPI_SOURCE;

            if (inicio > n) { tag = 99; stop++; }
            else {
                MPI_Irecv(&resultados[index], 1, MPI_INT, dest, MPI_ANY_TAG, MPI_COMM_WORLD, &req_recv[index]);
            }
            MPI_Send(&inicio, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
            inicio += TAMANHO;
        }
    } else {
        MPI_Request req_s, req_r;
        
        MPI_Irecv(&inicio, 1, MPI_INT, raiz, MPI_ANY_TAG, MPI_COMM_WORLD, &req_r);
        MPI_Wait(&req_r, &estado);
        
        while (estado.MPI_TAG != 99) {
            cont = 0;
            if (estado.MPI_TAG == 1) {
                for (i = inicio; i < (inicio + TAMANHO) && i < n; i += 2)
                    if (primo(i) == 1) cont++;
            }
            
            MPI_Isend(&cont, 1, MPI_INT, raiz, 1, MPI_COMM_WORLD, &req_s);
            MPI_Wait(&req_s, MPI_STATUS_IGNORE);
            
            MPI_Irecv(&inicio, 1, MPI_INT, raiz, MPI_ANY_TAG, MPI_COMM_WORLD, &req_r);
            MPI_Wait(&req_r, &estado);
        }
        t_final = MPI_Wtime();
    }

    if (meu_ranque == 0) {
        t_final = MPI_Wtime();
        printf("Bag of Tasks (Isend+Irecv) | Primos: %d | Tempo: %1.3f\n", total + 1, t_final - t_inicial);
    }
    MPI_Finalize();
    return 0;
}