package cases;

import java.util.LinkedHashMap;
import java.util.Random;
import org.testng.annotations.Test;

import base.TestBase;
import net.sf.json.JSONArray;
import net.sf.json.JSONObject;
import utils.APIUtil;
import utils.HttpUtil;
@Test
public class ApiTestGetUserInfo extends ApiGeneralCases {
	
	//definition of conditions
	
	final static int invalidUser	= 1<<3;
	final static int noUser	    	= 1<<4;
	final static int validUser		= 1<<5;
	final static int userHasManyAsset = 1<<6;
	
	final static int aftersendtoothers = 1<<7;
	final static int aftergranttoothers = 1<<8;
	final static int afternoasset = 1<<9;
	
	//integration test
	final static int afterinvalidsend = 1<<10;
	final static int afterinvalidgrant = 1<<11;
	final static int afterinvalidissue = 1<<12;
	final static int aftersendfromothers = 1<<13;
	final static int aftergrantfromothers = 1<<14;
	final static int afterchangepassword = 1<<15;
	final static int afteradd2issue		 = 1<<16;
		
	private int current_condition = 0;
	
	static Random rand = new Random();
	static int tradeNoBase = rand.nextInt(10000);
	static int index = 0;
	
	static String asset_unit = "元";
	static String asset_name = "某种资产";
	
	private void TestUserInfo(Object param, JSONArray detail)
	{
		access_token = getToken();
		StringBuffer user_name1 = new StringBuffer("");  
		StringBuffer address1 = new StringBuffer("");  
		StringBuffer user_name2 = new StringBuffer("");  
		StringBuffer address2 = new StringBuffer("");  
		createUser(access_token, user_name1,address1);
		createUser(access_token, user_name2,address2);
		StringBuffer assetcode1 = new StringBuffer("");
		StringBuffer assetcode2 = new StringBuffer("");
		if(APIUtil.getbool(current_condition&userHasManyAsset))
		{
			for(int i = 0;i<100;++i)
			{
				issueAsset(access_token, user_name1.toString(),address1.toString(),assetcode1,asset_name,asset_unit,null);
			}			
		}			
		else			
			issueAsset(access_token, user_name1.toString(),address1.toString(),assetcode1,asset_name,asset_unit,null);
		issueAsset(access_token, user_name2.toString(),address2.toString(),assetcode2,asset_name,asset_unit,null);
		
		if(APIUtil.getbool(current_condition&aftersendtoothers))
		{
			sendAsset(false,access_token,address1.toString(),user_name1.toString(),address2.toString(),assetcode1.toString(),50);
		}	
		if(APIUtil.getbool(current_condition&afternoasset))
		{
			sendAsset(false,access_token,address1.toString(),user_name1.toString(),address2.toString(),assetcode1.toString(),100);
		}	
		if(APIUtil.getbool(current_condition&afterinvalidsend))
		{
			sendAsset(false,access_token,address1.toString(),user_name1.toString(),address2.toString(),assetcode1.toString(),101);
		}
		
		if(APIUtil.getbool(current_condition&aftergranttoothers))
		{
			grantAsset(access_token,address1.toString(),user_name1.toString(),address2.toString(),assetcode1.toString(),50);
		}
		if(APIUtil.getbool(current_condition&afterinvalidgrant))
		{
			grantAsset(access_token,address1.toString(),user_name1.toString(),address2.toString(),assetcode1.toString(),101);
		}
		
		if(APIUtil.getbool(current_condition&aftersendfromothers))
		{
			sendAsset(false,access_token,address2.toString(),user_name2.toString(),address1.toString(),assetcode2.toString(),100);
		}
		
		if(APIUtil.getbool(current_condition&aftergrantfromothers))
		{
			grantAsset(access_token,address2.toString(),user_name2.toString(),address1.toString(),assetcode2.toString(),100);
		}
		if(APIUtil.getbool(current_condition&afteradd2issue))
		{
			add2Issue(access_token,user_name1.toString(),assetcode1.toString(),100);
		}
		
		if(APIUtil.getbool(current_condition&afterchangepassword))
		{
			changePassword(access_token,user_name1.toString(),"newpassword");
		}				
			
		String api_path = "/account/v1/info" ;
		parameter.clear();
		
		if(!APIUtil.getbool(current_condition&noUser))
		{
			parameter.put("bubi_address",(String)param);
			if(APIUtil.getbool(current_condition&invalidUser))
				parameter.put("bubi_address",(String)param);
			else
				parameter.put("bubi_address",address1.toString());
		}
		
		if(!APIUtil.getbool(current_condition&noAccessToken))
		{
			parameter.put("access_token",(String)param);
			if(APIUtil.getbool(current_condition&invalidAccessToken))
				parameter.put("access_token",(String)param);
			else
				parameter.put("access_token",access_token);
		}
		
		System.out.println("=========Get User Info========");
		String result = HttpUtil.dogetApi(api_url, api_path, parameter);
		JSONObject jsonObject =JSONObject.fromObject(result);  
		Verification(jsonObject);
		
	}
	private void Verification(JSONObject jsonObject)
	{
		switch (current_condition)
		{
			case validAccessToken	:
			case validUser:	
			case userHasManyAsset:
			case 0:
				check.equals(jsonObject.getString("err_code"), "0", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "操作成功", "msg is not as expected");
				break;
			
			case invalidAccessToken		:
			case noAccessToken		:
				check.equals(jsonObject.getString("err_code"), "20015", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "授权码不存在或已过期", "msg is not as expected");
				break;
			case noUser:
			case invalidUser:
				check.equals(jsonObject.getString("err_code"), "20031", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "账户不存在或账户未激活", "msg is not as expected");
				break;			
		}		
	}	

	public void a_normaltest()
	{
		current_condition = 0;
		TestUserInfo(null,null);
	}

	public void b_accessTokenTest()
	{
		current_condition = noAccessToken;
		TestUserInfo(null,null);
		
		current_condition = invalidAccessToken;
		String conditions[]={
				"",
				"wrong_condition"
				
		};
		for (int i =0 ;i<2;++i)
		{
			TestUserInfo(conditions[i],null);
		}
		
		current_condition = validAccessToken;
		TestUserInfo(null,null);
	}

	public void c_userTest()
	{
		current_condition = noUser;
		TestUserInfo(null,null);
		
		current_condition = invalidUser;
		String conditions[]={
				"",
				"wronguser",
				APIUtil.getRandomString(100)
				
		};
		for (int i =0 ;i<3;++i)
		{
			TestUserInfo(conditions[i],null);
		}
		
		current_condition = validUser;
		
		TestUserInfo(null,null);
		
		
	}
	
	public void d_integratedTest()
	{
		
		//current_condition = userHasManyAsset;		
		//TestUserInfo(null,null);
		//current_condition = aftersendtoothers ;
		//TestUserInfo(null,null);
		//current_condition = afternoasset;
		//TestUserInfo(null,null);
		current_condition = afterinvalidsend ;
		TestUserInfo(null,null);
		//current_condition = aftergranttoothers ;
		//TestUserInfo(null,null);				
		current_condition = afterinvalidgrant ;
		TestUserInfo(null,null);
		//current_condition = afterinvalidissue;
		//TestUserInfo(null,null);
		//current_condition = aftersendfromothers ;
		//TestUserInfo(null,null);
		//current_condition = aftergrantfromothers;
		//TestUserInfo(null,null);
		//current_condition = afteradd2issue;
		//TestUserInfo(null,null);
		//current_condition = afterchangepassword;
		//TestUserInfo(null,null);
	
	}
	

}