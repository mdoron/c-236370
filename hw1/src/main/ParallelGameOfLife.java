package main;

import java.util.ArrayList;

/** TODO:  Doron Mehsulam <doronmmm@hotmail.com> please add a description
 * @author  Doron Mehsulam <doronmmm@hotmail.com>
 * @since Nov 18, 2016
 */
public class ParallelGameOfLife implements GameOfLife {

	ArrayList<OurConcurrentQueue<Work>> queuesArray;

	public boolean[][][] invoke(boolean[][] initalField, int hSplit, int vSplit, int generations) {
		boolean[][][] x = new boolean[2][][];
		// TODO: improve this to be only one
		x[0] = get_gen(initalField, hSplit, vSplit, generations - 1);
		x[1] = (new SerialGameOfLife()).get_gen(x[0], hSplit, vSplit, 1);
		return x;
	}

	public boolean[][] get_gen(boolean[][] initialField, int hSplit, int vSplit, int generations) {

		// Retrieve field dimensions
		int height = initialField.length;
		int length = initialField[0].length;

		// Local field initialization
		boolean[][] input = new boolean[initialField.length][];
		for (int i = 0; i < initialField.length; i++) {
			input[i] = new boolean[initialField[0].length];
			for (int k = 0; k < input[i].length; k++) {
				input[i][k] = initialField[i][k];
			}
		}

		// Initialize global queues array - each block has a queue which it
		// writes its new state into it 8 times, and other blocks read from it
		queuesArray = new ArrayList<OurConcurrentQueue<Work>>(hSplit * vSplit);
		for (int i = 0; i < hSplit * vSplit; i++) {
			queuesArray.add(i, new OurConcurrentQueue<Work>());
		}

		// Calculate size of cells in inside block
		int rowCellNumber = (int) Math.floorDiv(height, hSplit);
		int colCellNumber = (int) Math.floorDiv(length, vSplit);
		
		// If the block is in the last row or column, it might not be in
		// the same size of the of the previous blocks because modulo !=
		// 0. So, we have to calculate the blocks of the last row/column separately
		int lastRowCellNumber = height - rowCellNumber * (hSplit - 1);
		int lastColCellNumber = length - colCellNumber * (vSplit - 1);
		
		// Dividing into sub-tasks. Each thread gets a block and runs it
		for (int row = 0; row < hSplit; row++) {
			for (int col = 0; col < vSplit; col++) {

				rowCellNumber = (int) Math.floorDiv(height, hSplit);
				colCellNumber = (int) Math.floorDiv(length, vSplit);

				// Initialize array of 8 queues which this block is dependent on
				// their information
				ArrayList<OurConcurrentQueue<Work>> nqa = new ArrayList<OurConcurrentQueue<Work>>();

				// queuesArray should be synchronized because in the next runs other
				// thread might use it. Mainly we do it just to be sure
				synchronized (queuesArray) {
					setNQA(queuesArray, nqa, row, col, hSplit, vSplit);
				}

				// Initialize MY queue
				OurConcurrentQueue<Work> blocks = queuesArray.get(calcIndex(vSplit, row, col));

				// The position of the row and column in the big array
				int rPos = row * rowCellNumber;
				int cPos = col * colCellNumber;
				if (row == hSplit - 1) {
					rowCellNumber = lastRowCellNumber;
				}
				if (col == vSplit - 1) {
					colCellNumber = lastColCellNumber;
				}
				
				// block holds a chunk of input. 
				// MATLAB syntax: block = input(rPos:rPos+rowCellNumber, cPos:cPos+colCellNumber)
				boolean[][] block = extractBlock(input, rPos, cPos, rowCellNumber, colCellNumber);
				
				// adding block as initial state to MY queue
				blocks.add(new Work(block, 0));

				// Start thread. blocks is a shared resource and must be synchronized
				synchronized (blocks) {
					new LifeConsumer(nqa, blocks, generations, row, col).start();
				}
			}
		}
		// Combine all information into input after everyone have finished.
		// state to our queue
		for (int row = 0; row < hSplit; row++) {
			for (int col = 0; col < vSplit; col++) {
				rowCellNumber = (int) Math.floorDiv(height, hSplit);
				colCellNumber = (int) Math.floorDiv(length, vSplit);

				Work w = null;
				while (w == null) {
                    for(int i=0;i<queuesArray.get(calcIndex(vSplit, row, col)).size();i++) {
					    Object w2 = queuesArray.get(calcIndex(vSplit, row, col)).get(i);
						if (generations <= ((Work) w2).getGen()) {
							w = (Work) w2;
							break;
						}
					}
				}
				// sets block into its right position in input
				setBlock(input, w.getBlock(), row * rowCellNumber, col * colCellNumber);
			}
		}

		return input;
	}

	/**
	 * Set the neighbors queue array to the right values
	 * @param queuesArray each block has a queue. queuesArray holds all of them.
	 * @param nqa this is our nqa, we insert references of neighbor blocks to it
	 * @param row position of this block
	 * @param col position of this block 
	 * @param hSplit number of horizontal splits
	 * @param vSplit number of vertical splits
	 */
	public void setNQA(ArrayList<OurConcurrentQueue<Work>> queuesArray, ArrayList<OurConcurrentQueue<Work>> nqa,
			int row, int col, int hSplit, int vSplit) {

		nqa.add(0, getQueue(queuesArray, hSplit, vSplit, row - 1, col - 1));
		nqa.add(1, getQueue(queuesArray, hSplit, vSplit, row - 1, col));
		nqa.add(2, getQueue(queuesArray, hSplit, vSplit, row - 1, col + 1));
		nqa.add(3, getQueue(queuesArray, hSplit, vSplit, row, col - 1));
		nqa.add(4, getQueue(queuesArray, hSplit, vSplit, row, col + 1));
		nqa.add(5, getQueue(queuesArray, hSplit, vSplit, row + 1, col - 1));
		nqa.add(6, getQueue(queuesArray, hSplit, vSplit, row + 1, col));
		nqa.add(7, getQueue(queuesArray, hSplit, vSplit, row + 1, col + 1));
	}

	/**
	 * @return the right neighbor queue or null
	 */
	public OurConcurrentQueue<Work> getQueue(ArrayList<OurConcurrentQueue<Work>> queuesArray, int hSplit,
			int vSplit, int row, int col) {
		if (row < 0 || col < 0 || row >= hSplit || col >= vSplit) {
			return null;
		}
		int index = calcIndex(vSplit, row, col);
		if (index >= hSplit * vSplit || index < 0) {
			return null;
		}
		return queuesArray.get(index);
	}

	public int calcIndex(int vSplit, int row, int col) {
		return col % vSplit + row * vSplit;
	}

	public boolean[][] extractBlock(boolean[][] initialField, int x, int y, int lx, int ly) {

		boolean[][] $ = new boolean[lx][ly];
		for (int i = 0; i < lx; i++) {
			for (int j = 0; j < ly; j++) {
				$[i][j] = initialField[x + i][y + j];
			}
		}
		return $;
	}

	public void setBlock(boolean[][] board, boolean[][] block, int x, int y) {
		for (int i = 0; i < block.length; i++) {
			for (int j = 0; j < block[0].length; j++) {
				board[x + i][y + j] = block[i][j];
			}
		}
	}

}
