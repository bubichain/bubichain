package cases;


import java.security.KeyManagementException;
import java.security.NoSuchAlgorithmException;
import java.util.Calendar;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Random;
import org.testng.annotations.Test;

import base.TestBase;
import net.sf.json.JSONArray;
import net.sf.json.JSONObject;
import utils.APIUtil;
import utils.HttpUtil;
@Test
public class BumengTestSetUp extends ApiGeneralCases {
	
	//set up env
	String server_url = "http://192.168.10.84:8080/api";
	String zhongtuobang_id = "33f2209047092831a3413f090e1f3630";
	String zhongtuobang_key = "d12ed7f0a30bebb157bdc81770fd2201";
	//String yangguang_id = "84b79b341adde491a6649f345fe0797b";
	//String yangguang_key = "059c59b81ac12d312a7227697dcb0a51";
	//beta env
	//String server_url = "https://beta_api.bumeng.cn/api";
	//String qianxiang_id = "62fb7101ca1e869e36f4a5144a2aa332";
	//String qianxiang_key = "e81e9c85a3ca26b40f0344b77a0453e2";
	
	//String server_url = "https://api.bubi.cn";
	//String qianxiang_id = "befa6fbf5548c9309abd3d32a93b3deb";
	//String qianxiang_key = "4603d752d86b7f7368634b079a9fe654";
	
	String app_id = "55b3e62a27218eff08f87cebe885cc9e";
	String app_key = "18ceb8053cdba240565f59a4f64cf965";
		
	static Random rand = new Random();
	static int tradeNoBase = rand.nextInt(255);
	static String trade_no = new String();
	static int index = 0;
	
	public static StringBuffer user_name1 = new StringBuffer("");  
	public static StringBuffer address1 = new StringBuffer("");  
	public static StringBuffer assetcode = new StringBuffer("");
	
	public static StringBuffer user_name2 = new StringBuffer("");  
	public static StringBuffer address2 = new StringBuffer("");  	
	
	public static StringBuffer user_name3 = new StringBuffer("");  
	public static StringBuffer address3 = new StringBuffer("");  	
	
	static String asset_unit = "最多六个字";
	static String asset_name = "某种资产";
	
	
	private void MultipleSend(String token,Map<String, Object> thrMap,JSONArray detail,int count)
	{
			
		String api_path = "/asset/v1/send" ;
		parameter.clear();
		parameter.put("access_token",token);
		for (int i = 0;i<count; i++)
		{
		Map<String, Object> thrMapforsign = new LinkedHashMap<String, Object>();
		
		
		thrMapforsign.put("current_string", thrMap.get("current_string"));			
		thrMapforsign.put("asset_code",thrMap.get("asset_code") );	
		thrMapforsign.put("from_bubi_address", thrMap.get("from_bubi_address"));	
		thrMapforsign.put("to_bubi_address", thrMap.get("to_bubi_address"));	
			
			
			
		if(detail!=null)
		{
			thrMapforsign.put("details", detail);		
			
		}
		
		
			trade_no = Long.toString((tradeNoBase++)+(System.currentTimeMillis()<<8));
			thrMapforsign.put("trade_no", trade_no);	
			thrMapforsign.put("asset_amount", Integer.toString(count+i));	
			String sign1 = APIUtil.sign(thrMapforsign, zhongtuobang_key);
			thrMapforsign.put("sign", sign1);					
			thrMapforsign.put("password", thrMap.get("password"));	
			JSONObject jsonBody = JSONObject.fromObject(thrMapforsign);
			System.out.println("=========Send asset========");				
			String result = HttpUtil.dopostApi(server_url, api_path, parameter, jsonBody);
			
		}
		
	
		
	}	
	

	public void SingleAssetSendManytimes(String assetcode)
	{
		String token  = getToken(server_url,zhongtuobang_id,zhongtuobang_key);
		Map<String, Object> thrMap = new LinkedHashMap<String, Object>();
		thrMap.put("current_string","MqUq6TASQju57VcH");
	    thrMap.put("asset_code",assetcode);
	    thrMap.put("from_bubi_address","bubiV8iEzmbVLwAk46eNWErR3X2MfNauVFqeWDEw");
	    thrMap.put("to_bubi_address","bubiV8hzekwc7SERS7y3JHxtEPzoNsAp8PeuH57g");
	    thrMap.put("password","MuhMUOYbjz");	    
	    MultipleSend(token,thrMap,null,2);
		
	}
		
