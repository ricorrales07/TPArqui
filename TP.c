//Ricardo Corrales Barquero, B32090
//Gabriel Vidaurre Rodríguez, B47568

#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

int *repartirFilas(int *M, int filas, int numprocs, int myid, int *filas_por_proceso);
void calcularQ(int *M_porcion, int *v, int myid, int *filas_por_proceso, int n, int *Q_parcial, int *Q);
void calcularP(int *M_porcion, int myid, int numprocs, int *filas_por_proceso, int n, int *P);
int calcularTp(int *P, int n);
void calcularB(int *M_porcion, int *B_parcial, int myid, int numprocs, int *filas_por_proceso, int n, int *B);
void desplegar_resultados(int n, int numprocs, int Tp, double big_start, double small_start, int *M, int *v, int *Q, int *P, int *B);

int i, j, k; //auxiliares para ciclos
int aux;

int main(int argc, char **argv) {

	/**************************
	n: tamano de la matriz
	M: matriz
	v: vector
	Q = Mv
	M_porcion: la porcion de la matriz que se reparte a cada proceso
	filas_por_proceso: # de filas que en teoría le tocan a cada proceso (puntero para pasarlo por referencia)
	P: vector que contiene en la entrada i la cantidad de primos en la columna i de M_porcion (en la mayoría de procesos) o M (en proceso 0 al final)
	Tp: número de primos en M
	big_start: momento del inicio del programa
	small_start: momento después de que el usuario inserta el valor de n 
	**************************/

	int n, numprocs, myid, Tp;
	int *M, *v, *Q, *Q_parcial, *M_porcion, *filas_por_proceso, *P, *B, *B_parcial;
	double small_start, big_start;

	big_start = MPI_Wtime();	
	
	srand(time(NULL));
	
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);
	
	if (myid == 0) {
	
		do {
			printf("Inserte el numero de filas/columnas (múltiplo de %d): ", numprocs);
			scanf("%d", &n);
		} while (n % numprocs != 0);
		small_start = MPI_Wtime();

		M = (int *) malloc(n * n * sizeof(int));
		B = (int *) malloc(n * n * sizeof(int));
		Q = (int *) malloc(n * sizeof(int));
		v = (int *) malloc(n * sizeof(int));

		if (M == NULL || B == NULL || Q == NULL || v == NULL) {
			printf("Error al asignar memoria en proceso 0\n");
			MPI_Abort(MPI_COMM_WORLD, 1);
		}

		//Se generan M y v
		for (i = 0; i < n; i++) {
			for (j = 0; j < n; j++) {
				*(M + i * n + j) = rand() % 10;	//aritmética de punteros porque es más fácil de entender que un puntero de punteros
			}
			v[i] = rand() % 6;
		} //end for
	} //end if



	MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
	//printf("Proceso %d recibió n: %d\n", myid, n);

	if (myid != 0) {
		v = (int *) malloc(n * sizeof(int));
		if (v == NULL) {
			printf("Error al asignar memoria en proceso 0\n");
			MPI_Abort(MPI_COMM_WORLD, 1);
		}
	}

	MPI_Bcast(v, n, MPI_INT, 0, MPI_COMM_WORLD);



	//cada proceso tiene un vector P para resultados parciales
	P = (int *) malloc(n * sizeof(int));
	if (P == NULL) {
		printf("Error al asignar memoria en proceso %d.\n", myid);
		MPI_Abort(MPI_COMM_WORLD, 1);
	}
	for (i = 0; i < n; i++)
		P[i] = 0;
	filas_por_proceso = (int *) malloc(sizeof(int));

	//repartir las filas de la matriz
	M_porcion = repartirFilas(M, n, numprocs, myid, filas_por_proceso);
	
	//calcular Q = Mv
	calcularQ(M_porcion, v, myid, filas_por_proceso, n, Q_parcial, Q);

	//calcular P tal que P[i] = cantidad de primos en la columna i de M
	calcularP(M_porcion, myid, numprocs, filas_por_proceso, n, P);
	
	//calcular tp = número total de primos en M
	if (myid == 0)
		Tp = calcularTp(P, n);

	//calcular matriz B (suma de elementos en M; ver dibujo en pdf)
	calcularB(M_porcion, B_parcial, myid, numprocs, filas_por_proceso, n, B);

	//desplegar resultados
	if (myid == 0) {
		desplegar_resultados(n, numprocs, Tp, big_start, small_start, M, v, Q, P, B);

		free(M);
		free(B);
		free(Q);
	}

	free(v);
	free(M_porcion);
	free(P);
	free(filas_por_proceso);
	
	MPI_Finalize();
	
	return 0;
}

