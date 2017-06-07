package cases;

import java.util.List;
import java.util.Map;
import java.util.Random;

import org.testng.annotations.Test;

import utils.APIUtil;
import utils.HttpPool;
import utils.HttpUtil;
import utils.Result;
import utils.SignUtil;
import utils.TxUtil;
import base.Log;
import base.TestBase;

//@Test
public class GetAssetUniqueTest extends TestBase {

	String tran = "getUniqueAsset";
	String asset_code = "abc";
	String asset_issuer = led_acc;
	String order = "desc";
	String asset_detailed = "1234";
	Object s_address = led_acc;
	Object s_key = led_pri;
	int start = 0;
	int limit = 10;
	String response = TxUtil.tx_uniIssue(7, asset_issuer, asset_code,
			asset_detailed, s_address, s_key);

//	 @Test
	public void asset_issuerCheck() {
		int type = 7; // 先发行唯一资产
		Map acc = TxUtil.createAccount();
		Object source_address = acc.get("address");
		String pri = acc.get("private_key").toString();
		Object pub = acc.get("public_key");
		String asset_code = "abc";
		String asset_detailed = "1234";
		String metadata = "1234";
		Object asset_issuer = source_address;
		List opers = TxUtil.operUniIssue(type, asset_issuer, asset_code,
				asset_detailed);
		long sequence_number = Result.seq_num(source_address);
		String result = SignUtil.tx(opers, source_address, fee,
				sequence_number+1, metadata, pri, pub);
		int error_code = Result.getErrorCode(result);
		if (error_code == 0) {
			String key = "asset_issuer";
			String response = HttpUtil.doget(tran, key, asset_issuer);
			// 查询返回结果中的asset_issuer,asset_code值是否和设置的一样
			String asset_code1 = Result.getasset_code(response);
			String asset_issuer_ = Result.getasset_issuer(response);
			int reSize = Result.getResultSize(response);
			check.equals(asset_code1, asset_code, "通过asset_code获取唯一资产错误");
			check.equals(asset_issuer_, asset_issuer.toString(),
					"通过asset_issuer获取唯一资产错误");
			check.assertEquals(reSize, 1, "通过asset_code获取唯一资产错误");
		}else {
			Log.info("唯一资产发行失败");
		}
	}

//	 @Test
	@SuppressWarnings("unused")
	// 发行唯一资产后查询asset_code的值是否一致
	public void asset_codeCheck() {
		// 先发行唯一资产
		int type = 7;
		@SuppressWarnings("rawtypes")
		Map acc = TxUtil.createAccount();
		Object source_address = acc.get("address");
		String pri = acc.get("private_key").toString();
		Object pub = acc.get("public_key");
		String asset_code = "abc";
		Object asset_issuer = source_address;
		String asset_detailed = "1234";
		String metadata = "1234";
		List opers = TxUtil.operUniIssue(type, asset_issuer, asset_code,
				asset_detailed);
		long sequence_number = Result.seq_num(baseUrl,source_address);
		String result = SignUtil.tx(opers, source_address, fee,sequence_number, metadata, pri, pub);
		String hash = Result.getHash(result);
		// 再进行查询
		String key = "asset_code";
		String response = HttpUtil.doget(tran, key, asset_code);
		int error_code = Result.getErrorCode(response);
		int reSize = Result.getResultSize(response);
		String asset_code1 = Result.getasset_code(response);
		check.equals(asset_code1, asset_code, "通过asset_code获取唯一资产错误");
		check.assertNotEquals(reSize, 0, "通过asset_code获取唯一资产错误");

	}

	 @Test
	// 通过asset_code和asset_issuer的组合进行查询，结果应该唯一
	public void codeissuerCheck() {
		// 先发行唯一资产
		int type = 7;
		@SuppressWarnings("rawtypes")
		Map acc = TxUtil.createAccount();
		Object source_address = acc.get("address");
		String pri = acc.get("private_key").toString();
		Object pub = acc.get("public_key");
		String asset_code = "abc";
		Object asset_issuer = source_address;
		String asset_detailed = "1234";
		String metadata = "1234";
		List opers = TxUtil.operUniIssue(type, asset_issuer, asset_code,
				asset_detailed);
		long sequence_number = Result.seq_num(source_address);
		String result = SignUtil.tx(opers, source_address, fee,
				sequence_number, metadata, pri, pub);
		// System.out.println(response);
		int error_code = Result.getErrorCode(result);
		if (error_code == 0) {
			String k1 = "asset_code";
			String k2 = "asset_issuer";
			String response = HttpUtil.doget(tran, k1, asset_code, k2,
					asset_issuer.toString());
			String asset_code1 = Result.getasset_code(response);
			String asset_issuer1 = Result.getasset_issuer(response);
			check.equals(asset_code1, asset_code, "通过asset_code组合获取唯一资产错误");
			check.assertEquals(asset_issuer1, asset_issuer,
					"通过asset_issuer组合获取唯一资产错误");
		}else {
			check.fail("唯一资产发行失败");
		}
	}

	// @Test

	// 排序，最好是随机取两个值，i<=size
//	public void order_descCheck() {
//
//		String k1 = "order";
//		String value = "desc";
//		int reSize = 0;
//		int num1 = 0;
//		int num2 = 0;
//		String response = HttpPool.doGet(tran, k1, value);
//		// System.out.println(response);
//		int error_code = Result.getErrorCode(response);
//		reSize = Result.getResultSize(response);
//		if (reSize > 1) {
//			// 随机获取结果中的任意两个值，num1 < num2
//			num1 = new Random().nextInt(reSize + 1);
//			num2 = new Random().nextInt(reSize - num1) + num1;
//		} else {
//			System.out.println("结果只有两条无法比较");
//		}
//		int ledger_seq1 = Result.getledger_seq(response, num1);
//		int ledger_seq2 = Result.getledger_seq(response, num2);
//		check.largerThan(ledger_seq1, ledger_seq2, "降序查询出错");
//		check.result("获取唯一资产校验成功");
//	}
//
//	// @Test
//	public void order_ascCheck() {
//
//		String k1 = "order";
//		String value = "asc";
//		int reSize = 0;
//		int num1 = 0;
//		int num2 = 0;
//		String response = HttpPool.doGet(tran, k1, value);
//		int error_code = Result.getErrorCode(response);
//		reSize = Result.getResultSize(response);
//		if (reSize > 1) {
//			num1 = new Random().nextInt(reSize + 1);
//			num2 = new Random().nextInt(reSize - num1) + num1;
//		} else {
//			System.out.println("结果只有两条无法比较");
//		}
//		int ledger_seq1 = Result.getledger_seq(response, num1);
//		int ledger_seq2 = Result.getledger_seq(response, num2);
//		check.smallerThan(ledger_seq1, ledger_seq2, "升序查询出错");
//
//	}
}
