#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// This is a forward declaration of tsp_main. The definition should
// reside in tsp_static.c on the static implementation, and in
// tsp.c on the dynamic implementation.
// Which c file is used depends on the make configuration used for
// building the executable.
int tsp_main(int citiesNum, int xCoord[], int yCoord[], int shortestPath[]);

// NOTE: you can change this file for your own testing, but you should not
// submit the modified file - 
// your solution is going to be tested with a different main file.
int main(int argc, char** argv)  
{
	// example input, with 18 cities. set citiesNum to less for an easier start.
	int citiesNum = 18;
	int xCoord[] = {1, 3, 5, 9, 2,  3,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1};   
	int yCoord[] = {1, 3, 5, 7, 9, 11, 13, 15, 17, 20, 22, 24, 26, 27, 29, 30, 31, 32};   
	int* shortestPath = malloc(citiesNum * sizeof(int));  
	int i, myRank, minPathLen;  
	MPI_Init(&argc, &argv);      // initialize MPI - no need to do that in your code!
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
	
	// run the computation. on the static solution, tsp_main should be implemented
	// in tsp_static.c, while on the dynamic solution, tsp_main should be in tsp.c.
	clock_t begin = clock();
	minPathLen = tsp_main(citiesNum, xCoord, yCoord, shortestPath);
	clock_t end = clock();

	if (myRank == 0)
	{
		printf("The shortest path, %d long, is:\n", minPathLen);  
		for (i = 0; i < citiesNum; i++)
		{
			// print the city (and its distance from the next city in the path)
			printf("%d (%d) ", shortestPath[i], 
					abs(xCoord[shortestPath[i]] - xCoord[shortestPath[(i + 1) % citiesNum]]) + 
					abs(yCoord[shortestPath[i]] - yCoord[shortestPath[(i + 1) % citiesNum]]) );
		}
		printf("%d\n", shortestPath[0]);
		printf("Execution time: %g seconds\n", (double)(end - begin) / CLOCKS_PER_SEC);
	}

	MPI_Finalize ();	// shut down MPI - no need to do that in your code!
	return 0;  
}  
 
