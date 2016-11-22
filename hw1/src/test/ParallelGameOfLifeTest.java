package test;

import main.ParallelGameOfLife;
import main.Work;

import static org.junit.Assert.*;

import java.util.ArrayList;
import java.util.concurrent.ConcurrentLinkedQueue;

import org.junit.Test;

/**
 * This class simple tests main.ParallelGameOfLife. No extreme case are being
 * checked, only legitimate values. We are checking the logics now because we
 * don't have time to take care of all irrelevant cases.
 * 
 * IMPORTANT: THIS IS NOT TDD !
 * 
 * @author doron
 *
 */
public class ParallelGameOfLifeTest {
	ParallelGameOfLife pgf = new ParallelGameOfLife();

	// setBlock tests
	@Test
	public void testVectorBlock() {
		boolean[][] board = { { true, true, false }, { true, false, false }, { false, true, false } };
		boolean[][] block = { { false, false, false } };
		assertNotEquals(board[0][0], block[0][0]);
		assertNotEquals(board[0][1], block[0][1]);
		assertEquals(board[0][2], block[0][2]);
		pgf.setBlock(board, block, 0, 0);
		assertEquals(board[0][0], block[0][0]);
		assertEquals(board[0][1], block[0][1]);
		assertEquals(board[0][2], block[0][2]);
	}

	@Test
	public void testMatrixBlock() {
		boolean[][] board = { { true, true, false }, { true, false, false }, { false, true, false } };
		boolean[][] block = { { false, false }, { true, true } };
		pgf.setBlock(board, block, 1, 1);
		assertEquals(board[1][1], block[0][0]);
		assertEquals(board[1][2], block[0][1]);
		assertEquals(board[2][1], block[1][0]);
		assertEquals(board[2][2], block[1][1]);
	}

	// extractBlock tests
	@Test
	public void testDifferentSizeBlockExtracting() {
		boolean[][] board = { { true, true, false }, { true, false, false }, { false, true, false } };
		boolean[][] b1 = pgf.extractBlock(board, 1, 0, 2, 2);
		boolean[][] b2 = pgf.extractBlock(board, 1, 0, 2, 3);
		assertEquals(board[1][0], b1[0][0]);
		assertEquals(board[1][1], b1[0][1]);
		assertEquals(board[2][0], b1[1][0]);
		assertEquals(board[2][1], b1[1][1]);

		assertEquals(board[1][0], b2[0][0]);
		assertEquals(board[1][1], b2[0][1]);
		assertEquals(board[1][2], b2[0][2]);
		assertEquals(board[2][0], b2[1][0]);
		assertEquals(board[2][1], b2[1][1]);
		assertEquals(board[2][2], b2[1][2]);

	}

	// calcIndex tests
	@Test
	public void testIndicesCorrect() {
		int vSplit = 10;
		int hSplit = 20;
		ArrayList<Integer> indices = new ArrayList<Integer>(hSplit * vSplit);
		for (int row = 0; row < hSplit; row++) {
			for (int col = 0; col < vSplit; col++) {
				int i = pgf.calcIndex(vSplit, row, col);
				indices.add(i);
			}
		}

		for (int i = 0; i < hSplit * vSplit; i++) {
			assertEquals(new Integer(i), indices.get(i));
		}

	}

	// setNQA tests
	@Test
	public void testSetInsiderCellNQA() {
		int row = 5, col = 10, hSplit = 10, vSplit = 20;
		
		ArrayList<ConcurrentLinkedQueue<Work>> queuesArray = new ArrayList<ConcurrentLinkedQueue<Work>>(hSplit*vSplit);
		for (int i=0; i<hSplit*vSplit;i++) {
			queuesArray.add(i, new ConcurrentLinkedQueue<Work>());
		}
		ArrayList<ConcurrentLinkedQueue<Work>> nqa = new ArrayList<ConcurrentLinkedQueue<Work>>();
		pgf.setNQA(queuesArray,  nqa, row, col, hSplit, vSplit);
		for(int i=0; i<nqa.size();i++) {
			assertNotNull(nqa.get(i));
		}
	}
	
	@Test
	public void testSetUpperLeftCellNQA() {
		int row = 0, col = 0, hSplit = 10, vSplit = 20;
		
		ArrayList<ConcurrentLinkedQueue<Work>> queuesArray = new ArrayList<ConcurrentLinkedQueue<Work>>(hSplit*vSplit);
		for (int i=0; i<hSplit*vSplit;i++) {
			queuesArray.add(i, new ConcurrentLinkedQueue<Work>());
		}
		ArrayList<ConcurrentLinkedQueue<Work>> nqa = new ArrayList<ConcurrentLinkedQueue<Work>>();
		pgf.setNQA(queuesArray,  nqa, row, col, hSplit, vSplit);
		for(int i=0; i<4;i++) {
			assertNull(nqa.get(i));
		}
		assertNotNull(nqa.get(4));
		assertNull(nqa.get(5));
		for(int i=6; i<7;i++) {
			assertNotNull(nqa.get(i));
		}
	}
	
	@Test
	public void testByRefQueue(){
		int row = 0, col = 0, hSplit = 10, vSplit = 20;
		
		ArrayList<ConcurrentLinkedQueue<Work>> queuesArray = new ArrayList<ConcurrentLinkedQueue<Work>>(hSplit*vSplit);
		for (int i=0; i<hSplit*vSplit;i++) {
			queuesArray.add(i, new ConcurrentLinkedQueue<Work>());
		}
		ArrayList<ConcurrentLinkedQueue<Work>> nqa = new ArrayList<ConcurrentLinkedQueue<Work>>();
		pgf.setNQA(queuesArray,  nqa, row, col, hSplit, vSplit);
		
		boolean [][] block = {{true}};
		nqa.get(4).add(new Work(block, 0));
		assertNotNull(pgf.calcIndex(vSplit, row+1, col));
	}
	
	// get_gen tests
	// invoke tests
}