package cases;

import org.testng.annotations.Test;

import net.sf.json.JSONArray;
import net.sf.json.JSONObject;
import utils.APIUtil;
import utils.HttpUtil;


@Test
public class ApiTestAdd2IssueAsync extends ApiGeneralCases {
	
	//definition of conditions
	
	final static int noAssetCode		= 1<<31;
	final static int invalidAssetCode	= 1<<30;
	final static int validAssetCode		= 1<<29;
	
	static String asset_amount = new String();
	static String asset_unit = "元";
	static String asset_name = "某种资产";
	static StringBuffer assetcode = new StringBuffer("");
	static StringBuffer user_name = new StringBuffer("");  
	static StringBuffer address = new StringBuffer("");  
	
	@Test(enabled = false)
	public void TestProcess(Object param, JSONArray detail)
	{
		access_token = getToken();
		createUser(access_token, user_name,address);
		
		issueAsset(access_token, user_name.toString(),address.toString(),assetcode,asset_name,asset_unit,null);
		
		String api_path = "/asset/v1/add2IssueAsync" ;
		//clean map
		parameter.clear();
		thrMap.clear();
		
		generalSetup(current_condition,noAccessToken,invalidAccessToken,"access_token",param,access_token,parameter);
		
		trade_no = Long.toString((tradeNoBase++)+(System.currentTimeMillis()<<8));
		generalSetup(current_condition,noTradeNo,invalidTradeNo,validTradeNo,"trade_no",param,trade_no,thrMap);
		trade_no = (String)thrMap.get("trade_no");
		generalSetup(current_condition,noAssetCode,invalidAssetCode,"asset_code",param,assetcode.toString(),thrMap);
		generalSetup(current_condition,noPassword,invalidPassword,"password",param,user_name.toString(),thrMap);
		generalSetup(current_condition,noAssetCount,invalidAssetCount,validAssetCount,"asset_amount",param,"1000",thrMap);
		asset_amount = (String)thrMap.get("asset_amount");
		
		//currently special
		if(!APIUtil.getbool(current_condition&noMetaData))
		{
			if(APIUtil.getbool(current_condition&invalidMetaData))
				thrMap.put("metadata", "xxx");	
			else
				thrMap.put("metadata", "444");	
		}
		
		if(detail!=null)
		{
			thrMap.put("details", detail);	
		}
		
		JSONObject jsonBody = JSONObject.fromObject(thrMap);
		System.out.println("=========Add to issue========");
		String result = HttpUtil.dopostApi(api_url, api_path, parameter, jsonBody);
		JSONObject jsonObject =JSONObject.fromObject(result);  
		Verification(jsonObject);
		assetcode.delete(0, assetcode.length());
		user_name.delete(0, user_name.length());
		address.delete(0, address.length());
		
	}
	private void Verification(JSONObject jsonObject)
	{
		verification(jsonObject);
		switch (current_condition)
		{
			
			case validAssetCount	:
			case validAssetCode		:			
			case validMetaData		:
			case noMetaData	:	
			case invalidMetaData:
			case validAccessToken	:
			case validPassword:
			case validTradeNo:
			case 0:
				check.equals(jsonObject.getString("err_code"), "0", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "资产正在追加发行", "msg is not as expected");
				check.equals(jsonObject.getJSONObject("data").getString("trade_no"),trade_no,"trade_no is not as expected");
				Confirmation(true);
				break;
			case invalidAssetCount    :
				check.equals(jsonObject.getString("err_code"), "21205", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "资产发行数量必须是整数", "msg is not as expected");
				Confirmation(false);
				break;
			case invalidAssetCode     :
				check.equals(jsonObject.getString("err_code"), "21222", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "资产编号不存在", "msg is not as expected");
				Confirmation(false);
				break;				
			case noAssetCount         :
				check.equals(jsonObject.getString("err_code"), "21204", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "资产发行数量不能为空", "msg is not as expected");
				Confirmation(false);
				break;
			case noAssetCode          :
				check.equals(jsonObject.getString("err_code"), "21222", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "资产编号不存在", "msg is not as expected");
				Confirmation(false);
				break;					
			case validAssetCount|invalidDetail:
				//if(current_condition_sub == overlimit)
				//{
					check.equals(jsonObject.getString("err_code"), "22708", "errorcode is not as expected");
					check.equals(jsonObject.getString("msg"), "资产的明细格式错误", "msg is not as expected");
					Confirmation(false);
				//}				
				break;
			case noPassword:	
			case invalidPassword:	
				check.equals(jsonObject.getString("err_code"), "20027", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "区块链账户与密码不匹配", "msg is not as expected");				
				Confirmation(false);
				break;
		
		}
		
	}
	
