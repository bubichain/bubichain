package cases;

import java.util.LinkedHashMap;
import java.util.Map;
import org.testng.annotations.Test;

import net.sf.json.JSONArray;
import net.sf.json.JSONObject;
import utils.APIUtil;
import utils.HttpUtil;

@Test
public class ApiTestRegister extends ApiGeneralCases {
	
		final static int noUserName	   		= 1<<31;
		final static int invalidUserName	= 1<<30;
		final static int validUserName	   	= 1<<29;
				
		final static int username_limit = 25;
				
		static int index = 0;
		
		static String address;
	
	@Test(enabled = false)
	public void TestProcess(Object param, JSONArray json_param)
	{
		access_token = getToken();
		
		String api_path = "/account/v1/register" ;
		//clean map
		parameter.clear();
		thrMap.clear();
		
		generalSetup(current_condition,noAccessToken,invalidAccessToken,"access_token",param,access_token,parameter);
		
		generalSetup(current_condition,noUserName,invalidUserName,validUserName,"user_name",param,APIUtil.getRandomString(10),thrMap);
		generalSetup(current_condition,noPassword,invalidPassword,validUserName,"password",param,APIUtil.getRandomString(10),thrMap);
		trade_no = Long.toString((tradeNoBase++)+(System.currentTimeMillis()<<8));
		generalSetup(current_condition,noTradeNo,invalidTradeNo,validTradeNo,"trade_no",param,trade_no,thrMap);
		trade_no = (String)thrMap.get("trade_no");
		//currently special
		if(!APIUtil.getbool(current_condition&noMetaData))
		{
			if(APIUtil.getbool(current_condition&invalidMetaData))
				thrMap.put("metadata", "xxx");	
			else
				thrMap.put("metadata", "444");	
		}
		JSONObject jsonBody = JSONObject.fromObject(thrMap);
		System.out.println("=========Register user========");
		String result = HttpUtil.dopostApi(api_url, api_path, parameter, jsonBody);
		JSONObject jsonObject =JSONObject.fromObject(result);  
		Verification(jsonObject);
		
	}
	private void Verification(JSONObject jsonObject)
	{
		verification(jsonObject);
		
		switch (current_condition)
		{
			case noUserName:	
				check.equals(jsonObject.getString("err_code"), "10001", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "用户名不能为空", "msg is not as expected");				
				break;
			case invalidUserName:	
				if(current_condition_sub == overlimit)
				{
					check.equals(jsonObject.getString("err_code"), "21104", "errorcode is not as expected");
					check.equals(jsonObject.getString("msg"), "用户名长度不能大于25", "msg is not as expected");				
				}
				else
				{
					check.equals(jsonObject.getString("err_code"), "10001", "errorcode is not as expected");
					check.equals(jsonObject.getString("msg"), "用户名非法", "msg is not as expected");				
				}
				Confirmation(false);
				break;
			case validUserName:		
			case 0:
			case validAccessToken	:
			case validPassword:
			case validTradeNo:
				check.equals(jsonObject.getString("err_code"), "0", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "操作成功", "msg is not as expected");
				String address = (String)jsonObject.getJSONObject("data").get("bubi_address");
				check.equals(address.length(),40,"address is not as expected");
				Confirmation(true,address);
				break;			
			default:
				break;
			case noPassword:	
				check.equals(jsonObject.getString("err_code"), "10001", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "用户密码不能为空", "msg is not as expected");				
				Confirmation(false);
				break;
			case invalidPassword:	
				if(current_condition_sub == overlimit)
				{
					check.equals(jsonObject.getString("err_code"), "21105", "errorcode is not as expected");
					check.equals(jsonObject.getString("msg"), "密码长度不能大于45", "msg is not as expected");				
				}	
				else
				{
					check.equals(jsonObject.getString("err_code"), "10001", "errorcode is not as expected");
					check.equals(jsonObject.getString("msg"), "用户密码不能为空", "msg is not as expected");				
				}

				Confirmation(false);
				break;
		}
		
	}
	@Test(enabled = false)
	public void Confirmation(boolean issuccess, String ...param)
	{
		System.out.println("=========Confirm the result========");
		System.out.println("=========check register status========");
		String result = checkStatus("/status/account/v1/register",access_token,trade_no,null);
		JSONObject jsonObject =JSONObject.fromObject(result);  
		if(issuccess)
		{
			check.equals(jsonObject.getString("err_code"), "0", "errorcode is not as expected");	
			check.equals(jsonObject.getString("msg"), "操作成功", "msg is not as expected");
			check.equals((String)jsonObject.getJSONObject("data").get("bubi_address"),param[0],"address is not as expected");
		}			
		else
		{
			if((current_condition==invalidTradeNo&&current_condition_sub==isnull)||current_condition==noTradeNo)
			{
				check.equals(jsonObject.getString("err_code"), "10001", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "凭据号不能为空", "msg is not as expected");	
				
			}
			else
			{
				
				check.equals(jsonObject.getString("err_code"), "21102", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "用户不存在或用户非法", "msg is not as expected");	
			}
		}	
		
	}

	public void a_general()
	{

		a_normaltest();
		b_accessTokenTest();
		c_passwordTest();
		d_tradeNoTest();
		
	}

	public void e_userNameTest()
	{
		current_condition = noUserName;
		TestProcess(null,null);
		current_condition = invalidUserName;
		LinkedHashMap<String,Integer> conditions = new LinkedHashMap<String,Integer> ();
		conditions.put("",isnull);
		conditions.put("-1",isminus);
		conditions.put("0",illegal);
		conditions.put("#$%#",illegal);	
		conditions.put(APIUtil.getRandomString(username_limit+1),overlimit);					
		for (Map.Entry<String, Integer> entry : conditions.entrySet()) {  
			 
			current_condition_sub = entry.getValue();
			TestProcess(entry.getKey(),null);
		}  

		current_condition = validUserName;
		String conditions1[]={
				APIUtil.getRandomString(username_limit),
				APIUtil.getRandomString(username_limit-1),			
		};
		for (int i =0 ;i<2;++i)
		{
			TestProcess(conditions1[i],null);
		}
		
	}
	
	
}