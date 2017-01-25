//package test;
//
//import main.Ex1;
//import main.LifeConsumer;
//import main.Work;
//import org.junit.Assert;
//import org.junit.Test;
//
//import java.util.ArrayList;
//import java.util.NoSuchElementException;
//import java.util.concurrent.ConcurrentLinkedQueue;
//
///** // * Created by raviv on 21/11/2016.
 * @author  Raviv Rachmiel <raviv.rachmiel@gmail.com>
 * @since Nov 22, 2016
 */
//public class LifeConsumerTest {
//
//    @Test
//    public void basicConstructionCorrectionTest() {
//        ArrayList<ConcurrentLinkedQueue<Work>> nqa = new ArrayList<>();
//        ConcurrentLinkedQueue<Work> blocks = new ConcurrentLinkedQueue<>();
//        boolean[][] block =  {{true,false},{false},{true}};
//        blocks.add(new Work(block ,0));
//        LifeConsumer lfc = new LifeConsumer(nqa, blocks, 2, 0, 0);
//    }
//
//    @Test
//    public void complicatedConstructionCorrectionTest() {
//        ArrayList<ConcurrentLinkedQueue<Work>> nqa = new ArrayList<>();
//        boolean[][] block =  {{true,false},{false},{true}};
//        ConcurrentLinkedQueue<Work> blocks = new ConcurrentLinkedQueue<>();
//        blocks.add(new Work(block ,0));
//        nqa.add(blocks);
//        LifeConsumer lfc = new LifeConsumer(nqa, blocks, 2, 0, 0);
//    }
//
//
//    @Test
//    public void checkEmptyNeighTest() throws InterruptedException {
//        ArrayList<ConcurrentLinkedQueue<Work>> nqa = new ArrayList<>();
//        ConcurrentLinkedQueue<Work> blocks = new ConcurrentLinkedQueue<>();
//        boolean[][] block =  {{true,false},{false},{true}};
//        blocks.add(new Work(block ,0));
//        //inits the nqa size to 8
//        nqa.add(null);
//        nqa.add(null);
//        nqa.add(null);
//        nqa.add(null);
//        nqa.add(null);
//        nqa.add(null);
//        nqa.add(null);
//        nqa.add(null);
//        LifeConsumer lfc = new LifeConsumer(nqa, blocks, 2, 0, 0);
//        Assert.assertNotNull(lfc.checkNeigh(LifeConsumer.DIR.UP,0,0));
//        Assert.assertEquals(lfc.checkNeigh(LifeConsumer.DIR.UP,0,0),0);
//    }
//
//    @Test(expected=IndexOutOfBoundsException.class)
//    public void checkFalseParamsNeighTest() throws InterruptedException {
//        ArrayList<ConcurrentLinkedQueue<Work>> nqa = new ArrayList<>();
//        ConcurrentLinkedQueue<Work> blocks = new ConcurrentLinkedQueue<>();
//        boolean[][] block =  {{true,false},{false},{true}};
//        blocks.add(new Work(block ,0));
//        //inits the nqa size to 1 instead of 9
//        nqa.add(null);
//        LifeConsumer lfc = new LifeConsumer(nqa, blocks, 2, 0, 0);
//        lfc.checkNeigh(LifeConsumer.DIR.UP,0,0);
//        //shouldn't get here though
//        Assert.assertNotNull(lfc.checkNeigh(LifeConsumer.DIR.UP,0,0));
//    }
//    @Test(expected=NoSuchElementException.class)
//    public void checkFalseParamsNeighTest2() throws InterruptedException {
//        ArrayList<ConcurrentLinkedQueue<Work>> nqa = new ArrayList<>();
//        ConcurrentLinkedQueue<Work> blocks = new ConcurrentLinkedQueue<>();
//        LifeConsumer lfc = new LifeConsumer(nqa, blocks, 2, 0, 0);
//        lfc.checkNeigh(LifeConsumer.DIR.UP,0,0);
//        //shouldn't get here though
//        Assert.assertNotNull(lfc.checkNeigh(LifeConsumer.DIR.UP,0,0));
//    }
//
//    @Test
//    public void checkNeighUpTest() throws InterruptedException {
//        ArrayList<ConcurrentLinkedQueue<Work>> nqa = new ArrayList<>();
//        ConcurrentLinkedQueue<Work> blocks = new ConcurrentLinkedQueue<>();
//        boolean[][] block =  {{true,false},{false,true}};
//        blocks.add(new Work(block ,0));
//        boolean[][] UPBlock =  {{true,false},{false,true}};
//        //inits the nqa size to 9
//        nqa.add(null);
//        ConcurrentLinkedQueue<Work> up =new ConcurrentLinkedQueue<>();
//        up.add(new Work(UPBlock,0));
//        nqa.add(up);
//        nqa.add(null);
//        nqa.add(null);
//        nqa.add(null);
//        nqa.add(null);
//        nqa.add(null);
//        nqa.add(null);
//        nqa.add(null);
//        LifeConsumer lfc = new LifeConsumer(nqa, blocks, 2, 0, 0);
//        Assert.assertEquals(lfc.checkNeigh(LifeConsumer.DIR.UP,0,0),1);
//    }
//
//    @Test
//    public void check2NeighUpTest() throws InterruptedException {
//        ArrayList<ConcurrentLinkedQueue<Work>> nqa = new ArrayList<>();
//        ConcurrentLinkedQueue<Work> blocks = new ConcurrentLinkedQueue<>();
//        boolean[][] block =  {{true,false},{false,true}};
//        blocks.add(new Work(block ,0));
//        boolean[][] UPBlock =  {{true,false},{true,true}};
//        //inits the nqa size to 9
//        nqa.add(null);
//        ConcurrentLinkedQueue<Work> up =new ConcurrentLinkedQueue<>();
//        up.add(new Work(UPBlock,0));
//        nqa.add(up);
//        nqa.add(null);
//        nqa.add(null);
//        nqa.add(null);
//        nqa.add(null);
//        nqa.add(null);
//        nqa.add(null);
//        nqa.add(null);
//        LifeConsumer lfc = new LifeConsumer(nqa, blocks, 2, 0, 0);
//        Assert.assertEquals(lfc.checkNeigh(LifeConsumer.DIR.UP,0,0),2);
//    }
//
//    @Test
//    public void checkNeighDownRightTest() throws InterruptedException {
//        ArrayList<ConcurrentLinkedQueue<Work>> nqa = new ArrayList<>();
//        ConcurrentLinkedQueue<Work> blocks = new ConcurrentLinkedQueue<>();
//        boolean[][] block =  {{true,false},{false,true}};
//        blocks.add(new Work(block ,0));
//        boolean[][] DRBlock =  {{true,true},{true,true}};
//        //inits the nqa size to 8
//        nqa.add(null);
//        nqa.add(null);
//        nqa.add(null);
//        nqa.add(null);
//        nqa.add(null);
//        nqa.add(null);
//        nqa.add(null);
//        ConcurrentLinkedQueue<Work> downRight =new ConcurrentLinkedQueue<>();
//        downRight.add(new Work(DRBlock,0));
//        nqa.add(downRight);
//        LifeConsumer lfc = new LifeConsumer(nqa, blocks, 2, 0, 0);
//        Assert.assertEquals(lfc.checkNeigh(LifeConsumer.DIR.DOWNRIGHT,0,0),1);
//    }
//
//    @Test
//    public void numNeighNotEdgeTest() {
//
//    }
//
//    @Test
//    public void numNeighEdgeTestBasic() throws InterruptedException {
//        ArrayList<ConcurrentLinkedQueue<Work>> nqa = new ArrayList<>();
//        ConcurrentLinkedQueue<Work> blocks = new ConcurrentLinkedQueue<>();
//        boolean[][] block =  {{true,false,true},{true,true,true},{true,true,true}};
//        blocks.add(new Work(block ,0));
//        boolean[][] UPBlock =  {{true,false,true},{true,true,true},{true,true,true}};
//        //inits the nqa size to 9
//        nqa.add(null);
//        ConcurrentLinkedQueue<Work> up =new ConcurrentLinkedQueue<>();
//        up.add(new Work(UPBlock,0));
//        nqa.add(up);
//        nqa.add(null);
//        nqa.add(null);
//        nqa.add(null);
//        nqa.add(null);
//        nqa.add(null);
//        nqa.add(null);
//        nqa.add(null);
//        LifeConsumer lfc = new LifeConsumer(nqa, blocks, 2, 0, 0);
//        Assert.assertEquals(lfc.numNeighbors(1,1,block),7);
//    }
//
//    @Test
//    public void numNeighEdgeOnlyOneNeighTest() {
//        ArrayList<ConcurrentLinkedQueue<Work>> nqa = new ArrayList<>();
//        ConcurrentLinkedQueue<Work> blocks = new ConcurrentLinkedQueue<>();
//        boolean[][] block =  {{true,true,true},{true,true,true},{true,true,true}};
//        blocks.add(new Work(block ,0));
//        boolean[][] UPBlock =  {{true,false,true},{true,true,true},{true,true,true}};
//        //inits the nqa size to 9
//        nqa.add(null);
//        ConcurrentLinkedQueue<Work> up =new ConcurrentLinkedQueue<>();
//        up.add(new Work(UPBlock,0));
//        nqa.add(up);
//        nqa.add(null);
//        nqa.add(null);
//        nqa.add(null);
//        nqa.add(null);
//        nqa.add(null);
//        nqa.add(null);
//        nqa.add(null);
//        LifeConsumer lfc = new LifeConsumer(nqa, blocks, 2, 0, 0);
//        Assert.assertEquals(lfc.numNeighbors(0,1,block),8);
//
//    }
//
//    @Test
//    public void numNeighEdgeAllNeighTest() {
//        ArrayList<ConcurrentLinkedQueue<Work>> nqa = new ArrayList<>();
//        ConcurrentLinkedQueue<Work> blocks = new ConcurrentLinkedQueue<>();
//        boolean[][] block =  {{true,true,true},{true,true,true},{true,true,true}};
//        blocks.add(new Work(block ,0));
//        boolean[][] UPBlock =  {{true,false,true},{true,true,true},{true,true,true}};
//        //inits the nqa size to 9
//        ConcurrentLinkedQueue<Work> upR =new ConcurrentLinkedQueue<>();
//        upR.add(new Work(UPBlock,0));
//        nqa.add(upR);
//        ConcurrentLinkedQueue<Work> up =new ConcurrentLinkedQueue<>();
//        up.add(new Work(UPBlock,0));
//        nqa.add(up);
//        nqa.add(null);
//        nqa.add(null);
//        nqa.add(null);
//        nqa.add(null);
//        nqa.add(null);
//        nqa.add(null);
//        nqa.add(null);
//        LifeConsumer lfc = new LifeConsumer(nqa, blocks, 2, 0, 0);
//        Assert.assertEquals(lfc.numNeighbors(0,0,block),7);
//
//    }
//
//    @Test
//    public void basicRunTest() {
//        //TODO: Debug this test
//        ArrayList<ConcurrentLinkedQueue<Work>> nqa = new ArrayList<>();
//        ConcurrentLinkedQueue<Work> blocks = new ConcurrentLinkedQueue<>();
//        boolean[][] block =  {{true,true,true},{false,false,true},{true,false,false}};
//        Ex1.printArray(block);
//        blocks.add(new Work(block ,0));
//        boolean[][] UPBlock =  {{true,false,true},{true,true,true},{true,true,true}};
//        //inits the nqa size to 9
//        ConcurrentLinkedQueue<Work> upR =new ConcurrentLinkedQueue<>();
//        upR.add(new Work(UPBlock,0));
//        upR.add(new Work(UPBlock,1));
//        upR.add(new Work(UPBlock,2));
//        upR.add(new Work(UPBlock,3));
//        nqa.add(upR);
//        ConcurrentLinkedQueue<Work> up =new ConcurrentLinkedQueue<>();
//        up.add(new Work(UPBlock,0));
//        up.add(new Work(UPBlock,1));
//        up.add(new Work(UPBlock,2));
//        up.add(new Work(UPBlock,3));
//        nqa.add(up);
//        nqa.add(null);
//        nqa.add(null);
//        nqa.add(null);
//        nqa.add(null);
//        nqa.add(null);
//        nqa.add(null);
//        nqa.add(null);
//        LifeConsumer lfc = new LifeConsumer(nqa, blocks, 2, 0, 0);
//        lfc.start();
//        Ex1.printArray(lfc.getBlock());
//    }
//
//    @Test
//    public void complicatedRunTest() {
//        //mock neighbours
//    }
//
//    @Test
//    public void runParallelBasicTest() {
//        //mock neighbours
//    }
//
//    @Test
//    public void runParallelComplicatedTest() {
//        //mock neighbours
//    }
//
//}
