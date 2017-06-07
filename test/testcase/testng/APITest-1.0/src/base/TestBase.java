package base;

import java.lang.reflect.Method;
import java.security.KeyManagementException;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Random;

import org.testng.annotations.AfterSuite;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.BeforeMethod;
import org.testng.annotations.BeforeSuite;

import listener.ExtentManager;
import net.sf.json.JSONArray;
import net.sf.json.JSONObject;
import utils.APIUtil;
import utils.DateHandler;
import utils.HttpUtil;

import com.relevantcodes.extentreports.ExtentReports;
import com.relevantcodes.extentreports.ExtentTest;

import utils.JsonReader;

public class TestBase {
	 
	 
	static JsonReader jr = new JsonReader();
	public static String access_token = null;
	
	//static String testjson = jr.ReadFile("./APITest.json");
	//static String testjson = jr.ReadFile("./APITest_84.json");
	//static String testjson = jr.ReadFile("./APITest_207.json");
	//static String testjson = jr.ReadFile("./APITest_214.json");
	//static String testjson = jr.ReadFile("./APITest_Beta.json");
	static String testjson = jr.ReadFile("./APITest_formal.json");

	public static String api_url = jr.parseString(testjson,"urlapi");//api-url
	public static String client_id = jr.parseString(testjson,"client_id");//api-url
	public static String client_secret = jr.parseString(testjson,"client_secret");//api-url
	

	public static int timeout = 25;
	
	public CheckPoint check = new CheckPoint();
	private static ExtentReports extentReports;
	protected ExtentTest test;
	private static String reportPath = "report/" + DateHandler.getTimeStamp()
			+ ".html";
	@BeforeMethod
	public void testBegin(Method method) {
		// System.out.println("ledger: " + ledger);
		Log.info("测试方法： " + method.getName() + "开始执行~~~~~");
		// System.out.println(method.getName());
	}

	@BeforeClass
	public void classBegin() {
		
		Log.info(this.getClass().getCanonicalName() + "开始执行~~~~~");
		// System.out.println(this.getClass().getCanonicalName());
	}

	private static String reportLocation = "report/ExtentReport.html";

	// protected static ExtentReports extent;

	@BeforeSuite
	public void beforeSuite() {
		extentReports = ExtentManager.getReporter(reportLocation);
	}
	// //
	@AfterSuite
	protected void afterSuite() {
		// extentReports.close();
	}

	public static ExtentReports getExtent() {
		return extentReports;
	}

	public static String getReportPath() {
		return reportPath;
	}
	//for api test
	public String getToken()
	{
		return this.getToken(api_url,client_id,client_secret);
	}
	
	public String getToken(String root_path,String client_id,String client_key) 
	{
		System.out.println("=========Get token========");
		String api_path = "/oauth2/token" ;
		LinkedHashMap<String, Object> parameter = new LinkedHashMap<String,Object>();
		parameter.put("client_id",client_id);
		parameter.put("client_secret",client_key);
		parameter.put("grant_type","client_credentials");
		String result = HttpUtil.dopostApi(root_path, api_path, parameter, null);
		JSONObject jsonObject =JSONObject.fromObject(result);  
		return jsonObject.getString("access_token");		
	}
	
	public static String createUser(String Token,StringBuffer name,StringBuffer address)
	{

		System.out.println("=========Create user========");
		String api_path = "/account/v1/register" ;
		LinkedHashMap<String, Object> parameter = new LinkedHashMap<String,Object>();
		
		Map<String, Object> thrMap = new LinkedHashMap<String, Object>();
		
		parameter.put("access_token",access_token);
		
		String s = APIUtil.getRandomString(10);
		
		Random rand = new Random();
		int tradeNoBase = rand.nextInt(10000);
		thrMap.put("user_name", s);		
		thrMap.put("password", s);	
		thrMap.put("trade_no", (tradeNoBase++)+(System.currentTimeMillis()<<8));	
		thrMap.put("metadata", "444");	
		
		JSONObject jsonBody = JSONObject.fromObject(thrMap);
		
		String result = HttpUtil.dopostApi(api_url, api_path, parameter, jsonBody);
		JSONObject jsonObject =JSONObject.fromObject(result); 
		if (jsonObject.getString("err_code").equals("0"))
		{
			name.append(s);
			address.append(jsonObject.getJSONObject("data").get("bubi_address"));
			
		}	
		return result;
		
	}
	
