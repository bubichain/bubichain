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

@Test
public class CreateAccountTest_ledger extends TestBase{
	String geturl = get_Url2;
	@SuppressWarnings("rawtypes")
	int type = 0;
	String account_metadata = "abcd";
	String metadata = "abcd";
	
	String asset_code = "abcd" ;
	int asset_amount = 100;
	long sequence_number = 0;//Result.seq_num(address);
	
	Object source_address = led_acc;
	String pri = led_pri;
	String pub = led_pub;
	
	//普通的创建账户,用ledger创建

	@Test
	public void createaccount(){
		long sequence_number = Result.seq_num(geturl,source_address);
		Map acc_gen = APIUtil.generateAcc();
		Object dest_add = acc_gen.get("address");
		List opers = TxUtil.operCreateAccount(type, dest_add, init_balance, account_metadata);	//ledger发行未初始化资产
		String result = SignUtil.tx(opers, source_address, fee, sequence_number, metadata, pri, pub);
		int err_code = Result.getErrorCode(geturl,result);
		check.assertEquals(err_code, 0, "创建账户失败");
	}
	
	public void threshold_master_weightCheck(){
//		Object master_weight = 2;
		Object low_threshold = 2;
		Object med_threshold = 2;
		Object high_threshold = 2;
			sequence_number = Result.seq_num(source_address);
			Object dest_add = APIUtil.generateAddress();
			Object[] thresholds = {-1,256,"abc","!@#","",TxUtil.getUint16(999999999)};
			for (Object master_weight : thresholds) {
				JSONObject threshold_json = TxUtil.threshold(master_weight, low_threshold, med_threshold, high_threshold);
				List opers = TxUtil.operCreateAccount(type, dest_add, init_balance,
						account_metadata, threshold_json); // ledger发行未初始化资产
				String result = SignUtil.unnormal_tx(opers, source_address, fee,
						sequence_number, metadata, pri, pub);
				int err_code = Result.getErrorCode(result);
				check.assertEquals(err_code, 2, "threshold_master_weight[" + master_weight
						+ "]校验失败");
			}
		}
		
//		@Test
		public void threshold_low_thresholdCheck(){
		Object master_weight = 2;
//		Object low_threshold = 2;
		Object med_threshold = 2;
		Object high_threshold = 2;
			sequence_number = Result.seq_num(source_address);
			Object dest_add = APIUtil.generateAddress();
			Object[] thresholds = {-1,256,"abc","!@#","",TxUtil.getUint32(999999999)};
			for (Object low_threshold : thresholds) {
				JSONObject threshold_json = TxUtil.threshold(master_weight, low_threshold, med_threshold, high_threshold);
				List opers = TxUtil.operCreateAccount(type, dest_add, init_balance,
						account_metadata, threshold_json); // ledger发行未初始化资产
				String result = SignUtil.unnormal_tx(opers, source_address, fee,
						sequence_number, metadata, pri, pub);
				int err_code = Result.getErrorCode(result);
				check.assertEquals(err_code, 2, "threshold_low_threshold[" + low_threshold
						+ "]校验失败");
			}
		}
		
//		@Test
		public void threshold_med_thresholdCheck(){
		Object master_weight = 2;
		Object low_threshold = 2;
//		Object med_threshold = 2;
		Object high_threshold = 2;
			sequence_number = Result.seq_num(source_address);
			Object dest_add = APIUtil.generateAddress();
			Object[] thresholds = {-1,256,"abc","!@#","",TxUtil.getUint32(999999999)};
			for (Object med_threshold : thresholds) {
				JSONObject threshold_json = TxUtil.threshold(master_weight, low_threshold, med_threshold, high_threshold);
				List opers = TxUtil.operCreateAccount(type, dest_add, init_balance,
						account_metadata, threshold_json); // ledger发行未初始化资产
				String result = SignUtil.unnormal_tx(opers, source_address, fee,
						sequence_number, metadata, pri, pub);
				int err_code = Result.getErrorCode(result);
				check.assertEquals(err_code, 2, "threshold_med_threshold[" + med_threshold
						+ "]校验失败");
			}
		}
		
//		@Test
		public void threshold_high_thresholdCheck(){
		Object master_weight = 2;
		Object low_threshold = 2;
		Object med_threshold = 2;
//		Object high_threshold = 2;
			sequence_number = Result.seq_num(source_address);
			Object dest_add = APIUtil.generateAddress();
			Object[] thresholds = {-1,256,"abc","!@#","",TxUtil.getUint32(999999999)};
			for (Object high_threshold : thresholds) {
				JSONObject threshold_json = TxUtil.threshold(master_weight, low_threshold, med_threshold, high_threshold);
				List opers = TxUtil.operCreateAccount(type, dest_add, init_balance,
						account_metadata, threshold_json); 
				String result = SignUtil.unnormal_tx(opers, source_address, fee,
						sequence_number, metadata, pri, pub);
				int err_code = Result.getErrorCode(result);
				check.assertEquals(err_code, 2, "threshold_high_threshold[" + high_threshold
						+ "]校验失败");
			}
		}
		