	private void Confirmation(boolean issuccess )
	{
		try   
		{   
			Thread.currentThread();
			Thread.sleep(10000);//毫秒   
		}   
		catch(Exception e){}  
		System.out.println("=========Confirm the result========");
		System.out.println("=========check issue status========");
		String result = checkStatus("/status/asset/v1/issue",access_token,trade_no,null);
		JSONObject jsonObject =JSONObject.fromObject(result);  
		if(issuccess)
		{
			check.equals(jsonObject.getString("err_code"), "0", "errorcode is not as expected");	
			check.equals(jsonObject.getString("msg"), "操作成功", "msg is not as expected");
			check.equals(Integer.toString((int) jsonObject.getJSONObject("data").get("asset_amount")),asset_amount,"amount is not as expected");
			check.equals((String)jsonObject.getJSONObject("data").get("asset_unit"),asset_unit,"unit is not as expected");
			check.equals((String)jsonObject.getJSONObject("data").get("asset_name"),asset_name,"asset name is not as expected");
			check.equals((String)jsonObject.getJSONObject("data").get("asset_code"),assetcode.toString(),"asset_code is not as expected");
			check.equals((String)jsonObject.getJSONObject("data").get("asset_issuer"),address.toString(),"asset_issuer is not as expected");	
			String hash = (String)jsonObject.getJSONObject("data").get("bc_hash");
			check.equals(hash.length(),64,"hash is not as expected");				
			
		}
		else 
		{
			if((current_condition==invalidTradeNo)&&(current_condition_sub==overlimit))
			{
				check.equals(jsonObject.getString("err_code"), "10001", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "凭据号长度范围[1, 55]", "msg is not as expected");
			}
			else if((current_condition==invalidTradeNo)&&(current_condition_sub==overlimit))
			{
				check.equals(jsonObject.getString("err_code"), "10001", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "凭据号不能为空", "msg is not as expected");			
			}
			else
			{
				check.equals(jsonObject.getString("err_code"), "21216", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "资产凭据号不存在", "msg is not as expected");
			}
		}	
	}
	
	public void a_generaltest()
	{
		a_normaltest();
		b_accessTokenTest();
		c_passwordTest();
		d_tradeNoTest();
		f_detailTest();
		
	}

	public void e_assetCodeTest()
	{
		current_condition = noAssetCode;
		TestProcess(null,null);
		
		current_condition = invalidAssetCode;
		String conditions[]={
				"",
				"$#@$#@",
				APIUtil.getRandomString(100),
				APIUtil.getRandomString(1000),
				
		};
		for (int i =0 ;i<4;++i)
		{
			TestProcess(conditions[i],null);
		}
		current_condition = validAssetCode;
		TestProcess(null,null);
		
	}
	

	public void f_assetCountTest()
	{
		current_condition = noAssetCount;
		TestProcess(null,null);
		
		current_condition = invalidAssetCount;
		String conditions[]={
				"",
				"-1",
				"0",
				"9999999999999999999999999999999999999999999999999999999"	,
				"3.11111"	,
				"数量"	,
				"@￥#@！￥"	,				
		};
		for (int i =0 ;i<7;++i)
		{
			TestProcess(conditions[i],null);
		}
		current_condition = validAssetCount;
		String conditions1[]={
				"123456",
				"43425",		
				"1"
		};
		for (int i =0 ;i<3;++i)
		{
			TestProcess(conditions1[i],null);
		}
	}
	
	
}