int *repartirFilas(int *M, int n, int numprocs, int myid, int *filas_por_proceso) {	

	/****************************
	sendcounts: cuántos elementos le tocan a cada proceso
	displs: a partir de dónde se toma cada conjunto de elementos
	recvcount: cuántos elementos recibe cada proceso
	****************************/
	int sendcounts[numprocs], displs[numprocs], recvcount;
	int *M_porcion;

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
	M_porcion = (int *) malloc(recvcount * sizeof(int));
	if (M_porcion == NULL) {
		printf("Error al asignar memoria en proceso %d.", myid);
		MPI_Abort(MPI_COMM_WORLD, 1);
	}
	
	

	MPI_Scatterv(M, sendcounts, displs, MPI_INT, M_porcion, recvcount, MPI_INT, 0, MPI_COMM_WORLD);

	//printf("Proceso %d recibió su porción de M.\n", myid);

	return M_porcion;
}

void calcularQ(int *M_porcion, int *v, int myid, int *filas_por_proceso, int n, int *Q_parcial, int *Q) {
	/****************************
	M_porcion: porción de M repartida a un proceso
	v: vector por el cual será multiplicado M
	myid: id del proceso actual
	Q_parcial: porción de Q a ser calculada por cada proceso
	Q: matriz resultante
	****************************/
	Q_parcial = (int *) malloc(*filas_por_proceso * sizeof(int)); 
	int start;
	int end;
	
	//llenar Q_parcial de 0s
	for(i = 0; i < *filas_por_proceso; i++) {
		*(Q_parcial + i) = 0;
	}
	
	start = (myid == 0) ? 0 : 1;
	end = start + *filas_por_proceso;
	i=0;
	//for (i = 0; i < *filas_por_proceso; i++) {
		for (j = start; j < end; j++, i++) {
			for (k = 0; k < n; k++) {
				Q_parcial[i] += *(M_porcion + j * n + k) * v[k];
				//printf("M[%d][%d]: %d, v: %d\n", j, k, *(M_porcion + j * n + k), v[k]);
				//printf("Q_parcial[%d]: %d\n", i, Q_parcial[i]);
			}
		}
	//}
	
	MPI_Gather(Q_parcial, *filas_por_proceso, MPI_INT,
				Q, *filas_por_proceso, MPI_INT,
				0, MPI_COMM_WORLD);
	
	free(Q_parcial);
}

