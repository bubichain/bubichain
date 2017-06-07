package cases;

import org.testng.annotations.Test;
import base.TestBase;
import net.sf.json.JSONObject;
import utils.HttpUtil;

public class BMInterfNodeList extends TestBase {

	
	private void TestNodeList()
	{
		String interf_path = "/node/v1/nodeList" ;
		
		System.out.println("=========Get Node List========");
		String result = HttpUtil.dogetApi(interface_url, interf_path, null);
		JSONObject jsonObject =JSONObject.fromObject(result);  
		//Verification(jsonObject);
	}
	
	private void TestTransactionCount()
	{
		String interf_path = "/transaction/v1/count" ;
		
		System.out.println("=========Get transaction count========");
		String result = HttpUtil.dogetApi(interface_url, interf_path, null);
		JSONObject jsonObject =JSONObject.fromObject(result);  
		//Verification(jsonObject);
	}
	
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
		TestNodeList();
		TestTransactionCount();
		TestAssetCount();
	}
	
	
}
