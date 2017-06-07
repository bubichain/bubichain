package cases;

import java.util.LinkedHashMap;
import java.util.Map;

import org.testng.annotations.Test;

import net.sf.json.JSONArray;
import net.sf.json.JSONObject;
import utils.APIUtil;
import utils.HttpUtil;

@Test
public class ApiTestIssueAsync extends ApiGeneralCases {
	
	//definition of conditions
		
	final static int noIssuer			= 1<<31;
	final static int invalidIssuer		= 1<<30;
	final static int validIssuer		= 1<<29;
	
	final static int noAssetName		= 1<<28;
	final static int invalidAssetName	= 1<<27;
	final static int validAssetName		= 1<<26;
	                                      
	final static int noAssetUnit		= 1<<25;
	final static int invalidAssetUnit	= 1<<24;
	final static int validAssetUnit		= 1<<23;
	
	final static int noAssetCount		= 1<<22;
	final static int invalidAssetCount	= 1<<21;
	final static int validAssetCount	= 1<<20;
	
	final static int noBody				= 1<<19;
	final static int invalidBody		= 1<<18;
	final static int validBody			= 1<<17;			
	
	static StringBuffer assetcode = new StringBuffer("");
	static StringBuffer user_name = new StringBuffer("");  
	static StringBuffer address = new StringBuffer("");  
	@Test(enabled = false)
	public void TestProcess(Object param, JSONArray detail)
	{
		access_token = getToken();
		
		createUser(access_token, user_name,address);
		
		String api_path = "/asset/v1/issueAsync" ;
		//clean map
		parameter.clear();
		thrMap.clear();
		
		generalSetup(current_condition,noAccessToken,invalidAccessToken,"access_token",param,access_token,parameter);
		
		trade_no = Long.toString((tradeNoBase++)+(System.currentTimeMillis()<<8));
		generalSetup(current_condition,noTradeNo,invalidTradeNo,validTradeNo,"trade_no",param,trade_no,thrMap);
		trade_no =(String)thrMap.get("trade_no");
	
		generalSetup(current_condition,noAssetCount,invalidAssetCount,validAssetCount,"asset_amount",param,"20",thrMap);
		generalSetup(current_condition,noAssetName,invalidAssetName,validAssetName,"asset_name",param,"测试的个数",thrMap);	
		generalSetup(current_condition,noAssetUnit,invalidAssetUnit,validAssetUnit,"asset_unit",param,"个",thrMap);	
		asset_name = (String) thrMap.get("asset_name");
		asset_unit = (String) thrMap.get("asset_unit");
		asset_amount = (String) thrMap.get("asset_amount");
		
		generalSetup(current_condition,noPassword,invalidPassword,"password",param,user_name.toString(),thrMap);
		generalSetup(current_condition,noIssuer,invalidIssuer,"asset_issuer",param,address.toString(),thrMap);
			
		if(!APIUtil.getbool(current_condition&noMetaData))
		{
			if(APIUtil.getbool(current_condition&invalidMetaData))
				thrMap.put("metadata", "xxx");	
			else
				thrMap.put("metadata", "444");	
		}
		
		if(!APIUtil.getbool(current_condition&noBody))
		{
			if(APIUtil.getbool(current_condition&invalidBody))
				thrMap.put("body", "xxx");	
			else
				thrMap.put("body", "test asset");	
		}
		if(detail!=null)
		{
			thrMap.put("details", detail);	
		}
				
		JSONObject jsonBody = JSONObject.fromObject(thrMap);
		
		System.out.println("=========Issue asset========");
		String result = HttpUtil.dopostApi(api_url, api_path, parameter, jsonBody);
		JSONObject jsonObject =JSONObject.fromObject(result);  
		Verification(jsonObject);
		assetcode.delete(0, assetcode.length());
		user_name.delete(0, user_name.length());		
		address.delete(0, address.length());
		
		
	}
	private void Verification(JSONObject jsonObject)
	{
		switch (current_condition)
		{
			case validAccessToken	:
			case validAssetCount	:
			case validAssetName		:
			case validAssetUnit		:  
			case validBody			:
			case validIssuer		:
			case validMetaData		:
			case validPassword   	:
			case validTradeNo		:
			case invalidMetaData      :
			case noMetaData           :
			case 0:
				check.equals(jsonObject.getString("err_code"), "0", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "资产正在发行", "msg is not as expected");
				check.equals(jsonObject.getJSONObject("data").getString("trade_no"),trade_no,"trade_no is not as expected");
				Confirmation(true);
				break;
			case invalidAssetCount    :
				check.equals(jsonObject.getString("err_code"), "21205", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "资产发行数量必须是整数", "msg is not as expected");
				Confirmation(false);
				break;
			case invalidAssetName     :
				if(current_condition_sub == isnull)
				{
					check.equals(jsonObject.getString("err_code"), "21202", "errorcode is not as expected");
					check.equals(jsonObject.getString("msg"), "资产名称不能为空", "msg is not as expected");
					
				}
				else if(current_condition_sub == overlimit)
				{
					check.equals(jsonObject.getString("err_code"), "21210", "errorcode is not as expected");
					check.equals(jsonObject.getString("msg"), "资产名称长度不能大于20", "msg is not as expected");
					
				}	
				Confirmation(false);
				break;
			case invalidAssetUnit     :
				if(current_condition_sub == isnull)
				{
					check.equals(jsonObject.getString("err_code"), "21203", "errorcode is not as expected");
					check.equals(jsonObject.getString("msg"), "资产单位不能为空", "msg is not as expected");
					
				}
				else if(current_condition_sub == overlimit)
				{
					check.equals(jsonObject.getString("err_code"), "21211", "errorcode is not as expected");
					check.equals(jsonObject.getString("msg"), "资产单位长度不能大于6", "msg is not as expected");
					
				}
				Confirmation(false);
				break;
			case invalidBody          :
				break;
			case invalidIssuer        :
				check.equals(jsonObject.getString("err_code"), "21702", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "资产发行方不能为空", "msg is not as expected");
				Confirmation(false);
				break;		
			
			case noAssetCount         :
				check.equals(jsonObject.getString("err_code"), "21204", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "资产发行数量不能为空", "msg is not as expected");
				Confirmation(false);
				break;
			case noAssetName          :
				check.equals(jsonObject.getString("err_code"), "21202", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "资产名称不能为空", "msg is not as expected");
				Confirmation(false);
				break;
			case noAssetUnit          :
				check.equals(jsonObject.getString("err_code"), "21203", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "资产单位不能为空", "msg is not as expected");
				Confirmation(false);
				break;
			case noBody               :
				break;
			case noIssuer             :
				check.equals(jsonObject.getString("err_code"), "21702", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "资产发行方不能为空", "msg is not as expected");
				Confirmation(false);
				break;
			
			case noTradeNo            :
				check.equals(jsonObject.getString("err_code"), "21200", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "资产发行凭据号不能为空", "msg is not as expected");
				Confirmation(false);
				break;
			case invalidTradeNo       :
				if(current_condition_sub == isnull)
				{
					check.equals(jsonObject.getString("err_code"), "文档未定义", "errorcode is not as expected");
					check.equals(jsonObject.getString("msg"), "文档未定义", "msg is not as expected");
				
				}
				if(current_condition_sub == illegal)
				{
					check.equals(jsonObject.getString("err_code"), "文档未定义", "errorcode is not as expected");
					check.equals(jsonObject.getString("msg"), "文档未定义", "msg is not as expected");
				
				}	
				if(current_condition_sub == overlimit)
				{
					check.equals(jsonObject.getString("err_code"), "20021", "errorcode is not as expected");
					check.equals(jsonObject.getString("msg"), "凭据号长度不能大于%d", "msg is not as expected");
				
				}	
				Confirmation(false);
				break;
			
				
			case invalidAccessToken		:
			case noAccessToken		:
				check.equals(jsonObject.getString("err_code"), "20015", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "授权码不存在或已过期", "msg is not as expected");
				Confirmation(false);
				break;
			case validAssetCount|invalidDetail:
				//if(current_condition_sub == overlimit)
				//{
					check.equals(jsonObject.getString("err_code"), "22708", "errorcode is not as expected");
					check.equals(jsonObject.getString("msg"), "资产的明细格式错误", "msg is not as expected");
					
				//}	
					Confirmation(false);
				break;
			case noPassword:	
			case invalidPassword:	
				check.equals(jsonObject.getString("err_code"), "20027", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "区块链账户与密码不匹配", "msg is not as expected");				
				Confirmation(false);
				break;
		}
		
	}
	
