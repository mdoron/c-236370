package main;

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
