package cases;

import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Random;

import org.testng.annotations.Test;

import base.RecordOutput;
import base.TestBase;
import net.sf.json.JSONArray;
import net.sf.json.JSONObject;
import utils.APIUtil;
import utils.HttpUtil;


@Test
public class ApiTestRecord extends ApiGeneralCases {
	
	//definition of conditions
		
		final static int noRecorder			= 1<<30;
		final static int invalidRecorder	= 1<<29;
		final static int validRecorder		= 1<<28;
		
		final static int noInput			= 1<<27;
		final static int invalidInput		= 1<<26;
		final static int validInput			= 1<<25;
		
		final static int noData				= 1<<24;
		//final static int invalidData		= 1<<23;
		//final static int validData			= 1<<22;
		  
		final static int noOutput			= 1<<21;
		final static int invalidOutput		= 1<<20;
		final static int validOutput		= 1<<19;
		
		//Sub condition
		//input
		final static int nohash				= 1<<0;
		final static int invalidhash		= 1<<1;
		final static int validhash			= 1<<2;
		
		final static int noIndex			= 1<<3;
		final static int invalidIndex		= 1<<4;
		final static int validIndex			= 1<<5;
		
		final static int noInputMetadata			= 1<<6;
		final static int invalidInputMetadata		= 1<<7;
		final static int validInputMetadata			= 1<<8;
		//output
		final static int noAddress			= 1<<9;
		final static int invalidAddress		= 1<<10;
		final static int validAddress		= 1<<11;
				             
		final static int noOutputMetadata		= 1<<12;
		final static int invalidOutputMetadata	= 1<<13;
		final static int validOutputMetadata	= 1<<14;
		
		final static int noSid			= 1<<15;
		final static int invalidSid		= 1<<16;
		final static int validSid		= 1<<17;
				
		final static int twoAddress		= 1<<18;
		final static int threeAddress	= 1<<19;
		final static int oneAddress		= 1<<20;
		
		
		
		public static StringBuffer user_name1 = new StringBuffer("");  
		public static StringBuffer address1 = new StringBuffer("");  
				
		public static StringBuffer user_name2 = new StringBuffer("");  
		public static StringBuffer address2 = new StringBuffer("");  	
		
		public static StringBuffer user_name3 = new StringBuffer("");  
		public static StringBuffer address3 = new StringBuffer(""); 
		
		StringBuffer hash = new StringBuffer();
		
		public static String bc_hash = null;
		public static String metadata_input = null;
		public static String metadata_output = null;
		public static String source_address = null;
		
		@Test(enabled = false)	
		public void TestProcess(Object param, JSONArray ArrayParam)
		{
			access_token = getToken();		
					
			user_name1.delete(0, user_name1.length());
			user_name2.delete(0, user_name2.length());
			user_name3.delete(0, user_name3.length());
			address1.delete(0, address1.length());
			address2.delete(0, address2.length());
			address3.delete(0, address3.length());		
			hash.delete(0, hash.length());
			//user 1 
			createUser_v2(access_token, user_name1,address1);	
			
			
			ArrayList<RecordOutput> outputlist = new ArrayList<RecordOutput>();
			
			outputlist = Record(access_token,address1.toString(),user_name1.toString(),2,0,hash);
			
			//create user 2		
			createUser_v2(access_token, user_name2,address2);
			
			//create user 3
			createUser_v2(access_token, user_name3,address3);   

			//clean map
			parameter.clear();
			thrMap.clear();
			
			generalSetup(current_condition,noAccessToken,invalidAccessToken,"access_token",param,access_token,parameter);
			
			trade_no = Long.toString((tradeNoBase++)+(System.currentTimeMillis()<<8));
			generalSetup(current_condition,noTradeNo,invalidTradeNo,validTradeNo,"trade_no",param,trade_no,thrMap);
			trade_no = (String)thrMap.get("trade_no");
			generalSetup(current_condition,noRecorder,invalidRecorder,"recorder_address",param,address1.toString(),thrMap);
				
			
			if(!APIUtil.getbool(current_condition&noData))
			{
				JSONObject Data = new JSONObject();
				if(!APIUtil.getbool(current_condition&noInput))
				{
					if(APIUtil.getbool(current_condition&(invalidInput|validInput)))
					{
						JSONObject member = ArrayParam.getJSONObject(0);
						if (member.get("hash")=="default")
						{
							member.put("hash", hash.toString());
						}
						if (member.get("index")=="INDEX OVERLIMIT")
						{
							member.put("index", "8");
						}
						ArrayParam.clear();
						ArrayParam.add(member);
						Data.put("inputs", ArrayParam);
						
					}
					else 
					{
						//add a standard input
						JSONArray standardInputs = new JSONArray();
						JSONObject standardInput = new JSONObject();
						standardInput.put("hash", hash.toString());
						standardInput.put("index", "1");
						standardInput.put("metadata", "This is a standard input");
						standardInputs.add(standardInput);
						Data.put("inputs", standardInputs);
					}
				}
				if(!APIUtil.getbool(current_condition&noOutput))
				{		
					if(APIUtil.getbool(current_condition&(invalidOutput|validOutput)))
					{
						JSONObject member = ArrayParam.getJSONObject(0);
						if (member.get("address")=="default")
						{
							member.put("address", address1.toString());
						}
						if (member.get("sid")=="INDEX OVERLIMIT")
						{
							member.put("sid", "8");
						}
						ArrayParam.clear();
						ArrayParam.add(member);
						Data.put("outputs", ArrayParam);
					}
					else 
					{
						//add a standard output
						JSONArray standardOutputs = new JSONArray();
						JSONObject standardOutput = new JSONObject();
						standardOutput.put("address", address1.toString());
						standardOutput.put("sid", "1");
						standardOutput.put("metadata", "This is a standard output for sid 1");
						standardOutputs.add(standardOutput);
						standardOutput.put("address", address2.toString());
						standardOutput.put("sid", "2");
						standardOutput.put("metadata", "This is a standard output for sid 2");
						standardOutputs.add(standardOutput);
						Data.put("outputs",standardOutputs);						
					}
				}
					
				thrMap.put("data", Data);	
				
			}									
			JSONObject jsonBody = JSONObject.fromObject(thrMap);
			System.out.println("=========Record========");
			String api_path = "/info/v2/record" ;
			String result = HttpUtil.dopostApi(api_url, api_path, parameter, jsonBody);
			JSONObject jsonObject =JSONObject.fromObject(result);  
			Verification(jsonObject);		
			
		}
		
