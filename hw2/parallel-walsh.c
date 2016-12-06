#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

// u = H*v

void fast_parallel_walsh(int* v, int vSize) {

}

void simple_parallel_walsh(int* v, int vSize) {
	int u[vSize];
	int h[vSize];
	// zerofy(u);
	// zerofy(h);
	for(int colNum = 1; colNum <= vSize; colNum) {
		create_walsh_vector(h, vSize, colNum);
		u[colNum] = multiply(h, v, vSize);
	}

	v = u;
}

// Precondition: length(x) == length(y) == size
int multiply(int* x, int* y, int size) {
	int mul = 0;
	for (int i=0; i<size; i++) {
		mul += x[i]*y[i];
	}
	return mul;
}

void create_walsh_vector(int* h, int vSize, int colNum) {
	
}
