import java.util.ArrayList;
import java.util.Queue;
import java.util.concurrent.ConcurrentLinkedQueue;

/**
 * Created by raviv on 16/11/2016.
 */
/*
each consumer holds a queue which has
gets the queue of blocks array and the block, the num of generations
 */
public class LifeConsumer extends Thread {
    ArrayList<ConcurrentLinkedQueue<Work>> NeighboursQueueArray;
    ConcurrentLinkedQueue<Work> blockHistory;
    boolean[][] block;    //same with producer
    int generations;
    boolean[][] nextBlock;
    //row and col of the block
    int row;
    int col;
    int genNow;

    public LifeConsumer(ArrayList<ConcurrentLinkedQueue<Work>> nqa, ConcurrentLinkedQueue<Work> blockHistory, int generations,int row,int col) {
        NeighboursQueueArray = nqa;
        this.blockHistory = blockHistory;
        this.generations = generations;
        this.block = blockHistory.element().getBlock();
        this.row = row;
        this.col = col;
    }


    public void run() {

        nextBlock = new boolean[block.length][];
        genNow = 1;
        for (genNow = 1; genNow <= generations; genNow++) {
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
            boolean[][] tmp;
            tmp = block;
            block = nextBlock;
            blockHistory.add(new Work(block,genNow));
            notifyAll();       //notify producer maybe we should do notify instead
            nextBlock = tmp;
        }
    }


    public enum DIR {
        UPLEFT,UP,UPRIGHT,LEFT,RIGHT,DOWNLEFT,DOWN,DOWNRIGHT
    }

    /*
    @param d - the direction in which the overflow was in
    @return - sum of neighbours from the overflowed block which are alive
     */
    public int checkNeigh(DIR d,int i,int j) throws InterruptedException {
        ConcurrentLinkedQueue q = NeighboursQueueArray.get(d.ordinal());
        if(q == null) return 0;
        if(q.isEmpty()) {
            wait();
        }
        Work w = (Work) q.poll();
        if(genNow > w.getGen()) {
            wait();
        }
        //assuming all is fine till here, now we take the neighs from block
        switch(d) {
            case UPLEFT:
                return w.getBlock()[w.getBlock().length][w.getBlock()[0].length]? 1:0;

            case UPRIGHT:
                return w.getBlock()[w.getBlock().length][0]? 1:0;
            case DOWNLEFT:
                return w.getBlock()[0][w.getBlock()[0].length]? 1:0;
            case DOWNRIGHT:
                return w.getBlock()[0][0]? 1:0;
            //TODO: UP RIGHT LEFT DOWN
            default:
                return 0;
        }
    }

    private int numNeighbors(int x, int y, boolean[][] field) {
        int counter = (field[x][y] ? -1 : 0);
        for (int i = x - 1; i <= x + 1; i++) {
            if ((i < 0) || (i >= field.length)) {
                //TODO: get from producers
                continue;
            }
            try {
                for (int j = y - 1; j <= y + 1; j++) {
                    if (i > 0 && i < field.length && j > 0 && j < field[0].length) counter += (field[i][j] ? 1 : 0);
                    if (i < 0 && j < 0) counter += checkNeigh(DIR.UPLEFT, i, j);
                    if (i < 0 && j >= field[0].length) counter += checkNeigh(DIR.UPRIGHT, i, j);
                    if (i < 0 && j > 0 && j < field[0].length) counter += checkNeigh(DIR.UP, i, j);
                    if (i >= field.length && j < 0) counter += checkNeigh(DIR.DOWNLEFT, i, j);
                    if (i >= field.length && j >= field[0].length) counter += checkNeigh(DIR.DOWNRIGHT, i, j);
                    if (i >= field.length && j > 0 && j < field[0].length) counter += checkNeigh(DIR.DOWN, i, j);
                    if (i > 0 && i < field.length && j < 0) counter += checkNeigh(DIR.LEFT, i, j);
                    if (i > 0 && i < field.length && j >= field[0].length) counter += checkNeigh(DIR.RIGHT, i, j);
                }
            }
            catch(Exception e) {
                e.printStackTrace();
            }
        }
        return counter;
    }
}