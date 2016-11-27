package main;

import java.util.ArrayList;

/**
 * Created by raviv on 26/11/2016.
 */


public class OurConcurrentQueue<T> {

    private ArrayList<T> queue;

    public OurConcurrentQueue() {
        queue = new ArrayList<T>();}

    public synchronized T poll() {
        return queue.isEmpty() ? null : queue.remove(0);
    }

    public synchronized T element() {
        return queue.isEmpty() ? null : queue.get(0);
    }


    public synchronized T get(int i) {
        return queue.get(i);
    }

    public synchronized void add(T e) {
        queue.add(queue.size() , e);
    }

    public synchronized boolean isEmpty(){return queue.isEmpty();}

    public synchronized int size() {
        return queue.size();
    }




}