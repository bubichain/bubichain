package utils;

import java.util.LinkedHashMap;
import java.util.Map;

import base.TestBase;

public class APIUtils extends TestBase {

	public static void wait(int sec){
		try {
			Thread.sleep(sec * 1000);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}
	
	/**
	 * 生成一个账号
	 * 存address\private_key\public_key (public_key)2.0上才有
	 * @return
	 */
	public static Map<String, String>generateAcc(){
		Map<String, String> map = new LinkedHashMap<String, String>();
		String url = "createAccount";
		String account = HttpUtil.doget(get_Url,url);
		int error_code = Result.getoutErrCodeFromGet(account);
		if (error_code != 0) {
			if (error_code == 4) {
				for (int i = 0; i < timeout; i++) {
					APIUtils.wait(1);
					String accountInfo1 = HttpUtil.doget(url);
					int error_code1 = Result.getoutErrCodeFromGet(accountInfo1);
					if (error_code1 == 0) {
						map.put("address", Result.getAddress(account));
						map.put("private_key", Result.getPrivatekey(account));
						map.put("public_key", Result.getPublickey(account));
						return map;
					}
				}
			}
		}
		map.put("address", Result.getAddress(account));
		map.put("private_key", Result.getPrivatekey(account));
		map.put("public_key", Result.getPublickey(account));
		return map;

	}
	
	public static String getDesAddress(String ip,int port){
		String url = "getPeerNodeAddress";
		String key = "token";
		String value = "bubiokqwer";
		String geturl = "http://" + ip +":"+ port +"/";
		String geturl1 = geturl + url +"?" + key + "=" + value ;
		String re = HttpUtil.get(geturl1);
		return re ;
	}
	
	public static Object generateAddress(){
		Object address = generateAcc().get("address");
		return address;
	}
}
