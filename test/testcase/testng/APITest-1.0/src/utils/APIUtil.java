package utils;

import java.util.Random;

import base.TestBase;

import java.util.HashMap;
import java.util.Map;

import cn.bubi.common.util.Tools;

public class APIUtil extends TestBase {

	//for api test
	public static  int bit(int i)
	{
		return (1<<i);
	}
	public static boolean getbool(long i)
	{
		return i!=0;
	}
	public static String getRandomString(int length){
	     String str="abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	     Random random=new Random();
	     StringBuffer sb=new StringBuffer();
	     for(int i=0;i<length;i++){
	       int number=random.nextInt(62);
	       sb.append(str.charAt(number));
	     }
	     return sb.toString();
	 }
	
	public static String getRandomChineseString(int length){
	     String str="一二三四五六七八九十赵钱孙李周吴郑王田高";
	     Random random=new Random();
	     StringBuffer sb=new StringBuffer();
	     for(int i=0;i<length;i++){
	       int number=random.nextInt(62);
	       sb.append(str.charAt(number));
	     }
	     return sb.toString();
	 }

	public static String sign(Map params,String key){
		@SuppressWarnings("unchecked")
		String signdata = Tools.buildRequestMySign(params, key);
		return signdata;
	}
	
	public static void wait(int sec){
		try {
			Thread.sleep(sec * 500);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}
	
	/*
		Map<String, String> params = new HashMap<String, String>();
		params.put("current_string", "MqUq6TASQju57VcH");
		params.put("trade_no", "trany1w9ujywlhmx99l1484208875606");
		params.put("asset_code", "2UX4xvQ4aXNXm7q79g3FYRSuJPv5UNosNufkTtxF2gW5WwYY6EYYnSyXzwGXMUoL3VgnKzUvPbM9deSBHpajRNqBLqKeferegvGuttdYz53ykA8Y6Lczc1CfarRYM5ygTj57mXeJCWfYH4v7");
		params.put("asset_amount", "1");
		params.put("from_bubi_address", "bubiV8i3PTMqnh1T9sM8TrFmJhCat6JW8aBpPcxx");
		params.put("to_bubi_address", "bubiV8iFVmGLFoR7B4xoRQByT7Xbe4dhD3AHkv4P");
		String signVerify = sign(params, "4cfb21144894ff7f5790af334733fa7d");
		
		String a = params.toString();
		System.out.println(signVerify);
//		System.out.println(params);
//		System.out.println(a);
	}
	
	*/
}
