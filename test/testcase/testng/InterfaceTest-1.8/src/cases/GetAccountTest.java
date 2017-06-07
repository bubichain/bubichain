package cases;

import java.util.List;
import java.util.Map;

import net.sf.json.JSONObject;

import org.testng.annotations.Test;

import utils.APIUtil;
import utils.Result;
import utils.SignUtil;
import utils.TxUtil;
import base.Log;
import base.TestBase;

@Test
public class GetAccountTest extends TestBase{

	Object address = TxUtil.createAccount().get("address");
	
//	@Test
	public void getAccount(){
		// System.out.println("address===="+address);
		String accountInfo = Result.getAccInfo(address);
		if(Result.getoutErrCodeFromGet(accountInfo)==0){
			String balance = Result.getBalanceInResponse(accountInfo);
			String add = Result.getAddress(accountInfo);
			check.assertEquals(add, address.toString(), "账户地址错误");
		}else{
			Log.info("地址["+address+"]不存在");
		}
	}
	
//	@Test
	@SuppressWarnings({ "rawtypes", "unused" })
	public void balanceCheck(){

		/**
		 * 转账后查询余额a减少m+fee，b增加m
		 * a1 transfer to a2
		 */
		//account1
		int asset_amount = 1000;
		Map acc1 = TxUtil.createAccount();
		Object ad1 = acc1.get("address");
		String balance1 = Result.getBalanceInAcc(led_acc);
		String balanc2 = Result.getBalanceInAcc(ad1);
		long sequence_number = Result.seq_num(led_acc);
		List opers = TxUtil
				.opertransfer(1, 0, ad1, asset_amount, led_acc, "abc");
		String result = SignUtil.tx(opers, led_acc, fee, sequence_number,
				"1234", led_pri, led_pub);
		int err_code = Result.getErrorCode(result);
		if (err_code == 0) {
			String balance1_r = Result.getBalanceInAcc(led_acc);
			long b1 = Long.parseLong(balance1) - asset_amount - fee;
			check.equals(Long.parseLong(balance1_r), b1, "转账后源账户余额有误");
			String balance2_r = Result.getBalanceInAcc(ad1);
			long b2_r = Long.parseLong(balance2_r);
			check.assertEquals(b2_r, Long.parseLong(balanc2) + asset_amount,
					"转账后目标账户余额有误");
		}
	}
	
//	@Test
	public void balance_reserveCheck(){
		//ERRCODE_ACCOUNT_LOW_RESERVE
		Map acc = TxUtil.createAccount();
		Object source_address = acc.get("address");
		String pri = acc.get("private_key").toString();
		Object pub = acc.get("public_key");
		List opers = TxUtil.operIssue(2, 1, source_address, "abc", 10);	
		String balance = Result.getBalanceInAcc(source_address);
		Long bal = Long.parseLong(balance);
		String result = SignUtil.tx(opers, source_address, fee+bal, "1234", pri, pub);
		System.out.println(result);
		int err_code = Result.getErrorCode(result);
		check.assertEquals(err_code, 4, "账户余额不足校验失败");
	}
	/**
	 * 发行资产后数量正确
	 * 转移资产后数量正确
	 * 初始化转账后正确
	 */
//	@Test
	public void assetCheck() {
		Map acc = TxUtil.createAccount();
		Object dest_address = acc.get("address");
		String pri = led_pri;
		Object pub = led_pub;
		Object type = 5;
		Object asset_type = 1;
		Object asset_code = "test";
		Object source_address = led_acc;
		Object asset_issuer = led_acc;
		int asset_amount = 10;
		String metadata = "abcd";
		long sequence_number = Result.seq_num(source_address);

		int type_ = 2;
		int asset_type_ = 1;
		List opers_ = TxUtil.operIssue(type_, asset_type_, source_address,
				asset_code, asset_amount);
		String result_ = SignUtil.tx(opers_, source_address, fee,
				sequence_number, metadata, pri, pub);
		int err_code1 = Result.getErrorCode(result_);
		check.equals(err_code1, 0, "资产发行失败"); // 先发行资产
		sequence_number = Result.seq_num(source_address);
		List opers = TxUtil.operInitTransfer(type, asset_type, dest_address,
				asset_amount, asset_issuer, asset_code);
		String result = SignUtil.tx(opers, source_address, fee,
				sequence_number, metadata, pri, pub);
		int err_code = Result.getErrorCode(result);
		check.equals(err_code, 0, "初始化转账失败");

		if (err_code == 0) {
			String hash = Result.getHash(result);
			System.out.println("hash=" + hash);
			String re = Result.getResult("getTransactionHistory", "hash", hash);
			int err = Result.getErrorCode(re);
			check.equals(err, 0, "通过hash查询交易记录失败");

			int ass_amo = Result.getasset_amount(re);

			check.assertEquals(ass_amo, asset_amount, "初始化转账后asset_amount出现错误");
		}
	}
	