	public static String createUser_v2(String Token,StringBuffer name,StringBuffer address)
	{

		System.out.println("=========Create user v2========");
		String api_path = "/account/v2/register" ;
		LinkedHashMap<String, Object> parameter = new LinkedHashMap<String,Object>();
		
		Map<String, Object> thrMap = new LinkedHashMap<String, Object>();
		
		parameter.put("access_token",access_token);
		
		String s = APIUtil.getRandomString(10);
		
		Random rand = new Random();
		int tradeNoBase = rand.nextInt(10000);
		thrMap.put("user_name", s);		
		thrMap.put("password", s);	
		thrMap.put("trade_no", (tradeNoBase++)+(System.currentTimeMillis()<<8));	
		thrMap.put("metadata", "444");	
		
		JSONObject jsonBody = JSONObject.fromObject(thrMap);
		
		String result = HttpUtil.dopostApi(api_url, api_path, parameter, jsonBody);
		JSONObject jsonObject =JSONObject.fromObject(result); 
		if (jsonObject.getString("err_code").equals("0"))
		{
			name.append(s);
			address.append(jsonObject.getJSONObject("data").get("bubi_address"));
			
		}	
		return result;
		
	}
	public static String issueAsset(String access_token, String user_name,String address,StringBuffer assetcode,String asset_name,String asset_unit,String metadata,JSONArray... detail)
	{
		System.out.println("=========Issue asset========");
		String api_path = "/asset/v1/issue" ;
		LinkedHashMap<String, Object> parameter = new LinkedHashMap<String,Object>();
		
		Map<String, Object> thrMap = new LinkedHashMap<String, Object>();
		
		parameter.put("access_token",access_token);
		
		Random rand = new Random();
		int tradeNoBase = rand.nextInt(10000);
		thrMap.put("asset_issuer",address );		
		thrMap.put("password", user_name);	
		thrMap.put("trade_no", (tradeNoBase++)+(System.currentTimeMillis()<<8));	
		thrMap.put("asset_name",asset_name);
		thrMap.put("asset_unit",asset_unit);
		thrMap.put("asset_amount","100");
		thrMap.put("metadata", metadata);	
		if(detail.length!= 0)
		{
			if(detail[0]!=null)
			{
				thrMap.put("details", detail[0]);	
			}			
		}
		
		
		JSONObject jsonBody = JSONObject.fromObject(thrMap);
		
		String result = HttpUtil.dopostApi(api_url, api_path, parameter, jsonBody);
		JSONObject jsonObject =JSONObject.fromObject(result); 
		if (jsonObject.getString("err_code").equals("0"))
		{
			assetcode.append(jsonObject.getJSONObject("data").get("asset_code"));			
		}	
		return result;		
	}
	
