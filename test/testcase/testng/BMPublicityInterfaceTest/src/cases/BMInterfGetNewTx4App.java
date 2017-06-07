package cases;

import org.testng.annotations.Test;
import base.TestBase;
import net.sf.json.JSONObject;
import utils.HttpUtil;

public class BMInterfGetNewTx4App extends TestBase {

	private void TestAssetCount()
	{
		String interf_path = "/transaction/v2/getNewTx4App" ;
		
		System.out.println("=========Get New Tx for App========");
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
