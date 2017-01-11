#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>


int abs(int x){
	if(x<0)
		return -x;
	return x;
}

void matrixCreate(int* xCoord,int* yCoord,int** Matrix,int n){//the matrix is always nXn
	for(int i=0;i<n;i++){
		for(int j=0;j<n;j++){
			if(i==j){
				Matrix[i][j]=-1;
			}else{
				Matrix[i][j]=(abs(xCoord[i]-xCoord[j])+abs(yCoord[i]-yCoord[j]));
			}
		}
	}
}

/*
*return the next city and fill traveldCities accordingly
*/
int nextCity(int* traveldCities,int zeroNumber,int size){
	int place=0;
	for(int i=0;i<size;i++){
		if(traveldCities[i]==0){
			zeroNumber--;
		}
		if(zeroNumber==0){
			place =i;
			break;
		}
	}
	traveldCities[place]=1;
	return place;
}

void copyPathes(int* dst,int* src,int size){
	for(int i=0;i<size;i++){
		dst[i]=src[i];
	}
}

//should implement it from advancing point and down
void nextPermutation(int* rout,int advancingPoint,int max){
	int carry=1;
	for(int i=advancingPoint;i>=0;i--){
		if((rout[i]==(max-i))&&(carry==1)){
			rout[i]=1;
			carry=1;
			continue;
		}
		if(carry==1){
			rout[i]++;
			carry=0;
		}
	}
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
int lightestPath (int citiesNum,int** matrix,int* rout,int level,int shortestPath[],int weight,int rank){
	int minPathWeight=weight;
	if(rout[0]!=1){
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
		if(matrix[currentCity][j-1]!=-1){
			tempPathWeight+=matrix[currentCity][j-1];
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
			tempPathWeight+=matrix[currentCity][tempCity];
			if(tempPathWeight>minPathWeight && minPathWeight!=-1){// if break need to advance in other rout which mean that the next premutation should advance cell "i"
				advancingPoint=i; //from here there can be faster solution think about it...
				break;
			}
			currentCity=tempCity;
			tempShortestPath[i]=currentCity;
		}
		tempPathWeight+=matrix[currentCity][0];
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


// The static parellel algorithm main function.
int tsp_main(int citiesNum, int xCoord[], int yCoord[], int shortestPath[])
{
	int myrank, numberOfProcs;
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcs);
	MPI_Status status;
	int* rbuf;
	int* sendBuf;

	int localCitiesNum;
	int* localX;
	int* localY;
	int level=0;
	int NumberOfRoutsPerLevel=1;

	if (myrank==0){
		/*initialize the send variabels*/
		localCitiesNum=citiesNum;
		localX = malloc(sizeof(int)*localCitiesNum);
		localY = malloc(sizeof(int)*localCitiesNum);
		for(int i=0;i<localCitiesNum;i++){
			localX[i]=xCoord[i];
			localY[i]=yCoord[i];
		}
		while(NumberOfRoutsPerLevel<numberOfProcs){
			level++;
			NumberOfRoutsPerLevel*=(localCitiesNum-level);
		}
		NumberOfRoutsPerLevel = (NumberOfRoutsPerLevel+numberOfProcs-1)/numberOfProcs;
		/*finished initialize the send variables*/

		/*sending*/
		for(int i=1;i<numberOfProcs;i++){
			MPI_Bsend(&localCitiesNum,1,MPI_INT,i,0,MPI_COMM_WORLD);
			MPI_Bsend(localX,localCitiesNum,MPI_INT,i,1,MPI_COMM_WORLD);
			MPI_Bsend(localY,localCitiesNum,MPI_INT,i,2,MPI_COMM_WORLD);
			MPI_Bsend(&NumberOfRoutsPerLevel,1,MPI_INT,i,3,MPI_COMM_WORLD);
			MPI_Bsend(&level,1,MPI_INT,i,4,MPI_COMM_WORLD);
		}
	}
	else{
		MPI_Recv(&localCitiesNum,1,MPI_INT,0,0,MPI_COMM_WORLD,&status);
		localX=malloc(sizeof(int)*localCitiesNum);
		localY=malloc(sizeof(int)*localCitiesNum);
		MPI_Recv(localX,localCitiesNum,MPI_INT,0,1,MPI_COMM_WORLD,&status);
		MPI_Recv(localY,localCitiesNum,MPI_INT,0,2,MPI_COMM_WORLD,&status);
		MPI_Recv(&NumberOfRoutsPerLevel,1,MPI_INT,0,3,MPI_COMM_WORLD,&status);
		MPI_Recv(&level,1,MPI_INT,0,4,MPI_COMM_WORLD,&status);
	}
	MPI_Barrier(MPI_COMM_WORLD);

	int* TempSPath = malloc(sizeof(int)*(localCitiesNum));
	int* sPath = malloc(sizeof(int)*(localCitiesNum));
	int** matrix = malloc(sizeof(int*)*localCitiesNum);
	for(int i=0;i<localCitiesNum;i++){
		matrix[i]=malloc(sizeof(int*)*localCitiesNum);
	}
	matrixCreate(localX,localY,matrix,localCitiesNum);
	/*each proccess initial it's rout*/
	int* rout= malloc(sizeof(int)*localCitiesNum);
	for(int i=0;i<localCitiesNum;i++){
		rout[i]=1;
	}

	int count = NumberOfRoutsPerLevel;
	int weight=-1,tempWeight=0;
	if(level!=0){
		for(int i=0;i<myrank*NumberOfRoutsPerLevel;i++){
			nextPermutation(rout,level,localCitiesNum);
		}
	}

	//while count !=0 advance whith level as advancing point
	while(count!=0){
		tempWeight = lightestPath(localCitiesNum,matrix,rout,level,TempSPath,weight,myrank);
		if((tempWeight<weight || weight == -1)&&(tempWeight!=-1)){
			weight=tempWeight;
			copyPathes(sPath,TempSPath,localCitiesNum);
		}

		nextPermutation(rout,level,localCitiesNum);
		count--;
	}
	//sending back to rank=0 problem here all the reset seems to work ok
	if(myrank==0){
		rbuf=malloc(sizeof(int)*numberOfProcs*(localCitiesNum+1));
		for(int i=0;i<localCitiesNum;i++){
			shortestPath[i]=sPath[i];
		}
	}
	sendBuf = malloc(sizeof(int)*(localCitiesNum+1));
	for(int i=0;i<localCitiesNum;i++){
		sendBuf[i]=sPath[i];
	}
	sendBuf[localCitiesNum]=weight;
	//copyPathes(sendBuf,sPath,localCitiesNum);
	MPI_Gather(sendBuf,localCitiesNum+1,MPI_INT,rbuf,localCitiesNum+1,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD);
	if(myrank==0){
		for(int i=0;i<numberOfProcs;i++){
			if(rbuf[(i+1)*(localCitiesNum+1)-1]<weight && rbuf[(i+1)*(localCitiesNum+1)-1]!=-1){
				weight=rbuf[(i+1)*(localCitiesNum+1)-1];
				for(int j=0;j<localCitiesNum;j++){
					shortestPath[j]=rbuf[i*(localCitiesNum)+i+j];
				}
			}
		}
	}

	MPI_Barrier(MPI_COMM_WORLD);
	return weight;
	//there is still litle problem - probably because some part of the matrix that is i,i is counted and should not be count
}
