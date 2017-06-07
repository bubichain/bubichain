package cases;

import java.util.LinkedHashMap;

import org.testng.annotations.Test;
import base.TestBase;
import net.sf.json.JSONObject;
import utils.APIUtil;
import utils.HttpUtil;

public class BMInterfGetBlock extends TestBase {
	
	//definition of conditions
	final static int noSize				= 1<<1;
	final static int invalidSize		= 1<<2;
	final static int validSize			= 1<<3;
	
	static int current_condition;
	static int current_condition_sub;

	private void TestGetBlock(Object param)
	{
		String interf_path = "/block/v1/history" ;
		
		LinkedHashMap<String, String> parameter = new LinkedHashMap<String,String>();
		
		if(!APIUtil.getbool(current_condition&noSize))
		{			
			if(APIUtil.getbool(current_condition&(invalidSize|validSize)))
				parameter.put("size",(String)param);
			else
				parameter.put("size","2");
		}
		
		System.out.println("=========Get Block========");
		String result = HttpUtil.dogetApi(interface_url, interf_path, parameter);
		JSONObject jsonObject =JSONObject.fromObject(result);  
		//Verification(jsonObject);
	}

	public void normaltest()
	{
		TestGetBlock(null);
	}
	
	@Test
	public void c_SizeTest()
	{
		current_condition = noSize;
		TestGetBlock(null);
		
		/*current_condition = invalidSize;
		String conditions[]={
				"-1",
				"0",
				"****&&%",
				"3.1415",
				" ",
				"1001"	
		};
		for (int i =0 ;i<6;++i)
		{
			TestGetBlock(conditions[i]);
		}	*/	
		
		current_condition = validSize;
		String conditions2[]={
				"1",
				"1000",
				"50"
		};
		for (int i =0 ;i<3;++i)
		{
			TestGetBlock(conditions2[i]);
		}		
	}
	
}
