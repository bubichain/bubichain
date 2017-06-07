package cases;

import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Random;

import org.testng.annotations.Test;

import base.TestBase;
import net.sf.json.JSONArray;
import net.sf.json.JSONObject;
import utils.APIUtil;
import utils.HttpUtil;

public class ApiTestCreateEvidence extends ApiGeneralCases {
	
	//definition of conditions
		
		final static int noSigners		= 1<<4;
		final static int invalidSigners	= 1<<5;
		final static int validSigners		= 1<<6;
		             
		
		final static int oneaddress = 1<<10;
		final static int threeaddress = 1<<11;
		final static int wrongaddress = 1<<12;
		final static int wrongpassword = 1<<13;
		final static int sameaddress = 1<<14;
		final static int nosigners = 1<<15;
		final static int normalsign = 1<<16;
		
	
		public static StringBuffer user_name1 = new StringBuffer("");  
		public static StringBuffer address1 = new StringBuffer("");  
				
		public static StringBuffer user_name2 = new StringBuffer("");  
		public static StringBuffer address2 = new StringBuffer("");  	
		
		public static StringBuffer user_name3 = new StringBuffer("");  
		public static StringBuffer address3 = new StringBuffer(""); 
		
		public static String bc_hash = null;
		public static String metadata = null;
		public static String source_address = null;
		

