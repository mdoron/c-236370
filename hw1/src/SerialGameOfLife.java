public class SerialGameOfLife implements GameOfLife {
	public boolean[][][] invoke(boolean[][] initalField, int hSplit, int vSplit, int generations) {
		boolean[][][] x=new boolean[2][][];	
		x[0] = get_gen(initalField,hSplit,vSplit, generations-1);
		x[1] = get_gen(initalField,hSplit,vSplit, generations);
		return x;
	}
	
	public boolean[][] get_gen(boolean[][] initalField, int hSplit, int vSplit, int generations) {
	
		// init the field -> copy the field to the local array
		boolean[][] input=new boolean[initalField.length][];
		
		for (int i=0;i<initalField.length;i++){
			input[i]=new boolean[initalField[0].length];
			for (int k=0;k<input[i].length;k++){
				input[i][k]=initalField[i][k];
			}
		}

		boolean[][] result=new boolean[initalField.length][];
		
		for (int g=0;g<generations;g++){
			for (int i=0;i<initalField.length;i++){
				if (result[i]==null){
					// using first time -> copy the array
					result[i]=new boolean[initalField[i].length];
				}
				
				for (int j=0;j<initalField[i].length;j++){
					int numNeighbors=numNeighbors(i,j,input);
					result[i][j]=false;
					if (numNeighbors==3 || (input[i][j]&& numNeighbors==2)){
						result[i][j]=true;
					}
				}
			}
			boolean[][] tmp;
			tmp=input;
			input=result;
			result=tmp;
		}
		return input;
	}
	
	private int numNeighbors(int x,int y, boolean[][] field ){
		int counter=(field[x][y]?-1:0);
		for (int i=x-1; i<=x+1;i++ ){
			if (i<0||i>=field.length){ continue ; }
			for (int j=y-1; j<=y+1;j++){
				if (j<0||j>=field[0].length){ continue ; }
				counter+=(field[i][j]?1:0);
			}
		}
		return counter;
	}
}
