#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

int* calcularQ(int* M, int* v, int myid);

int main(int argc, char** argv) {

	int n, i, j, numprocs, myid;
	int* M, v, Q;
	srand(time(NULL));
	
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);
	
	if (myid == 0) {
	
		printf("Inserte el numero de filas/columnas (múltiplo de %d): ", numprocs);
		scanf("%d", &n);

		M = (int*) malloc(n * n * sizeof(int)); //tal vez sea mejor alocar por filas/columnas
		v = (int*) malloc(n * sizeof(int));

		//Se generan M y v
		for (i = 0; i < n; i++) {
			for (j = 0; j < n; j++) {
				M[i][j] = rand() % 10;	
			}
			v[i] = rand() % 6;
		} //end for
	} //end if
		
	//calcular Q = Mv
	Q = calcularQ(M, v, myid);
	
	//calcular tp = número total de primos en M
	//calcularTp(M);
	
	//calcular P tal que P[i] = cantidad de primos en la columna i de M
	//calcularP(M);
	
	//calcular matriz B (suma de elementos en M; ver dibujo en pdf)
	//calcularB(M);
			  
	//desplegar resultados
	
	if (myid == 0) {
		free(M);
		free(v);
	}
	
	MPI_Finalize();
	
	return 0;
}

int* calcularQ(int* M, int* v, int myid) {
	int* Q;
	
	//repartir filas de M y broadcastear v
	//multiplicar la fila recibida por v
	//retornar la respuesta al proceso 0
	
	return Q;
}
