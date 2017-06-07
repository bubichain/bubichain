package cases;

import org.testng.annotations.Test;
import base.TestBase;
import net.sf.json.JSONObject;
import utils.HttpUtil;

public class BMInterfAssetCount extends TestBase {

	private void TestAssetCount()
	{
		String interf_path = "/asset/v1/count" ;
		
		System.out.println("=========Get asset count========");
		String result = HttpUtil.dogetApi(interface_url, interf_path, null);
		JSONObject jsonObject =JSONObject.fromObject(result);  
		//Verification(jsonObject);
	}
	@Test
	public void normaltest()
	{
		TestAssetCount();
	}
	
	
}
