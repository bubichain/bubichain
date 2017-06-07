package utils;

import redis.clients.jedis.Jedis;

public class RedisUtil {

	private static Jedis jedis ;
	public static void main(String[] args) {
		 jedis = new Jedis("192.168.10.201",6379);
		 System.out.println("Server is running: "+jedis.ping());
	}
}
