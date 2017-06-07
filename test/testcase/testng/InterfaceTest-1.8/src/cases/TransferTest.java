package cases;

import java.util.List;
import java.util.Map;

import org.testng.annotations.Test;

import utils.APIUtil;
import utils.Result;
import utils.SignUtil;
import utils.TxUtil;
import base.TestBase;
import model.Account;

@Test
public class TransferTest extends TestBase {
	
	@SuppressWarnings("rawtypes")
	Map acc = TxUtil.createAccount();
	Object address = acc.get("address");
	Map acc1 = TxUtil.createAccount();
	Object source_address = acc1.get("address");
	String pri = acc1.get("private_key").toString();
	Object pub = acc1.get("public_key");
	int type = 1;
	int asset_type = 0;
	Object dest_address = address;
	int asset_amount = 10;
	Object asset_issuer = source_address;
	Object asset_code = "abcd";
	String metadata = "1234";

	// @Test
	@SuppressWarnings("rawtypes")
	public void transferCheck(){
		Account srcAcc = TxUtil.createNewAccount();
		List opers = TxUtil.opertransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code);
		String result = SignUtil.tx(opers, srcAcc.getAddress(), fee, metadata, srcAcc.getPri_key(), srcAcc.getPub_key());
		int err_code = Result.getErrorCode(result);
		check.assertEquals(err_code, 0, "转账校验失败");
		
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void typeCheck(){
		Object[] types = {0,20,-1,"abc","!@#","",null};
		for (Object type : types) {
			List opers = TxUtil.opertransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 2, "转账type[" + type + "]校验失败");
		}
		
	}

//	@Test
	@SuppressWarnings("rawtypes")
	public void dest_addressCheck(){
		Object[] dest_adds = {0,-1,"abc","!@#","",null};
		for (Object dest_add : dest_adds) {
			List opers = TxUtil.opertransfer(type, asset_type, dest_add, asset_amount, asset_issuer, asset_code);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					 metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 2, "转账dest_address[" + dest_add
					+ "]校验失败");
		}
		
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void asset_typeCheck(){
		Object[] asset_types = {-1,"abc","!@#","",null};
		for (Object asset_type : asset_types) {
			List opers = TxUtil.opertransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 2, "转账asset_type[" + asset_type
					+ "]校验失败");
		}
		
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void asset_issuerCheck(){
		int asset_type = 1;
		Object[] asset_issuers = {-1,"abc","!@#","",null};
		for (Object asset_issuer : asset_issuers) {
			List opers = TxUtil.opertransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 2, "转账asset_issuer[" + asset_issuer
					+ "]校验失败");
		}
	}
	
//	@Test
	public void asset_issuerNotEqualSourceAddCheck() {
		Account srcAcc = TxUtil.createNewAccount();
		int asset_type = 1;
		List opers = TxUtil.opertransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code);
		String result = SignUtil.unnormal_tx(opers, srcAcc.getAddress(), fee, metadata, srcAcc.getPri_key(), srcAcc.getPub_key());
		int err_code = Result.getErrorCode(result);
		check.assertEquals(err_code, 104, "转账asset_issuer[" + asset_issuer + "]校验失败");
	}

//	@Test
	@SuppressWarnings("rawtypes")
	public void asset_codeCheck(){
		int asset_type = 1;
//		Object[] asset_codes = {-1,0,"qqqqqqqq"};  //这几种类型没有做校验，checkvalid是校验通过的
		Object[] asset_codes = {"",null};
		for (Object asset_code : asset_codes) {
			List opers = TxUtil.opertransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					 metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 2, "转账asset_code[" + asset_code
					+ "]校验失败");
		}
		
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void asset_amountinValidCheck(){
		String balance = Result.getBalanceInAcc(source_address);
		Object[] asset_amounts = {-1,0,"abc","!@#","",null,balance};
//		Object asset_amount = balance;
		for (Object asset_amount : asset_amounts) {
			List opers = TxUtil.opertransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 2, "转账asset_amount[" + asset_amount
					+ "]失败");
		}
	}
//	@Test
	public void asset_amountNotEnoughCheck(){
		String balance = Result.getBalanceInAcc(source_address);
			List opers = TxUtil.opertransfer(type, asset_type, dest_address, balance, asset_issuer, asset_code);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 2, "转账asset_amount[" + asset_amount
					+ "]失败");
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void details_amountCheck(){
		Object start = 0;
		Object length = 31536000;
		Object ext = "abcd";
		int asset_type = 1;
		Object[] amounts = {-1,0,"abc","!@#","",null,6};
		for (Object amount : amounts) {
			List details = TxUtil.details(amount, start, length, ext);
			List opers = TxUtil.opertransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code,details);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertNotEquals(err_code, 0, "转账details_amount[" + amount
					+ "]校验失败");
		}
		
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void details_startCheck(){
		Object length = 31536000;
		Object ext = "abcd";
		int amount = 10;
		int asset_type = 1;
		// long s = System.currentTimeMillis()/1000-10000 ;
		//预期失败
		Object[] detatils_starts = { -1, "", null, "abc", "!@#" };
		for (Object start : detatils_starts) {
			List details = TxUtil.details(amount, start, length, ext);
			List opers = TxUtil.opertransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code,details);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertNotEquals(err_code, 0, "转账details_start[" + start
					+ "]校验失败");
		}
		
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void details_lengthCheck(){
		Object ext = "abcd";
		int start = 0;
		int amount = 10;
		int asset_type = 1;
		//预期失败
		Object[] detatils_lengths = {-2,"",null,"abc","!@#"};
		for (Object length : detatils_lengths) {
			List details = TxUtil.details(amount, start, length, ext);
			List opers = TxUtil.opertransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code,details);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					 metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertNotEquals(err_code, 0, "转账detatils_length[" + length
					+ "]校验失败");
		}
		
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void details_extCheck(){
		int start = 0;
		int amount = 10;
		int length = 31536000;
		int asset_type = 1;
		//预期失败
		Object[] detatils_exts = {-2,"",null,"1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef"};
		for (Object ext : detatils_exts) {
			List details = TxUtil.details(amount, start, length, ext);
			List opers = TxUtil.opertransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code,details);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					 metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertNotEquals(err_code, 0, "转账detatils_ext[" + ext
					+ "]校验失败");
		}
		
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void feeCheck(){
		Object[] fees = {-1,0,999,"abc","!@#","",null};
		for (Object fee : fees) {
			List opers = TxUtil.opertransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertNotEquals(err_code, 0, "转账fee[" + fee + "]校验失败");
		}
		
	}
	
//	@Test(priority = 4)
	@SuppressWarnings("rawtypes")
	public void source_addressCheck(){
		String address = APIUtil.generateAcc().get("address");
		Object[] source_adds = {-1,0,"abc","!@#",address};
		for (Object source_address : source_adds) {
			List opers = TxUtil.opertransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertNotEquals(err_code, 0, "转账source_address["
					+ source_address
					+ "]校验失败");
		}
		
	}
	
//	@Test(priority = 5)
	@SuppressWarnings("rawtypes")
	public void private_keyCheck(){
		Object pri1 = TxUtil.createAccount().get("private_key");
		Object pri2 = APIUtil.generateAcc().get("private_key");

		Object[] pri_keys = { pri1, pri2 };
		// Object[] pri_keys = {-1,10,"abc","!@#","",null,pri};
		for (Object private_key : pri_keys) {
			String pri = private_key.toString();
			List opers = TxUtil.opertransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					 metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertNotEquals(err_code, 0, "转账private_key[" + pri + "]校验失败");
			
		}
	}
}
