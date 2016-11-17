import java.util.ArrayList;
import java.util.Queue;
import java.util.concurrent.ConcurrentLinkedQueue;

import javafx.scene.control.cell.ComboBoxListCell;

public class ParallelGameOfLife implements GameOfLife {

	ArrayList<ConcurrentLinkedQueue<Work>> queuesArray;

	/*
	 * this should run the simulation game in parallell mode
	 */
	public boolean[][][] invoke(boolean[][] initalField, int hSplit, int vSplit, int generations) {
		boolean[][][] x = new boolean[2][][];
		// TODO: improve this to be only one
		x[0] = get_gen(initalField, hSplit, vSplit, generations - 1);
		x[1] = get_gen(x[0], hSplit, vSplit, 1);
		return x;
	}

	public boolean[][] get_gen(boolean[][] initialField, int hSplit, int vSplit, int generations) {

		// Retrieve field dimensions 
		int height = initialField.length;
		int length = initialField[0].length;
		
		// Local field initialization
		// TODO: parallelize this @mdoron @ravivos
		boolean[][] input = new boolean[initialField.length][];
		for (int i = 0; i < initialField.length; i++) {
			input[i] = new boolean[initialField[0].length];
			for (int k = 0; k < input[i].length; k++) {
				input[i][k] = initialField[i][k];
			}
		}
		
		// Initialize global queues array - each block has a queue which it writes its new state into it 8 times, and other blocks read from it
		queuesArray = new ArrayList<ConcurrentLinkedQueue<Work>>(hSplit * vSplit); 
		for (int i = 0; i < hSplit * vSplit; i++) {
			queuesArray.set(i, new ConcurrentLinkedQueue<Work>());
		}
		
		// Dividing into sub-tasks. Each thread gets a block and runs it
		for (int row = 0; row < hSplit; row++) {
			for (int col = 0; col < vSplit; col++) {
				// Calculate size of cells in inside block
				int rowCellNumber = (int) Math.floorDiv(height, hSplit);
				int colCellNumber = (int) Math.floorDiv(length, vSplit);
				// If the block is in the last row or column, it might not be in the same size of the of the previous blocks because modulo != 0
				if (row == hSplit) {
					Math.ceil(((double)height)/hSplit);
				}
				if (col == vSplit) {
					Math.ceil(((double)length)/vSplit);
				}
				
				// Initialize array of 8 queues which this block is dependent on their information
				ArrayList<ConcurrentLinkedQueue<Work>> nqa = new ArrayList<ConcurrentLinkedQueue<Work>>(); 
				
				nqa.add(queuesArray.get(calcIndex(vSplit, row - 1, col - 1)));
				nqa.add(queuesArray.get(calcIndex(vSplit, row - 1, col)));
				nqa.add(queuesArray.get(calcIndex(vSplit, row - 1, col + 1)));
				nqa.add(queuesArray.get(calcIndex(vSplit, row, col - 1)));
				nqa.add(queuesArray.get(calcIndex(vSplit, row, col + 1)));
				nqa.add(queuesArray.get(calcIndex(vSplit, row + 1, col - 1)));
				nqa.add(queuesArray.get(calcIndex(vSplit, row + 1, col)));
				nqa.add(queuesArray.get(calcIndex(vSplit, row + 1, col + 1)));

				// Initialize MY queue
				ConcurrentLinkedQueue<Work> blocks = queuesArray.get(calcIndex(vSplit, row, col));
				boolean[][] block = extractBlock(input, row*hSplit, col*vSplit, rowCellNumber, colCellNumber);
				blocks.add(new Work(block, 0));
				
				//Start thread
				new LifeConsumer(nqa, blocks, generations, row, col).start();
			}
		}
		// Combine all information into input. 
		// TODO: @ravivos, in the last generation we still push our current state to our queue
		for (int row = 0; row<hSplit; row++) {
			for(int col = 0; col<vSplit; col++) {
				setBlock(input, queuesArray.get(calcIndex(vSplit, row, col)).poll().getBlock(), row*hSplit, col*vSplit);
			}
		}
		
		return input;
	}

	private int calcIndex(int vSplit, int row, int col) {
		return col % vSplit + row * vSplit;
	}
	
	private boolean[][] extractBlock(boolean[][] initialField, int x, int y, int lx, int ly) {
		boolean[][] $ = new boolean[lx][ly];
		for (int i = 0; i < lx; i++) {
			for (int j = 0; j < ly; j++) {
				$[i][j] = initialField[x + i][y + j];
			}
		}
		return $;
	}

	private void setBlock(boolean[][] board, boolean[][] block, int x, int y) {
		for(int i=0; i<block.length; i++){
			for (int j=0; j<block[0].length; j++){
				board[x+i][y+j] = block[i][j];
			}
		}
	}
	
	private int numNeighbors(int x, int y, boolean[][] field) {
		int counter = (field[x][y] ? -1 : 0);
		for (int i = x - 1; i <= x + 1; i++) {
			if ((i < 0) || (i >= field.length)) {
				continue;
			}
			for (int j = y - 1; j <= y + 1; j++) {
				if (j < 0 || j >= field[0].length) {
					continue;
				}
				counter += (field[i][j] ? 1 : 0);
			}
		}
		return counter;
	}

}