		private void TestCreateEvidence(Object param, JSONArray sign)
		{
			
			String api_path = "/evidence/v2/create" ;
			parameter.clear();	
			
			if(!APIUtil.getbool(current_condition&noAccessToken))
			{
				if(APIUtil.getbool(current_condition&invalidAccessToken))
					parameter.put("access_token",(String)param);
				else
					parameter.put("access_token",access_token);
			}
			
			if(!APIUtil.getbool(current_condition&noMetaData))
			{
				if(APIUtil.getbool(current_condition&invalidMetaData))
				{
					metadata = (String)param;
					thrMap.put("metadata",metadata);
				}
				else
				{
					metadata = "这是一条存证信息";
					thrMap.put("metadata",metadata);
				}
				
			}
			
			if(!APIUtil.getbool(current_condition&noTradeNo))
			{
				//need test limits
				if(APIUtil.getbool(current_condition&(invalidTradeNo|validTradeNo)))
				{
					thrMap.put("trade_no", (String)param);	
					trade_no = (String)param;
				}
					
				else//put a standard one
				{
					trade_no = Long.toString((tradeNoBase++)+(System.currentTimeMillis()<<8));
					thrMap.put("trade_no", trade_no);							
				}
					
			}	 
			if(sign!=null)
			{
				thrMap.put("signers", sign);	
			}
			JSONObject jsonBody = JSONObject.fromObject(thrMap);
			System.out.println("=========Create Evidence========");
			
			String result = HttpUtil.dopostApi(api_url, api_path, parameter, jsonBody);
			JSONObject jsonObject =JSONObject.fromObject(result);  
			Verification(jsonObject);
			
			user_name1.delete(0, user_name1.length());
			user_name2.delete(0, user_name2.length());
			user_name3.delete(0, user_name3.length());
			address1.delete(0, address1.length());
			address2.delete(0, address2.length());
			address3.delete(0, address3.length());		
			
		}
		
		
		private void Verification(JSONObject jsonObject)
		{
			String evidence;
			switch (current_condition)
			{
				case validAccessToken	:	
				     
				case validMetaData		:
				
				case validTradeNo		:
				case invalidMetaData      :
				case noMetaData           :					
				case 0:									
					if(APIUtil.getbool(current_condition_sub &(normalsign|oneaddress|threeaddress)))
					{
						check.equals(jsonObject.getString("err_code"), "0", "errorcode is not as expected");
						check.equals(jsonObject.getString("msg"), "操作成功", "msg is not as expected");
						bc_hash =(String) jsonObject.getJSONObject("data").get("bc_hash");
						check.equals(bc_hash.length(),64,"hash is not as expected");
						evidence =(String) jsonObject.getJSONObject("data").get("evidence_id");
						check.equals(evidence.length(),40,"evidence is not as expected");					
						Confirmation(true);
					}
				
					else if(APIUtil.getbool(current_condition_sub &(wrongaddress|wrongpassword)))
					{
						check.equals(jsonObject.getString("err_code"), "21103", "errorcode is not as expected");
						check.equals(jsonObject.getString("msg"), "用户名与密码不匹配", "msg is not as expected");
					}
					
					else if(current_condition_sub == sameaddress)
					{
						check.equals(jsonObject.getString("err_code"), "文档未定义", "errorcode is not as expected");
						check.equals(jsonObject.getString("msg"), "两个地址一样的存证错误，文档未定义", "msg is not as expected");						
					}
					else if(current_condition_sub == nosigners)
					{
						check.equals(jsonObject.getString("err_code"), "文档未定义", "errorcode is not as expected");
						check.equals(jsonObject.getString("msg"), "没有signer的情况，文档未定义", "msg is not as expected");						
										
					}
					break;								
			
				default:
					break;
			}
		}
		//the format of get evidence info is not clear, temporarily comment out.
		private void Confirmation(boolean issuccess)
		{
			System.out.println("=========Confirm the result========");
			System.out.println("=========check evidence status========");
			String result = checkStatus("/evidence/v1/info",access_token,null,bc_hash);
			JSONObject jsonObject =JSONObject.fromObject(result);  
			if(issuccess)
			{
				check.equals(jsonObject.getString("err_code"), "0", "errorcode is not as expected");	
				check.equals(jsonObject.getString("msg"), "操作成功", "msg is not as expected");
				check.equals((String)jsonObject.getJSONObject("data").get("hash"),bc_hash,"hash is not as expected");
				check.equals((String)jsonObject.getJSONObject("data").get("metadata"),metadata,"metadata is not as expected");
				check.equals((String)jsonObject.getJSONObject("data").get("source_address"),source_address,"source address is not as expected");
				String index1 = jsonObject.getJSONObject("data").getJSONArray("operations").getString(0);
				jsonObject= JSONObject.fromObject(index1);
				check.equals((String)jsonObject.get("type"),"4","type is not as expected");
				
			}
			else
				check.equals(jsonObject.getString("err_code"), "21216", "errorcode is not as expected");		
		}
		
		
		private JSONArray SignerLayout()
		{
			access_token = getToken();		
			StringBuffer assetcode = new StringBuffer();
			
			//user 1 issue a kind of asset		
			createUser(access_token, user_name1,address1);
			
			//create user 2		
			createUser(access_token, user_name2,address2);
			
			//create user 3
			createUser(access_token, user_name3,address3);
			
			//to be written to block
			issueAsset(access_token, user_name1.toString(),address1.toString(),assetcode,"测试","ge","meta");
			issueAsset(access_token, user_name2.toString(),address2.toString(),assetcode,"测试","ge","meta");
			issueAsset(access_token, user_name3.toString(),address3.toString(),assetcode,"测试","ge","meta");
			
			//sign
			JSONArray signers = new JSONArray();
			JSONObject singlesigner = new JSONObject();
			
				
			switch(current_condition_sub)
			{
				case normalsign:
					singlesigner.put("bubi_address", address1.toString());				
					singlesigner.put("password" ,user_name1.toString());	
					source_address = address1.toString();
					signers.add(singlesigner);
					singlesigner.put("bubi_address", address2.toString());				
					singlesigner.put("password" ,user_name2.toString());					
					signers.add(singlesigner);
					break;
				case oneaddress:
					singlesigner.put("bubi_address", address1.toString());				
					singlesigner.put("password" ,user_name1.toString());	
					source_address = address1.toString();
					signers.add(singlesigner);
					break;
				case threeaddress:
					singlesigner.put("bubi_address", address1.toString());				
					singlesigner.put("password" ,user_name1.toString());
					source_address = address1.toString();
					signers.add(singlesigner);
					singlesigner.put("bubi_address", address2.toString());				
					singlesigner.put("password" ,user_name2.toString());					
					signers.add(singlesigner);
					singlesigner.put("bubi_address", address3.toString());				
					singlesigner.put("password" ,user_name3.toString());					
					signers.add(singlesigner);
					break;
				case wrongaddress :
					singlesigner.put("bubi_address", "abcdefg");				
					singlesigner.put("password" ,user_name1.toString());					
					signers.add(singlesigner);
					singlesigner.put("bubi_address", address2.toString());				
					singlesigner.put("password" ,user_name2.toString());					
					signers.add(singlesigner);
					break;
				case wrongpassword:
					singlesigner.put("bubi_address", address1.toString());				
					singlesigner.put("password" ,"abcde");					
					signers.add(singlesigner);
					singlesigner.put("bubi_address", address2.toString());				
					singlesigner.put("password" ,user_name2.toString());					
					signers.add(singlesigner);
					break;
				case sameaddress:
					singlesigner.put("bubi_address", address1.toString());				
					singlesigner.put("password" ,user_name1.toString());					
					signers.add(singlesigner);
					signers.add(singlesigner);
					break;
				case nosigners:
					break;
			
			}
					
			return signers;
		}
		@Test

		public void a_normaltest()
		{
			current_condition = 0;
			current_condition_sub = normalsign;
			TestCreateEvidence(null,SignerLayout());
		}

		public void b_invalidsigner()
		{
			current_condition = 0;
			current_condition_sub = oneaddress;
			TestCreateEvidence(null,SignerLayout());
			current_condition_sub = threeaddress;
			TestCreateEvidence(null,SignerLayout());
			current_condition_sub = wrongaddress;
			TestCreateEvidence(null,SignerLayout());
			current_condition_sub = wrongpassword;
			TestCreateEvidence(null,SignerLayout());
			current_condition_sub = sameaddress;
			TestCreateEvidence(null,SignerLayout());
			
		}
		
}
