#include <stdlib.h>
#include <omp.h>
#include <math.h>

/**
 * Declerations
 **/
void create_walsh_vector(int* h, int vSize, int colNum);
int multiply(int* x, int* y, int size);

// u = H*v

void fast_parallel_walsh(int* v, int vSize) {

}

void simple_parallel_walsh(int* v, int vSize) {
	int u[vSize];
	int h[vSize];
	#pragma omp parallel for
	for(int colNum = 1; colNum <= vSize; colNum++) {
		create_walsh_vector(h, vSize, colNum);
		u[colNum] = multiply(h, v, vSize);
	}

	v = u;
}

// Precondition: length(x) == length(y) == size
int multiply(int* x, int* y, int size) {
	int mul = 0;
	#pragma omp parallel for
	for (int i=0; i<size; i++) {
		mul += x[i]*y[i];
	}
	return mul;
}


//TODO - DORON document?
int NumberOfSetBits(int i) {
	#pragma omp parallel
     i = i - ((i >> 1) & 0x55555555);
     i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
     return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

//TODO - DORON I did understand this from the wiki but shuoldve been documenteed
//TODO @mdoron - pow doesnt recognize the second parameter
void create_walsh_vector(int* h, int vSize, int colNum) {
	#pragma omp parallel for
	for (int rowNum=0; rowNum<vSize; rowNum++) {
		long double temp = NumberOfSetBits(rowNum ^ colNum);
		h[rowNum] = pow(-1.0,2); //TODO @mdoron - change 2 to something meaningful like temp but without breaking the code
	}
}
