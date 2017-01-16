#include <mpi.h>

int abs(int x);
void setDistances(int* x, int* y, int n, int** dist);
void nextTreePath(int* tree_path, int height, int max);

enum Tag { 
	CITIES_NUM_TAG,
	XCOORD_TAG,
	YCOORD_TAG,
	PATHS_NUM_TAG,
	HEIGHT_TAG,
	TREE_PATH_TAG
};

int tsp_main(int citiesNum, int xCoord[], int yCoord[], int shortestPath[]) {
	int my_rank, procs_num;
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &procs_num);
	MPI_Status status;
	int* rec_buf, send_buf, worker_x_coord, worker_y_coord, tree_path;
	int worker_cities_num, height = 0, paths_num = 1;

	if(my_rank == 0) {
		while(paths_num<procs_num){
			height++;
			paths_num *= citiesNum-height;
		}
		paths_num = (paths_num+procs_num-1)/procs_num;

		tree_path = malloc(sizeof(*tree_path)*citiesNum);
		for(int i=0; i<citiesNum; i++){
			tree_path[i]=1;
		}

		for(int i=1; i < procs_num; i++){
			MPI_Bsend(&citiesNum, 1, MPI_INT, i, CITIES_NUM_TAG, MPI_COMM_WORLD);
			MPI_Bsend(xCoord, citiesNum, MPI_INT,i, XCOORD_TAG, MPI_COMM_WORLD);
			MPI_Bsend(yCoord, citiesNum, MPI_INT,i, YCOORD_TAG, MPI_COMM_WORLD);
			MPI_Bsend(tree_path, citiesNum, MPI_INT, i, TREE_PATH_TAG, MPI_COMM_WORLD);
			MPI_Bsend(&paths_num, 1, MPI_INT, i, PATHS_NUM_TAG, MPI_COMM_WORLD);
			MPI_Bsend(&height, 1, MPI_INT, i, HEIGHT_TAG, MPI_COMM_WORLD);
			initTreePath(tree_path, height, citiesNum, i);
		}
	} else {
		MPI_Recv(&worker_cities_num, 1, MPI_INT, 0, CITIES_NUM_TAG, MPI_COMM_WORLD, &status);
		worker_x_coord = malloc(sizeof(*worker_x_coord)*worker_cities_num);
		worker_y_coord = malloc(sizeof(*worker_x_coord)*worker_cities_num);
		MPI_Recv(worker_x_coord, worker_cities_num, MPI_INT, 0, XCOORD_TAG, MPI_COMM_WORLD, &status);
		MPI_Recv(worker_y_coord, worker_cities_num, MPI_INT,0, YCOORD_TAG, MPI_COMM_WORLD, &status);
		MPI_Recv(&tree_path, worker_cities_num, MPI_INT, 0, TREE_PATH_TAG, MPI_COMM_WORLD, &status);
		MPI_Recv(&paths_num, 1, MPI_INT, 0, PATHS_NUM_TAG, MPI_COMM_WORLD, &status);
		MPI_Recv(&height, 1, MPI_INT, 0, HEIGHT_TAG, MPI_COMM_WORLD, &status);
	}
	MPI_Barrier(MPI_COMM_WORLD);

	int** dist = malloc(sizeof(*dist) * worker_cities_num);
	for(int i=0; i < worker_cities_num; i++)
		dist[i] = malloc(sizeof(*dist[i]) * worker_cities_num); // May yield an error - not like assaf
	
	setDistances(worker_x_coord, worker_y_coord, worker_cities_num, dist);
	 
	// Copied from Assaf ===================================================================
	//======================================================================================
	int* TempSPath = malloc(sizeof(int)*(localCitiesNum));
	int* sPath = malloc(sizeof(int)*(localCitiesNum));
	while(count!=0){
		tempWeight = lightestPath(worker_cities_num,dist,rout,level,TempSPath,weight,myrank);
		if(tempWeight<weight || weight == -1){
			weight=tempWeight;
			copyPathes(sPath,TempSPath,worker_cities_num);
		}

		nextPermutation(rout,level,worker_cities_num);
		count--;
	}
	//sending back to rank=0 problem here all the reset seems to work ok
	if(myrank==0){
		rec_buf=malloc(sizeof(int)*proc_num*(worker_cities_num+1));
		for(int i=0;i<worker_cities_num;i++){
			shortestPath[i]=sPath[i];
		}
	}
	send_buf = malloc(sizeof(int)*(worker_cities_num+1));
	for(int i=0;i<worker_cities_num;i++){
		send_buf[i]=sPath[i];
	}
	send_buf[worker_cities_num]=weight;
	MPI_Gather(send_buf,worker_cities_num+1,MPI_INT,rec_buf,worker_cities_num+1,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD);
	if(myrank==0){
		for(int i=0;i<proc_num;i++){
			if(rec_buf[(i+1)*(worker_cities_num+1)-1]<weight && rec_buf[(i+1)*(worker_cities_num+1)-1]!=-1){
				weight=rec_buf[(i+1)*(worker_cities_num+1)-1];
				for(int j=0;j<worker_cities_num;j++){
					shortestPath[j]=rec_buf[i*(worker_cities_num)+i+j];
				}
			}
		}
	}

	MPI_Barrier(MPI_COMM_WORLD);
	return weight;	

}

