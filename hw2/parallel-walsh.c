#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>	

 

void create_walsh_vector(int* h, int vSize, int colNum);
int multiply(int* x, int* y, int size);


void printVec(int* v, int vSize) {
	for (int i=0; i<vSize; i++) {
		printf("%d\n", v[i]);
	}
}
// u = H*v

void fast_parallel_walsh(int* v, int vSize) {

}

void simple_parallel_walsh(int* v, int vSize) {
	int u[vSize];
	int h[vSize];
	for(int colNum = 0; colNum < vSize; colNum++) {
		create_walsh_vector(h, vSize, colNum);
		u[colNum] = multiply(h, v, vSize);
	}
	#pragma omp parallel
	{
		#pragma omp for
		for (int i=0; i<vSize; i++) {
			v[i] = u[i];
		}
		#pragma omp barrier
	}
}

// Precondition: length(x) == length(y) == size
int multiply(int* x, int* y, int size) {
	int mul = 0;
	#pragma omp parallel
	{
		#pragma omp for
		for (int i=0; i<size; i++) {
			#pragma omp atomic
			mul += x[i]*y[i];
		}
		#pragma omp barrier
	}

	return mul;
}


int NumberOfSetBits(int i) {
	//#pragma omp atomic
     i = i - ((i >> 1) & 0x55555555);
     i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
     return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

void create_walsh_vector(int* h, int vSize, int colNum) {
	#pragma omp parallel
	{
		#pragma omp for
		for (int rowNum=0; rowNum<vSize; rowNum++) {
			h[rowNum] = pow(-1.0, NumberOfSetBits(rowNum & colNum)); 
		}
		#pragma omp barrier
	}
}