	public void issueAndSendMultiAsset(String testurl,String access_token, String user_name,String address,String asset_name,String asset_unit,int howmany,JSONArray... detail)
	{
		System.out.println("=========Issue multiple asset========");
		String api_path = "/asset/v1/issue" ;
		LinkedHashMap<String, Object> parameter = new LinkedHashMap<String,Object>();
		String to_address = new String();
		
		String assetcode = new String();
		parameter.put("access_token",access_token);
		
		
		Random rand = new Random();
		int tradeNoBase = rand.nextInt(10000);
		
		for(int i = 0;i<howmany;i++)
		{
			api_path = "/asset/v1/issue" ;
			Map<String, Object> thrMap = new LinkedHashMap<String, Object>();
			
			thrMap.put("asset_issuer",address );		
			thrMap.put("password", user_name);	
				
			thrMap.put("trade_no", (tradeNoBase++)+(System.currentTimeMillis()<<8));	
			thrMap.put("asset_name",asset_name+"-"+rand.nextInt(99999));
			thrMap.put("asset_unit",rand.nextInt(30000));
			thrMap.put("asset_amount",Integer.toString(rand.nextInt(3000000)+3000000));
			//thrMap.put("asset_amount","222");
			thrMap.put("metadata", "444");	
			if(detail.length!= 0)
			{
				if(detail[0]!=null)
				{
					thrMap.put("details", detail[0]);	
				}			
			}
			
			
			JSONObject jsonBody = JSONObject.fromObject(thrMap);
			
			String result = HttpUtil.dopostApi(testurl, api_path, parameter, jsonBody);
			
			JSONObject jsonObject =JSONObject.fromObject(result); 
			if (jsonObject.getString("err_code").equals("0"))
			{
				assetcode=(String)jsonObject.getJSONObject("data").get("asset_code");			
			}	
			
			System.out.println("=========Create user========");
			api_path = "/account/v1/register" ;
		
			Map<String, Object> thrMap1 = new LinkedHashMap<String, Object>();
			
			parameter.put("access_token",access_token);
			
			String s = APIUtil.getRandomString(10);
			
			tradeNoBase = rand.nextInt(10000);
			thrMap1.put("user_name", s);		
			thrMap1.put("password", s);	
			thrMap1.put("trade_no", (tradeNoBase++)+(System.currentTimeMillis()<<8));	
			thrMap1.put("metadata", "444");	
			
			jsonBody = JSONObject.fromObject(thrMap1);
			
			result = HttpUtil.dopostApi(testurl, api_path, parameter, jsonBody);
			jsonObject =JSONObject.fromObject(result); 
			if (jsonObject.getString("err_code").equals("0"))
			{
				to_address = (String)jsonObject.getJSONObject("data").get("bubi_address");
				
			}	
			//send
			api_path = "/asset/v1/send" ;
			
			Map<String, Object> thrMap2 = new LinkedHashMap<String, Object>();
			thrMap2.put("current_string", APIUtil.getRandomString(32));
			
			long trade1 = ((tradeNoBase++)+(System.currentTimeMillis()<<8));
			thrMap2.put("trade_no", Long.toString(trade1));		
			thrMap2.put("asset_code", assetcode);	
			thrMap2.put("asset_amount", Integer.toString(rand.nextInt(2900000)));
			//thrMap2.put("asset_amount", "222");
				
			thrMap2.put("from_bubi_address", address);	
			thrMap2.put("to_bubi_address", to_address);	
			String sign1 = APIUtil.sign(thrMap2, "d12ed7f0a30bebb157bdc81770fd2201");
			thrMap2.put("sign", sign1);	
			thrMap2.put("password", user_name);	
			thrMap2.put("metadata", "444");	
		
			jsonBody = JSONObject.fromObject(thrMap2);
		
			result = HttpUtil.dopostApi(testurl, api_path, parameter, jsonBody);
			
		}
		
		
	}
	
	
	public static String sendAsset(Boolean isAsync,String access_token, String from_address,String user_name,String to_address,String assetcode,int amount)
	{
		System.out.println("=========Send asset========");
		String api_path = new String();
		if(isAsync)
			api_path = "/asset/v1/sendAsync" ;
		else
			api_path = "/asset/v1/send" ;
		LinkedHashMap<String, Object> parameter = new LinkedHashMap<String,Object>();
		
		Map<String, Object> thrMap = new LinkedHashMap<String, Object>();
		
		parameter.put("access_token",access_token);
		
		thrMap.put("current_string", APIUtil.getRandomString(32));
		Random rand = new Random();
		int tradeNoBase = rand.nextInt(10000);
		long trade1 = ((tradeNoBase++)+(System.currentTimeMillis()<<8));
		thrMap.put("trade_no", Long.toString(trade1));		
		thrMap.put("asset_code", assetcode);	
		thrMap.put("asset_amount", Integer.toString(amount));
			
		thrMap.put("from_bubi_address", from_address);	
		thrMap.put("to_bubi_address", to_address);	
		String sign1 = APIUtil.sign(thrMap, client_secret);
		thrMap.put("sign", sign1);	
		thrMap.put("password", user_name);	
		thrMap.put("metadata", "444");	
	
		JSONObject jsonBody = JSONObject.fromObject(thrMap);
	
		String result = HttpUtil.dopostApi(api_url, api_path, parameter, jsonBody);
		return result;
		
		
	}
	
	
	
