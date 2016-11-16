public class ParallelGameOfLife implements GameOfLife {

	/*
	this should run the simulation game in parallell mode
	 */
	public boolean[][][] invoke(boolean[][] initalField, int hSplit, int vSplit,
			int generations) {
		boolean[][][] x=new boolean[2][][];
		//TODO: improve this to be only one
		x[0] = get_gen(initalField,hSplit,vSplit, generations-1);
		x[1] = get_gen(x[0],hSplit,vSplit, 1);
		return x;
	}

	public boolean[][] get_gen(boolean[][] initalField, int hSplit, int vSplit, int generations) {

		// init the field -> copy the field to the local array
		//TODO: parallelize this
		boolean[][] input=new boolean[initalField.length][];
		for (int i=0;i<initalField.length;i++){
			input[i]=new boolean[initalField[0].length];
			for (int k=0;k<input[i].length;k++){
				input[i][k]=initalField[i][k];
			}
		}

		/*
		for each block we initiate a producer (maybe not needed) ,consumer , the block and we add it to the queues of neighs
		 */
		for(int row=0;row<hSplit;row++) {
			for(int col = 0;col < vSplit;col++) {
				new LifeConsumer().start();
			}
		}
		//then we need to combine the blocks to a matrix and return it
		return input;
	}

	private int numNeighbors(int x,int y, boolean[][] field ){
		int counter=(field[x][y]?-1:0);
		for (int i=x-1; i<=x+1;i++ ){
			if ((i < 0) || (i >= field.length)){ continue ; }
			for (int j=y-1; j<=y+1;j++){
				if (j<0||j>=field[0].length){ continue ; }
				counter+=(field[i][j]?1:0);
			}
		}
		return counter;
	}



}
