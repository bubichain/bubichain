package cases;


import java.util.List;
import java.util.Map;

import net.sf.json.JSONObject;

import org.testng.annotations.Test;

import utils.APIUtil;
import utils.Result;
import utils.SignUtil;
import utils.TxUtil;
import base.TestBase;
import model.Account;

@Test
public class CreateAccountTest extends TestBase{
	
	int type = 0;
	String account_metadata = "abcd";
	String metadata = "abcd";
	
	Map acc = TxUtil.createAccount();
	Object address = acc.get("address");
	String pri = acc.get("private_key").toString();
	Object pub = acc.get("public_key");
	String asset_code = "abcd" ;
	int asset_amount = 100;
	Object source_address = address;
	
	/*
	 * 设置交易类型，目标地址，metadata，初始金额
	 * 验证创建账号交易成功
	 */
	@Test
	public void createaccount(){
		Object dest_add = APIUtil.generateAccount().getAddress();
		List opers = TxUtil.operCreateAccount(type, dest_add, init_balance, account_metadata);	//ledger发行未初始化资产
		String result = SignUtil.tx(opers, led_acc, fee, metadata, led_pri, led_pub);
		int err_code = Result.getErrorCode(result);
		check.assertEquals(err_code, 0, "创建账户失败");
	}
	/*
	 * 验证创建账号交易，type值非法校验正确
	 */
//	@Test
	public void typeCheck(){
		Object dest_add = APIUtil.generateAccount().getAddress();
		Object[] types = {-1,10,"abc","!@#","",null};
		for (Object type : types) {
			List opers = TxUtil.operCreateAccount(type, dest_add, init_balance, account_metadata);	//ledger发行未初始化资产
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 2, "type[" + type + "]校验失败");
		}
	}
	
	/*
	 * 验证目标地址字段校验正确
	 */
//	@Test
	public void dest_addressCheck(){
		Object[] dest_adds = {-1,10,"abc","!@#",""};
		for (Object dest_add : dest_adds) {
			List opers = TxUtil.operCreateAccount(type, dest_add, init_balance, account_metadata);	//ledger发行未初始化资产
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 2, "dest_address[" + dest_add
					+ "]校验失败" + err_code);
		}
	}
	/*
	 * 验证目标地址已存在，交易失败
	 */
//	@Test
	public void dest_address_existCheck(){
		Object[] dest_adds = {led_acc};
		for (Object dest_add : dest_adds) {
			List opers = TxUtil.operCreateAccount(type, dest_add, init_balance, account_metadata);	//ledger发行未初始化资产
			System.out.println("!!!!source_address: "+source_address);
			System.out.println("pri : "+pri);
			System.out.println("pub: "+pub);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 102, "dest_address[" + dest_add
					+ "]校验失败" + err_code);
		}
	}
	
	/*
	 * 验证初始化金额字段校验正确
	 */
//	@Test
	public void init_balanceCheck(){
		Object dest_add =APIUtil.generateAccount().getAddress();
		Object[] init_balances = { -1, "abc", "!@#", ""};
		for (Object init_balance : init_balances) {
			List opers = TxUtil.operCreateAccount(type, dest_add, init_balance, account_metadata);	//ledger发行未初始化资产
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 2, "init_balance[" + init_balance
					+ "]校验失败");
		}
	}
	/*
	 * 验证初始化金额0小于系统默认值校验正确
	 */
//	@Test
	public void init_balance_lowCheck(){
		Object dest_add = APIUtil.generateAccount().getAddress();
		Object[] init_balances = { 0};
		for (Object init_balance : init_balances) {
			List opers = TxUtil.operCreateAccount(type, dest_add, init_balance, account_metadata);	//ledger发行未初始化资产
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 100, "init_balance[" + init_balance
					+ "]校验失败");
		}
	}
	/*
	 * 验证account_metadata字段校验正确
	 */
//	@Test
	public void account_metadataCheck() {
		Object dest_add = APIUtil.generateAccount().getAddress();
		Object[] account_metadatas = { -1, "z", "abc", "!@" };
		for (Object account_metadata : account_metadatas) {
			List opers = TxUtil.operCreateAccount(type, dest_add, init_balance, account_metadata); // ledger发行未初始化资产
			String result = SignUtil.unnormal_tx(opers, source_address, fee, metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 2, "account_metadata[" + account_metadata + "]校验失败");
		}
	}
	