	public static String grantAsset(String access_token, String from_address,String user_name,String to_address,String assetcode,int amount)
	{
		
		System.out.println("=========Grant asset========");
		
		String api_path = "/asset/v1/grant" ;
		LinkedHashMap<String, Object> parameter = new LinkedHashMap<String,Object>();
		
		Map<String, Object> thrMap = new LinkedHashMap<String, Object>();
		
		parameter.put("access_token",access_token);
		
		thrMap.put("current_string", APIUtil.getRandomString(32));
		Random rand = new Random();
		int tradeNoBase = rand.nextInt(10000);
		long trade1 = ((tradeNoBase++)+(System.currentTimeMillis()<<8));
		thrMap.put("trade_no", Long.toString(trade1));		
		thrMap.put("asset_code", assetcode);	
		thrMap.put("asset_amount", Integer.toString(amount));
			
		thrMap.put("from_bubi_address", from_address);	
		thrMap.put("to_bubi_address", to_address);	
		String sign1 = APIUtil.sign(thrMap, client_secret);
		thrMap.put("sign", sign1);	
		thrMap.put("password", user_name);	
		thrMap.put("metadata", "444");	
	
		JSONObject jsonBody = JSONObject.fromObject(thrMap);
	
		String result = HttpUtil.dopostApi(api_url, api_path, parameter, jsonBody);
		return result;
		
		
	}
	public static String add2Issue(String access_token, String user_name,String assetcode,int amount)
	{
		
		System.out.println("=========Add 2 issue========");
		String api_path = "/asset/v1/add2Issue" ;
		LinkedHashMap<String, Object> parameter = new LinkedHashMap<String,Object>();
	
		Map<String, Object> thrMap = new LinkedHashMap<String, Object>();
	
		parameter.put("access_token",access_token);
		Random rand = new Random();
		int tradeNoBase = rand.nextInt(10000);
		long trade1 = ((tradeNoBase++)+(System.currentTimeMillis()<<8));
		thrMap.put("trade_no", Long.toString(trade1));		
		thrMap.put("asset_code", assetcode.toString());	
		thrMap.put("password", user_name.toString());	
		thrMap.put("asset_amount", Integer.toString(amount));
		
		thrMap.put("metadata", "444");	
		JSONObject jsonBody = JSONObject.fromObject(thrMap);
		
		String result = HttpUtil.dopostApi(api_url, api_path, parameter, jsonBody);
		return result;
	}
	
	public static String changePassword(String access_token, String user_name,String newpw)
	{
		String api_path = "/account/v1/alterPwd" ;
		LinkedHashMap<String, Object> parameter = new LinkedHashMap<String,Object>();
		Map<String, Object> thrMap = new LinkedHashMap<String, Object>();
		
		parameter.put("access_token",access_token);
		thrMap.put("user_name", user_name);		
		thrMap.put("new_password", newpw);	
		Random rand = new Random();
		int tradeNoBase = rand.nextInt(10000);
		long trade1 = ((tradeNoBase++)+(System.currentTimeMillis()<<8));
		thrMap.put("trade_no", Long.toString(trade1));	
		JSONObject jsonBody = JSONObject.fromObject(thrMap);	
		String result = HttpUtil.dopostApi(api_url, api_path, parameter, jsonBody);	
		return result;
	}
	
