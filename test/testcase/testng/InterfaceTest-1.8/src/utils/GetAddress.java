package utils;

public class GetAddress {

	public static String getDesAddress(String ip,int port){
		String url = "getPeerNodeAddress";
		String key = "token";
		String value = "bubiokqwer";
		String geturl = "http://" + ip +":"+ port +"/";
		String geturl1 = geturl + url +"?" + key + "=" + value ;
		System.out.println(geturl1);
		String re = HttpUtil.get(geturl1);
		return re ;
	}
}
