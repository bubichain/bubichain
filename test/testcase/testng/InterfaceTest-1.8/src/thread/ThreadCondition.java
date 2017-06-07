package thread;

import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import cn.bubi.common.util.Tools;

public class ThreadCondition {
	
	private Lock lock = new ReentrantLock();
	private Condition condition = lock.newCondition();
	
	/**
	 * 等待交易结果
	 * @param condition
	 */
	public int waitThread(){
		lock.lock();
		try {
			try {
				condition.awaitNanos(TimeUnit.SECONDS.toNanos(60));
				//超时
				return 1;
				//condition.await();
			} catch (Exception e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		} finally {
			lock.unlock();
		}
		return 0;
	}
	
	public void notifyThread(){
		lock.lock();
		try {
			try {
				condition.signal();
			} catch (Exception e) {
				e.printStackTrace();
			}
		} finally {
			lock.unlock();
		}
	}
	
	
}