	private void Confirmation(boolean issuccess)
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
			check.equals((String)jsonObject.getJSONObject("data").get("asset_issuer"),address.toString(),"address is not as expected");
			
			check.equals((String)jsonObject.getJSONObject("data").get("asset_unit"),asset_unit,"unit is not as expected");
			check.equals((String)jsonObject.getJSONObject("data").get("asset_name"),asset_name,"name is not as expected");
			String asset_code = (String)jsonObject.getJSONObject("data").get("asset_code");
			check.equals(asset_code.length(),144,"asset_code is not as expected");	
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
	
	@Test
	public void a_generalTest()
	{
		a_normaltest();
		b_accessTokenTest();
		c_passwordTest();
		d_tradeNoTest();
		f_detailTest();
		g_signTest();		
	}

	public void e_assetIssuerTest()
	{
		current_condition = noIssuer;
		TestProcess(null,null);
		
		current_condition = invalidIssuer;
		String conditions[]={
				"",
				"$#@$#@",
				APIUtil.getRandomString(100),
				
		};
		for (int i =0 ;i<3;++i)
		{
			TestProcess(conditions[i],null);
		}
		current_condition = validIssuer;
		TestProcess(null,null);
		
	}

	public void f_assetNameTest()
	{
		current_condition = noAssetName;
		TestProcess(null,null);
		
		current_condition = invalidAssetName;
		LinkedHashMap<String,Integer> conditions = new LinkedHashMap<String,Integer> ();
		conditions.put("",isnull);
		conditions.put("#$%#",illegal);	
		conditions.put(APIUtil.getRandomString(21),overlimit);	
		conditions.put(APIUtil.getRandomString(1000),overlimit);	
		for (Map.Entry<String, Integer> entry : conditions.entrySet()) {  
			 
			current_condition_sub = entry.getValue();
			TestProcess(entry.getKey(),null);
		}  
		
		current_condition = validAssetName;
		String conditions1[]={
				APIUtil.getRandomString(19),
				APIUtil.getRandomString(20),				
		};
		for (int i =0 ;i<2;++i)
		{
			TestProcess(conditions1[i],null);
		}
	}

	public void g_assetUnitTest()
	{
		current_condition = noAssetUnit;
		TestProcess(null,null);
		
		current_condition = invalidAssetUnit;
		
		LinkedHashMap<String,Integer> conditions = new LinkedHashMap<String,Integer> ();
		conditions.put("",isnull);
		conditions.put("#$%#",illegal);	
		conditions.put(APIUtil.getRandomString(7),overlimit);	
		conditions.put(APIUtil.getRandomString(100),overlimit);	
		for (Map.Entry<String, Integer> entry : conditions.entrySet()) {  
			 
			current_condition_sub = entry.getValue();
			TestProcess(entry.getKey(),null);
		}  
		
		current_condition = validAssetUnit;
		String conditions1[]={
				"123456",
				"一二三四五六",		
				"正常|单位"
		};
		for (int i =0 ;i<3;++i)
		{
			TestProcess(conditions1[i],null);
		}
	}

	public void h_assetCountTest()
	{
		current_condition = noAssetCount;
		TestProcess(null,null);
		
		current_condition = invalidAssetCount;
		String conditions[]={
				"",
				"-1",
				"0",
				"65536"	,
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