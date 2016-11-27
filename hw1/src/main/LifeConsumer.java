package main;

import java.util.ArrayList;
import java.util.Objects;

/**
 * Created by raviv on 16/11/2016.
 */
/*
 * LifeConsumer algorithm is placed in the outer documentary
 */
public class LifeConsumer extends Thread {
	ArrayList<OurConcurrentQueue<Work>> nqa;
	OurConcurrentQueue<Work> blockHistory;
	boolean[][] block; // same with producer
	int generations;
	boolean[][] nextBlock;
	// row and col of the block
	int row;
	int col;
	int genNow;

	// Call is synchronized in ParallelGameOfLife
	public LifeConsumer(ArrayList<OurConcurrentQueue<Work>> nqa, OurConcurrentQueue<Work> blockHistory,
			int generations, int row, int col) {
		this.nqa = nqa;
		this.blockHistory = blockHistory;
		this.generations = generations;
		this.block = blockHistory.element().getBlock();
		this.row = row;
		this.col = col;
		this.genNow = 1;
	}

	public boolean[][] getBlock() {
		return block;
	}

	public boolean[][] getPrevBlock() {
		// TODO: check this
		return nextBlock;
	}

	public void run() {

		// initiate the block we work on
		nextBlock = new boolean[block.length][];
		
		// generations loop - same logic as serial, the difference is in numNeighbors
		for (genNow = 1; genNow <= generations; genNow++) {
			for (int i = 0; i < block.length; i++) {
				if (nextBlock[i] == null) {
					nextBlock[i] = new boolean[block[i].length];
				}
				for (int j = 0; j < block[i].length; j++) {
					int numNeighbors = numNeighbors(i, j, block);
					nextBlock[i][j] = false;
					if (numNeighbors == 3 || (block[i][j] && numNeighbors == 2)) {
						nextBlock[i][j] = true;
					}
				}
			}

			boolean[][] tmp;
			tmp = block;
			block = nextBlock;
			nextBlock = tmp;

			// finally, we add the calculated block to our queue, meaning the blockHistory, and notify all
			synchronized (blockHistory) {
				blockHistory.add(new Work(block, genNow));
				blockHistory.notifyAll(); // notify producer maybe we should do
											// notify instead
			}

		}
	}
	public enum DIR {
		UPLEFT, UP, UPRIGHT, LEFT, RIGHT, DOWNLEFT, DOWN, DOWNRIGHT
	}

	/*
	 * @param d - the direction in which the overflow was in
	 * @return - sum of neighbours from the overflowed block which are alive
	 */
	public int checkNeigh(DIR d, int i, int j) throws InterruptedException {
		
		// Part A - its purpose is to take over q and w. 
		if (nqa == null)
			return 0;
		if (nqa.get(d.ordinal()) == null) {
			return 0;
		}
		// we get the reference for q (by java implementation (OOP), doesn't need to be synchronized)
		// q is the right q that we need to take the value from it
		OurConcurrentQueue<Work> q = nqa.get(d.ordinal());
		if (q == null)
			return 0;

		Work w = null;

		// q is shared and other may need it, so we have to synchronize
		synchronized (q) {
			while (q.isEmpty()) {
				q.wait();
			}
			
			// waiting that our neighbors and neighbors only will reach our generation so we can work on the next one
			while (w == null) {
                for(int ii=0;ii<q.size();ii++) {
                    Object w2 = q.get(ii);
					if (genNow == ((Work) w2).getGen() + 1) {
						w = (Work) w2;
						break;
					}
				}
				if (w == null) {
					q.wait();
				}
			}
			q.notifyAll();
		}
		
		
		// Part B - getting the value from the right queue.
		// because of d, we know the orientation of the neighbor block,
		// and because of i, j we know the exact cell we want from the neighbor block
		int $ = 0;
		
		switch (d) {
		case UPLEFT:
			$ = w.getBlock()[w.getBlock().length - 1][w.getBlock()[0].length - 1] ? 1 : 0;
			break;

		case UPRIGHT:
			$ = w.getBlock()[w.getBlock().length - 1][0] ? 1 : 0;
			break;

		case DOWNLEFT:
			$ = w.getBlock()[0][w.getBlock()[0].length - 1] ? 1 : 0;
			break;

		case DOWNRIGHT:
			$ = w.getBlock()[0][0] ? 1 : 0;
			break;

		case UP:
			$ = w.getBlock()[w.getBlock().length - 1][j] ? 1 : 0;
			break;

		case DOWN:
			$ = w.getBlock()[0][j] ? 1 : 0;
			break;

		case LEFT:
			$ = w.getBlock()[i][w.getBlock()[i].length - 1] ? 1 : 0;
			break;

		case RIGHT:
			$ = w.getBlock()[i][0] ? 1 : 0;
			break;

		default:
			$ = 0;
			break;
		}
		return $;

	}

	/**
	 * Calculates the number of alive neighbors of a cell.
	 * @param x position of cell
	 * @param y position of cell
	 * @param field current that we work on
	 * @return number of alive neighbors
	 */
	public int numNeighbors(int x, int y, boolean[][] field) {
		int counter = (field[x][y] ? -1 : 0);
		for (int i = x - 1; i <= x + 1; i++) {
			try {
				for (int j = y - 1; j <= y + 1; j++) {
					// if the neighbor cell is in our block, no need to talk to strangers...
					if (i >= 0 && i < field.length && j >= 0 && j < field[0].length) {
						counter += (field[i][j] ? 1 : 0);
						continue;
					}
					// calculating the orientation of the cell neighbor, which block is it in?
					if (i < 0 && j < 0) {
						counter += checkNeigh(DIR.UPLEFT, i, j);
						continue;
					}
					if (i < 0 && j >= field[0].length) {
						counter += checkNeigh(DIR.UPRIGHT, i, j);
						continue;
					}
					if (i < 0 && j >= 0 && j < field[0].length) {
						counter += checkNeigh(DIR.UP, i, j);
						continue;
					}
					if (i >= field.length && j < 0) {
						counter += checkNeigh(DIR.DOWNLEFT, i, j);
						continue;
					}
					if (i >= field.length && j >= field[0].length) {
						counter += checkNeigh(DIR.DOWNRIGHT, i, j);
						continue;
					}
					if (i >= field.length && j >= 0 && j < field[0].length) {
						counter += checkNeigh(DIR.DOWN, i, j);
						continue;
					}
					if (i >= 0 && i < field.length && j < 0) {
						counter += checkNeigh(DIR.LEFT, i, j);
						continue;
					}
					if (i >= 0 && i < field.length && j >= field[0].length) {
						counter += checkNeigh(DIR.RIGHT, i, j);
						continue;
					}
				}
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
		return counter;
	}
}