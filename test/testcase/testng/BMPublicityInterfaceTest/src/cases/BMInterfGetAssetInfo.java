package cases;

import java.util.LinkedHashMap;

import org.testng.annotations.Test;

import base.TestBase;
import net.sf.json.JSONObject;
import utils.APIUtil;
import utils.HttpUtil;

public class BMInterfGetAssetInfo extends TestBase {
	//definition of conditions
	final static int noAssetCode			= 1<<1;
	final static int invalidAssetCode		= 1<<2;
	final static int validAssetCode			= 1<<3;
	
		
	static int current_condition;
	static int current_condition_sub;
	
	private void TestGetAssetInfo(Object param)
	{
		String api_path = "asset/v1/assetInfo" ;
		LinkedHashMap<String, String> parameter = new LinkedHashMap<String,String>();
		
		if(!APIUtil.getbool(current_condition&noAssetCode))
		{			
			if(APIUtil.getbool(current_condition&(invalidAssetCode|validAssetCode)))
				parameter.put("asset_code",(String)param);
			else
				parameter.put("asset_code","4e926fd49caaa1c7073b3f028909e08f24fce117d9978a4eb029811c1e3f87e8");
		}
		
					
		System.out.println("=========Get Asset Info========");
		String result = HttpUtil.dogetApi(interface_url, api_path, parameter);
		JSONObject jsonObject =JSONObject.fromObject(result);  
		//Verification(jsonObject);
	}
	
	@Test	
	public void a_normalTest()
	{
		current_condition = 0;
		TestGetAssetInfo(null);
		
	}

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

}
