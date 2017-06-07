package cases;

import java.util.List;
import java.util.Map;

import org.testng.annotations.Test;

import utils.HttpPool;
import utils.HttpUtil;
import utils.Result;
import utils.SignUtil;
import utils.TxUtil;
import base.Log;
import base.TestBase;

@Test

//查询存证 交易
public class GetStorageTest extends TestBase{

	String id = null;
	String record_participant = null;
	String record_address = null;
	int start = 0;
	int limit = 0;
	String order = "desc";
	String transaction = "getRecord";
	
	public void recordAddressCheck(){
		int type = 9;
		Map acc = TxUtil.createAccount();
		Object source_address = acc.get("address");
		String pri = acc.get("private_key").toString();
		Object pub = acc.get("public_key");
		String record_id = "1234";
		String record_ext = "1234";
		String metadata = "1234";
		long sequence_number = Result.seq_num(source_address);
		List opers = TxUtil.operStorage(type, record_id, record_ext);
		String response = SignUtil.tx(opers, source_address, fee,
				sequence_number, metadata, pri, pub);
		int e1 = Result.getErrorCode(response);
		check.equals(e1, 0, "发行存证出错");

		String response_ = Result.getStorageByAdd(source_address.toString());
		int error_code = Result.getoutErrCodeFromGet(response_);
		check.equals(error_code, 0, "查询存证出错");

		int resultsize = Result.getResultSize(response_);
		check.assertEquals(resultsize, 1, "查询结果不唯一");
		check.result("获取存证校验成功");
	}
	
	
	
	public void participantCheck(){
		String key = "record_participant";
		String address = led_acc;
		String response = HttpUtil.doget(transaction, key, address);
		int error_code = Result.getoutErrCodeFromGet(response);
		check.assertEquals(error_code, 0, "查询存证出错");
	}
	
	public void limitCheck(){
		String key = "limit";
		int limit = 10;
		String response = HttpUtil.doget(transaction, key, limit);
		int error_code = Result.getoutErrCodeFromGet(response);
		check.assertEquals(error_code, 0, "查询存证出错");
	}
//	@Test
	public void idValidCheck(){
		String url = "getRecord";
		String key = "id";
		String value = "1234";
		String result = HttpUtil.doget(url, key, value);
		int error_code = Result.getErrorCode(result);
		check.assertEquals(error_code, 0,"根据id["+value+"]查询存证失败");
	}
//	@Test
	//当record_participant值为all时查询出原声明以及所有对该原声明的追加声明
	public void record_participantAllCheck(){
		String url = "getRecord";
		String key = "record_participant";
		String value = "all";
		String result = HttpUtil.doget(url, key, value);
		int error_code = Result.getErrorCode(result);
		check.assertEquals(error_code, 0,"根据id["+value+"]查询存证失败");
	}
	@Test
	//当record_participant值为账号地址时,查询出原声明以及该账号对原声明的追加声明
	public void record_partiAndAddCheck(){
		String url = "getRecord";
		String key = "record_participant";
		Map acc = TxUtil.createAccount();
		Object address = acc.get("address");
		String pri = acc.get("private_key").toString();
		Object pub = acc.get("public_key");
		String value = address.toString();
		String key1 = "record_address";
		String value1 = address.toString();
		
		//先发起存证（原声明）交易
		int type = 9;
		String record_id = "1234";
		String record_ext = "1234";
		List opers = TxUtil.operStorage(type, record_id, record_ext);
		long sequence_number = Result.seq_num(address);
		String response = SignUtil.tx(opers, address, fee, sequence_number, "1234", pri, pub);
		
		if(Result.getErrorCode(response)==0){
			String result =HttpUtil.doget (url, key, value,key1,value1);
			int error_code = Result.getErrorCode(result);
			check.assertEquals(error_code, 0,"根据id["+value+"]查询存证失败");
		}else {
			Log.error("存证交易失败，无法进行查询操作");
		}
		
	}
//	@Test
	public void order_ascCheck(){
		String key = "order";
		String[] order_a = {"asc","ASC"};
		for (String order : order_a) {
			String response = HttpUtil.doget(transaction, key, order);
			int error_code = Result.getoutErrCodeFromGet(response);
			check.assertEquals(error_code, 0, "查询存证出错");
		}
	}
//	@Test
	public void order_descCheck(){
		String key = "order";
		String[] order_d = {"desc","DESC"};
		for (String order : order_d) {
			String response = HttpUtil.doget(transaction, key, order);
			int error_code = Result.getoutErrCodeFromGet(response);
			check.assertEquals(error_code, 0, "查询存证出错");
		}
	}
	
}
