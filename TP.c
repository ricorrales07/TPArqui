#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

void magia();

int main(int argc, char** argv) {

	int n, i, j, numprocs = 4;
	int* M, v;
	srand(time(NULL));
	
	MPI_Init(&argc, &argv);
	
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	
	printf("Inserte el numero de filas/columnas (múltiplo de %d): ", numprocs);
	scanf("%d", &n);

	M = (int*) malloc(n * n * sizeof(int));
	v = (int*) malloc(n * sizeof(int));
			  
	//Se generan M y v
	for (i = 0; i < n; i++) {
		for (j = 0; j < n; j++) {
			M[i][j] = rand() % 10;	
		}
		v[i] = rand() % 6;
	}
			  
	//magia
	magia();
			  
	//desplegar resultados
	
	
	MPI_Finalize();
	free(M);
	free(v);
	return 0;
}

void magia() {}