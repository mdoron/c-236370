#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
@author - Raviv Rachmiel
@since - 07/01/2017
*/

#define MAX_PATH 10000000
#define SERIAL_VAR 7
#define PREF_SIZE 4


//normal ABSolute function as implemented in math.h
int ABS(int a) {
  return a>0? a : a*(-1);
}

//gets 2 cities and coordinates and citiesNum
//returns the manhatten distance
int getDist(int city1,int city2,int *xCoord,int* yCoord,int citiesNum) {
  return city1==city2? 0 : (ABS(xCoord[city1]-xCoord[city2])+ABS(yCoord[city1]-yCoord[city2]));
}


/*
@param current - the current index we are working on - last one will be the stop
@param path - current path
@param curWeight - curr path weight
@param inside - the already used cities
@param bestPath - will hold at each recursive call the best path
@param xCoord,yCoord,citiesNum - as always
@return - will return the best path's weight
*/
int findRec(int current, int curWeight, int* path, int* inside, int* bestPath,int* xCoord,int* yCoord,int citiesNum) {
	int res[citiesNum];
	int bestWeight = MAX_PATH;
	if(citiesNum - 1 <= current) {
		memcpy(bestPath, path, citiesNum * sizeof(int));
		return curWeight + getDist(path[0],path[citiesNum - 1],xCoord,yCoord,citiesNum);
	}
	for(int i = 0; i < citiesNum; ++i) {
		if(inside[i] == 1)
			continue;
		int check_now = curWeight + getDist(path[current],i,xCoord,yCoord,citiesNum);
		path[current + 1] = i;
		inside[i] = 1;
		int weight = findRec(current + 1, check_now, path, inside, res,xCoord,yCoord,citiesNum);
		inside[i] = 0;
		if(weight < bestWeight) {
			bestWeight = weight;
			memcpy(bestPath, res, citiesNum * sizeof(int));
		}
	}
	return bestWeight;
}



/*
@param prefix - gets prefix of a path
@param len - the prefix length
@param initialWeight - prefix path weight
@param bestPath - will hold at each recursive call the best path
@param xCoord,yCoord,citiesNum - as always
@return - will return the best path's weight
*/
int find(int* prefix, int len, int initialWeight, int* bestPath,int* xCoord,int* yCoord,int citiesNum) {
	int inside[citiesNum];
  int path[citiesNum];
	memset(inside, 0, citiesNum * sizeof(int));
	memcpy(path, prefix, len * sizeof(int));
	for(int i = 0; i < len; ++i)
		inside[prefix[i]] = 1;
	return findRec(len - 1, initialWeight, path, inside, bestPath,xCoord, yCoord,citiesNum);
}


void fillPrefs(int procsNum,int citiesNum, int r, int size, int firstIndex, int** prefs) {
	int prefCol = 0;
	for(int i = firstIndex; i < firstIndex + size; i++) {
		prefs[prefCol] = malloc(PREF_SIZE * sizeof(int));
		prefs[prefCol][0] = 0;
		prefs[prefCol][2] = 1 + i % (citiesNum - 3);
		prefs[prefCol][1] = 1 + i / (citiesNum - 2);
		if(prefs[prefCol][1] <= prefs[prefCol][2]) {
			prefs[prefCol][2]++;
		}
		prefCol++;
	}
}

// The static parellel algorithm main function.
/*
gets the cities num and coordinates
returns the shortestPath
*/
int tsp_main(int citiesNum, int xCoord[], int yCoord[], int shortestPath[]) {
  int rank, procsNum;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &procsNum);
	MPI_Status status;

  if (rank==0){
		for(int i=1;i<procsNum;i++){
			MPI_Bsend(&citiesNum,1,MPI_INT,i,0,MPI_COMM_WORLD);
			MPI_Bsend(xCoord,citiesNum,MPI_INT,i,1,MPI_COMM_WORLD);
			MPI_Bsend(yCoord,citiesNum,MPI_INT,i,2,MPI_COMM_WORLD);
		}
	}
	else{
		MPI_Recv(&citiesNum,1,MPI_INT,0,0,MPI_COMM_WORLD,&status);
		MPI_Recv(xCoord,citiesNum,MPI_INT,0,1,MPI_COMM_WORLD,&status);
		MPI_Recv(yCoord,citiesNum,MPI_INT,0,2,MPI_COMM_WORLD,&status);
	}
	MPI_Barrier(MPI_COMM_WORLD);

  if(citiesNum < SERIAL_VAR) {
    if(rank > 0)
      return MAX_PATH;
    int prefix[citiesNum];
    prefix[0] = 0;
    int bestPath[citiesNum];
    int minWeight = find(prefix, 1, 0, bestPath,xCoord,yCoord,citiesNum);
    memcpy(shortestPath, bestPath, citiesNum * sizeof(int));
    return minWeight;
  }

	int prefNum = (citiesNum - 1) * (citiesNum - 2);
	int count = prefNum / procsNum;
	int firstIndex = rank * count + (rank < prefNum % procsNum ? rank : prefNum % procsNum);
	int size = rank < prefNum % procsNum ? count + 1 : count;
	int** prefs = malloc(size * sizeof(*prefs));

	fillPrefs(procsNum,citiesNum, rank, size, firstIndex, prefs);
	int minWeight = MAX_PATH;
	int bestPath[citiesNum];
	int path[citiesNum];
	for(int i = 0; i < size; ++i) {
		int dist = getDist(prefs[i][0],prefs[i][1],xCoord,yCoord,citiesNum) + getDist(prefs[i][1],prefs[i][2],xCoord,yCoord,citiesNum);
		int weight = find(prefs[i], PREF_SIZE, dist, path,xCoord,yCoord,citiesNum);
		free(prefs[i]);
		if(weight < minWeight) {
			minWeight = weight;
			memcpy(bestPath, path, sizeof(*path)*citiesNum);
		}
	}
	free(prefs);
	int* weights;
	int* paths;
	if(rank == 0) {
		weights = malloc(sizeof(int) * procsNum);
		paths = malloc(sizeof(int) * (procsNum * citiesNum));
	}
	MPI_Gather(&minWeight, 1, MPI_INT, weights, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Gather(bestPath, citiesNum, MPI_INT, paths, citiesNum, MPI_INT, 0, MPI_COMM_WORLD);
	if(rank == 0) {
		int best = 0;
		for(int i = 0; i < procsNum; ++i)
			if(weights[i] < minWeight) {
				minWeight = weights[i];
				best = i;
			}
		memcpy(shortestPath, paths + best*citiesNum, citiesNum * sizeof(int));
		free(weights);
		free(paths);
	}

	return minWeight;

}
