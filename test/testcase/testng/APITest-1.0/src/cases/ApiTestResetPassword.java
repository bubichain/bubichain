package cases;

import org.testng.annotations.Test;
import net.sf.json.JSONArray;
import net.sf.json.JSONObject;
import utils.APIUtil;
import utils.HttpUtil;

@Test

public class ApiTestResetPassword extends ApiGeneralCases {


	final static int noUserName		    = 1<<31;
	final static int invalidUserName 	= 1<<30;
	final static int validUserName		= 1<<29;
	final static int noNewPassword	    = 1<<28;
	final static int invalidNewPassword   = 1<<27;
	final static int validNewPassword	= 1<<26;
	

	
	static StringBuffer user_name = new StringBuffer("");  
	static StringBuffer address = new StringBuffer("");  
	static String asset_unit = "元";
	static String asset_name = "某种资产";
	
	@Test(enabled = false)
	public void TestProcess(Object param, JSONArray json_param)
	{
		access_token = getToken();
		
		createUser(access_token, user_name,address);
		String api_path = "/account/v1/alterPwd" ;
		
		//clean map
		parameter.clear();
		thrMap.clear();
		
		generalSetup(current_condition,noAccessToken,invalidAccessToken,"access_token",param,access_token,parameter);
		
		generalSetup(current_condition,noUserName,invalidUserName,validUserName,"user_name",param,user_name.toString(),thrMap);
		
		//special
		if(!APIUtil.getbool(current_condition&noNewPassword))
		{
			if(APIUtil.getbool(current_condition&(invalidNewPassword|validNewPassword)))
			{
				if(current_condition_sub==issame)
					//password is same this name
					param = user_name.toString();
				
					thrMap.put("new_password", param);
				
			}				
			else
				thrMap.put("new_password", user_name.toString()+"new");		
		}
		
		trade_no = Long.toString((tradeNoBase++)+(System.currentTimeMillis()<<8));
		generalSetup(current_condition,noTradeNo,invalidTradeNo,validTradeNo,"trade_no",param,trade_no,thrMap);
			
		JSONObject jsonBody = JSONObject.fromObject(thrMap);
		System.out.println("=========Reset Password========");
		String result = HttpUtil.dopostApi(api_url, api_path, parameter, jsonBody);
		JSONObject jsonObject =JSONObject.fromObject(result);  
		Verification(jsonObject,address.toString(),(String)thrMap.get("new_password"));	
		user_name.delete(0, user_name.length());
		address.delete(0, address.length());
		
	}

	private void Verification(JSONObject jsonObject,String address,String param)
	{
		verification(jsonObject);
		switch (current_condition)
		{
			case noUserName		    :			
			case invalidUserName 		:
				check.equals(jsonObject.getString("err_code"), "21102", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "用户不存在或用户非法", "msg is not as expected");
				Confirmation(false);
				break;
			case noNewPassword	    :
			case invalidNewPassword   :
				check.equals(jsonObject.getString("err_code"), "10001", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "参数校验失败", "msg is not as expected");
				Confirmation(false);
				break;		
			case validNewPassword:
			case validUserName:			
			case invalidMetaData:
			case validAccessToken	:
			case validTradeNo:
			case 0:
				check.equals(jsonObject.getString("err_code"), "0", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "操作成功", "msg is not as expected");
				check.equals(jsonObject.getString("data"), address, "address is not as expected");
				Confirmation(true,param);
				break;			
			default:
				break;
		}		
	}
	@Test(enabled = false)
	public void Confirmation(boolean issuccess, String param)
	{
		System.out.println("=========Confirm the result========");
		System.out.println("=========try new password========");
		String pwnew = param;
		StringBuffer assetcode= new StringBuffer("");  
		String result = issueAsset(access_token,pwnew,address.toString(),assetcode,asset_name,asset_unit,null);
		JSONObject jsonObject =JSONObject.fromObject(result);  
		if(issuccess)
		{
			check.equals(jsonObject.getString("err_code"), "0", "errorcode is not as expected");
			check.equals(jsonObject.getString("msg"), "操作成功", "errorcode is not as expected");
		}
		else
		{
			check.equals(jsonObject.getString("err_code"), "20027", "errorcode is not as expected");		
			check.equals(jsonObject.getString("msg"), "区块链账户与密码不匹配", "errorcode is not as expected");		
		}	
		System.out.println("=========try old password========");
		result = issueAsset(access_token,user_name.toString(),address.toString(),assetcode,asset_name,asset_unit,null);
		jsonObject =JSONObject.fromObject(result);  
		if(issuccess&&current_condition_sub!=issame)
		{
			check.equals(jsonObject.getString("err_code"), "20027", "errorcode is not as expected");		
			check.equals(jsonObject.getString("msg"), "区块链账户与密码不匹配", "errorcode is not as expected");		
		}
		else
		{
			check.equals(jsonObject.getString("err_code"), "0", "errorcode is not as expected");
			check.equals(jsonObject.getString("msg"), "操作成功", "errorcode is not as expected");
		}	
	}
	

	public void a_general()
	{
		a_normaltest();
		b_accessTokenTest();
		d_tradeNoTest();
		
	}

	public void b_userNametest()
	{
		current_condition = noUserName;
		TestProcess(null,null);
		current_condition = invalidUserName;
		String conditions[]={
				"",
				"wrongusername",
				"$#@$",
				APIUtil.getRandomString(100)
					
		};
		for (int i =0 ;i<4;++i)
		{
			TestProcess(conditions[i],null);
		}		
	}

	public void c_newPasswordTest()
	{
		current_condition = noNewPassword;
		TestProcess(null,null);
		current_condition = invalidNewPassword;
		String conditions[]={
				"",
				APIUtil.getRandomString(password_limit+1),	
					
		};
		for (int i =0 ;i<2;++i)
		{
			TestProcess(conditions[i],null);
		}	
	
		current_condition = validNewPassword;
		String conditions1[]={
				"$#@$#@$",
				APIUtil.getRandomString(password_limit),
				APIUtil.getRandomString(password_limit-1),
				
		};
		for (int i =0 ;i<3;++i)
		{
			TestProcess(conditions1[i],null);
		}	
		current_condition_sub=issame;
		TestProcess(null,null);
		current_condition_sub=0;
		
		
	}	
	 
}