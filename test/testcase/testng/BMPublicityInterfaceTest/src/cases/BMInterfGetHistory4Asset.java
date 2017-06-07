package cases;

import java.util.LinkedHashMap;

import org.testng.annotations.Test;

import base.TestBase;
import net.sf.json.JSONObject;
import utils.APIUtil;
import utils.HttpUtil;

public class BMInterfGetHistory4Asset extends TestBase {
	//definition of conditions
	final static int noAssetCode			= 1<<1;
	final static int invalidAssetCode		= 1<<2;
	final static int validAssetCode			= 1<<3;
	
	final static int noStartTime			= 1<<4;
	final static int invalidStartTime		= 1<<5;
	final static int validStartTime			= 1<<6;
	
	final static int noEndTime				= 1<<7;
	final static int invalidEndTime			= 1<<8;
	final static int validEndTime			= 1<<9;
	
		
	static int current_condition;
	static int current_condition_sub;
	
	private void TestGetAssetInfo(Object param)
	{
		String api_path = "transaction/v1/history4Asset" ;
		LinkedHashMap<String, String> parameter = new LinkedHashMap<String,String>();
		
		if(!APIUtil.getbool(current_condition&noAssetCode))
		{			
			if(APIUtil.getbool(current_condition&(invalidAssetCode|validAssetCode)))
				parameter.put("assetCode",(String)param);
			else
				parameter.put("assetCode","73275041b528d56d19a4d1d060ee11dd11caab4e96b88d33c5b27ece6a3bff9b");
		}
		
		if(!APIUtil.getbool(current_condition&noStartTime))
		{			
			if(APIUtil.getbool(current_condition&(invalidStartTime|validStartTime)))
				parameter.put("startTime",(String)param);
			else
				parameter.put("startTime","1477906867");
		}
		
		if(!APIUtil.getbool(current_condition&noEndTime))
		{			
			if(APIUtil.getbool(current_condition&(invalidEndTime|validEndTime)))
				parameter.put("endTime",(String)param);
			else
				parameter.put("endTime","1477906867");
		}
		
		
					
		System.out.println("=========Get History 4 asset========");
		String result = HttpUtil.dogetApi(interface_url, api_path, parameter);
		JSONObject jsonObject =JSONObject.fromObject(result);  
		//Verification(jsonObject);
	}
	

	public void a_normalTest()
	{
		current_condition = 0;
		TestGetAssetInfo(null);
		
	}
	@Test
	public void b_assetcodeTest()
	{
		current_condition = noAssetCode;
		TestGetAssetInfo(null);
		
		current_condition = invalidAssetCode;
		String conditions[]={
				"",
				"wrong_code",
				"****&&%",
				APIUtil.getRandomString(64)				
		};
		for (int i =0 ;i<4;++i)
		{
			TestGetAssetInfo(conditions[i]);
		}		
	}	

	public void b_startTimeTest()
	{
		current_condition = noStartTime;
		TestGetAssetInfo(null);
		
		current_condition = invalidStartTime;
		String conditions[]={
				"",
				"wrong_code",
				"****&&%",
				APIUtil.getRandomString(64)				
		};
		for (int i =0 ;i<4;++i)
		{
			TestGetAssetInfo(conditions[i]);
		}		
	}	

}
