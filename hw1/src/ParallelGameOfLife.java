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

	private boolean[][] extractBlock(boolean[][] initialField, int x, int y, int lx, int ly) {
		boolean[][] $ = new boolean[lx][ly];
		for (int i = 0; i < lx; i++) {
			for (int j = 0; j < ly; j++) {
				$[i][j] = initialField[x + i][y + j];
			}
		}
		return $;
	}

	// private boolean[][][] splitFieldToBlocks(boolean[][] initialField, int
	// hSplit, int vSplit) {
	// int height = initialField.length/hSplit;
	// int length = initialField[0].length/vSplit;
	// boolean[][][] $ = new boolean[hSplit*vSplit][][];
	// for (int i=0; i<initialField.length; i++) {
	// for (int j=0; j<initialField[0].length; j++) {
	// $[][i % height][j % length] = ;
	// }
	// }
	// return $;
	// }

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
		for (int row = 0; row < vSplit; row++) {
			for (int col = 0; col < hSplit; col++) {
				// Calculate size of cells in inside block
				int rowCellNumber = (int) Math.floorDiv(height, vSplit);
				int colCellNumber = (int) Math.floorDiv(length, hSplit);
				// If the block is in the last row or column, it might not be in the same size of the of the previous blocks because modulo != 0
				if (row == vSplit) {
					Math.ceil(((double)height)/vSplit);
				}
				if (col == hSplit && colCellNumber != ((double)length)/hSplit) {
					Math.ceil(((double)length)/hSplit);
				}
				
				// Initialize array of 8 queues which this block is dependent on their information
				ArrayList<ConcurrentLinkedQueue<Work>> nqa = new ArrayList<ConcurrentLinkedQueue<Work>>(); 
				
				nqa.add(queuesArray.get(calcIndex(hSplit, row - 1, col - 1)));
				nqa.add(queuesArray.get(calcIndex(hSplit, row - 1, col)));
				nqa.add(queuesArray.get(calcIndex(hSplit, row - 1, col + 1)));
				nqa.add(queuesArray.get(calcIndex(hSplit, row, col - 1)));
				nqa.add(queuesArray.get(calcIndex(hSplit, row, col + 1)));
				nqa.add(queuesArray.get(calcIndex(hSplit, row + 1, col - 1)));
				nqa.add(queuesArray.get(calcIndex(hSplit, row + 1, col)));
				nqa.add(queuesArray.get(calcIndex(hSplit, row + 1, col + 1)));

				// Initialize MY queue
				ConcurrentLinkedQueue<Work> blocks = queuesArray.get(calcIndex(hSplit, row, col));
				boolean[][] block = extractBlock(input, row, col, rowCellNumber, colCellNumber);
				blocks.add(new Work(block, 0));
				
				//Start thread
				new LifeConsumer(nqa, blocks, generations, row, col).start();
			}
		}
		// then we need to combine the blocks to a matrix and return it
		return input;
	}

	private int calcIndex(int hSplit, int row, int col) {
		return col % hSplit + row * hSplit;
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