	//设置metadata后查询与设置值是否一致
	public void metadataCheck() {
		@SuppressWarnings({ "rawtypes", "unused" })
		Object source_address = led_acc;
		long sequence_number = Result.seq_num(source_address);
		String pri = led_pri;
		String pub = led_pub;
		String account_metadata = "ab";
		String metadata = "8989";
		int type = 0;

		Map acc_gen = APIUtil.generateAcc();
		Object dest_add = acc_gen.get("address");
		List opers = TxUtil.operCreateAccount(type, dest_add, init_balance,
				account_metadata);
		String result = SignUtil.tx(opers, source_address, fee,
				sequence_number, metadata, pri, pub);

		String meta_data = Result.getMetadata(dest_add);

		check.assertEquals(meta_data, account_metadata, "metadata与设置的不一致");
	}
	
	/**
	 * 1.不填写metadata_version，每次修改metadata后，version是否自动加1
	 * 2.只填写metadata_version，不填写metadata，会报错
	 * 3.填写正确的metadata_version,提交正确
	 * 4.填写错误的metadata_version,提交出错
	 */
	public void metadata_versionCheck() {
		Object source_address = led_acc;
		long sequence_number = Result.seq_num(source_address);
		String pri = led_pri;
		String pub = led_pub;
		String account_metadata = "ab";
		String metadata = "";
		int type = 0;

		Map acc_gen = APIUtil.generateAcc();
		Object dest_add = acc_gen.get("address");
		List opers = TxUtil.operCreateAccount(type, dest_add, init_balance,
				account_metadata);
		String result = SignUtil.tx(opers, source_address, fee,
				sequence_number, metadata, pri, pub);

		int metadata_version = Result.getMetadata_version(dest_add);

		check.assertEquals(metadata_version, 1, "新创建的账户metadata_version错误["
				+ metadata_version + "]");
	}
	
//	@Test
	//用户设置权重成功后做查询权重是否设置正确
	public void thresholdCheck(){
		int master_weight = 5;
		int med_threshold = 2;
		int low_threshold = 2;
		int high_threshold = 2;
		int type = 4;
		Map acc = TxUtil.createAccount();
		Object source_address = acc.get("address");
		String pri = acc.get("private_key").toString();
		Object pub = acc.get("public_key");
		long sequence_number = Result.seq_num(source_address);
		String metadata = "1234";
		
		
		JSONObject threshold = TxUtil.threshold(master_weight, med_threshold, low_threshold, high_threshold);
		List operations = TxUtil.operSetOption(type, threshold);
		String response = TxUtil.tx_result(operations, source_address,
				fee, sequence_number, metadata, pri, pub);//SignUtil.unnormal_tx(operations, source_address,
//				fee, sequence_number, metadata, pri, pub);
		int error_code = Result.getErrorCode(response);
		if(error_code==0){
			String hash = Result.getHash(response);
			System.out.println("hash="+hash);
			String re = Result.getResult("getTransactionHistory", "hash", hash);
			int err = Result.getErrorCode(response);
			check.equals(err, 0, "通过hash查询交易记录失败");
			
			int mas_wei = Result.getThresholdTh(re,"master_weight");
			check.equals(mas_wei, master_weight, "master_weight校验失败");
			int med_thr = Result.getThresholdTh(re,"low_threshold");
			check.equals(med_thr, med_threshold, "low_threshold校验失败");
			int low_thr = Result.getThresholdTh(re,"med_threshold");
			check.equals(low_thr, low_threshold, "med_threshold校验失败");
			int hig_thr = Result.getThresholdTh(re,"high_threshold");
			check.assertEquals(hig_thr, high_threshold, "high_threshold校验失败");
		}
	}

	//用户设置联合签名后，查询签名信息是否正确
	public void signersCheck() {
		Map acc = TxUtil.createAccount(); // 创建一个账户，给它设置联合签名
		Object source_address = acc.get("address");
		String pri = acc.get("private_key").toString();
		Object pub = acc.get("public_key");
		long sequence_number = Result.seq_num(source_address);
		// 设置联合签名的账户权重为6
		String address1 = TxUtil.createAccount().get("address").toString();
		Object weight1 = 6;
		int type = 4;
		String metadata = "1234";
		//设置账户属性
		List signers = TxUtil.signers(address1, weight1);
		List operations = TxUtil.operSetOption(type,signers);
		String response = SignUtil.tx(operations, source_address, fee, sequence_number, metadata, pri, pub);
		int error_code = Result.getErrorCode(response);
		if(error_code==0){
			String result = Result.getAccInfo(source_address);
			String addr = Result
.getThInAccountInfo(result, "signers",
					"address");
			check.equals(addr, address1, "设置属性后，账户中signers校验失败");
			String weight = Result.getThInAccountInfo(result, "signers",
					"weight");
			check.assertEquals(weight, weight1.toString(),
					"设置属性后，账户中weight校验失败");
		}

	}
	
	
	
}
