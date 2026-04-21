#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

int primo(long int n) {
	int i;
       
	for (i = 3; i < (int)(sqrt(n) + 1); i+=2) {
		if(n%i == 0) return 0;
	}
	return 1;
}

int main(int argc, char *argv[]) {
    double t_inicial, t_final;
    int cont = 0, total = 0;
    long int i, n;
    int meu_ranque, num_procs, inicio, salto;
    
    if (argc < 2) {
        printf("Valor inválido!\n");
        return 0;
    } else {
        n = strtol(argv[1], (char **) NULL, 10);
    }

    MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &meu_ranque);
	MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    
    t_inicial = MPI_Wtime();

    MPI_Request requests[num_procs-1];
    int results[num_procs-1];
    if(meu_ranque == 0){
        for(i = 1; i < num_procs; i++){
            MPI_Request request;
            MPI_Irecv(&results[i-1], 1, MPI_INT, i, 0, MPI_COMM_WORLD, &requests[i-1]);
        }
    }

    inicio = 3 + meu_ranque*2;
    salto = num_procs*2;
	for (i = inicio; i <= n; i += salto) {
		if(primo(i) == 1) cont++;
    }

    if(num_procs > 1){
        if(meu_ranque == 0){
            MPI_Waitall(num_procs-1, requests, MPI_STATUS_IGNORE);
            for(i = 0; i < num_procs-1; i++){
                cont += results[i];
            }
        }
        else{
            MPI_Request requests;
            MPI_Isend(&cont, 1 , MPI_INT, 0, 0, MPI_COMM_WORLD, &requests);
            MPI_Wait(&requests, MPI_STATUS_IGNORE);
        }
    }

    t_final = MPI_Wtime();
    if(meu_ranque == 0){
        cont++;
        printf("Total de primos: %d\n", cont);
        printf("Tempo: %f\n", t_final - t_inicial);
    }

    MPI_Finalize();
    return 0;
}