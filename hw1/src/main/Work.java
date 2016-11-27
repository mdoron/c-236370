package main;

/**
 * 
 * @author doron
 * Wrapping class for boolean[][] & int. Holds a block and 
 * the generation the block was calculated for.
 */
public class Work {
	boolean [][] block;
	int gen;
	
	public Work(boolean[][] block, int gen) {
		this.block = block;
		this.gen = gen;
	}
	
	public boolean[][] getBlock() {
		return this.block;
	}
	
	public int getGen() {
		return this.gen;
	}
}