		public void signer_weightCheck() {
			 Object master_weight = 2;
				Object low_threshold = 2;
				Object med_threshold = 2;
				Object high_threshold = 2;
				JSONObject threshold_json = TxUtil.threshold(master_weight, low_threshold, med_threshold, high_threshold);
				sequence_number = Result.seq_num(source_address);
				Object dest_add = APIUtil.generateAddress();
				Object[] weights = { -1, "abc", "!@#", "", null ,TxUtil.getUint16(999999999)};
				for (Object weight : weights) {
					Object address = TxUtil.createAccount().get("address");
					List signers = TxUtil.signers(address, weight);
					List opers = TxUtil.operCreateAccount(type, dest_add, init_balance,
							account_metadata, threshold_json,signers); 
					String response = SignUtil.unnormal_tx(opers, source_address,
							fee, sequence_number, metadata, pri, pub);
					//属性设置成功
					int error_code = Result.getErrorCode(response);
					check.assertEquals(error_code, 2, "创建账户signer_weight[" + weight
							+ "]校验失败");
				}
			}
		
		public void opera_sourceaddressvalidCheck(){
			Object dest_add = APIUtil.generateAddress();
			String asset_code = "abc" ;
			int asset_amount = 10;
			Map acc = TxUtil.createAccount();
			Object source_add = acc.get("address");
			String pri = acc.get("private_key").toString();
			Object pub = acc.get("public_key");
				List opers = TxUtil.operCreateAccount1(type, dest_add, init_balance,source_add);
				sequence_number = Result.seq_num(source_add);
				String result = SignUtil.unnormal_tx(opers, source_add, fee,
						sequence_number, metadata, pri, pub);
				int err_code = Result.getErrorCode(result);
				check.assertEquals(err_code, 0, "operation source_address[" + source_add + "]校验失败");
		}
		
		public void noDestAddandInitBalanceCheck(){
//			!js.isMember("dest_address")|| !js.isMember("init_balance")
			Object dest_add = APIUtil.generateAddress();
			String asset_code = "abc" ;
			int asset_amount = 10;
			Map acc = TxUtil.createAccount();
			Object source_add = acc.get("address");
			String pri = acc.get("private_key").toString();
			Object pub = acc.get("public_key");
				List opers = TxUtil.operCreateAccount1(type, source_add);
				sequence_number = Result.seq_num(source_add);
				String result = SignUtil.unnormal_tx(opers, source_add, fee,
						sequence_number, metadata, pri, pub);
				int err_code = Result.getErrorCode(result);
				check.assertEquals(err_code, 2, "operation source_address[" + source_add + "]校验失败");
		}
		
		public void opera_sourceaddressinvalidCheck(){
			Object dest_add = APIUtil.generateAddress();
			String asset_code = "abc" ;
			int asset_amount = 10;
			Object source_add = APIUtil.generateAddress();
				List opers = TxUtil.operCreateAccount1(type, dest_add, init_balance,source_add);
				sequence_number = Result.seq_num(source_add);
				String result = SignUtil.unnormal_tx(opers, source_add, fee,
						sequence_number, metadata, pri, pub);
				int err_code = Result.getErrorCode(result);
				check.assertEquals(err_code, 2, "operation source_address[" + source_add + "]校验失败");
		}
		
		public void lowBalanceCheck(){
//			Source address balance is low
//			sourceaccount_ptr_->GetAccountBalance() - base_reserve < operation_createaccount_.init_balance()
			Object dest_add = APIUtil.generateAddress();
			String asset_code = "abc" ;
			int asset_amount = 10;
			Map acc = TxUtil.createAccount();
			Object source_add = acc.get("address");
			String pri = acc.get("private_key").toString();
			Object pub = acc.get("public_key");
				List opers = TxUtil.operCreateAccount1(type, dest_add, Long.valueOf(Result.getBalanceInAcc(source_add))-Result.base_reserve-10000000,source_add);
				sequence_number = Result.seq_num(source_add);
				String result = SignUtil.unnormal_tx(opers, source_add, fee,
						sequence_number, metadata, pri, pub);
				int err_code = Result.getErrorCode(result);
				check.assertEquals(err_code, 2, "operation source_address[" + source_add + "]校验失败");
		}
	
@Test
//	public void typeCheck(){
//		
//		Object dest_add = APIUtil.generateAddress();
//		Object[] types = {-1,10,"abc","!@#","",null};
//		for (Object type : types) {
//			List opers = TxUtil.operCreateAccount(type, dest_add, init_balance, account_metadata);	//ledger发行未初始化资产
//			long sequence_number = Result.seq_num(geturl,source_address);
//			String result = SignUtil.unnormal_tx(opers, source_address, fee,
//					sequence_number, metadata, pri, pub);
//			int err_code = Result.getErrorCode(geturl,result);
//			check.assertEquals(err_code, 2, "type[" + type + "]校验失败");
//		}
//	}
	
//	@Test
	public void dest_addressCheck(){
		sequence_number = Result.seq_num(source_address);
		Object[] dest_adds = {-1,10,"abc","!@#",""};
		for (Object dest_add : dest_adds) {
			List opers = TxUtil.operCreateAccount(type, dest_add, init_balance, account_metadata);	//ledger发行未初始化资产
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					sequence_number, metadata, pri, pub);
			int err_code = Result.getErrorCode(geturl,result);
			check.assertEquals(err_code, 4, "dest_address[" + dest_add
					+ "]校验失败" + err_code);
		}
	}
