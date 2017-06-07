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
@Test
public class ApiTestModifyEvidence extends ApiGeneralCases {
	
	//definition of conditions
		final static int noTradeNo			= 1<<1;
		final static int invalidTradeNo		= 1<<2;
		final static int validTradeNo		= 1<<3;
			         
		final static int noSigners		= 1<<4;
		final static int invalidSigners	= 1<<5;
		final static int validSigners		= 1<<6;
		
		final static int noEvidenceId			= 1<<7;
		final static int invalidEvidenceId		= 1<<8;
		final static int validEvidenceId		= 1<<9;
		             
		final static int invalidAccessToken	= 1<<22;
		final static int noAccessToken	    = 1<<23;
		final static int validAccessToken	= 1<<24;
		             
		final static int noMetaData			= 1<<25;
		final static int invalidMetaData	= 1<<26;
		final static int validMetaData		= 1<<27;
		
		//definition of sub conditions
		final static int overlimit = 1<<0;
		final static int specialchar = 1<<1;
		final static int floatnumber = 1<<2;
		final static int isnull		= 1<<3;
		final static int isminus	 = 1<<4;
		final static int illegal	=1<<5;
		final static int notexist	=1<<6;
		final static int iszero		=1<<7;
		final static int isChinese  =1<<8;
		final static int underlimit = 1<<9;
	
		//when modify with the same users with create
		final static int normalsign = 1<<16;
		final static int oneaddress = 1<<10;
		final static int threeaddress = 1<<11;
				
		//when modify with the different users with create		
		final static int sameaddress = 1<<14;
		final static int nosigners = 1<<15;
		final static int wrongaddress = 1<<12;
		final static int wrongpassword = 1<<13;
		final static int signernumber_diff = 1<<17;
		
		//when modify with metadata conditions
		final static int nullmeta = 1<<24;
		final static int longmeta = 1<<25;
		final static int specialmeta = 1<<26;		

		static Random rand = new Random();
		static int tradeNoBase = rand.nextInt(255);
		static String trade_no = new String();
		
		public static StringBuffer user_name1 = new StringBuffer("");  
		public static StringBuffer address1 = new StringBuffer("");  
				
		public static StringBuffer user_name2 = new StringBuffer("");  
		public static StringBuffer address2 = new StringBuffer("");  	
		
		public static StringBuffer user_name3 = new StringBuffer("");  
		public static StringBuffer address3 = new StringBuffer(""); 
		
		public static StringBuffer evidence_id = new StringBuffer("");
		public static StringBuffer bchash = new StringBuffer("");
		
		
		private int current_condition = 0;
		private int current_condition_sub = 0;
		
