#include <stdio.h>
#include <stdlib.h>

int main() {

	int n, i, j;
	int* M, v;
	srand(time(NULL));
	
	printf("Inserte el numero de filas/columnas: ");
	scanf("%d", &n);

	M = (int*) malloc(n * n * sizeof(int));
	v = (int*) malloc(n * sizeof(int));
			  
	//Se generan M y v
	for (i = 0; i < n; i++) {
		for (j = 0; j < n; j++) {
			M[i][j] = rand();	
		}
		v[i] = rand();
	}
			  
	//magia
	magia();
			  
	//desplegar resultados
	
	
	free(M);
	free(v);
	return 0;
}