//	@Test
	public void threshold_master_weightCheck(){
	Object low_threshold = 2;
	Object med_threshold = 2;
	Object high_threshold = 2;
		Object dest_add = APIUtil.generateAccount().getAddress();
		Object[] thresholds = {-1,256,"abc","!@#",""};
		for (Object master_weight : thresholds) {
			JSONObject threshold_json = TxUtil.threshold(master_weight, low_threshold, med_threshold, high_threshold);
			List opers = TxUtil.operCreateAccount(type, dest_add, init_balance,
					account_metadata, threshold_json); // ledger发行未初始化资产
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 2, "threshold_master_weight[" + master_weight
					+ "]校验失败");
		}
	}
	
//	@Test
	public void threshold_low_thresholdCheck(){
	Object master_weight = 2;
	Object med_threshold = 2;
	Object high_threshold = 2;
		Object dest_add = APIUtil.generateAccount().getAddress();
		Object[] thresholds = {-1,256,"abc","!@#",""};
		for (Object low_threshold : thresholds) {
			JSONObject threshold_json = TxUtil.threshold(master_weight, low_threshold, med_threshold, high_threshold);
			List opers = TxUtil.operCreateAccount(type, dest_add, init_balance,
					account_metadata, threshold_json); // ledger发行未初始化资产
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 2, "threshold_low_threshold[" + low_threshold
					+ "]校验失败");
		}
	}
	
//	@Test
	public void threshold_med_thresholdCheck(){
	Object master_weight = 2;
	Object low_threshold = 2;
	Object high_threshold = 2;
		Object dest_add = APIUtil.generateAccount().getAddress();
		Object[] thresholds = {-1,256,"abc","!@#",""};
		for (Object med_threshold : thresholds) {
			JSONObject threshold_json = TxUtil.threshold(master_weight, low_threshold, med_threshold, high_threshold);
			List opers = TxUtil.operCreateAccount(type, dest_add, init_balance,
					account_metadata, threshold_json); // ledger发行未初始化资产
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					 metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 2, "threshold_med_threshold[" + med_threshold
					+ "]校验失败");
		}
	}
	
//	@Test
	public void threshold_high_thresholdCheck(){
	Object master_weight = 2;
	Object low_threshold = 2;
	Object med_threshold = 2;
		Object dest_add = APIUtil.generateAccount().getAddress();
		Object[] thresholds = {-1,256,"abc","!@#",""};
		for (Object high_threshold : thresholds) {
			JSONObject threshold_json = TxUtil.threshold(master_weight, low_threshold, med_threshold, high_threshold);
			List opers = TxUtil.operCreateAccount(type, dest_add, init_balance,
					account_metadata, threshold_json); 
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 2, "threshold_high_threshold[" + high_threshold
					+ "]校验失败");
		}
	}
	
//	@Test
	public void signer_addressCheck() {
		Object master_weight = 2;
		Object low_threshold = 2;
		Object med_threshold = 2;
		Object high_threshold = 2;
		JSONObject threshold_json = TxUtil.threshold(master_weight, low_threshold, med_threshold, high_threshold);
		Object[] sign_adds = {-1,10,"abc","!@#","",null};
		Object dest_add = APIUtil.generateAccount().getAddress();
		for (Object sign_add : sign_adds) {
			int weight = 2;
			List signers = TxUtil.signers(sign_add, weight);
			List opers = TxUtil.operCreateAccount(type, dest_add, init_balance,
					account_metadata, threshold_json,signers); 
			String response = SignUtil.unnormal_tx(opers, source_address,
					fee, metadata, pri, pub);
			int error_code = Result.getErrorCode(response);
			check.assertEquals(error_code, 2, "创建账户signer_address["
					+ sign_add
					+ "]校验失败");
		}
	}
	
//	 @Test
		public void signer_weightCheck() {
		 Object master_weight = 2;
			Object low_threshold = 2;
			Object med_threshold = 2;
			Object high_threshold = 2;
			JSONObject threshold_json = TxUtil.threshold(master_weight, low_threshold, med_threshold, high_threshold);
			Object dest_add = APIUtil.generateAccount().getAddress();
			Object[] weights = { -1, "abc", "!@#", "", null };
			for (Object weight : weights) {
				Object address = TxUtil.createAccount().get("address");
				List signers = TxUtil.signers(address, weight);
				List opers = TxUtil.operCreateAccount(type, dest_add, init_balance,
						account_metadata, threshold_json,signers); 
				String response = SignUtil.unnormal_tx(opers, source_address,
						fee, metadata, pri, pub);
				//属性设置成功
				int error_code = Result.getErrorCode(response);
				check.assertEquals(error_code, 2, "创建账户signer_weight[" + weight
						+ "]校验失败");
			}
		}
	