		public static String bc_hash = null;
		public static String metadata = null;
		public static String source_address = null;	
		//public static String evidence = null;
		
		
		private void TestModifyEvidence(Object param, JSONArray sign,int signernum)
		{
			
			JSONArray signers = new JSONArray();
			
			JSONObject singlesigner = new JSONObject();
						
			if(signernum >0)
			{
				singlesigner.put("bubi_address", address1.toString());				
				singlesigner.put("password" ,user_name1.toString());					
				signers.add(singlesigner);
				if(signernum>1)
				{
					singlesigner.put("bubi_address", address2.toString());				
					singlesigner.put("password" ,user_name2.toString());					
					signers.add(singlesigner);
					if(signernum>2)
					{
						singlesigner.put("bubi_address", address3.toString());				
						singlesigner.put("password" ,user_name3.toString());					
						signers.add(singlesigner);						
					}
				}
				
			}
					
			
			CreateEvidence(access_token,signers,"create an evidence",evidence_id,bchash);
			
			String api_path = "/evidence/v1/modify" ;
		
			
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
					metadata = "this is a metadata";
					thrMap.put("metadata",metadata);
				}
				
			}
			
			if(!APIUtil.getbool(current_condition&noEvidenceId))
			{
				if(APIUtil.getbool(current_condition&invalidEvidenceId))
					thrMap.put("evidence_id",(String)param);
				else
					thrMap.put("evidence_id",evidence_id.toString());
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
			System.out.println("=========Modify Evidence========");
			
			String result = HttpUtil.dopostApi(api_url, api_path, parameter, jsonBody);
			JSONObject jsonObject =JSONObject.fromObject(result);  
			Verification(jsonObject);
			
			user_name1.delete(0, user_name1.length());
			user_name2.delete(0, user_name2.length());
			user_name3.delete(0, user_name3.length());
			address1.delete(0, address1.length());
			address2.delete(0, address2.length());
			address3.delete(0, address3.length());
			evidence_id.delete(0, evidence_id.length());
			
		}
		
		
		private void Verification(JSONObject jsonObject)
		{
			
			switch (current_condition)
			{
				case validAccessToken	:					     
				case validMetaData		:				
				case validTradeNo		:
				case invalidMetaData      :
				case noMetaData           :
				case validEvidenceId	:
					check.equals(jsonObject.getString("err_code"), "0", "errorcode is not as expected");
					check.equals(jsonObject.getString("msg"), "操作成功", "msg is not as expected");
					bc_hash =(String) jsonObject.getJSONObject("data").get("bc_hash");
					check.equals(bc_hash.length(),64,"hash is not as expected");
				case 0:
					if(APIUtil.getbool(current_condition_sub &(normalsign|oneaddress|threeaddress)))
					{
						check.equals(jsonObject.getString("err_code"), "0", "errorcode is not as expected");
						check.equals(jsonObject.getString("msg"), "操作成功", "msg is not as expected");
						bc_hash =(String) jsonObject.getJSONObject("data").get("bc_hash");
						check.equals(bc_hash.length(),64,"hash is not as expected");
						
						//Confirmation(true);						
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
					else if(current_condition_sub == signernumber_diff)
					{
						check.equals(jsonObject.getString("err_code"), "文档未定义", "errorcode is not as expected");
						check.equals(jsonObject.getString("msg"), "和create时signer的数量不同的情况", "msg is not as expected");						
												
					}
					break;							
						
				case noTradeNo            :
					check.equals(jsonObject.getString("err_code"), "21200", "errorcode is not as expected");
					check.equals(jsonObject.getString("msg"), "资产发行凭据号不能为空", "msg is not as expected");
					
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
					break;
				
					
				case invalidAccessToken		:
				case noAccessToken		:
					check.equals(jsonObject.getString("err_code"), "20015", "errorcode is not as expected");
					check.equals(jsonObject.getString("msg"), "授权码不存在或已过期", "msg is not as expected");
					
					break;
				case invalidEvidenceId:
					check.equals(jsonObject.getString("err_code"), "20015", "errorcode is not as expected");
					check.equals(jsonObject.getString("msg"), "授权码不存在或已过期", "msg is not as expected");
					
					break;
				default:
					break;
			}
		}
		
		
		private JSONArray SignerLayout()
		{
			access_token = getToken();		
			
			//user 1 issue a kind of asset		
			createUser(access_token, user_name1,address1);
			
			//create user 2		
			createUser(access_token, user_name2,address2);
			
			//create user 3
			createUser(access_token, user_name3,address3);
			
			//sign
			JSONArray signers = new JSONArray();
			JSONObject singlesigner = new JSONObject();
			
				
			switch(current_condition_sub)
			{
				case normalsign:
					singlesigner.put("bubi_address", address1.toString());				
					singlesigner.put("password" ,user_name1.toString());					
					signers.add(singlesigner);
					singlesigner.put("bubi_address", address2.toString());				
					singlesigner.put("password" ,user_name2.toString());					
					signers.add(singlesigner);
					break;
				case oneaddress:
					singlesigner.put("bubi_address", address1.toString());				
					singlesigner.put("password" ,user_name1.toString());					
					signers.add(singlesigner);
					break;
				case threeaddress:
					singlesigner.put("bubi_address", address1.toString());				
					singlesigner.put("password" ,user_name1.toString());					
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
				case signernumber_diff:
					singlesigner.put("bubi_address", address1.toString());				
					singlesigner.put("password" ,user_name1.toString());					
					signers.add(singlesigner);
					singlesigner.put("bubi_address", address2.toString());				
					singlesigner.put("password" ,user_name2.toString());					
					signers.add(singlesigner);
					break;
			
			}
					
			return signers;
		}
		

		public void a_normaltest()
		{
			current_condition = 0;
			current_condition_sub = normalsign;
			TestModifyEvidence(null,SignerLayout(),2);
		}

		public void b_invalidsigner()
		{
			current_condition = 0;
			current_condition_sub = oneaddress;
			TestModifyEvidence(null,SignerLayout(),1);
			current_condition_sub = threeaddress;
			TestModifyEvidence(null,SignerLayout(),3);
			current_condition_sub = wrongaddress;
			TestModifyEvidence(null,SignerLayout(),2);
			current_condition_sub = wrongpassword;
			TestModifyEvidence(null,SignerLayout(),2);
			current_condition_sub = sameaddress;
			TestModifyEvidence(null,SignerLayout(),2);
			current_condition_sub = nosigners;
			TestModifyEvidence(null,SignerLayout(),2);
			current_condition_sub = signernumber_diff;
			TestModifyEvidence(null,SignerLayout(),1);
			TestModifyEvidence(null,SignerLayout(),3);
			
			
		}
		
}
