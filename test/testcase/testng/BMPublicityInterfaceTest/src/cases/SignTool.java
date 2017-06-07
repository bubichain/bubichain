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
		
		thrMap.put("current_string", "UuVuVaEOgk631l1HDVUiPtchjhrQvyN1");				
		thrMap.put("trade_no", "381438852823478");							
		thrMap.put("asset_code", "2UX4xvQ4aXNRetehXaSe1EYwqyKZ6FLMsvN6AzBA9BKbdiRWFBEAxEgRzK1gfb39tQX538ckiJ7qxHXQj9381eci4CSRcNrbkh8covRiobidADgzEjSPnNNMS2Awd4hrNbs3nHf9DDtm3kvy");	
		thrMap.put("asset_amount", "123");
		thrMap.put("from_bubi_address", "bubiV8i2gZFtn74ka6jwcY51nRhGq6wodyqviAYK");	
		thrMap.put("to_bubi_address", "bubiV8i4CwsveLoJQU3dKajLsVxxmsyTtrLDsMwv");	
		//String sign1 = APIUtil.sign(thrMap, client_secret);
		String sign1 = APIUtil.sign(thrMap, "zhongtuobang_key");
		
		System.out.println("sign:"+sign1);
	}
	/*
		"current_string":"UuVuVaEOgk631l1HDVUiPtchjhrQvyN1"
"trade_no":"381438852823477"
"asset_code":"2UX4xvQ4aXNRetehXaSe1EYwqyKZ6FLMsvN6AzBA9BKbdiRWFBEAxEgRzK1gfb39tQX538ckiJ7qxHXQj9381eci4CSRcNrbkh8covRiobidADgzEjSPnNNMS2Awd4hrNbs3nHf9DDtm3kvy"
"asset_amount":"100"
"from_bubi_address":"bubiV8i2gZFtn74ka6jwcY51nRhGq6wodyqviAYK"
"to_bubi_address":"bubiV8i4CwsveLoJQU3dKajLsVxxmsyTtrLDsMwv"
"sign":"9acebbc7625b67f644e690e6db45d53a"
"password":"sKuY2CJ506"
"metadata":"444"
	
	*/
}