	public static String checkStatus(String api_path,String access_token, String trade_no,String bc_hash)
	{
		LinkedHashMap<String, Object> parameter = new LinkedHashMap<String,Object>();
		if(trade_no != null)
		{
			parameter.put("trade_no",trade_no);
		}
		if(bc_hash!= null)
		{
			parameter.put("hash",bc_hash);
		}		
		parameter.put("access_token",access_token);		
		String result = HttpUtil.dogetApi(api_url, api_path, parameter);
		return result;
	}
	
	
	public static void batchSendAsset(String url_path, String access_token, String address,String password,String assetname,String key,int count) throws KeyManagementException, NoSuchAlgorithmException
	{
		String api_path = "/asset/v1/issue" ;
		LinkedHashMap<String, Object> parameter = new LinkedHashMap<String,Object>();
		String to_address = new String();
		
		String assetcode = new String();
		parameter.put("access_token",access_token);
		
		
		Random rand = new Random();
		int tradeNoBase = rand.nextInt(10000);
		
		api_path = "/asset/v1/issue" ;
			Map<String, Object> thrMap = new LinkedHashMap<String, Object>();
			
			thrMap.put("asset_issuer",address );		
			thrMap.put("password", password);	
				
			thrMap.put("trade_no", (tradeNoBase++)+(System.currentTimeMillis()<<8));	
			thrMap.put("asset_name",assetname+"-"+rand.nextInt(99999));
			thrMap.put("asset_unit",rand.nextInt(30000));
			//thrMap.put("asset_amount",Integer.toString(rand.nextInt(3000000)+3000000));
			thrMap.put("asset_amount",Integer.toString(count*4));
			thrMap.put("metadata", "444");	
					
			
			JSONObject jsonBody = JSONObject.fromObject(thrMap);
			
			String result = HttpUtil.dopostApi(url_path, api_path, parameter, jsonBody);
			
			JSONObject jsonObject =JSONObject.fromObject(result); 
			if (jsonObject.getString("err_code").equals("0"))
			{
				assetcode=(String)jsonObject.getJSONObject("data").get("asset_code");			
			}	
			
			JSONArray senders = new JSONArray();
			
			long tradeno = (tradeNoBase++)+(System.currentTimeMillis()<<8)+count;
			
			for(int i = 0; i<count; i++)
			{
				//create a user
				System.out.println("=========Create user========");
				api_path = "/account/v1/register" ;
			
				Map<String, Object> thrMap1 = new LinkedHashMap<String, Object>();
				
				parameter.put("access_token",access_token);
				
				String s = APIUtil.getRandomString(10);
				
				tradeNoBase = rand.nextInt(10000);
				thrMap1.put("user_name", s);		
				thrMap1.put("password", s);	
				
				thrMap1.put("trade_no", (tradeNoBase++)+(System.currentTimeMillis()<<8));	
				thrMap1.put("metadata", "444");	
				
				jsonBody = JSONObject.fromObject(thrMap1);
				
				result = HttpUtil.dopostApi(url_path, api_path, parameter, jsonBody);
				jsonObject =JSONObject.fromObject(result); 
				if (jsonObject.getString("err_code").equals("0"))
				{
					to_address = (String)jsonObject.getJSONObject("data").get("bubi_address");
					
				}	
				//add to batch send
				Map<String, Object> thrMap_single = new LinkedHashMap<String, Object>();
				
				thrMap_single.put("trade_no", Long.toString(tradeno));
				thrMap_single.put("from_bubi_address", address);
				thrMap_single.put("asset_code", assetcode);
				thrMap_single.put("to_bubi_address" , to_address);
				thrMap_single.put("asset_amount" , "2");				
				thrMap_single.put("current_string" , "1467345122019");
				String sign1 = APIUtil.sign(thrMap_single, key);
				thrMap_single.put("type" , "asset.send");
				thrMap_single.put("sign", sign1);	
				thrMap_single.put("password", password);	
				jsonBody = JSONObject.fromObject(thrMap_single);
				senders.add(jsonBody);		
				
			}
			System.out.println("=========blend========");
			api_path = "/blend";
			Map<String, Object> thrMap_blend = new LinkedHashMap<String, Object>();
			thrMap_blend.put("trade_no", Long.toString(tradeno));
			thrMap_blend.put("opts", senders);
			jsonBody = JSONObject.fromObject(thrMap_blend);
			result = HttpUtil.dopostApi(url_path, api_path, parameter, jsonBody);		
		
	}
	
