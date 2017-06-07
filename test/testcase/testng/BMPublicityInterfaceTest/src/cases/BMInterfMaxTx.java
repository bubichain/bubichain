package cases;

import org.testng.annotations.Test;
import base.TestBase;
import net.sf.json.JSONObject;
import utils.HttpUtil;

public class BMInterfMaxTx extends TestBase {

	private void TestMaxTx()
	{
		String interf_path = "transaction/v1/maxTx" ;
		
		System.out.println("=========Get 10 assets with highest frequence========");
		String result = HttpUtil.dogetApi(interface_url, interf_path, null);
		JSONObject jsonObject =JSONObject.fromObject(result);  
		//Verification(jsonObject);
	}
	@Test
	public void normaltest()
	{
		TestMaxTx();
	}
	
}
