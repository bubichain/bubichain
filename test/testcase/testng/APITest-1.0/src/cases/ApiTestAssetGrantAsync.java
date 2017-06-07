package cases;


import java.util.LinkedHashMap;
import java.util.Map;
import org.testng.annotations.Test;

import net.sf.json.JSONArray;
import net.sf.json.JSONObject;
import utils.APIUtil;
import utils.HttpUtil;

@Test
public class ApiTestAssetGrantAsync extends ApiGeneralCases {
	
	//definition of conditions	
	final static int noAssetCode		= 1<<31;
	final static int invalidAssetCode	= 1<<30;
	final static int validAssetCode		= 1<<29;
	   
	final static int noCurrentString	= 1<<28;
	final static int invalidCurrentString 	=1<<27;
	final static int validCurrentString 	=1<<26;
	                                  
	final static int noFromBubi			=1<<25;
	final static int invalidFromBubi		=1<<24;
	final static int validFromBubi			=1<<23;
	                                     
	final static int noToBubi			=1<<22;
	final static int invalidToBubi		=1<<21;
	final static int validToBubi		=1<<20;
	
	public static StringBuffer user_name1 = new StringBuffer("");  
	public static StringBuffer address1 = new StringBuffer("");  
	public static StringBuffer assetcode = new StringBuffer("");
	
	public static StringBuffer user_name2 = new StringBuffer("");  
	public static StringBuffer address2 = new StringBuffer("");  	
	
	public static StringBuffer user_name3 = new StringBuffer("");  
	public static StringBuffer address3 = new StringBuffer("");  	
	static String asset_unit = "元";
	static String asset_name = "某种资产";
	