	/*
	 "current_string":"MqUq6TASQju57VcH",
    "trade_no":"38057506609377cm11111k",
    "asset_code":"2UX4xvQ4aXQdfKMP2bGnfKDAakjcRrvizTxKJrZqsdKmvWsw8CLUyA7J65bgLtUsCCpgdZiDtmBFZ5UboaQYQGYYnSWuUhpDRnanV5v14m8YGmgVHRLNNQAbNtifg83KFm7ihnDRFtejuF8Z",
    "asset_amount":"123",
    "from_bubi_address":"bubiV8iEzmbVLwAk46eNWErR3X2MfNauVFqeWDEw",
    "to_bubi_address":"bubiV8hzekwc7SERS7y3JHxtEPzoNsAp8PeuH57g",
    "sign":"bce07af8b8678b46a7170b69655b926e",
    "password":"MuhMUOYbjz",
    "metadata":"444"
	 * */
	
	public void MultiAssetSendManytimes()
	{
		String assetlist[]={
					"2UX4xvQ4aXQdfKMP2bGnfKDAakjcRrvizTxKJrZqsdKmvWsw8CLUyA7J65bgLtUsCCpgdZiDtmBFZ5UboaQYQGYYnSWuUhpDRnanV5v14m8YGmgVHRLNNQAbNtifg83KFm7ihnDRFtejuF8Z",
					"2UX4xvQ4aXQdfKMP2bGnfKDAakjcRrvizTxKJrZqsdKmvWsw8CLUyA7J64yYef8y2krW3QGA2MraLhwewwkFqvGw6o3RQXMNo5DQ1ai6CHC2i7m3TPa7DZvUZiRoJNrKNMZXn4bYBYbHFBFL",
					"2UX4xvQ4aXQdfKMP2bGnfKDAakjcRrvizTxKJrZqsdKmvWsw8CLUyA7J58gYu2QCZrfnEXzVCM2joiGtdTYCSWpyJi7y6T9T8aqFCbaxwfMji6HT1EX5Q6NvLQ6zd9jM1PGG1xJVcVWWBMao",
					"2UX4xvQ4aXQdfKMP2bGnfKDAakjcRrvizTxKJrZqsdKmvWsw8CLUyA7J5Cb6kkDhmWBgdvBLY4t1NRKw81VnVmxp3D9tcYbKSFvAFMB8Mv6ALruUtabmtmjPRBRf2rTzhLdsJE3mkpeZ1PyC"
		};
		 for(String tmp:assetlist){
			 SingleAssetSendManytimes(tmp);
	    }
	}
	
	//issue multiple asset, and for a single asset send it to a different user

	public void MultiAssetToMultiUser()
	{
		int count = 20;
		
		String token = getToken(server_url,zhongtuobang_id,zhongtuobang_key);
		Calendar now = Calendar.getInstance();  
		String header  = Integer.toString(now.get(Calendar.YEAR))+Integer.toString(now.get(Calendar.MONTH) + 1)+Integer.toString(now.get(Calendar.DAY_OF_MONTH))+"多资产发给多用户";
		issueAndSendMultiAsset(server_url,token, "v3nC4uELR0","bubiV8hwAKE6uyiswZWcLfnynbpcRbc3gvyU79pp",header,"f",count);
		
	}

	public void batchOpt() throws KeyManagementException, NoSuchAlgorithmException
	{
		int batchnum = 1;
		
		String token  = getToken(server_url,zhongtuobang_id,zhongtuobang_key);
		//String token  = getToken(server_url,qianxiang_id,qianxiang_key);
		//String token  = getToken(server_url,app_id,app_key);
		Calendar now = Calendar.getInstance();  
		String header  = Integer.toString(now.get(Calendar.YEAR))+Integer.toString(now.get(Calendar.MONTH) + 1)+Integer.toString(now.get(Calendar.DAY_OF_MONTH))+"批量";
		batchSendAsset(server_url, token, "bubiV8hwL34mY1TH1WsL4LHDUtqyjn4mLGPRgp6q","0WlbvwGtE9",header,app_key,batchnum);
			
	}
	
	
}