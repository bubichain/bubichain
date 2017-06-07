package cases;


import org.testng.annotations.Test;

import net.sf.json.JSONArray;
import net.sf.json.JSONObject;

@Test
//this a test for complex condition
public class APIIntegrationTest extends ApiGeneralCases {
	
	
	public static StringBuffer user_name1 = new StringBuffer("");  
	public static StringBuffer address1 = new StringBuffer("");  
	public static StringBuffer assetcode = new StringBuffer("");
	
	public static StringBuffer user_name2 = new StringBuffer("");  
	public static StringBuffer address2 = new StringBuffer("");  	
	
	public static StringBuffer user_name3 = new StringBuffer("");  
	public static StringBuffer address3 = new StringBuffer("");  
	
	public static StringBuffer user_name4 = new StringBuffer("");  
	public static StringBuffer address4 = new StringBuffer("");  
		
	static String asset_unit = "元";
	static String asset_name = "某种资产";
	
	/*//to test when sum is less than amount
	public void a_SumLessThanAmount()
	{
		
	}
	//to test when sum is more than amount
	public void b_SumMoreThanAmount()
	{
		
	}
	//to test when funds is not sufficient
	public void c_FundsNotEnough()
	{
		
	}*/
	

	//to test when try to grant asset whose length is not -1
	public void d_GrantAnInitializedAsset()
	{
		access_token = getToken();
		
		//user 1 issue a kind of asset		
		createUser(access_token, user_name1,address1);
		
		//detail layout
		JSONArray detail = new JSONArray();
		JSONObject detailmember = new JSONObject();
		
		detailmember.put("amount", 200);				
		detailmember.put("start" ,"1451535556");
		detailmember.put("length" ,"0");
		detailmember.put("ext" ,"xxxx");	
		
		detail.add(detailmember);
		
		detailmember.put("amount", -100);				
		detailmember.put("start" ,"1451535556");
		detailmember.put("length" ,"-1");
		detailmember.put("ext" ,"xxxx");	
		
		detail.add(detailmember);
		
		issueAsset(access_token, user_name1.toString(),address1.toString(),assetcode,asset_name,asset_unit,null,detail);
		
		//create user 2		
		createUser(access_token, user_name2,address2);
		
		grantAsset(access_token, address1.toString(),user_name1.toString(),address2.toString(),assetcode.toString(),50);
		
		
	}

	//to test when use async interface to send/grant amount larger than it has
	public void e_SendAmountLargerThanHave()
	{
		

		access_token = getToken();
		
		//user 1 issue a kind of asset		
		createUser(access_token, user_name1,address1);
		issueAsset(access_token, user_name1.toString(),address1.toString(),assetcode,asset_name,asset_unit,null);
		
		//create user 2		
		createUser(access_token, user_name2,address2);
		
		//create user 3
		createUser(access_token, user_name3,address3);
		
		//create user 4
		createUser(access_token, user_name4,address4);
		
		String result = sendAsset(true,access_token, address1.toString(),user_name1.toString(),address2.toString(), assetcode.toString(),99);
		JSONObject jsonObject =JSONObject.fromObject(result);
		String trade_no_1 = jsonObject.getJSONObject("data").getString("trade_no");
		check.equals(jsonObject.getString("err_code"), "0", "errorcode is not as expected");
		check.equals(jsonObject.getString("msg"), "资产正在转移", "msg is not as expected");
		
		result = sendAsset(true,access_token, address1.toString(),user_name1.toString(),address3.toString(), assetcode.toString(),99);
		jsonObject =JSONObject.fromObject(result);
		String trade_no_2 = jsonObject.getJSONObject("data").getString("trade_no");
		check.equals(jsonObject.getString("err_code"), "0", "errorcode is not as expected");
		check.equals(jsonObject.getString("msg"), "资产正在转移", "msg is not as expected");
		
		result = sendAsset(true,access_token, address1.toString(),user_name1.toString(),address4.toString(), assetcode.toString(),99);
		jsonObject =JSONObject.fromObject(result);
		String trade_no_3 = jsonObject.getJSONObject("data").getString("trade_no");
		check.equals(jsonObject.getString("err_code"), "0", "errorcode is not as expected");
		check.equals(jsonObject.getString("msg"), "资产正在转移", "msg is not as expected");
		
		try   
		{   
			Thread.currentThread();
			Thread.sleep(10000);//毫秒   
		}   
		catch(Exception e){}  
		
		checkStatus("/status/asset/v1/send",access_token,trade_no_1,null);
		checkStatus("/status/asset/v1/send",access_token,trade_no_2,null);
		checkStatus("/status/asset/v1/send",access_token,trade_no_3,null);
		
		
	}
	
	//to test when add to issue
	
	
}