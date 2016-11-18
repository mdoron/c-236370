package test;

import main.ParallelGameOfLife;
import static org.junit.Assert.*;
import org.junit.Test;

public class ParallelGameOfLifeTest {
	ParallelGameOfLife pgf = new ParallelGameOfLife();
	// setBlock tests
	@Test
	public void testVectorBlock () {
		boolean[][] board = {{true, true, false}, {true, false, false}, {false, true, false}};
		boolean[][] block = {{false, false, false}};
		assertNotEquals(board[0][0], block[0][0]);
		assertNotEquals(board[0][1], block[0][1]);
		assertEquals(board[0][2], block[0][2]);
		pgf.setBlock(board, block, 0, 0);
		assertEquals(board[0][0], block[0][0]);
		assertEquals(board[0][1], block[0][1]);
		assertEquals(board[0][2], block[0][2]);
	}
	
	@Test
	public void testMatrixBlock () {
		boolean[][] board = {{true, true, false}, {true, false, false}, {false, true, false}};
		boolean[][] block = {{false, false}, {true, true}};
		pgf.setBlock(board, block, 1, 1);
		assertEquals(board[1][1], block[0][0]);
		assertEquals(board[1][2], block[0][1]);
		assertEquals(board[2][1], block[1][0]);
		assertEquals(board[2][2], block[1][1]);
	}

	// extractBlock tests
	// calcIndex tests
	// setNQA tests
	// get_gen tests
	// invoke tests

}