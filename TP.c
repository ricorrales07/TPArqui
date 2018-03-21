#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

void magia();

int main(int argc, char** argv) {

	int n, i, j, numprocs;
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

void magia() {
	//calcular Q = Mv
	//calcular tp = número total de primos en M
	//calcular P tal que P[i] = cantidad de primos en la columna i de MK_FP
	//calcular matriz B (suma de elementos en M; ver dibujo en pdf)
}