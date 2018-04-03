#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

int* repartirFilas(int* M, int filas, int numprocs, int myid, int* filas_por_proceso);
int* calcularQ(int* M, int* v, int myid);
void calcularP(int* M_porcion, int myid, int numprocs, int* filas_por_proceso, int n, int* P);
int calcularTp(int* P, int n);

int i, j, k; //auxiliares para ciclos
int aux;

int main(int argc, char** argv) {

	/**************************
	n: tamano de la matriz
	M: matriz
	v: vector
	Q = Mv
	M_porcion: la porcion de la matriz que se reparte a cada proceso
	filas_por_proceso: # de filas que en teoría le tocan a cada proceso (puntero para pasarlo por referencia)
	P: vector que contiene en la entrada i la cantidad de primos en la columna i de M_porcion (en la mayoría de procesos) o M (en proceso 0 al final)
	Tp: número de primos en M
	**************************/

	int n, numprocs, myid, Tp;
	int* M, v, Q, M_porcion, filas_por_proceso, P;
	srand(time(NULL));
	
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);
	
	if (myid == 0) {
	
		printf("Inserte el numero de filas/columnas (múltiplo de %d): ", numprocs);
		scanf("%d", &n);
		//TODO: revisar que de verdad sea un múltiplo

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

	//cada proceso tiene un vector P para resultados parciales
	P = (int*) malloc(n * sizeof(int));
	//TODO: revisar que no sea NULL
	for (i = 0; i < n; i++)
		P[i] = 0;

	//repartir las filas de la matriz
	M_porcion = repartirFilas(M, n, numprocs, myid);
		
	//calcular Q = Mv
	//Q = calcularQ(M, v, myid);

	//calcular P tal que P[i] = cantidad de primos en la columna i de M
	calcularP(M_porcion, myid, numprocs, filas_por_proceso, n);
	
	//calcular tp = número total de primos en M
	if (myid == 0)
		Tp = calcularTp(P, n);
	
	//calcular matriz B (suma de elementos en M; ver dibujo en pdf)
	//calcularB(M);
			  
	//desplegar resultados
	
	if (myid == 0) {
		free(M);
		free(v);
	}

	free(P);
	
	MPI_Finalize();
	
	return 0;
}

int* repartirFilas(int* M, int n, int numprocs, int myid, int* filas_por_proceso) {	

	/****************************
	sendcounts: cuántos elementos le tocan a cada proceso
	displs: a partir de dónde se toma cada conjunto de elementos
	recvcount: cuántos elementos recibe cada proceso
	TODO: revisar si esta info se ocupa más adelante y sacarla de acá si es necesario
	****************************/
	int sendcounts[numprocs], displs[numprocs], recvcount;
	int* M_porcion;

	*filas_por_proceso = n / numprocs; //no debería dar problemas porque numprocs divide a n

	//en proceso 0:
	if (myid == 0) {

		//calcular sendcounts
		for (i = 0; i < numprocs; i++)
			sendcounts[i] = (*filas_por_proceso + 2) * n;

		//el primero y el último proceso reciben una fila menos
		sendcounts[0] -= n;
		sendcounts[numprocs - 1] -= n;

		//calcular displs
		for (i = 0; i < numprocs; i++)
			displs[i] = (*filas_por_proceso * i - 1) * n;
		
		//excepcion para el primer proceso
		displs[0] = 0;
	} //end if



	//en cada proceso:
	//calcular recvcount
	if (myid == 0 || myid == numprocs - 1)
		recvcount = (*filas_por_proceso + 1) * n;
	else
		recvcount = (*filas_por_proceso + 2) * n;

	//asignar M_porcion
	M_porcion = (int*) malloc(recvcount * sizeof(int));
	//TODO: revisar que no sea NULL
	//TODO: liberar esta memoria
	
	

	MPI_Scatterv(M, sendcounts, displs, MPI_INT, M_porcion, recvcount, MPI_INT, 0, MPI_COMM_WORLD);

	return M_porcion;
}

void calcularP(int* M_porcion, int myid, int numprocs, int* filas_por_proceso, int n, int* P) {
	int primos[4] = {2, 3, 5, 7};

	for (i = 0; i < filas_por_proceso + ((myid == 0 || myid == numprocs - 1) ? 1 : 2); i++) {
		for (j = 0; j < n; j++) {
			for (k = 0; k < 4; k++) {
				if (M_porcion[i][j] == primos[k])
					P[j]++;
			}
		}
	}

	//No estoy seguro si se puede usar P tanto en el sendbuf como en el recvbuf; si da errores, revisar esta parte.
	MPI_Reduce(P, P, n, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
}

int calcularTp(int* P, int n) {
	int accum = 0;
	for (i = 0; i < n; i++)
		accum += P[i];
	return accum;
}
