package cases;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import org.testng.annotations.Test;

import base.TestBase;
import net.sf.json.JSONObject;
import utils.APIUtil;
import utils.HttpUtil;
import utils.Result;
import utils.TxUtil;

@Test
public class GetMutliQueryTest extends TestBase {
	 @Test
	public void singlePostCheck() {

		Map acc = TxUtil.createAccount();
		Object address = acc.get("address");
		String pri = acc.get("private_key").toString();
		Object pub = acc.get("public_key");
		int type = 0;
		int init_balance = 100000;
		// long fee = fee;
		String account_metadata = "abcd";
		Object source_address = address;
		long sequence_number = Result.seq_num(source_address);

		Object dest_add = APIUtil.generateAcc().get("address");
		// 创建账户operation
		List opers = TxUtil.operCreateAccount(type, dest_add, init_balance,
				account_metadata);
		// 创建账户transaction
		JSONObject tran = TxUtil.transaction(source_address, opers,
				sequence_number, fee);
		// 发送请求
		String result = TxUtil.mutiPost(opers, tran, pri, pub);
		System.out.println(result);
		int error_code = Result.getoutErrCodeFromGet(result);
		check.assertEquals(error_code, 0, "单个POST请求查询失败");
	}
	// @Test
	public void singleGetCheck() {
		String url = "getLedger";
		JSONObject get1 = TxUtil.mutiGet(url);
		List item = TxUtil.items(get1);
		JSONObject tx = TxUtil.tx(item);
		String result = TxUtil.txPost("mutliQuery", tx);
		int error_code = Result.getErrCodeFromPost(result);
		check.assertEquals(error_code, 0, "单个GET请求查询失败");
	}

//	@Test
//	public void combineCheck() {
//		int type = 2;
//		int asset_type = 1;
//		Map acc = TxUtil.createAccount();
//		Object address = acc.get("address");
//		String pri = acc.get("private_key").toString();
//		Object pub = acc.get("public_key");
//		Object asset_issuer = address;
//		String asset_code = "abc";
//		int asset_amount = 100;
//		Object source_address = address;
//		long sequence_number = Result.seq_num(source_address);
//		String metadata = "abcd";
//		// post请求为发行资产
//		List opers = TxUtil.operIssue(type, asset_type, source_address,
//				asset_code, asset_amount);
//		JSONObject tran = TxUtil.transaction(source_address, opers,
//				sequence_number, fee);
//		JSONObject item_ = TxUtil.items(opers, tran, pri, pub); // 得到post的jsondata
//		JSONObject mutipost = TxUtil.mutiPost(item_);
//
//		String url = "getModulesStatus";
//		JSONObject get1 = TxUtil.mutiGet(url);
//		List item = new ArrayList<>();
//		item.add(mutipost);
//		item.add(get1);
//
//		JSONObject items = TxUtil.items(item); // 拼成最后的一个items jsonobject对象
//
//		String re = TxUtil.txPost("mutliQuery", items);
//		int error_code = Result.getoutErrCodeFromGet(re);
//		check.assertEquals(error_code, 0, "组合批量操作验证失败");
//	}
	
	public void MultiQueryNonJsonCheck() {
		String testBody = "test";
		String result = HttpUtil.dopost(baseUrl,"mutliQuery",testBody);
		int err_code = Result.getoutErrCodeFromGet(result);
		check.assertEquals(err_code, 2,"mutliQuery json body check failed");
	}
	
	public void MultiQueryOverloadCheck() {
		
		List item = new ArrayList<>();
		for(int i = 0;i<101;i++)
		item.add("onejson");		

		JSONObject items = TxUtil.items(item); // 拼成最后的一个items jsonobject对象

		String re = TxUtil.txPost("mutliQuery", items);
		int error_code = Result.getoutErrCodeFromGet(re);
		check.assertEquals(error_code, 2, "mutliQuery overload check failed");
	}

}
