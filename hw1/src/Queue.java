public class Queue {
	Work myWork = null;

	public synchronized Work getNext() throws InterruptedException {
		while (myWork == null) {
			wait();
		}
		Work tmp = myWork;
		myWork = null;
		notifyAll();
		return tmp;
	}

	public synchronized void enqueue(Work w) throws InterruptedException {
		while (myWork != null) {
			wait();
		}
		myWork = w;
		notifyAll();
	}
}