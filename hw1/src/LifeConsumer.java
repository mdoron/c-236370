import java.util.Queue;

/**
 * Created by raviv on 16/11/2016.
 */
/*
each consumer holds a queue which has
gets the queue of blocks array and the block, the num of generations
 */
public class LifeConsumer extends Thread {
    Queue<boolean[][]> NeighboursQueueArray[];
    boolean[][] block;    //same with producer
    int generations;
    boolean[][] nextBlock;
    boolean[][] sendToProd;

    public LifeConsumer(Queue<boolean[][]> nqa[], boolean[][] block, int generations) {
        NeighboursQueueArray = nqa;
        this.block = block;
        this.generations = generations;
        this.sendToProd = sendToProd;
    }


    public void run() {

        nextBlock = new boolean[block.length][];

        for (int g = 0; g < generations; g++) {
            for (int i = 0; i < block.length; i++) {
                if (nextBlock[i]==null){
                    nextBlock[i]=new boolean[block[i].length];
                }
                for (int j = 0; j < block[i].length; j++) {
                    int numNeighbors = numNeighbors(i, j, block);
                    nextBlock[i][j] = false;
                    if (numNeighbors == 3 || (block[i][j] && numNeighbors == 2)) {
                        nextBlock[i][j] = true;
                    }
                }
            }
            synchronized (sendToProd) {
                sendToProd = nextBlock;
            }
            boolean[][] tmp;
            tmp = block;
            block = nextBlock;
            notify();       //notify producer
            nextBlock = tmp;
        }
    }


    private int numNeighbors(int x, int y, boolean[][] field) {
        int counter = (field[x][y] ? -1 : 0);
        for (int i = x - 1; i <= x + 1; i++) {
            if ((i < 0) || (i >= field.length)) {
                //TODO: get from producers
                continue;
            }
            for (int j = y - 1; j <= y + 1; j++) {
                if (j < 0 || j >= field[0].length) {
                    //TODO: get from producers
                    continue;
                }
                counter += (field[i][j] ? 1 : 0);
            }
        }
        return counter;
    }
}