	public static String CreateEvidence(String access_token, JSONArray signers,String metadata, StringBuffer evidence,StringBuffer bchash)
	{
		
		System.out.println("=========create evidence========");
		
		
		evidence.delete(0, evidence.length());
		bchash.delete(0, bchash.length());
		String api_path = "/evidence/v1/create" ;
		LinkedHashMap<String, Object> parameter = new LinkedHashMap<String,Object>();
		
		Map<String, Object> thrMap = new LinkedHashMap<String, Object>();
		
		parameter.put("access_token",access_token);
		
		Random rand = new Random();
		int tradeNoBase = rand.nextInt(10000);
		long trade1 = ((tradeNoBase++)+(System.currentTimeMillis()<<8));
		thrMap.put("trade_no", Long.toString(trade1));		
					
		thrMap.put("signers", signers);	
		thrMap.put("metadata", metadata);	
	
		JSONObject jsonBody = JSONObject.fromObject(thrMap);
	
		String result = HttpUtil.dopostApi(api_url, api_path, parameter, jsonBody);
		
		JSONObject jsonObject =JSONObject.fromObject(result); 
		if (jsonObject.getString("err_code").equals("0"))
		{
			evidence.append(jsonObject.getJSONObject("data").get("evidence_id"));	
			bchash.append(jsonObject.getJSONObject("data").get("bc_hash"));	
		}	
		return result;
		
		
		
	}
	
	public static String ModifyEvidence(String access_token, JSONArray signers,String metadata, String evidence,StringBuffer bchash)
	{
		
		System.out.println("=========modify evidence========");
		
		String api_path = "/evidence/v1/modify" ;
		LinkedHashMap<String, Object> parameter = new LinkedHashMap<String,Object>();
		
		Map<String, Object> thrMap = new LinkedHashMap<String, Object>();
		
		parameter.put("access_token",access_token);
		
		Random rand = new Random();
		int tradeNoBase = rand.nextInt(10000);
		long trade1 = ((tradeNoBase++)+(System.currentTimeMillis()<<8));
		thrMap.put("trade_no", Long.toString(trade1));		
					
		thrMap.put("signers", signers);	
		thrMap.put("evidence_id", evidence);	
		thrMap.put("metadata", metadata);	
	
		JSONObject jsonBody = JSONObject.fromObject(thrMap);
	
		String result = HttpUtil.dopostApi(api_url, api_path, parameter, jsonBody);
		
		JSONObject jsonObject =JSONObject.fromObject(result); 
		if (jsonObject.getString("err_code").equals("0"))
		{
			bchash.append(jsonObject.getJSONObject("data").get("bc_hash"));	
		}	
		return result;
		
	}
	