		private JSONArray inputLayout(String parameter)
		{
			JSONArray input = new JSONArray();
			JSONObject inputmember = new JSONObject();
			LinkedHashMap<String, Object> inputmember1 = new LinkedHashMap<String, Object>();
			generalSetup(current_condition_sub,nohash,invalidhash,"hash",parameter,"default",inputmember1);
			generalSetup(current_condition_sub,noIndex,invalidIndex,"index",parameter,"1",inputmember1);
			generalSetup(current_condition_sub,noInputMetadata,invalidInputMetadata,validInputMetadata,"metadata",parameter,"this is a metadata",inputmember1);
			inputmember = JSONObject.fromObject(inputmember1);
			input.add(inputmember);
			return input;
		}
		
		private JSONArray outputLayout(String parameter)
		{
			JSONArray output = new JSONArray();
			JSONObject outputmember = new JSONObject();
			LinkedHashMap<String, Object> outputmember1 = new LinkedHashMap<String, Object>();
			
			generalSetup(current_condition_sub,noAddress,invalidAddress,"address",parameter,"default",outputmember1);
			generalSetup(current_condition_sub,noSid,invalidSid,"sid",parameter,"1",outputmember1);
			generalSetup(current_condition_sub,noInputMetadata,invalidInputMetadata,validInputMetadata,"metadata",parameter,"this is a metadata",outputmember1);
			outputmember = JSONObject.fromObject(outputmember1);
			output.add(outputmember);
			return output;
		}
		private void Verification(JSONObject jsonObject)
		{
			verification(jsonObject);
			switch (current_condition)
			{
				case validAccessToken	:					     
				case validMetaData		:				
				case validTradeNo		:
				case invalidMetaData      :
				case noMetaData           :
				case validRecorder:
				case noInput:
				case 0:	
					check.equals(jsonObject.getString("err_code"), "0", "errorcode is not as expected");
					check.equals(jsonObject.getString("msg"), "操作成功", "msg is not as expected");
					bc_hash =(String) jsonObject.getJSONObject("data").get("hash");
					check.equals(bc_hash.length(),64,"hash is not as expected");
					Confirmation(true);
					break;
				case noRecorder:
					check.equals(jsonObject.getString("err_code"), "10001", "errorcode is not as expected");
					check.equals(jsonObject.getString("msg"), "信息录入者布比地址不能为空", "msg is not as expected");
					Confirmation(false);
					break;
				case invalidRecorder:
					if(current_condition_sub==isnull)
					{
						check.equals(jsonObject.getString("err_code"), "10001", "errorcode is not as expected");
						check.equals(jsonObject.getString("msg"), "信息录入者布比地址不能为空", "msg is not as expected");
									
					}
					else
					{
						check.equals(jsonObject.getString("err_code"), "20024", "errorcode is not as expected");
						check.equals(jsonObject.getString("msg"), "无效的布比地址", "msg is not as expected");						
					}
					
					break;
				case noData:
					check.equals(jsonObject.getString("err_code"), "未定义", "errorcode is not as expected");
					check.equals(jsonObject.getString("msg"), "没有data，文档未定义", "msg is not as expected");						
							
					break;
				case noOutput:
					check.equals(jsonObject.getString("err_code"), "未定义", "errorcode is not as expected");
					check.equals(jsonObject.getString("msg"), "没有output，文档未定义", "msg is not as expected");						
					
					break;
					//Confirmation(true);
				case invalidInput:
					if(current_condition_sub == nohash)
					{
						check.equals(jsonObject.getString("err_code"), "未定义", "errorcode is not as expected");
						check.equals(jsonObject.getString("msg"), "input没有hash，文档未定义", "msg is not as expected");						
						
					}
					else if(current_condition_sub == invalidhash)
					{
						check.equals(jsonObject.getString("err_code"), "未定义", "errorcode is not as expected");
						check.equals(jsonObject.getString("msg"), "input，hash不对，文档未定义", "msg is not as expected");						
						
					}
					else if(current_condition_sub == noIndex)
					{
						check.equals(jsonObject.getString("err_code"), "未定义", "errorcode is not as expected");
						check.equals(jsonObject.getString("msg"), "input没有index，文档未定义", "msg is not as expected");						
						
					}
					else if(current_condition_sub == invalidIndex)
					{
						check.equals(jsonObject.getString("err_code"), "未定义", "errorcode is not as expected");
						check.equals(jsonObject.getString("msg"), "input，index不对，文档未定义", "msg is not as expected");						
						
					}
					else if(current_condition_sub == noOutputMetadata)
					{
						check.equals(jsonObject.getString("err_code"), "0", "errorcode is not as expected");
						check.equals(jsonObject.getString("msg"), "操作成功", "msg is not as expected");
						bc_hash =(String) jsonObject.getJSONObject("data").get("hash");
						check.equals(bc_hash.length(),64,"hash is not as expected");
						Confirmation(true);
						
					}
					else if(current_condition_sub == invalidOutputMetadata)
					{
						check.equals(jsonObject.getString("err_code"), "0", "errorcode is not as expected");
						check.equals(jsonObject.getString("msg"), "操作成功", "msg is not as expected");
						bc_hash =(String) jsonObject.getJSONObject("data").get("hash");
						check.equals(bc_hash.length(),64,"hash is not as expected");
						Confirmation(true);
					}
					break;
				case invalidOutput:
					if(current_condition_sub == noAddress)
					{
						check.equals(jsonObject.getString("err_code"), "20031", "errorcode is not as expected");
						check.equals(jsonObject.getString("msg"), "账户不存在或账户未激活", "msg is not as expected");
						
					}
					else if(current_condition_sub == invalidAddress)
					{
						check.equals(jsonObject.getString("err_code"), "20031", "errorcode is not as expected");
						check.equals(jsonObject.getString("msg"), "账户不存在或账户未激活", "msg is not as expected");
						
					}
					else if(current_condition_sub == noSid)
					{
						check.equals(jsonObject.getString("err_code"), "未定义", "errorcode is not as expected");
						check.equals(jsonObject.getString("msg"), "缺少sid字段，文档未定义", "msg is not as expected");
						
					}
					else if(current_condition_sub == invalidSid)
					{
						check.equals(jsonObject.getString("err_code"), "未定义", "errorcode is not as expected");
						check.equals(jsonObject.getString("msg"), "sid字段错误，文档未定义", "msg is not as expected");
						
					}
					else if(current_condition_sub == noOutputMetadata)
					{
						check.equals(jsonObject.getString("err_code"), "0", "errorcode is not as expected");
						check.equals(jsonObject.getString("msg"), "操作成功", "msg is not as expected");
						bc_hash =(String) jsonObject.getJSONObject("data").get("hash");
						check.equals(bc_hash.length(),64,"hash is not as expected");
						Confirmation(true);
					}
					else if(current_condition_sub == invalidOutputMetadata)
					{
						check.equals(jsonObject.getString("err_code"), "0", "errorcode is not as expected");
						check.equals(jsonObject.getString("msg"), "操作成功", "msg is not as expected");
						bc_hash =(String) jsonObject.getJSONObject("data").get("hash");
						check.equals(bc_hash.length(),64,"hash is not as expected");
						Confirmation(true);
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
			System.out.println("=========check record status========");
			String result = checkStatus("/info/v1/get",access_token,null,bc_hash);
			JSONObject jsonObject =JSONObject.fromObject(result);  
			if(issuccess)
			{
				check.equals(jsonObject.getString("err_code"), "0", "errorcode is not as expected");	
				check.equals(jsonObject.getString("msg"), "success", "msg is not as expected");
				check.equals((String)jsonObject.getJSONObject("data").get("hash"),bc_hash,"hash is not as expected");
				//check.equals((String)jsonObject.getJSONObject("data").get("metadata"),metadata,"metadata is not as expected");
				//check.equals((String)jsonObject.getJSONObject("data").get("source_address"),source_address,"source address is not as expected");
				//String index1 = jsonObject.getJSONObject("data").getJSONArray("operations").getString(0);
				//jsonObject= JSONObject.fromObject(index1);
				//check.equals((String)jsonObject.get("type"),"4","type is not as expected");
				
			}
			else
				check.equals(jsonObject.getString("err_code"), "21216", "errorcode is not as expected");		
		}
		


		public void a_generalTest()
		{
			a_normaltest();
			//b_accessTokenTest();
			
			//d_tradeNoTest();
			
		}

		public void d_recorderTest()
		{
			current_condition = noRecorder;
			TestProcess(null,null);
			
			current_condition = invalidRecorder;
			LinkedHashMap<String,Integer> conditions = new LinkedHashMap<String,Integer> ();
			conditions.put("",isnull);
			conditions.put("$#@$#@",illegal);
			conditions.put(" "+APIUtil.getRandomString(4),illegal);
			
			
			for (Map.Entry<String, Integer> entry : conditions.entrySet()) {  
				 
				current_condition_sub = entry.getValue();
				TestProcess(entry.getKey(),null);
			}  		
			
		}

		public void d_DataTest()
		{
			current_condition = noData;
			TestProcess(null,null);
			current_condition = noInput;
			TestProcess(null,null);
			current_condition = noOutput;
			TestProcess(null,null);
		}

		public void d_InputTest()
		{
					
			current_condition = invalidInput;
			
			current_condition_sub = nohash;			
			{
				
				TestProcess(null,inputLayout(null));	
			}
			
			current_condition_sub = invalidhash;
			String conditions[]={
					"",
					"wronghash",
					"other hash"				
			};
			
			for (int i = 0;i<3;i++)
			{
				TestProcess(null,inputLayout(conditions[i]));		
			}
		
			
			current_condition_sub = noIndex;			
			{
				TestProcess(null,inputLayout(null));	
			}
			
			current_condition_sub = invalidIndex;
			String conditions1[]={
					"",
					"wrongINDEX",
					"INDEX OVERLIMIT"				
			};
			
			for (int i = 0;i<3;i++)
			{
				TestProcess(null,inputLayout(conditions1[i]));			
			}
			current_condition_sub = noInputMetadata;			
			{
				TestProcess(null,inputLayout(null));		
			}
			
			current_condition_sub = invalidInputMetadata;
			String conditions2[]={
					"",							
			};
			
			for (int i = 0;i<1;i++)
			{
				TestProcess(null,inputLayout(conditions2[i]));			
			}					
			
		}
	
		public void d_OutTest()
		{
			current_condition = invalidOutput;
			
			current_condition_sub = noAddress;			
			{
				TestProcess(null,outputLayout(null));	
			}
			
			current_condition_sub = invalidAddress;
			LinkedHashMap<String,Integer> conditions = new LinkedHashMap<String,Integer> ();
			conditions.put("",isnull);
			conditions.put("-1",isminus);
			conditions.put("0",illegal);
			conditions.put("#$%#",illegal);	
			conditions.put("数量",illegal);	
			conditions.put(APIUtil.getRandomString(100),illegal);	
			conditions.put("user2",illegal);
				
			for (Map.Entry<String, Integer> entry : conditions.entrySet()) {  
				 
				TestProcess(null,outputLayout(entry.getKey()));				
			}  
			
			
			current_condition_sub = noSid;			
			{
				
				TestProcess(null,outputLayout(null));	
			}
			
			current_condition_sub = invalidSid;
			String conditions1[]={
					"",
					"-1",
					"这是sid",
					"999999999999999999999999999999",
					"3.1415"					
			};
			
			for (int i = 0;i<5;i++)
			{
				TestProcess(null,outputLayout(conditions1[i]));		
			}	
			
			current_condition_sub = noOutputMetadata;			
			{
				TestProcess(null,outputLayout(null));	
			}
			
			current_condition_sub = invalidOutputMetadata;
			String conditions2[]={
					"",							
			};
			
			for (int i = 0;i<1;i++)
			{
				TestProcess(null,outputLayout(conditions2[i]));
			}				
			
		}
		
}
