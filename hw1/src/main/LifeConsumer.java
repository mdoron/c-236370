package main;

import java.util.ArrayList;
import java.util.concurrent.ConcurrentLinkedQueue;

/**
 * Created by raviv on 16/11/2016.
 */
/*
 * each consumer holds a queue which has gets the queue of blocks array and the
 * block, the num of generations
 */
public class LifeConsumer extends Thread {
	ArrayList<ConcurrentLinkedQueue<Work>> nqa;
	ConcurrentLinkedQueue<Work> blockHistory;
	boolean[][] block; // same with producer
	int generations;
	boolean[][] nextBlock;
	// row and col of the block
	int row;
	int col;
	int genNow;

	// Call is synchronized in ParallelGameOfLife
	public LifeConsumer(ArrayList<ConcurrentLinkedQueue<Work>> nqa, ConcurrentLinkedQueue<Work> blockHistory,
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

		nextBlock = new boolean[block.length][];
		// TODO: check when debugging that genNowe has the right values
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

			synchronized (blockHistory) {
				blockHistory.add(new Work(block, genNow));
				blockHistory.notifyAll(); // notify producer maybe we should do
											// notify instead
			}

		}
	}
/**
	public int getValueFromNeighbor(DIR d, int i, int j) throws InterruptedException {
		if (nqa == null)
			return 0;
		if (nqa.get(d.ordinal()) == null) {
			return 0;
		}
		ConcurrentLinkedQueue<Work> q = nqa.get(d.ordinal());
		if (q == null)
			return 0;

		Work w = null;

		synchronized (q) {
			while (q.isEmpty()) {
				q.wait();
			}
			while (w == null) {
				for (Object w2 : q) {
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

		boolean $ = false;

		switch (d) {
		case UPLEFT:
			$ = w.getBlock()[w.getBlock().length - 1][w.getBlock()[0].length - 1];
			break;

		case UPRIGHT:
			$ = w.getBlock()[w.getBlock().length - 1][0];
			break;

		case DOWNLEFT:
			$ = w.getBlock()[0][w.getBlock()[0].length - 1];
			break;
			
		case DOWNRIGHT:
			$ = w.getBlock()[0][0];
			break;

		case UP:
			$ = w.getBlock()[w.getBlock().length - 1][j];
			break;

		case DOWN:
			$ = w.getBlock()[0][j];
			break;

		case LEFT:
			$ = w.getBlock()[i][w.getBlock()[i].length - 1];
			break;

		case RIGHT:
			$ = w.getBlock()[i][0];
			break;

		default:
			$ = false;
			break;
		}
		return $ ? 1 : 0;
	}

	public int getValue(int i, int j) {
		int val = -1;
		try {
			if (i < 0 && j < 0) {
				val = getValueFromNeighbor(DIR.UPLEFT, i, j);
			}
			if (i < 0 && j >= block[0].length) {
				val = getValueFromNeighbor(DIR.UPRIGHT, i, j);
			}
			if (i < 0 && j > 0 && j < block[0].length) {
				val = getValueFromNeighbor(DIR.UP, i, j);
			}
			if (i >= block.length && j < 0) {
				val = getValueFromNeighbor(DIR.DOWNLEFT, i, j);
			}
			if (i >= block.length && j >= block[0].length) {
				val = getValueFromNeighbor(DIR.DOWNRIGHT, i, j);
			}
			if (i >= block.length && j > 0 && j < block[0].length) {
				val = getValueFromNeighbor(DIR.DOWN, i, j);
			}
			if (i > 0 && i < block.length && j < 0) {
				val = getValueFromNeighbor(DIR.LEFT, i, j);
			}
			if (i > 0 && i < block.length && j >= block[0].length) {
				val = getValueFromNeighbor(DIR.RIGHT, i, j);
			}
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		return val;
	}

	public int numNeighbors2(int x, int y, boolean[][] field) {
		int counter = (field[x][y] ? -1 : 0);
		for (int i = x - 1; i <= x + 1; i++) {
			for (int j = y - 1; j <= y + 1; j++) {
				// The position is in our block, no need to talk to neighbors
				if (i >= 0 && i < field.length && j >= 0 && j < field[0].length) {
					counter += (field[i][j] ? 1 : 0);
				} else {
					counter += getValue(i, j);
				}
				continue;
			}
		}
		return counter;
	}
**/
	public enum DIR {
		UPLEFT, UP, UPRIGHT, LEFT, RIGHT, DOWNLEFT, DOWN, DOWNRIGHT
	}

	/*
	 * @param d - the direction in which the overflow was in
	 * 
	 * @return - sum of neighbours from the overflowed block which are alive
	 */
	public int checkNeigh(DIR d, int i, int j) throws InterruptedException {
		if (nqa == null)
			return 0;
		if (nqa.get(d.ordinal()) == null) {
			return 0;
		}
		ConcurrentLinkedQueue<Work> q = nqa.get(d.ordinal());
		if (q == null)
			return 0;

		Work w = null;

		synchronized (q) {
			while (q.isEmpty()) {
				q.wait();
			}
			
			while (w == null) {
				for (Object w2 : q) {
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
		
		int $ = 0;
		// assuming all is fine till here, now we take the neighs from block
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

	public int numNeighbors(int x, int y, boolean[][] field) {
		int counter = (field[x][y] ? -1 : 0);
		for (int i = x - 1; i <= x + 1; i++) {
			try {
				for (int j = y - 1; j <= y + 1; j++) {
					if (i >= 0 && i < field.length && j >= 0 && j < field[0].length) {
						counter += (field[i][j] ? 1 : 0);
						continue;
					}
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