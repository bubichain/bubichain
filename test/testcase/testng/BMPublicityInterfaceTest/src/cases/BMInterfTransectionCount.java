package cases;

import org.testng.annotations.Test;
import base.TestBase;
import net.sf.json.JSONObject;
import utils.HttpUtil;

public class BMInterfTransectionCount extends TestBase {

	private void TestTransactionCount()
	{
		String interf_path = "/transaction/v1/count" ;
		
		System.out.println("=========Get transaction count========");
		String result = HttpUtil.dogetApi(interface_url, interf_path, null);
		JSONObject jsonObject =JSONObject.fromObject(result);  
		//Verification(jsonObject);
	}
	
	
	@Test
	public void normaltest()
	{
		TestTransactionCount();		
	}
	
	
}
