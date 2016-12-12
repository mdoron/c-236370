#include "parallel-walsh.c"
#include <assert.h>
#include <stdio.h>


// USAGE - gcc -fopenmp -g -lgomp -lm -std=c99 -m64 pwTest.c  

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


 void testNumberOfBits() {
		//TODO - @mdoron
		return;
	}

 void	testNumberOfBits2() {
		//TODO - @mdoron
		return;
	}


 void testCreate_walsh_vector() {
		//TODO - @mdoron
		return;
	}

 void	testCreate_walsh_vector2() {
		//TODO - @mdoron
		return;
	}

 void	testSimple_parallel_walsh() {
		//TODO - @mdoron
		return;
	}

 void	testSimple_parallel_walsh2() {
		//TODO - @mdoron
		return;
	}


int main() {
	testmultiply1();
	testmultiply2();
	testNumberOfBits();
	testNumberOfBits2();
	testCreate_walsh_vector();
	testCreate_walsh_vector2();
	testSimple_parallel_walsh();
	testSimple_parallel_walsh2();
	printf("SUCCESS!!\n");
}