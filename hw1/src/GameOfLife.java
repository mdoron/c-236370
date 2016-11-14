public interface GameOfLife {
	/**
	 * This is the only function to be implemented
	 * 
	 * @param initalField : this is the field with the initial data
	 * @param hSplit : number of threads over X dimension
	 * @param vSplit  : number of threads over Y dimension
	 * @param generations : number of generations 
	 * @return
	 */ 
	boolean[][][] invoke(boolean[][] initalField, int hSplit, int vSplit, int generations);


}
