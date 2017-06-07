package cases;

import java.util.LinkedHashMap;
import java.util.Map;
import org.testng.annotations.Test;
import base.TestBase;
import utils.APIUtil;


@Test
public class SignTool extends TestBase {
	
		
	public void calculateSign()
	{
		Map<String, Object> thrMap = new LinkedHashMap<String, Object>();
		
		thrMap.put("current_string", "1467345122019");				
		thrMap.put("trade_no", "381345971364865");							
		thrMap.put("asset_code", "2UX4xvQ4aXNXpKZ73VyocXUn1gQGuxcHzPA6xRXjvLjNhNZYuBBS2rqySTQZJqNJmKaP9cD7gsznGLUGQdYyC2CqipHCSeY9X3gsD66qmVJitRmpsuBrV5FF46JsqtpvH4aaSQWP4LGja1X6");	
		thrMap.put("asset_amount", "100");
		thrMap.put("from_bubi_address", "bubiV8i3Rot3u1buQ88UWUGwj6sThB8FV8BWM4dd");	
		thrMap.put("to_bubi_address", "bubiV8iEgYtdKfGY9K5RYUqTwEtgEms3r5knB6yL");	
		//String sign1 = APIUtil.sign(thrMap, client_secret);
		String sign1 = APIUtil.sign(thrMap, "d12ed7f0a30bebb157bdc81770fd2201");
		
		System.out.println("sign:"+sign1);
	}
	/*
		"asset_issuer":"bubiV8iCtADHANJ1GGqaFueCYC5KvLTMe6UbUz4k",
"password":"PxyrNcx2ZL",
"trade_no":"381501557153998",
"asset_name":"某种资产",
"asset_unit":"个",
"asset_amount":"100",
"metadata":"{\"asset_type\":\"4\",\"asset_unit_code\":\"555\",\"annualized_rate\":\"7\",\"asset_description\":\"this is a description\"}"
	
	*/
}