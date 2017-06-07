package thread;

import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;

public class CacheCondition {
	
	public static ConcurrentMap<String, ThreadCondition> conditionMap = new ConcurrentHashMap<String, ThreadCondition>();
	
	public static void putThreadCondition(String hash,ThreadCondition threadCondition){
		conditionMap.putIfAbsent(hash, threadCondition);
	}
	
	
	public static ThreadCondition getThreadCondition(String hash){
		return conditionMap.get(hash);
	}
	
	public static void remove(String hash){
		conditionMap.remove(hash);
	}
}
