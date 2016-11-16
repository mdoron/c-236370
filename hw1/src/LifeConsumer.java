import java.util.Queue;

/**
 * Created by raviv on 16/11/2016.
 */
/*
each consumer holds a queue which has
 */
public class LifeConsumer extends Thread {
    Queue<boolean[][]> NeighboursQeueArray[];
    boolean[][] block;
    int generations;
    boolean[][] nextBlock;

    public LifeConsumer(Queue<boolean[][]> nqa[], boolean[][] block, int generations) {
        NeighboursQeueArray = nqa;
        this.block = block;
        this.generations = generations;
    }


    public void run() {

        nextBlock = new boolean[block.length][];

        for (int g = 0; g < generations; g++) {
            for (int i = 0; i < block.length; i++) {
                for (int j = 0; j < block[i].length; j++) {
                    int numNeighbors = numNeighbors(i, j, block);
                    if (numNeighbors == 3 || (block[i][j] && numNeighbors == 2)) {
                        block[i][j] = true;
                    }
                    else {
                        block[i][j] = false;
                    }
                }
            }
            boolean[][] tmp;
            tmp = block;
            block = nextBlock;
            nextBlock = tmp;
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