void calcularP(int *M_porcion, int myid, int numprocs, int *filas_por_proceso, int n, int *P) {
	int primos[4] = {2, 3, 5, 7};
	int start, end;
	
	start = (myid == 0) ? 0 : 1;
	end = start + *filas_por_proceso;

	for (i = start; i < end; i++) {
		for (j = 0; j < n; j++) {
			for (k = 0; k < 4; k++) {
				if (*(M_porcion + i * n + j) == primos[k])
					P[j]++;
			}
		}
	}

	MPI_Reduce(P, P, n, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
}

int calcularTp(int *P, int n) {
	int accum = 0;
	for (i = 0; i < n; i++)
		accum += P[i];
	return accum;
}

void calcularB(int *M_porcion, int *B_parcial, int myid, int numprocs, int *filas_por_proceso, int n, int *B) {
	/****************************
	M_porcion: porción de M repartida a un proceso
	B_parcial: porción de B a ser calculada por cada proceso
	myid: id del proceso actual
	numprocs: número total de procesos
	B: matriz resultante
	****************************/
	B_parcial = (int *) malloc(*filas_por_proceso * n * sizeof(int));
	if (B_parcial == NULL) {
		printf("Error al asignar memoria en proceso %d\n", myid);
		MPI_Abort(MPI_COMM_WORLD, 1);
	}
	int start;
	int end;
	int B_i;
	
	start = (myid == 0) ? 0 : 1;
	end = start + *filas_por_proceso;
	B_i = 0;

	for (i = start; i < end; i++) {
		for (j = 0; j < n; j++) {
			*(B_parcial + B_i * n + j) = *(M_porcion + i * n + j); //M[i][j]
			//printf("Proceso %d fila %d col %d: %d.\n", myid, i, j, *(B_parcial + B_i * n + j));
			if(myid != 0 || i > 0) { //si no es la primera fila
				*(B_parcial + B_i * n + j) += *(M_porcion + (i - 1) * n + j); //M[i-1][j]
			}
			if(myid != numprocs - 1 || i < end - 1) { //si no es la ultima fila
				*(B_parcial + B_i * n + j) += *(M_porcion + (i + 1) * n + j); //M[i+1][j]
			}
			if(j > 0) { //si no es la primera columna
				*(B_parcial + B_i * n + j) += *(M_porcion + i * n + j - 1); //M[i][j-1]
			}
			if(j < n - 1) { //si no es la ultima columna
				*(B_parcial + B_i * n + j) += *(M_porcion + i * n + j + 1); //M[i][j+1]
			}
			//printf("Proceso %d fila %d col %d: %d.\n", myid, i, j, *(B_parcial + B_i * n + j));
		}
		B_i++;
	}
	
	MPI_Gather(B_parcial, *filas_por_proceso * n, MPI_INT,
				B, *filas_por_proceso * n, MPI_INT,
				0, MPI_COMM_WORLD);
	
	free(B_parcial);
}


void desplegar_resultados(int n, int numprocs, int Tp, double big_start, double small_start, int *M, int *v, int *Q, int *P, int *B) {
	double small_end, big_end;
	FILE *M_file, *v_file, *Q_file, *P_file, *B_file;

	small_end = MPI_Wtime();

	printf("Tiempo desde que se recibió el valor de n hasta ahora: %f segundos\n", small_end - small_start);
	printf("n: %d\n", n);
	printf("numero total de procesos: %d\n", numprocs);
	printf("total de primos en M: %d\n", Tp);
	
	if (n > 100) {
		M_file = fopen("M.txt", "w");
		if (M_file == NULL)
			printf("Error durante la creación de archivo M.txt\n");
		else {
			for (i = 0; i < n; i++) {
				for (j = 0; j < n; j++) {
					//printf("escribiendo entrada %d %d...\n", i, j);
					fprintf(M_file, "%d ", *(M + i * n + j));
					//printf("entrada %d %d escrita\n", i, j);	
				}
				fprintf(M_file, "\n");
			} //end for
			fclose(M_file);
		} //end else

		v_file = fopen("v.txt", "w");
		if (v_file == NULL)
			printf("Error durante la creación de archivo v.txt\n");
		else {
			for (i = 0; i < n; i++)
				fprintf(v_file, "%d ", v[i]);
			fclose(v_file);
		} //end else

		Q_file = fopen("Q.txt", "w");
		if (Q_file == NULL)
			printf("Error durante la creación de archivo Q.txt\n");
		else {
			for (i = 0; i < n; i++)
				fprintf(Q_file, "%d ", Q[i]);
			fclose(Q_file);
		} //end else

		P_file = fopen("P.txt", "w");
		if (P_file == NULL)
			printf("Error durante la creación de archivo P.txt\n");
		else {
			for (i = 0; i < n; i++)
				fprintf(P_file, "%d ", P[i]);
			fclose(P_file);
		} //end else

		B_file = fopen("B.txt", "w");
		if (B_file == NULL)
			printf("Error durante la creación de archivo B.txt\n");
		else {
			for (i = 0; i < n; i++) {
				for (j = 0; j < n; j++) {
					fprintf(B_file, "%d ", *(B + i * n + j));	
				}
				fprintf(B_file, "\n");
			} //end for
			fclose(B_file);
		} //end else
	} //end if
	else {
		printf("Matriz M:\n");
		for (i = 0; i < n; i++) {
			for (j = 0; j < n; j++) {
				printf("%d ", *(M + i * n + j));	
			}
			printf("\n");
		} //end for

		printf("\nVector v:\n");
		for (i = 0; i < n; i++)
			printf("%d ", v[i]);
		printf("\n");

		printf("\nVector Q:\n");
		for (i = 0; i < n; i++)
			printf("%d ", Q[i]);
		printf("\n");

		printf("\nVector P:\n");
		for (i = 0; i < n; i++)
			printf("%d ", P[i]);
		printf("\n");

		printf("\nMatriz B:\n");
		for (i = 0; i < n; i++) {
			for (j = 0; j < n; j++) {
				printf("%d ", *(B + i * n + j));	
			}
			printf("\n");
		} //end for
	}

	big_end = MPI_Wtime();
	printf("Tiempo total de ejecución: %f segundos\n", big_end - big_start);
}