	public static ArrayList<RecordOutput> Record(String access_token,String recorder ,String password,int output_num,int layers,StringBuffer hash)
	{
		System.out.println("=========First Record========");
		
		String api_path = "/info/v2/record" ;
		
		//to return a map about output
		ArrayList<RecordOutput> ret = new ArrayList<RecordOutput>();
		
		{
			LinkedHashMap<String, Object> parameter = new LinkedHashMap<String,Object>();
			
			Map<String, Object> thrMap = new LinkedHashMap<String, Object>();
			
			parameter.put("access_token",access_token);
			
			Random rand = new Random();
			int tradeNoBase = rand.nextInt(10000);
			long trade1 = ((tradeNoBase++)+(System.currentTimeMillis()<<8));
			thrMap.put("trade_no", Long.toString(trade1));		
						
			thrMap.put("recorder_address", recorder);	
			
			thrMap.put("password", password);
				
			JSONArray outputs = new JSONArray();
			
			for (int i = 0; i<output_num;i++)
			{
				StringBuffer name = new StringBuffer();
				StringBuffer address = new StringBuffer();
				createUser_v2(access_token, name, address);
				
				RecordOutput recordOutput = new RecordOutput();
				recordOutput.address = address.toString();
				recordOutput.sid = Integer.toString(i+1);
				recordOutput.metadata =  "this is the output for address:"+address.toString()+" sid:"+Integer.toString(i+1);
				ret.add(recordOutput);
				
				//format output
				JSONObject output = new JSONObject();
				output.put("address", recordOutput.address);
				output.put("sid", recordOutput.sid);
				output.put("metadata", recordOutput.metadata);
				outputs.add(output);
				
			}
			JSONObject data = new JSONObject();
			data.put("outputs", outputs);
			thrMap.put("data", data);	
			JSONObject jsonBody = JSONObject.fromObject(thrMap);
		
			String result = HttpUtil.dopostApi(api_url, api_path, parameter, jsonBody);
			
			JSONObject jsonObject =JSONObject.fromObject(result); 
			if (jsonObject.getString("err_code").equals("0"))
			{
				hash.append(jsonObject.getJSONObject("data").get("hash"));	
			}	
		}
		for (int i = 0;i<layers;i++)
		{
			ret.clear();
			
			System.out.println(("=========Record time ")+i+("========"));
			
			LinkedHashMap<String, Object> parameter = new LinkedHashMap<String,Object>();
			
			Map<String, Object> thrMap = new LinkedHashMap<String, Object>();
			
			parameter.put("access_token",access_token);
			
			Random rand = new Random();
			int tradeNoBase = rand.nextInt(10000);
			long trade1 = ((tradeNoBase++)+(System.currentTimeMillis()<<8));
			thrMap.put("trade_no", Long.toString(trade1));		
						
			thrMap.put("recorder_address", recorder);	
			
			thrMap.put("password", password);
			
			
				
			JSONArray inputs = new JSONArray();
			JSONArray outputs = new JSONArray();
			
			for (int j = 0; j<output_num;j++)
			{
				RecordInput recordInput = new RecordInput();
				recordInput.hash = hash.toString();
				recordInput.index = Integer.toString(j+1);
				recordInput.metadata =  "this is the input of hash:"+hash.toString()+" index:"+Integer.toString(j+1);
							
				//format input
				JSONObject input = new JSONObject();
				input.put("hash", recordInput.hash);
				input.put("index", recordInput.index);
				//input.put("metadata", recordInput.metadata);
				input.put("metadata", "");
				inputs.add(input);				
			}
			
			for (int j = 0; j<output_num;j++)
			{
				StringBuffer name = new StringBuffer();
				StringBuffer address = new StringBuffer();
				createUser_v2(access_token, name, address);
				
				RecordOutput recordOutput = new RecordOutput();
				recordOutput.address = address.toString();
				recordOutput.sid = Integer.toString(j+1);
				recordOutput.metadata =  "this is the output for address:"+address.toString()+" sid:"+Integer.toString(j+1);
				ret.add(recordOutput);
				
				//format output
				JSONObject output = new JSONObject();
				output.put("address", recordOutput.address);
				output.put("sid", recordOutput.sid);
				//output.put("metadata", recordOutput.metadata);
				output.put("metadata", "");
				outputs.add(output);
				
			}
			JSONObject data = new JSONObject();
			data.put("inputs", inputs);
			data.put("outputs", outputs);
			thrMap.put("data", data);	
			JSONObject jsonBody = JSONObject.fromObject(thrMap);
		
			String result = HttpUtil.dopostApi(api_url, api_path, parameter, jsonBody);
			
			JSONObject jsonObject =JSONObject.fromObject(result); 
			if (jsonObject.getString("err_code").equals("0"))
			{
				hash.delete(0, hash.length());
				hash.append(jsonObject.getJSONObject("data").get("hash"));	
			}	
			
			
		}
		return ret;
		
	}
	
	public static String metadataFormat(String asset_type,String asset_unit_code, String annualized_rate,String asset_description)
	
	{
		String ret = "{";
		
		if(asset_type!=null)
		{
			ret += "\"asset_type\":"+"\""+asset_type+"\",";
		}
		if(asset_unit_code!=null)
		{
			ret += "\"asset_unit_code\":"+"\""+asset_unit_code+"\",";
		}
		if(annualized_rate!=null)
		{
			ret += "\"annualized_rate\":"+"\""+annualized_rate+"\",";
		}
		if(asset_description!=null)
		{
			ret += "\"asset_description\":"+"\""+asset_description+"\",";
		}
		if(ret.charAt(ret.length()-1) == ',')
		{
			ret = ret.substring(0,ret.length()-1);
		}
		ret+="}";
		return ret;
	}	
	
	
	
}
