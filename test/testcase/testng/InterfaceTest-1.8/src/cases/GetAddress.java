package cases;

import org.testng.annotations.Test;

import utils.HttpPool;
import utils.Result;
import base.TestBase;

@Test
public class GetAddress extends TestBase{

//	@Test
	public void getAddessCheck(){
		String url = "getAddress";
		String key = "private_key";
		String value = led_pri;
		String result = HttpPool.doGet(url, key, value);
		String address = Result.getResultTh(result, "address");
		check.equals(address, led_acc, "getAddress私钥获取的地址错误");
	}
	
//	@Test
	public void private_keyinValidCheck(){
		String url = "getAddress";
		String key = "private_key";
		String value = "aa";
		String result = HttpPool.doGet(url, key, value);
		int error_code = Result.getErrorCode(result);
		check.assertEquals(error_code, 2, "getAddress非法私钥校验失败");
	}
	
//	@Test
	public void public_keyinValidCheck(){
		String url = "getAddress";
		String key = "public_key";
		String value = "aa";
		String result = HttpPool.doGet(url, key, value);
		int error_code = Result.getErrorCode(result);
		check.assertEquals(error_code, 2, "getAddress非法公钥校验失败");
	}
	
//	@Test
	public void public_keyValidCheck(){
		String url = "getAddress";
		String key = "public_key";
		String value = led_pub;
		String result = HttpPool.doGet(url, key, value);
		int error_code = Result.getErrorCode(result);
		check.assertEquals(error_code, 0, "getAddress公钥获取信息校验失败");
	}
}