//	 @Test
	@SuppressWarnings("rawtypes")
	public void normalFeeCheck(){

		int init_balance = 90000000;
		Object[] fees = { fee, fee + 1, fee * 10 };
		for (Object fee : fees) {
			Object dest_add = APIUtil.generateAccount().getAddress();
			List opers = TxUtil.operCreateAccount(type, dest_add, init_balance, account_metadata);	//ledger发行未初始化资产
			String result = SignUtil.tx(opers, led_acc, fee,metadata, led_pri, led_pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 0, "fee[" + fee + "]校验失败");
		}
	}
//	@Test
	public void unnormalFeeCheck(){
		Object dest_add = APIUtil.generateAddress();
		//base_fee = 1000
		Object[] fees = { -1, 0, 999, "abc", "!@#", "", null };
		for (Object fee : fees) {
			List opers = TxUtil.operCreateAccount(type, dest_add, init_balance, account_metadata);	//ledger发行未初始化资产
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					 metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertNotEquals(err_code, 0, "fee[" + fee + "]校验失败");
		}
	}
//	@Test
	public void source_addressCheck(){
		Object dest_add = APIUtil.generateAccount().getAddress();
		Object[] source_adds = {-1,10,"abc","!@#",""};
		for (Object source_add : source_adds) {
			List opers = TxUtil.operCreateAccount(type, dest_add, init_balance, account_metadata);	//ledger发行未初始化资产
			String result = SignUtil.unnormal_tx(opers, source_add, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertNotEquals(err_code, 0, "source_address[" + source_add
					+ "]校验失败");
		}
	}
	
	public void source_addressinValidCheck(){
		Object dest_add = APIUtil.generateAccount().getAddress();
			List opers = TxUtil.operCreateAccount(type, dest_add, init_balance, account_metadata);	//ledger发行未初始化资产
			String result = SignUtil.unnormal_tx(opers, led_acc, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertNotEquals(err_code, 0, "source_address[" + led_acc
					+ "]校验失败");
	}
	
	public void sequence_numberCheck(){
		Long sequence_number = Result.seq_num(source_address);
		Object dest_add = APIUtil.generateAccount().getAddress();
			List opers = TxUtil.operCreateAccount(type, dest_add, init_balance, account_metadata);	
			String result = SignUtil.unnormal_tx(opers, led_acc, fee,
					sequence_number+10, metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertNotEquals(err_code, 0, "source_address[" + led_acc
					+ "]校验失败");
	}
	
	public void feeCheck(){
		Object dest_add = APIUtil.generateAddress();
		Object[] fees = {-1,0,999,"abc","!@#","",null};
		for (Object fee : fees) {
			List opers = TxUtil.operCreateAccount(type, dest_add, init_balance, account_metadata);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertNotEquals(err_code, 0, "资产发行fee[" + fee + "]校验失败");
		}
	}
	
	public void private_keyCheck(){
		Object dest_add = APIUtil.generateAddress();
		Object pri1 = TxUtil.createAccount().get("private_key");
		Object pri2 = APIUtil.generateAcc().get("private_key");

		Object[] pri_keys = { pri1, pri2 };
		for (Object pri_key : pri_keys) {
			String pri = pri_key.toString();
			List opers = TxUtil.operCreateAccount(type, dest_add, init_balance, account_metadata);	//ledger发行未初始化资产
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertNotEquals(err_code, 0, "private_key[" + pri_key
					+ "]校验失败");
		}
	}
	
	// @Test
	public void checkResult(){
		Map acc = TxUtil.createAccount();
		Object address = acc.get("address");
		String accinfo = Result.getAccInfo(address);
		String balance = Result.getBalanceInResponse(accinfo);
		int metadata_version = Result.getMetadata_version(address);
		check.assertEquals(metadata_version, 1, "新账户的metadata_version错误");

	}
	
	public void opera_metadataCheck(){
		Object dest_add = APIUtil.generateAccount().getAddress();
		Object[] metadatas = {-1,0,999,"abc","!@#"};
		for (Object metadata_ : metadatas) {
			List opers = TxUtil.operCreateAccount1(type, dest_add, init_balance, account_metadata,metadata_);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 2, "operation metadata[" + metadata_ + "]校验失败");
			
		}
		
	}
//	@Test
	public void opera_sourceaddressCheck(){
		Object dest_add = APIUtil.generateAccount().getAddress();
		
		Object[] sourceadds = {-1,0,999,"abc","!@#",""};
		for (Object sourceadd : sourceadds) {
			List opers = TxUtil.operCreateAccount1(type, dest_add, init_balance, account_metadata,sourceadd);
			String result = SignUtil.unnormal_tx(opers, sourceadd.toString(), fee,
					 metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 2, "operation source_address[" + sourceadd + "]校验失败");
			
		}
	}
}