int abs(int x) {
	return x > 0 ? x : -x;
}

void setDistances(int* x, int* y, int n, int** dist) {
	for(int i=0; i<n; i++)
		for(int j=0; j<n; j++)
			Matrix[i][j] = abs(x[i]-x[j]) + abs(y[i]-y[j]);
}

void nextTreePath(int* tree_path, int height, int max) {
	int carry=1;
	for(int i=height; i>=0; i--){
		if ( ( tree_path[i] == max-i ) && carry==1 ){
			tree_path[i] = 1;
			carry = 1;
		} else if(carry == 1){
			tree_path[i]++;
			carry = 0;
		}
	}
}

void initTreePath(int* tree_path, int height, int max, int proc_idx) {
	if (tree_path[height] == max-height)
		return;
	else
		tree_path[height] = proc_idx;
}

//clear traveldCities, tempShortestPath and weight
void clearPath(int* rout,int level,int* traveldCities,int* tempShortestPath,int size){
	/*for(int i=0;i<size;i++){
		traveldCities[i]=0;
		tempShortestPath[i]=0;
	}*/
	memset(traveldCities,0,size*sizeof(int));
	memset(tempShortestPath,0,size*sizeof(int));
	for(int i=0;i<=level;i++){
		int temp=rout[i];
		int j=0;
		while(temp>0){
			if(traveldCities[j]==0){
				temp--;
			}
			j++;
		}
		traveldCities[j-1]=1;
		tempShortestPath[i]=j-1;
	}


}

/*
*the algoritem will be as follow:
	get the start of the rout and calculate all the posebillities of this start
	while reducing the path which are greater than the minimum

	rout - array in length of citiesNum, the number in each array cell represent the next untraveled city
	(i.e if the number is 5 it mean the fifth untraveled city in traveldCities)
	level - the last cell in rout that should be refrenced to as starter
*/
int lightestPath (int citiesNum,int** dist,int* rout,int level,int shortestPath[],int weight,int rank){
	int minPathWeight=weight;
	if(rout[0]!=1)	{
		return -1;
	}
	int tempPathWeight=0;
	int startingWeight=0;
	int currentCity=0;
	int startCity=0;
	int* tempShortestPath= malloc(sizeof(int)*citiesNum);
	int* tempRout = malloc(sizeof(int)*citiesNum);
	for(int i=0;i<citiesNum;i++){
		tempRout[i]=rout[i];
	}
	/*initiate all from  rout*/
	int* traveldCities = malloc(sizeof(int)*citiesNum);
	for(int i=0;i<citiesNum;i++){
		traveldCities[i]=0;
	}
	for(int i=0;i<=level;i++){
		int temp=rout[i];
		int j=0;
		while(temp>0){
			if(traveldCities[j]==0){
				temp--;
			}
			j++;
		}
		traveldCities[j-1]=1;
		if(dist[currentCity][j-1]!=-1){
			tempPathWeight+=dist[currentCity][j-1];
		}
		currentCity=j-1;
		shortestPath[i]=j-1;
		tempShortestPath[i]=j-1;
	}
	startCity = currentCity;
	int advancingPoint=citiesNum-1;
	startingWeight=tempPathWeight;
	/*initiate done*/

	while(tempRout[level]==rout[level]){
		currentCity = startCity;
		advancingPoint=citiesNum-1;

		for(int i=level+1;i<citiesNum;i++){
			int tempCity = nextCity(traveldCities,tempRout[i],citiesNum);
			tempPathWeight+=dist[currentCity][tempCity];
			if(tempPathWeight>minPathWeight && minPathWeight!=-1){// if break need to advance in other rout which mean that the next premutation should advance cell "i"
				advancingPoint=i; //from here there can be faster solution think about it...
				break;
			}
			currentCity=tempCity;
			tempShortestPath[i]=currentCity;
		}
		tempPathWeight+=dist[currentCity][0];
		if(minPathWeight==-1 || tempPathWeight<minPathWeight){
			minPathWeight=tempPathWeight;
			copyPathes(shortestPath,tempShortestPath,citiesNum);
			/*int* x= tempShortestPath;
			tempShortestPath=shortestPath;
			shortestPath=x;*/
		}
		nextPermutation(tempRout,advancingPoint,citiesNum);
		clearPath(tempRout,level,traveldCities,tempShortestPath,citiesNum);
		tempPathWeight=startingWeight;
	}
	free(traveldCities);
	return minPathWeight;
}

int nextCity(int* traveldCities,int zeroNumber,int size){
	int place=0;
	for(int i=0;i<size;i++){
		if(traveldCities[i]==0){
			zeroNumber--;
		}
		if(zeroNumber==0){
			place = i;
			break;
		}
	}
	traveldCities[place]=1;`
	return place;
}

void copyPathes(int* dst,int* src,int size){
	for(int i=0;i<size;i++){
		dst[i]=src[i];
	}
}