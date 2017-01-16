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
@param resPath - will hold at each recursive call the best path
@param xCoord,yCoord,citiesNum - as always
@return - will return the best path's weight
*/
int findRec(int current, int curWeight, int* path, int* inside, int* resPath,int* xCoord,int* yCoord,int citiesNum) {
	int res[citiesNum];
	int min_w = MAX_PATH;
	if(citiesNum - 1 <= current) {
		memcpy(resPath, path, citiesNum * sizeof(int));
		return curWeight + getDist(path[0],path[citiesNum - 1],xCoord,yCoord,citiesNum);
	}
	for(int i = 0; i < citiesNum; ++i) {
		if(inside[i] == 1)
			continue;
		int check_now = curWeight + getDist(path[current],i,xCoord,yCoord,citiesNum);
		path[current + 1] = i;
		inside[i] = 1;
		int wNow = findRec(current + 1, check_now, path, inside, res,xCoord,yCoord,citiesNum);
		inside[i] = 0;
		if(wNow < min_w) {
			min_w = wNow;
			memcpy(resPath, res, citiesNum * sizeof(int));
		}
	}
	return min_w;
}



/*
@param prefix - gets prefix of a path
@param len - the prefix length
@param initialWeight - prefix path weight
@param resPath - will hold at each recursive call the best path
@param xCoord,yCoord,citiesNum - as always
@return - will return the best path's weight
*/
int find(int* prefix, int len, int initialWeight, int* resPath,int* xCoord,int* yCoord,int citiesNum) {
	int inside[citiesNum];
  int path[citiesNum];
	memset(inside, 0, citiesNum * sizeof(int));
	memcpy(path, prefix, len * sizeof(int));
	for(int i = 0; i < len; ++i)
		inside[prefix[i]] = 1;
	return findRec(len - 1, initialWeight, path, inside, resPath,xCoord, yCoord,citiesNum);
}


void fillPrefs(int procs_num,int citiesNum, int r, int size, int firstIndex, int** prefs) {
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
  int rank, procs_num;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &procs_num);
	MPI_Status status;

  if (rank==0){
		for(int i=1;i<procs_num;i++){
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
    int prefix[citiesNum], bestPath[citiesNum];
    prefix[0] = 0;
    int min_w = find(prefix, 1, 0, bestPath,xCoord,yCoord,citiesNum);
    memcpy(shortestPath, bestPath, citiesNum * sizeof(int));
    return min_w;
  }

	int prefNum = (citiesNum - 1) * (citiesNum - 2);
	int count = prefNum / procs_num;
	int firstIndex = rank * count + (rank < prefNum % procs_num ? rank : prefNum % procs_num);
	int size = rank < prefNum % procs_num ? count + 1 : count;
	int** prefs = malloc(size * sizeof(*prefs));

	fillPrefs(procs_num,citiesNum, rank, size, firstIndex, prefs);
	int min_w = MAX_PATH;
	int bestPath[citiesNum];
	int path[citiesNum];
	for(int i = 0; i < size; ++i) {
		int dist = getDist(prefs[i][0],prefs[i][1],xCoord,yCoord,citiesNum) + getDist(prefs[i][1],prefs[i][2],xCoord,yCoord,citiesNum);
		int weight = find(prefs[i], PREF_SIZE, dist, path,xCoord,yCoord,citiesNum);
		free(prefs[i]);
		if(weight < min_w) {
			min_w = weight;
			memcpy(bestPath, path, sizeof(*path)*citiesNum);
		}
	}
	free(prefs);
	int* w, *paths;
	if(rank == 0) {
		w = malloc(sizeof(int) * procs_num);
		paths = malloc(sizeof(int) * (procs_num * citiesNum));
	}
	MPI_Gather(&min_w, 1, MPI_INT, w, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Gather(bestPath, citiesNum, MPI_INT, paths, citiesNum, MPI_INT, 0, MPI_COMM_WORLD);
	if(rank == 0) {
		int min_idx = 0;
		for(int i = 0; i < procs_num; ++i)
			if(w[i] < min_w) {
				min_w = w[i];
				min_idx = i;
			}
		memcpy(shortestPath, paths + min_idx*citiesNum, citiesNum * sizeof(int));
		free(w);
		free(paths);
	}

	return min_w;

}
