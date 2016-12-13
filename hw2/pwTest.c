#include "parallel-walsh.c"
#include <assert.h>
#include <stdio.h>


// USAGE - gcc -fopenmp -g -lgomp  -std=c99 -m64 pwTest.c   -lm

void testmultiply1() {
	int x[3];
	x[0]=1;x[1]=2;x[2]=3;
	int y[3];
	y[0]=4;y[1]=5;y[2]=6;
	int a = multiply(x, y, 3);
	assert (a == 32);

}

void testmultiply2() {
	int x[5];
	x[0]=1;x[1]=2;x[2]=3;x[3]=0;x[4]=0;
	int y[5];
	y[0]=4;y[1]=5;y[2]=6;y[3]=7;y[4]=8;
	int a = multiply(x, y, 5); 
	assert (a == 32);
}

 // Ignore
 void testNumberOfBits() {
		//TODO - @mdoron
		return;
	}


 // Ignore
 void	testNumberOfBits2() {
		//TODO - @mdoron
		return;
	}


 void testCreate_walsh_vector() {
 		int h[4] = {0};
		create_walsh_vector(h, 4, 0);
		for (int i=0; i< 4; i++) {
			assert(h[i] == 1);
		};
		return;
	}

 void	testCreate_walsh_vector2() {
 		int h[16] = {0};
		create_walsh_vector(h, 16, 0);
		for (int i=0; i< 16; i++) {
			assert(h[i] == 1);
		};
		return;
	}

 void	testCreate_walsh_vector3() {
 		int h[16] = {0};
		create_walsh_vector(h, 16, 0);
		for (int i=0; i< 16; i++) {
			assert(h[i] == 1);
		}
		return;
	}



 void	testSimple_parallel_walsh() {
 		int v[4] = {1, 2, 3, 4};
 		simple_parallel_walsh(v, 4);
 		printVec(v, 4);
		return;
	}

void test_fast_parallel_walsh() {
	int v[4] = {1, 2, 3, 4};
	simple_parallel_walsh(v, 4);
	printVec(v, 4);
	return;
}




int main() {
	testmultiply1();
	testmultiply2();
	testNumberOfBits();
	testNumberOfBits2();
	testCreate_walsh_vector();
	testCreate_walsh_vector2();
	testCreate_walsh_vector3();
	testSimple_parallel_walsh();
	test_fast_parallel_walsh();
	printf("SUCCESS!!\n");
}