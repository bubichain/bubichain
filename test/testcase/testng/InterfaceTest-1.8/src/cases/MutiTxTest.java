package cases;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import org.testng.annotations.Test;

import net.sf.json.JSONObject;
import utils.APIUtil;
import utils.HttpPool;
import utils.Result;
import utils.SignUtil;
import utils.TxUtil;
import base.TestBase;

@Test
public class MutiTxTest extends TestBase{

	int tx_times = 200;
	Object source_address = led_acc;
	String pri = led_pri;
	Object pub = led_pub;
	/**
	 * 一个请求发送大于200个transaction，覆盖多线程处理的代码
	 *  创世账号发行资产
	 */
	@SuppressWarnings("rawtypes")
	public void mutiIssueCheck(){
		//tx_times
		List muti_tran = new ArrayList<>();
		for (int i = 0; i <tx_times; i++) {
			int type = 2;
			int asset_type = 1;
			Object asset_issuer = led_acc;
			String asset_code = "abc" ;
			int asset_amount = 10;
			long sequence_number = Result.seq_num(source_address);
			List operations = TxUtil.operIssue(type, asset_type, asset_issuer, asset_code, asset_amount);	
			JSONObject tran = TxUtil.transaction(source_address, operations, sequence_number+i, fee);
			JSONObject items_ = TxUtil.item(operations, tran, pri, pub);
			muti_tran.add(items_);
		}
		
		String result = TxUtil.muti_txPost(muti_tran);
		for (int j = 0; j < tx_times; j++) {
			int err_code = Result.getErrCodeFromPost(result,j);
			check.assertEquals(err_code, 0,"MutiTx_issue failed[" + j + "]");
		}
	}
	
	/**
	 * 发送2000个transaction
	 */
	public void mutiIssue_100Check(){
		int tx_times = 2000;
		List muti_tran = new ArrayList<>();
		for (int i = 0; i <tx_times; i++) {
			int type = 2;
			int asset_type = 1;
			Object asset_issuer = led_acc;
			String asset_code = "abc" ;
			int asset_amount = 10;
			long sequence_number = Result.seq_num(source_address);
			List operations = TxUtil.operIssue(type, asset_type, asset_issuer, asset_code, asset_amount);	
			JSONObject tran = TxUtil.transaction(source_address, operations, sequence_number+i, fee);
			JSONObject items_ = TxUtil.item(operations, tran, pri, pub);
			muti_tran.add(items_);
		}
		
		String result = TxUtil.muti_txPost(muti_tran);
		for (int j = 0; j < tx_times; j++) {
			int err_code = Result.getErrCodeFromPost(result,j);
			check.assertEquals(err_code, 0,"MutiTx_issue failed[" + j + "]");
		}
	}
	
	/**
	 * 发送2000个transaction
	 */
	public void mutiIssue_20000Check(){
		int tx_times = 20000;
		List muti_tran = new ArrayList<>();
		for (int i = 0; i <tx_times; i++) {
			int type = 2;
			int asset_type = 1;
			Object asset_issuer = led_acc;
			String asset_code = "abc" ;
			int asset_amount = 10;
			long sequence_number = Result.seq_num(source_address);
			List operations = TxUtil.operIssue(type, asset_type, asset_issuer, asset_code, asset_amount);	
			JSONObject tran = TxUtil.transaction(source_address, operations, sequence_number+i, fee);
			JSONObject items_ = TxUtil.item(operations, tran, pri, pub);
			muti_tran.add(items_);
		}
		
		String result = TxUtil.muti_txPost(muti_tran);
		for (int j = 0; j < tx_times; j++) {
			int err_code = Result.getErrCodeFromPost(result,j);
			check.assertEquals(err_code, 0,"MutiTx_issue failed[" + j + "]");
		}
	}
	
//	@Test
	public void mutiCreateCheck(){
		//tx_times
		List muti_tran = new ArrayList<>();
		for (int i = 0; i <tx_times; i++) {
			int type = 0; //创建账户
			String account_metadata = "abcd";
			long sequence_number = Result.seq_num(source_address);
			Map acc_gen = APIUtil.generateAcc();
			Object dest_add = acc_gen.get("address");
			List opers = TxUtil.operCreateAccount(type, dest_add, init_balance, account_metadata);	
			JSONObject tran = TxUtil.transaction(source_address, opers, sequence_number+i, fee);
			JSONObject items_ = TxUtil.item(opers, tran, pri, pub);
//			System.out.println(items_);
			muti_tran.add(items_);
//			System.out.println(i);
		}
		
		String result = TxUtil.muti_txPost(muti_tran);
//		System.out.println("=====");
		for (int j = 0; j < tx_times; j++) {
			int err_code = Result.getErrCodeFromPost(result,j);
			check.assertEquals(err_code, 0,"MutiTx_createAccount failed[" + j + "]");
		}
	}
	
//	@Test
	public void mutiTransferCheck(){
		List muti_tran = new ArrayList<>();
		Object dest_address = TxUtil.createAccount().get("address");
		for (int i = 0; i <tx_times; i++) {
			int type = 1; //转账
			int asset_type = 0;
			int asset_amount = 10;
			Object asset_issuer = source_address;
			Object asset_code = "abcd";
			long sequence_number = Result.seq_num(source_address);
			Map acc_gen = APIUtil.generateAcc();
			Object dest_add = acc_gen.get("address");
			List opers = TxUtil.opertransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code);
			JSONObject tran = TxUtil.transaction(source_address, opers, sequence_number+i, fee);
			JSONObject items_ = TxUtil.item(opers, tran, pri, pub);
			muti_tran.add(items_);
		}
		
		String result = TxUtil.muti_txPost(muti_tran);
		for (int j = 0; j < tx_times; j++) {
			int err_code = Result.getErrCodeFromPost(result,j);
			check.assertEquals(err_code, 0,"MutiTx_createAccount failed[" + j + "]");
		}
	}
}
