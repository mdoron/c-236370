package main;

import java.lang.reflect.Array;
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
        synchronized (blockHistory) {
            this.block = blockHistory.element().getBlock();
        }
        this.row = row;
        this.col = col;
    }


    public boolean[][] getBlock() {
        return block;
    }

    public boolean[][] getPrevBlock() {
        //TODO: check this
        return nextBlock;
    }

    public void run() {

        nextBlock = new boolean[block.length][];
        genNow = 1;
        //TODO: check when debugging that genNowe has the right values
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
            blockHistory.notifyAll();       //notify producer maybe we should do notify instead
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
            q.wait();
        }
        Work w = (Work)q.element();
        //Work[] warr = (Work[]) q.;
        for(Object w2 : q) {
            if(genNow<=((Work)w2).getGen()) {
                w = (Work)w2;
                break;
            }

        }
        if(genNow > w.getGen()) {
            q.wait();
        }
        int counter = 0;
        //assuming all is fine till here, now we take the neighs from block
        switch(d) {
            case UPLEFT:
                return w.getBlock()[w.getBlock().length-1][w.getBlock()[0].length-1]? 1:0;

            case UPRIGHT:
                return w.getBlock()[w.getBlock().length-1][0]? 1:0;
            case DOWNLEFT:
                return w.getBlock()[0][w.getBlock()[0].length-1]? 1:0;
            case DOWNRIGHT:
                return w.getBlock()[0][0]? 1:0;
            case UP:
                for(int col = 0; col < w.getBlock()[w.getBlock().length-1].length;col++) {
                    if(col >= j-1 && col <= j+1) {
                        counter+= w.getBlock()[w.getBlock().length-1][col]? 1 : 0;
                    }
                }
                return counter;
            case DOWN:
                for(int col = 0; col < w.getBlock()[0].length;col++) {
                    if(col >= j-1 && col <= j+1) {
                        counter+= w.getBlock()[0][col]? 1 : 0;
                    }
                }
                return counter;
            case LEFT:
                for(int row = 0; row < w.getBlock().length;row++) {
                    if(row >= i-1 && row <= i+1) {
                        counter+= w.getBlock()[row][w.getBlock()[row].length]? 1 : 0;
                    }
                }
                return counter;
            case RIGHT:
                for(int row = 0; row < w.getBlock().length;row++) {
                    if(row >= i-1 && row <= i+1) {
                        counter+= w.getBlock()[row][0]? 1 : 0;
                    }
                }
                return counter;

            default:
                return 0;
        }
    }


    public int numNeighbors(int x, int y, boolean[][] field) {
        boolean[] flags = {false,false,false,false,false,false,false};
        int counter = (field[x][y] ? -1 : 0);
        for (int i = x - 1; i <= x + 1; i++) {
            try {
                for (int j = y - 1; j <= y + 1; j++) {
                    if (i >= 0 && i < field.length && j >= 0 && j < field[0].length) { counter += (field[i][j] ? 1 : 0); continue; }
                    if (i < 0 && j < 0 && !flags[DIR.UPLEFT.ordinal()]) {  counter += checkNeigh(DIR.UPLEFT, i, j); flags[DIR.UPLEFT.ordinal()]=true; continue; }
                    if (i < 0 && j >= field[0].length && !flags[DIR.UPRIGHT.ordinal()]) {  counter += checkNeigh(DIR.UPRIGHT, i, j); flags[DIR.UPRIGHT.ordinal()]=true;  continue; }
                    if (i < 0 && j > 0 && j < field[0].length && !flags[DIR.UP.ordinal()])  { counter += checkNeigh(DIR.UP, i, j);  flags[DIR.UP.ordinal()]=true; continue; }
                    if (i >= field.length && j < 0 && !flags[DIR.DOWNLEFT.ordinal()])  { counter += checkNeigh(DIR.DOWNLEFT, i, j);  flags[DIR.DOWNLEFT.ordinal()]=true;  continue; }
                    if (i >= field.length && j >= field[0].length && !flags[DIR.DOWNRIGHT.ordinal()])  { counter += checkNeigh(DIR.DOWNRIGHT, i, j);  flags[DIR.DOWNRIGHT.ordinal()]=true;  continue; }
                    if (i >= field.length && j > 0 && j < field[0].length && !flags[DIR.DOWN.ordinal()])  { counter += checkNeigh(DIR.DOWN, i, j);  flags[DIR.DOWN.ordinal()]=true; continue; }
                    if (i > 0 && i < field.length && j < 0 && !flags[DIR.LEFT.ordinal()])  { counter += checkNeigh(DIR.LEFT, i, j); flags[DIR.LEFT.ordinal()]=true;  continue; }
                    if (i > 0 && i < field.length && j >= field[0].length && !flags[DIR.RIGHT.ordinal()])  { counter += checkNeigh(DIR.RIGHT, i, j); flags[DIR.RIGHT.ordinal()]=true;  continue; }
                }
            }
            catch(Exception e) {
                e.printStackTrace();
            }
        }
        return counter;
    }
}