	@Test(enabled = false)
	public void TestProcess(Object param, JSONArray detail)
	{
		access_token = getToken();
		
		//user 1 issue a kind of asset		
		createUser(access_token, user_name1,address1);
		issueAsset(access_token, user_name1.toString(),address1.toString(),assetcode,asset_name,asset_unit,null);
		
		//create user 2		
		createUser(access_token, user_name2,address2);
		
		//create user 3
		createUser(access_token, user_name3,address3);
		
		String api_path = "/asset/v1/grantAsync" ;
		//clean map
		parameter.clear();
		thrMap.clear();
		
		generalSetup(current_condition,noAccessToken,invalidAccessToken,"access_token",param,access_token,parameter);
		
		trade_no = Long.toString((tradeNoBase++)+(System.currentTimeMillis()<<8));
		generalSetup(current_condition,noTradeNo,invalidTradeNo,validTradeNo,"trade_no",param,trade_no,thrMap);
		trade_no =(String)thrMap.get("trade_no");
		thrMap.put("current_string","MqUq6TASQju57VcH");
		generalSetup(current_condition,noAssetCode,invalidAssetCode,"asset_code",param,assetcode.toString(),thrMap);
		
		generalSetup(current_condition,noAssetCount,invalidAssetCount,validAssetCount,"asset_amount",param,"20",thrMap);
		
		
		if(!APIUtil.getbool(current_condition&noFromBubi))
		{
			if(APIUtil.getbool(current_condition&invalidFromBubi))
			{
				if(param == "user2")
				{
					thrMap.put("from_bubi_address", address2.toString());						
				}
				else
				{
					thrMap.put("from_bubi_address", param);	
				}
			}
			else
				thrMap.put("from_bubi_address", address1.toString());	
		}
		
		if(!APIUtil.getbool(current_condition&noToBubi))
		{
			if(APIUtil.getbool(current_condition&invalidToBubi))
			{
				if(param == "user1")
				{
					thrMap.put("to_bubi_address", address1.toString());	
				}
				else
				{
					thrMap.put("to_bubi_address", param);
				}
			}					
			else
				thrMap.put("to_bubi_address", address2.toString());	
		}
		
		generalSetup(current_condition,noSign,invalidSign,"sign",param,APIUtil.sign(thrMap, client_secret),thrMap);
		generalSetup(current_condition,noPassword,invalidPassword,"password",param,user_name1.toString(),thrMap);
		
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
		System.out.println("=========Grant asset========");
		
		/*
		String url = api_url + api_path;
		
		do
		{
			if(parameter.size()==0)
				break;
			url=url+"?";
			for (Iterator it =  parameter.keySet().iterator();it.hasNext();)
		   {
		    Object key = it.next();
		    url+= key +"=" + parameter.get(key) + "&";  
		   }		
		
		url = url.substring(0,url.length()-1);//discard the last &
			
		}while (false);
		System.out.println( "posturl:"+url);
		
		System.out.println( "body:");
		if(jsonBody!=null)
		{
			 Iterator iterator = jsonBody.keys();
			 while(iterator.hasNext()){
				 String key = (String) iterator.next();
				 System.out.println("\""+key+"\""+":" +"\""+jsonBody.getString(key)+"\""+",");
			 }		      
		}*/
		String result = HttpUtil.dopostApi(api_url, api_path, parameter, jsonBody);
		JSONObject jsonObject =JSONObject.fromObject(result);  
		Verification(jsonObject);
		
		user_name1.delete(0, user_name1.length());
		user_name2.delete(0, user_name2.length());
		user_name3.delete(0, user_name3.length());
		address1.delete(0, address1.length());
		address2.delete(0, address2.length());
		address3.delete(0, address3.length());
		assetcode.delete(0, assetcode.length());
		
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
			case validFromBubi		:
			case validToBubi:
			case validSign:
			case validCurrentString :	

			case 0:
				check.equals(jsonObject.getString("err_code"), "0", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "资产正在发放", "msg is not as expected");
				check.equals(jsonObject.getJSONObject("data").getString("trade_no"), trade_no, "trade_no is not as expected");	
				Confirmation(true);
				
				break;
			case (int)invalidAssetCount    :
				if(current_condition_sub == isnull)
				{
					check.equals(jsonObject.getString("err_code"), "21307", "errorcode is not as expected");
					check.equals(jsonObject.getString("msg"), "资产转移数量不能为空", "msg is not as expected");
				
				}
				if(current_condition_sub == illegal)
				{
					check.equals(jsonObject.getString("err_code"), "21316", "errorcode is not as expected");
					check.equals(jsonObject.getString("msg"), "待资产转移数量必须是整数", "msg is not as expected");
				
				}
				if(current_condition_sub == overlimit)
				{
					check.equals(jsonObject.getString("err_code"), "21821", "errorcode is not as expected");
					check.equals(jsonObject.getString("msg"), "资产数量不足", "msg is not as expected");
				
				}
				if(current_condition_sub == isminus)
				{
					check.equals(jsonObject.getString("err_code"), "21316", "errorcode is not as expected");
					check.equals(jsonObject.getString("msg"), "待资产转移数量必须是整数", "msg is not as expected");
				
				}
				Confirmation(false);
				break;
			case (int)invalidAssetCode     :
				if(current_condition_sub == isnull)
				{
					check.equals(jsonObject.getString("err_code"), "21306/21807", "errorcode is not as expected");
					check.equals(jsonObject.getString("msg"), "资产代码不能为空/待转移资产代码不能为空", "msg is not as expected");
				
				}
				if(current_condition_sub == illegal)
				{
					check.equals(jsonObject.getString("err_code"), "21215", "errorcode is not as expected");
					check.equals(jsonObject.getString("msg"), "资产代码格式错误", "msg is not as expected");
				
				}	
				Confirmation(false);
				break;
				
			case (int)noAssetCount         :
				check.equals(jsonObject.getString("err_code"), "21307", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "资产转移数量不能为空", "msg is not as expected");
				Confirmation(false);
				break;
			case (int)noAssetCode          :
				check.equals(jsonObject.getString("err_code"), "21306", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "资产代码不能为空", "msg is not as expected");
				Confirmation(false);
				break;
		
			case (validAssetCount|invalidDetail):
				//if(current_condition_sub == overlimit)
				//{
					check.equals(jsonObject.getString("err_code"), "22708", "errorcode is not as expected");
					check.equals(jsonObject.getString("msg"), "资产的明细格式错误", "msg is not as expected");
					Confirmation(false);	
				//}				
				break;
	
	
			case noCurrentString:
				check.equals(jsonObject.getString("err_code"), "21300", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "随机字符串不能为空", "msg is not as expected");
				Confirmation(false);
				break;
			case invalidCurrentString: 
				if(current_condition_sub == overlimit)
				{
					check.equals(jsonObject.getString("err_code"), "21301", "errorcode is not as expected");
					check.equals(jsonObject.getString("msg"), "随机字符串长度不能大于32", "msg is not as expected");
				}
				else
				{
					check.equals(jsonObject.getString("err_code"), "21300", "errorcode is not as expected");
					check.equals(jsonObject.getString("msg"), "随机字符串不能为空", "msg is not as expected");
				}
				
				Confirmation(false);
			break;
		
		case noFromBubi		:	
			check.equals(jsonObject.getString("err_code"), "21311", "errorcode is not as expected");
			check.equals(jsonObject.getString("msg"), "资产转出方不存在", "msg is not as expected");
			Confirmation(false);
			break;
		case invalidFromBubi:
			check.equals(jsonObject.getString("err_code"), "21311", "errorcode is not as expected");
			check.equals(jsonObject.getString("msg"), "资产转出方不存在", "msg is not as expected");
			Confirmation(false);
			break;
		
		case noToBubi	  : 
			check.equals(jsonObject.getString("err_code"), "20319", "errorcode is not as expected");
			check.equals(jsonObject.getString("msg"), "资产转入方布比ID不能为空", "msg is not as expected");
			Confirmation(false);
			break;
			
		case invalidToBubi  :
			check.equals(jsonObject.getString("err_code"), "20319", "errorcode is not as expected");
			check.equals(jsonObject.getString("msg"), "资产转入方布比ID不能为空", "msg is not as expected");
			Confirmation(false);
			break;					
		case noSign	  : 
			check.equals(jsonObject.getString("err_code"), "21310", "errorcode is not as expected");
			check.equals(jsonObject.getString("msg"), "资产转移签名验证失败", "msg is not as expected");
			Confirmation(false);
			
			break;
	
		case invalidSign  :
			check.equals(jsonObject.getString("err_code"), "21310", "errorcode is not as expected");
			check.equals(jsonObject.getString("msg"), "资产转移签名验证失败", "msg is not as expected");
			Confirmation(false);
			break;

		case noPassword:	
		case invalidPassword:	
			check.equals(jsonObject.getString("err_code"), "20027", "errorcode is not as expected");
			check.equals(jsonObject.getString("msg"), "区块链账户与密码不匹配", "msg is not as expected");				
			Confirmation(false);
			break;
		default:
			break;
		}
		
	}
	@Test(enabled = false)
	public void Confirmation(boolean issuccess,String...param)
	{
		try   
		{   
			Thread.currentThread();
			Thread.sleep(10000);//毫秒   
		}  
		catch(Exception e){}  
		System.out.println("=========Confirm the result========");
		System.out.println("=========check grant status========");
		String result = checkStatus("/status/asset/v1/grant",access_token,trade_no,null);
		JSONObject jsonObject =JSONObject.fromObject(result);  
		if(issuccess)
		{
			check.equals(jsonObject.getString("err_code"), "0", "errorcode is not as expected");	
			check.equals(jsonObject.getString("msg"), "操作成功", "msg is not as expected");	
			String hash = jsonObject.getJSONObject("data").getString("bc_hash");
			check.equals(hash.length(), 64, "hash is not as expected");	
			check.equals(jsonObject.getJSONObject("data").getString("trade_no"), trade_no, "trade_no is not as expected");	
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

	public void a_generalTest()
	{
		a_normaltest();
		b_accessTokenTest();
		c_passwordTest();
		d_tradeNoTest();
		f_detailTest();
		g_signTest();		
	}

	public void d_assetCodeTest()
	{
		current_condition = noAssetCode;
		TestProcess(null,null);
		
		current_condition = invalidAssetCode;
		LinkedHashMap<String,Integer> conditions = new LinkedHashMap<String,Integer> ();
		conditions.put("",isnull);
		conditions.put("$#@$#@",illegal);
		conditions.put(APIUtil.getRandomString(100),illegal);
		conditions.put(APIUtil.getRandomString(1000),illegal);		
		
		for (Map.Entry<String, Integer> entry : conditions.entrySet()) {  
			 
			current_condition_sub = entry.getValue();
			TestProcess(entry.getKey(),null);
		}  
		
		current_condition = validAssetCode;
		TestProcess(null,null);
		
	}

	public void d_assetCountTest()
	{
		current_condition = noAssetCount;
		TestProcess(null,null);
		
		current_condition = invalidAssetCount;
		LinkedHashMap<String,Integer> conditions = new LinkedHashMap<String,Integer> ();
		conditions.put("",isnull);
		conditions.put("-1",isminus);
		conditions.put("0",illegal);
		conditions.put("#$%#",illegal);	
		conditions.put("数量",illegal);	
		conditions.put("3.1111",illegal);	
		conditions.put("101",overlimit);		
		
		for (Map.Entry<String, Integer> entry : conditions.entrySet()) {  
			 
			current_condition_sub = entry.getValue();
			TestProcess(entry.getKey(),null);
		}  
		
		current_condition = validAssetCount;
		String conditions1[]={
				"100",
				"99",		
				"1"
		};
		for (int i =0 ;i<3;++i)
		{
			TestProcess(conditions1[i],null);
		}
	}
	
	
	public void d_fromAddressTest()
	{
		addressTest(noFromBubi,invalidFromBubi);	
	}
	
	public void d_toAddressTest()
	{
		addressTest(noToBubi,invalidToBubi);		
	}
	
	public void d_currentStringTest()
	{
		current_condition = noCurrentString;
		TestProcess(null,null);
		
		current_condition = invalidCurrentString;
		LinkedHashMap<String,Integer> conditions = new LinkedHashMap<String,Integer> ();
		conditions.put("",isnull);
		conditions.put(APIUtil.getRandomString(33),overlimit);
		conditions.put(APIUtil.getRandomString(34),overlimit);
		
		for (Map.Entry<String, Integer> entry : conditions.entrySet()) {  
			 
			current_condition_sub = entry.getValue();
			TestProcess(entry.getKey(),null);
		}  
				
		current_condition = validCurrentString;
		String conditions1[]={
				APIUtil.getRandomString(32),
				APIUtil.getRandomString(31),	
				"$#@$#@$@#",				
		};
		for (int i =0 ;i<3;++i)
		{
			TestProcess(conditions1[i],null);
		}		
	}	
	

	
}