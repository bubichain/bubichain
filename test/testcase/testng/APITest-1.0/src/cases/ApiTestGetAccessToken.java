package cases;

import java.util.LinkedHashMap;
import java.util.Map;

import org.testng.annotations.Test;
import net.sf.json.JSONObject;
import utils.APIUtil;
import utils.HttpUtil;
@Test
public class ApiTestGetAccessToken extends ApiGeneralCases {

	final static int noClientId		    = 1<<31;
	final static int invalidClientId 	= 1<<30;
	final static int validClientId	    = 1<<29;
	final static int noClientSecret	    = 1<<28;
	final static int invalidClientSecret= 1<<27;
	final static int validClientSecret	= 1<<26;
	final static int noGrantType		= 1<<25;
	final static int invalidGrantType	= 1<<24;
	final static int validGrantType		= 1<<23;
	
		
	private void TestProcess(Object param)
	{
		String api_path = "/oauth2/token" ;
		
		//cleanup the maps
		parameter.clear();
		thrMap.clear();

		//setup the content
		generalSetup(current_condition,noClientId,invalidClientId,"client_id",param,client_id,parameter);
		generalSetup(current_condition,noClientSecret,invalidClientSecret,"client_secret",param,client_secret,parameter);
		generalSetup(current_condition,noGrantType,invalidGrantType,"grant_type",param,"client_credentials",parameter);
		
		System.out.println("=========Get token========");
		String result = HttpUtil.dopostApi(api_url, api_path, parameter, null);
		JSONObject jsonObject =JSONObject.fromObject(result);  
		Verification(jsonObject);
		
	}
	private void Verification(JSONObject jsonObject)
	{
		switch (current_condition)
		{
		case noClientId:
			check.equals(jsonObject.getString("err_code"), "10001", "errorcode is not as expected");
			check.equals(jsonObject.getString("msg"), "client_id参数错误", "msg is not as expected");
			break;
		case invalidClientId:
			if(APIUtil.getbool(current_condition_sub&(isnull|illegal)))
			{
				check.equals(jsonObject.getString("err_code"), "10001", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "client_id参数错误", "msg is not as expected");
			}
			else
			{
				check.equals(jsonObject.getString("err_code"), "20010", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "AppId不存在或已删除", "msg is not as expected");
				
			}
			break;
		case noClientSecret:
			check.equals(jsonObject.getString("err_code"), "10001", "errorcode is not as expected");
			check.equals(jsonObject.getString("msg"), "client_secret参数错误", "msg is not as expected");
			break;
		case invalidClientSecret:
			if(APIUtil.getbool(current_condition_sub&(isnull|illegal)))
			{
				check.equals(jsonObject.getString("err_code"), "10001", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "client_secret参数错误", "msg is not as expected");
			}
			else
			{
				check.equals(jsonObject.getString("err_code"), "20014", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "appID和appKey不匹配", "msg is not as expected");
				
			}
			break;
		case noGrantType:
		case invalidGrantType:
			check.equals(jsonObject.getString("err_code"), "10001", "errorcode is not as expected");
			check.equals(jsonObject.getString("msg"), "grant_type参数错误", "msg is not as expected");
			break;
		case 0:
		case validClientId:
		case validClientSecret:
		case validGrantType:
			access_token = jsonObject.getString("access_token");
			String expires_in = jsonObject.getString("expires_in");			   
			check.notEquals(access_token, "", "access token is null");
			check.equals(expires_in, "7200", "expires_in is not 7200");		
			break;
			
		default:
			break;
		}
		
	}

	public void a_normaltest()
	{
		current_condition = 0;
		TestProcess(null);
	}

	public void b_clientidtest()
	{
		current_condition = noClientId;
		TestProcess(null);
		
		current_condition = invalidClientId;
		LinkedHashMap<String,Integer> conditions = new LinkedHashMap<String,Integer> ();
		conditions.put("",isnull);
		conditions.put("wrong_id",notexist);
		conditions.put(" ",illegal);
		
		for (Map.Entry<String, Integer> entry : conditions.entrySet()) {  
			 
			current_condition_sub = entry.getValue();
			TestProcess(entry.getKey());
		}  
	}

	public void c_clientsecret()
	{
		current_condition = noClientSecret;
		TestProcess(null);
		
		current_condition = invalidClientSecret;
		LinkedHashMap<String,Integer> conditions = new LinkedHashMap<String,Integer> ();
		conditions.put("",isnull);
		conditions.put("wrong_secret",notexist);
		conditions.put(" ",illegal);
		
		for (Map.Entry<String, Integer> entry : conditions.entrySet()) {  
			 
			current_condition_sub = entry.getValue();
			TestProcess(entry.getKey());
		}  
	
	}

	public void d_granttype()
	{
		current_condition = noGrantType;
		TestProcess(null);
		
		current_condition = invalidGrantType;
		
		LinkedHashMap<String,Integer> conditions = new LinkedHashMap<String,Integer> ();
		conditions.put("",isnull);
		conditions.put("wrong_type",notexist);
		conditions.put("$@$#@",illegal);
		
		for (Map.Entry<String, Integer> entry : conditions.entrySet()) {  
			 
			current_condition_sub = entry.getValue();
			TestProcess(entry.getKey());
		}  
		
	}
	
	 
}