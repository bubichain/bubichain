package base;

import java.lang.reflect.Method;
import java.security.KeyManagementException;
import java.security.NoSuchAlgorithmException;
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
	

	static String testjson = jr.ReadFile("./BMPublicityInterfaceTest.json");	
	
	public static String interface_url = jr.parseString(testjson,"interface_url");//api-url
	public static String api_url = jr.parseString(testjson,"api_url");//api-url
	public static String zhongtuobang_id = jr.parseString(testjson,"zhongtuobang_id");//api-url
	public static String zhongtuobang_key = jr.parseString(testjson,"zhongtuobang_secret");//api-url

	public static String yangguang_id = jr.parseString(testjson,"yangguang_id");//api-url
	public static String yangguang_key = jr.parseString(testjson,"yangguang_secret");//api-url
	
	public static String gege_id = jr.parseString(testjson,"gege_id");
	public static String gege_secret = jr.parseString(testjson,"gege_secret");
	public static String qianxiang_id = jr.parseString(testjson,"qianxiang_id");
	public static String qianxiang_secret = jr.parseString(testjson,"qianxiang_secret");
	public static String banma_id = jr.parseString(testjson,"banma_id");
	public static String banma_secret = jr.parseString(testjson,"banma_secret");
	public static String haohuo_id = jr.parseString(testjson,"haohuo_id");
	public static String haohuo_secret = jr.parseString(testjson,"haohuo_secret");
	public static String renren_id = jr.parseString(testjson,"renren_id");
	public static String renren_secret = jr.parseString(testjson,"renren_secret");
	public static String huaan_id = jr.parseString(testjson,"huaan_id");
	public static String huaan_secret = jr.parseString(testjson,"huaan_secret");
	public static String madianzhang_id = jr.parseString(testjson,"madianzhang_id");
	public static String madianzhang_secret = jr.parseString(testjson,"madianzhang_secret");
	public static String jinxiang_id = jr.parseString(testjson,"jinxiang_id");
	public static String jinxiang_secret = jr.parseString(testjson,"jinxiang_secret");
	
	public static String asset_decoder = jr.parseString(testjson, "decoder_url");
	public static String asset_encoder = jr.parseString(testjson, "encoder_url");

	
	public LinkedHashMap<String, String> companyname = new LinkedHashMap<String,String>();
	
	
	
	
	
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
		companyname.put(gege_id,"格格翔云");
		companyname.put(qianxiang_id,"钱香-倾信");
		companyname.put(banma_id,"斑马-青云");
		companyname.put(zhongtuobang_id,"众托-仲托");
		companyname.put(renren_id,"人人-九一");
		companyname.put(huaan_id,"华安");
		companyname.put(madianzhang_id,"马-八斗");
		companyname.put(jinxiang_id,"金香-崇浩");
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
	public String getToken(String id, String secret)
	{
		return this.getToken(api_url,id,secret);
	}
	
	public String getToken(String root_path,String client_id,String client_key) 
	{
		System.out.println("=========Get token========");
		String api_path = "/oauth2/token" ;
		LinkedHashMap<String, String> parameter = new LinkedHashMap<String,String>();
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
		LinkedHashMap<String, String> parameter = new LinkedHashMap<String,String>();
		
		Map<String, Object> thrMap = new LinkedHashMap<String, Object>();
		
		parameter.put("access_token",Token);
		
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
	public static String issueAsset(String access_token, String user_name,String address,StringBuffer assetcode,String asset_name,String asset_unit,String amount,JSONArray... detail)
	{
		System.out.println("=========Issue asset========");
		String api_path = "/asset/v1/issue" ;
		LinkedHashMap<String, String> parameter = new LinkedHashMap<String,String>();
		
		Map<String, Object> thrMap = new LinkedHashMap<String, Object>();
		
		parameter.put("access_token",access_token);
		
		Random rand = new Random();
		int tradeNoBase = rand.nextInt(10000);
		thrMap.put("asset_issuer",address );		
		thrMap.put("password", user_name);	
		thrMap.put("trade_no", (tradeNoBase++)+(System.currentTimeMillis()<<8));	
		thrMap.put("asset_name",asset_name);
		thrMap.put("asset_unit",asset_unit);
		thrMap.put("asset_amount",amount);
		thrMap.put("metadata", "444");	
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
	
	/*
	 String api_path = "/asset/v1/add2Issue" ;
		
		//clean map
		parameter.clear();
		thrMap.clear();
		
		isparameter = true;
		generalSetup(noAccessToken,invalidAccessToken,"access_token",param,access_token);
		
		isparameter = false;		
		
		trade_no = Long.toString((tradeNoBase++)+(System.currentTimeMillis()<<8));
		generalSetup(noTradeNo,invalidTradeNo,validTradeNo,"trade_no",param,trade_no);
		trade_no = (String)thrMap.get("trade_no");
		
		generalSetup(noAssetCode,invalidAssetCode,"asset_code",param,assetcode.toString());
		generalSetup(noPassword,invalidPassword,"password",param,user_name.toString());
		generalSetup(noAssetCount,invalidAssetCount,validAssetCount,"asset_amount",param,"1000");
		
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
	*/
	public static String addToIssue(String access_token, String user_name,String assetcode,String amount,JSONArray... detail)
	{
		System.out.println("=========add to issue========");
		String api_path = "/asset/v1/add2Issue" ;
		LinkedHashMap<String, String> parameter = new LinkedHashMap<String,String>();
		
		Map<String, Object> thrMap = new LinkedHashMap<String, Object>();
		
		parameter.put("access_token",access_token);
		
		Random rand = new Random();
		int tradeNoBase = rand.nextInt(10000);
		
		thrMap.put("password", user_name);	
		thrMap.put("trade_no", (tradeNoBase++)+(System.currentTimeMillis()<<8));	
		thrMap.put("asset_code",assetcode);
		thrMap.put("asset_amount",amount);
		thrMap.put("metadata", "444");	
		if(detail.length!= 0)
		{
			if(detail[0]!=null)
			{
				thrMap.put("details", detail[0]);	
			}			
		}	
		
		JSONObject jsonBody = JSONObject.fromObject(thrMap);
		
		String result = HttpUtil.dopostApi(api_url, api_path, parameter, jsonBody);
		
		return result;
		
	}
	
	public void issueAndSendMultiAsset(String testurl,String access_token, String user_name,String address,String asset_name,String asset_unit,int howmany,JSONArray... detail)
	{
		System.out.println("=========Issue multiple asset========");
		String api_path = "/asset/v1/issue" ;
		LinkedHashMap<String, String> parameter = new LinkedHashMap<String,String>();
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
	
	
	public static String sendAsset(Boolean isAsync,String access_token, String from_address,String password,String to_address,String assetcode,String amount,String secret)
	{
		System.out.println("=========Send asset========");
		String api_path = new String();
		if(isAsync)
			api_path = "/asset/v1/sendAsync" ;
		else
			api_path = "/asset/v1/send" ;
		LinkedHashMap<String, String> parameter = new LinkedHashMap<String,String>();
		
		Map<String, Object> thrMap = new LinkedHashMap<String, Object>();
		
		parameter.put("access_token",access_token);
		
		thrMap.put("current_string", APIUtil.getRandomString(32));
		Random rand = new Random();
		int tradeNoBase = rand.nextInt(10000);
		long trade1 = ((tradeNoBase++)+(System.currentTimeMillis()<<8));
		thrMap.put("trade_no", Long.toString(trade1));		
		thrMap.put("asset_code", assetcode);	
		thrMap.put("asset_amount", amount);
			
		thrMap.put("from_bubi_address", from_address);	
		thrMap.put("to_bubi_address", to_address);	
		String sign1 = APIUtil.sign(thrMap, secret);
		thrMap.put("sign", sign1);	
		thrMap.put("password", password);	
		thrMap.put("metadata", "444");	
	
		JSONObject jsonBody = JSONObject.fromObject(thrMap);
	
		String result = HttpUtil.dopostApi(api_url, api_path, parameter, jsonBody);
		return result;
		
		
	}
	
	
	
	public static String grantAsset(String access_token, String from_address,String user_name,String to_address,String assetcode,String amount,String secret)
	{
		
		System.out.println("=========Grant asset========");
		
		String api_path = "/asset/v1/grant" ;
		LinkedHashMap<String, String> parameter = new LinkedHashMap<String,String>();
		
		Map<String, Object> thrMap = new LinkedHashMap<String, Object>();
		
		parameter.put("access_token",access_token);
		
		thrMap.put("current_string", APIUtil.getRandomString(32));
		Random rand = new Random();
		int tradeNoBase = rand.nextInt(10000);
		long trade1 = ((tradeNoBase++)+(System.currentTimeMillis()<<8));
		thrMap.put("trade_no", Long.toString(trade1));		
		thrMap.put("asset_code", assetcode);	
		thrMap.put("asset_amount", amount);
			
		thrMap.put("from_bubi_address", from_address);	
		thrMap.put("to_bubi_address", to_address);	
		String sign1 = APIUtil.sign(thrMap, secret);
		thrMap.put("sign", sign1);	
		thrMap.put("password", user_name);	
		thrMap.put("metadata", "444");	
	
		JSONObject jsonBody = JSONObject.fromObject(thrMap);
	
		String result = HttpUtil.dopostApi(api_url, api_path, parameter, jsonBody);
		return result;
		
		
	}
	public static String add2Issue(String access_token, String user_name,String assetcode,String amount)
	{
		
		System.out.println("=========Add 2 issue========");
		String api_path = "/asset/v1/add2Issue" ;
		LinkedHashMap<String, String> parameter = new LinkedHashMap<String,String>();
	
		Map<String, Object> thrMap = new LinkedHashMap<String, Object>();
	
		parameter.put("access_token",access_token);
		Random rand = new Random();
		int tradeNoBase = rand.nextInt(10000);
		long trade1 = ((tradeNoBase++)+(System.currentTimeMillis()<<8));
		thrMap.put("trade_no", Long.toString(trade1));		
		thrMap.put("asset_code", assetcode.toString());	
		thrMap.put("password", user_name.toString());	
		thrMap.put("asset_amount", amount);
		
		thrMap.put("metadata", "444");	
		JSONObject jsonBody = JSONObject.fromObject(thrMap);
		
		String result = HttpUtil.dopostApi(api_url, api_path, parameter, jsonBody);
		return result;
	}
	
	public static String changePassword(String access_token, String user_name,String newpw)
	{
		String api_path = "/account/v1/alterPwd" ;
		LinkedHashMap<String, String> parameter = new LinkedHashMap<String,String>();
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
		LinkedHashMap<String, String> parameter = new LinkedHashMap<String,String>();
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
	
	
	public static void batchSendAsset(String access_token, String address,String password,String assetname,String key,String issueamount,String singleamount,String batchnum) 
	{
		String api_path = "/asset/v1/issue" ;
		LinkedHashMap<String, String> parameter = new LinkedHashMap<String,String>();
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
			//thrMap.put("asset_amount",Integer.toString(count*4));
			thrMap.put("asset_amount",issueamount);
			thrMap.put("metadata", "444");	
					
			
			JSONObject jsonBody = JSONObject.fromObject(thrMap);
			
			String result = HttpUtil.dopostApi(api_url, api_path, parameter, jsonBody);
			
			JSONObject jsonObject =JSONObject.fromObject(result); 
			if (jsonObject.getString("err_code").equals("0"))
			{
				assetcode=(String)jsonObject.getJSONObject("data").get("asset_code");			
			}	
			
			JSONArray senders = new JSONArray();
			
			long tradeno = (tradeNoBase++)+(System.currentTimeMillis()<<8)+Integer.parseInt(batchnum);
			
			for(int i = 0; i<Integer.parseInt(batchnum); i++)
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
				
				result = HttpUtil.dopostApi(api_url, api_path, parameter, jsonBody);
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
				thrMap_single.put("asset_amount" , singleamount);				
				thrMap_single.put("current_string" , "1467345122019");
				String sign1 = APIUtil.sign(thrMap_single, key);
				thrMap_single.put("type" , "asset.send");
				thrMap_single.put("password", password);	
				thrMap_single.put("sign", sign1);	
				
				jsonBody = JSONObject.fromObject(thrMap_single);
				senders.add(jsonBody);		
				
			}
			System.out.println("=========blend========");
			api_path = "/blend";
			Map<String, Object> thrMap_blend = new LinkedHashMap<String, Object>();
			thrMap_blend.put("trade_no", Long.toString(tradeno));
			thrMap_blend.put("opts", senders);
			jsonBody = JSONObject.fromObject(thrMap_blend);
			result = HttpUtil.dopostApi(api_url, api_path, parameter, jsonBody);		
		
	}
	
	public static String CreateEvidence(String access_token, JSONArray signers,String metadata, StringBuffer evidence,StringBuffer bchash)
	{
		
		System.out.println("=========create evidence========");
		
		String api_path = "/evidence/v1/create" ;
		LinkedHashMap<String, String> parameter = new LinkedHashMap<String,String>();
		
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
		LinkedHashMap<String, String> parameter = new LinkedHashMap<String,String>();
		
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
	
	//assetcode decoder
	public static String assetDecoder(String encodedAsset,StringBuffer decodedAsset, StringBuffer issuer)
	{
		
		System.out.println("=========Decode asset code========");
		
		LinkedHashMap<String, String> parameter = new LinkedHashMap<String,String>();
		
		parameter.put("request",encodedAsset);

		String result = HttpUtil.dogetApi(asset_decoder, "", parameter);
		
		JSONObject jsonObject =JSONObject.fromObject(result); 
		if (jsonObject.getString("err_code").equals("0"))
		{
			decodedAsset.append(jsonObject.getJSONObject("data").get("assetCode"));	
			issuer.append(jsonObject.getJSONObject("data").get("issuerAddr"));
		}	
		return result;		
	}
	
	//assetcode encoder
	public static String assetEncoder(String assetcode,String issuer,StringBuffer encodedAsset)
	{
		
		System.out.println("=========Encode asset code========");
		
		LinkedHashMap<String, String> parameter = new LinkedHashMap<String,String>();
		
		parameter.put("request.assetCode",assetcode);
		parameter.put("request.issuerAddr",issuer);

		String result = HttpUtil.dogetApi(asset_encoder, "", parameter);
		
		JSONObject jsonObject =JSONObject.fromObject(result); 
		if (jsonObject.getString("err_code").equals("0"))
		{
			encodedAsset.append(jsonObject.getJSONObject("data"));				
		}	
		return result;		
	}
	
}
