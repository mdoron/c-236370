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

void copyVector(int* v, int* u, int vSize) {
	#pragma omp parallel
	{
		#pragma omp for
		for (int i=0; i<vSize; i++) {
			v[i] = u[i];
		}
		#pragma omp barrier
	}
}
// u = H*v

void fast_parallel_walsh2(int* v, int vSize) {
	int u[vSize];
	for (int i=1; i<=log(vSize)/log(2); i++) {
		int divid = pow(2, i);
		for(int k=0; k<divid; k++) {
			if (k % 2 == 0) {
				for (int j=0; j<vSize/divid; j++) {
					u[j+k*vSize/divid] = v[j+k*vSize/divid] + v[j+(k+1)*vSize/divid];
				}
			} else {
				for (int j=0; j<vSize/divid; j++) {
					u[j+k*vSize/divid] = - v[j+k*vSize/divid] + v[j+(k-1)*vSize/divid];
				}
			}
		}
		copyVector(v, u, vSize);
	}
}

void fast_parallel_walsh(int* v, int vSize) {
	#pragma omp parallel
	{
		register int base_size = vSize;
		register int len = base_size >> 1;


        #pragma omp for schedule(static)
		for (register int j=0; j<len; ++j) {
			register int temp = v[j+len];
			v[j+len] = v[j] - temp; 
			v[j] += temp;
		}
        base_size >>= 1;
		len >>= 1;

		for (register int itr=2; itr!=vSize; itr <<= 1) {

			#pragma omp for schedule(static)
			for(register int k=0; k<itr; ++k) {

				register int base_pos = k*base_size;

				for (register int j=base_pos; j<base_pos+len; ++j) {
					register int temp = v[j+len];
					v[j+len] = v[j] - temp; 
					v[j] += temp;
				}
			}
			base_size >>= 1;
			len >>= 1;
		}
	}
}

void simple_parallel_walsh(int* v, int vSize) {
	int u[vSize];
	int h[vSize];
	for(int colNum = 0; colNum < vSize; colNum++) {
		create_walsh_vector(h, vSize, colNum);
		u[colNum] = multiply(h, v, vSize);
	}
	copyVector(v, u, vSize);
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