//	@Test
	public void dest_addressEqualSourceAddCheck(){
		sequence_number = Result.seq_num(source_address);
		List opers = TxUtil.operCreateAccount(type, source_address, init_balance, account_metadata);	//ledger发行未初始化资产
		String result = SignUtil.unnormal_tx(opers, source_address, fee,
				sequence_number, metadata, pri, pub);
		int err_code = Result.getErrorCode(geturl,result);
		check.assertEquals(err_code, 4, "dest_address[" + source_address
				+ "]校验失败" + err_code);
	}
	
	@Test
	public void dest_addressEqualSourceAdd2Check(){
		String geturl = baseUrl;
//		LOG_ERROR("Destination address is equal source address");
//	    return(protocol::ERRCODE_ACCOUNT_SOURCEDEST_EQUAL);
		sequence_number = Result.seq_num(source_address);
		List opers = TxUtil.operCreateAccount(type, source_address, init_balance, account_metadata);	//ledger发行未初始化资产
		String result = SignUtil.unnormal_tx(opers, source_address, fee,
				sequence_number, metadata, pri, pub);
		int err_code = Result.getErrorCode(geturl,result);
		check.assertEquals(err_code, 4, "dest_address[" + source_address
				+ "]校验失败" + err_code);
	}
	
//	@Test
	public void dest_addressNotEqualSourceAddCheck(){
		Object dest_add = led_acc;
		sequence_number = Result.seq_num(source_address);
		List opers = TxUtil.operCreateAccount(type, dest_add, init_balance, account_metadata);	//ledger发行未初始化资产
		String result = SignUtil.unnormal_tx(opers, source_address, fee,
				sequence_number, metadata, pri, pub);
		int err_code = Result.getErrorCode(geturl,result);
		check.assertEquals(err_code, 4, "dest_address[" + dest_add
				+ "]校验失败" + err_code);
	}
	
//@Test
	public void init_balanceinValidCheck(){
		sequence_number = Result.seq_num(source_address);
		Object dest_add = APIUtil.generateAddress();
		Object[] init_balances = { -1, "abc", "!@#", ""};
		for (Object init_balance : init_balances) {
			List opers = TxUtil.operCreateAccount(type, dest_add, init_balance, account_metadata);	//ledger发行未初始化资产
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					sequence_number, metadata, pri, pub);
			int err_code = Result.getErrorCode(geturl,result);
			check.assertEquals(err_code,2, "init_balance[" + init_balance
					+ "]校验失败");
		}
		// check.result("init_balance校验完成");
	}
//@Test
	public void init_balanceNotEnoughCheck(){
		
		Object dest_add = APIUtil.generateAddress();
		Object[] init_balances = { 0,base_reserve-1000};//
		for (Object init_balance : init_balances) {
			sequence_number = Result.seq_num(source_address);
			List opers = TxUtil.operCreateAccount(type, dest_add, init_balance, account_metadata);	//ledger发行未初始化资产
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					sequence_number, metadata, pri, pub);
			int err_code = Result.getErrorCode(geturl,result);
			check.assertEquals(err_code,100, "init_balance[" + init_balance
					+ "]校验失败");
		}
	}
	
//	@Test
	public void account_metadataCheck(){
		Object dest_add = APIUtil.generateAddress();
		Object[] account_metadatas = {-1,"z","abc","!@"};
		for (Object account_metadata : account_metadatas) {
			sequence_number = Result.seq_num(source_address);
			List opers = TxUtil.operCreateAccount(type, dest_add, init_balance, account_metadata);	//ledger发行未初始化资产
			String result = SignUtil.unnormal_tx(opers, source_address, fee, sequence_number, metadata, pri, pub);
			int err_code = Result.getErrorCode(geturl,result);
			check.assertEquals(err_code, 2, "account_metadata["
					+ account_metadata + "]校验失败");
		}
}

}
