public class Queue {
	Work myWork = null;

	public synchronized Work getNext() {
		while (myWork == null) {
			wait();
		}
		Work tmp = myWork;
		myWork = null;
		notifyAll();
		return tmp;
	}

	public synchronized void enqueue(Work w) {
		while (myWork != null) {
			wait();
		}
		myWork = w;
		notifyAll();
	}
}