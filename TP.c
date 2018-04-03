#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

int* repartirFilas(int* M, int filas, int numprocs, int myid);
int* calcularQ(int* M, int* v, int myid);

int i, j, k; //auxiliares para ciclos
int aux;

int main(int argc, char** argv) {

	/**************************
	n: tamano de la matriz
	M: matriz
	v: vector
	Q = Mv
	M_porcion: la porcion de la matriz que se reparte a cada proceso
	**************************/

	int n, numprocs, myid;
	int* M, v, Q, M_porcion;
	srand(time(NULL));
	
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);
	
	if (myid == 0) {
	
		printf("Inserte el numero de filas/columnas (múltiplo de %d): ", numprocs);
		scanf("%d", &n);

		M = (int*) malloc(n * n * sizeof(int)); //tal vez sea mejor alocar por filas (mejor no; complica el scatterv)
		v = (int*) malloc(n * sizeof(int));

		//TODO: revisar que no sean NULL

		//Se generan M y v
		for (i = 0; i < n; i++) {
			for (j = 0; j < n; j++) {
				M[i][j] = rand() % 10;	
			}
			v[i] = rand() % 6;
		} //end for
	} //end if

	//repartir las filas de la matriz
	M_porcion = repartirFilas(M, n, numprocs, myid);
		
	//calcular Q = Mv
	//Q = calcularQ(M, v, myid);
	
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

int* repartirFilas(int* M, int n, int numprocs, int myid) {	

	/****************************
	sendcounts: cuántos elementos le tocan a cada proceso
	displs: a partir de dónde se toma cada conjunto de elementos
	recvcount: cuántos elementos recibe cada proceso
	TODO: revisar si esta info se ocupa más adelante y sacarla de acá si es necesario
	****************************/
	int sendcounts[numprocs], displs[numprocs], recvcount;
	int filas_por_proceso;
	int* M_porcion;

	filas_por_proceso = n / numprocs; //no debería dar problemas porque numprocs divide a n

	//en proceso 0:
	if (myid == 0) {

		//calcular sendcounts
		for (i = 0; i < numprocs; i++)
			sendcounts[i] = (filas_por_proceso + 2) * n;

		//el primero y el último proceso reciben una fila menos
		sendcounts[0] -= n;
		sendcounts[numprocs - 1] -= n;

		//calcular displs
		for (i = 0; i < numprocs; i++)
			displs[i] = (filas_por_proceso * i - 1) * n;
		
		//excepcion para el primer proceso
		displs[0] = 0;
	} //end if



	//en cada proceso:
	//calcular recvcount
	if (myid == 0 || myid == numprocs - 1)
		recvcount = (filas_por_proceso + 1) * n;
	else
		recvcount = (filas_por_proceso + 2) * n;

	//asignar M_porcion
	M_porcion = (int*) malloc(recvcount * sizeof(int));
	//TODO: revisar que no sea NULL
	
	

	MPI_Scatterv(M, sendcounts, displs, MPI_INT, M_porcion, recvcount, MPI_INT, 0, MPI_COMM_WORLD);

	return M_porcion;
}
