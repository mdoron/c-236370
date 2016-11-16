import java.util.Queue;
import java.util.concurrent.ConcurrentLinkedQueue;

public class ParallelGameOfLife implements GameOfLife {

	boolean[][][] blocksArray;
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
	
	private boolean[][] extractBlock(boolean[][] initialField, int x, int y, int lx, int ly) {
		boolean[][] $ = new boolean[lx][ly];
		for(int i=0; i<lx ; i++) {
			for (int j=0; j<ly ; j++) {
				$[i][j] = initialField[x+i][y+j];
			}
		}
		return $;
	}
	
//	private boolean[][][] splitFieldToBlocks(boolean[][] initialField, int hSplit, int vSplit) {
//		int height = initialField.length/hSplit;
//		int length = initialField[0].length/vSplit;
//		boolean[][][] $ = new boolean[hSplit*vSplit][][];
//		for (int i=0; i<initialField.length; i++) {
//			for (int j=0; j<initialField[0].length; j++) {
//				$[][i % height][j % length] = ;
//			}
//		}
//		return $;
//	}
	
	public boolean[][] get_gen(boolean[][] initialField, int hSplit, int vSplit, int generations) {

		int height = initialField.length;
		int length = initialField[0].length;
		// init the field -> copy the field to the local array
		//TODO: parallelize this
		boolean[][] input=new boolean[initialField.length][];
		for (int i=0;i<initialField.length;i++){
			input[i]=new boolean[initialField[0].length];
			for (int k=0;k<input[i].length;k++){
				input[i][k]=initialField[i][k];
			}
		}

		/*
		for each block we initiate a producer (maybe not needed) ,consumer , the block and we add it to the queues of neighs
		 */
		ConcurrentLinkedQueue<Work> nqa = null; // TODO: this is bullshit, but i'm too tired to fix this
		for(int row=0;row<vSplit-1;row++) {
			for(int col = 0;col < hSplit-1;col++) {
				boolean[][] block = extractBlock(input, row, col, (int)Math.floorDiv(height, vSplit), (int)Math.floorDiv(length, hSplit));
				new LifeConsumer(nqa, block, generations).start(); // TODO: Fix to queue who will hold the block as his first record
			}
		}
		for(int row=vSplit-1;row<vSplit;row++) {
			for(int col = hSplit-1;col < hSplit;col++) {
				boolean[][] block = extractBlock(input, row, col, (int)Math.ceil(height/vSplit), (int)Math.ceil(length/hSplit));
				new LifeConsumer(nqa, block, generations).start();
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
