package cases;

import java.util.List;
import java.util.Map;

import org.testng.annotations.Test;

import utils.APIUtil;
import utils.Result;
import utils.SignUtil;
import utils.TxUtil;
import base.TestBase;

@Test
public class TransferTest_ledger extends TestBase {
	String geturl = get_Url2;
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
	long sequence_number = Result.seq_num(source_address);
	String metadata = "1234";

//	 @Test
	@SuppressWarnings("rawtypes")
	public void transferCheck(){
		sequence_number = Result.seq_num(source_address);
		List opers = TxUtil.opertransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code);
		String result = SignUtil.tx(opers, source_address, fee, sequence_number, metadata, pri, pub);
		int err_code = Result.getErrorCode(geturl,result);
		check.assertEquals(err_code, 0, "转账校验失败");
		
	}
	
	public void oper_sourceaddressCheck(){
		sequence_number = Result.seq_num(source_address);
		List opers = TxUtil.opertransfer1(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code,source_address);
		String result = SignUtil.tx(opers, source_address, fee, sequence_number, metadata, pri, pub);
		int err_code = Result.getErrorCode(geturl,result);
		check.assertEquals(err_code, 0, "转账校验失败");
		
	}
	
	
//	@Test
	public void opera_sourceaddressCheck(){
		sequence_number = Result.seq_num(address);
		String asset_code = "abc" ;
		int asset_amount = 10;
		Object[] sourceadds = {-1,0,999,"abc","!@#","",null};
		for (Object sourceadd : sourceadds) {
			List opers = TxUtil.opertransfer(type, sourceadd,asset_type, dest_address, asset_amount, asset_issuer, asset_code);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					sequence_number, metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 4, "operation source_address[" + sourceadd + "]校验失败");
			
		}
		
	}

	@Test
	@SuppressWarnings("rawtypes")
	public void dest_addressCheck(){
		//check ERRCODE_INVALID_ADDRESS
		sequence_number = Result.seq_num(source_address);
		Object dest_add = "q";
			List opers = TxUtil.opertransfer(type, asset_type, dest_add, asset_amount, asset_issuer, asset_code);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					sequence_number, metadata, pri, pub);
			int err_code = Result.getErrorCode(geturl,result);
			check.assertEquals(err_code, 4, "转账dest_address[" + dest_add
					+ "]校验失败");
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void dest_addressEqualSourceAddCheck(){
		//check ERRCODE_ACCOUNT_SOURCEDEST_EQUAL
		sequence_number = Result.seq_num(source_address);
		Object dest_add = source_address;
			List opers = TxUtil.opertransfer(type, asset_type, dest_add, asset_amount, asset_issuer, asset_code);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					sequence_number, metadata, pri, pub);
			int err_code = Result.getErrorCode(geturl,result);
			check.assertEquals(err_code, 4, "转账dest_address[" + dest_add
					+ "]校验失败");
	}
//	@Test
	public void oper_sourceAddinValidCheck(){
		//check operation_.source_address()
		sequence_number = Result.seq_num(source_address);
		Object dest_add = TxUtil.createAccount().get("address");
		Object sour_add = APIUtil.generateAddress();
		List opers = TxUtil.opertransfer(type, sour_add,asset_type, dest_add, asset_amount, asset_issuer, asset_code);
		String result = SignUtil.unnormal_tx(opers, source_address, fee,
				sequence_number, metadata, pri, pub);
		int err_code = Result.getErrorCode(geturl,result);
		check.assertEquals(err_code, 4, "转账operation_source_address[" + sour_add
				+ "]校验失败");
	}
//	@Test
	public void accountBalanceCheck(){
		//check ERRCODE_ACCOUNT_LOW_RESERVE
		String asset_amount = "1000000000000";
		sequence_number = Result.seq_num(source_address);
		Object dest_add = TxUtil.createAccount().get("address");
		List opers = TxUtil.opertransfer(type, asset_type, dest_add, asset_amount, asset_issuer, asset_code);
		String result = SignUtil.unnormal_tx(opers, source_address, fee,
				sequence_number, metadata, pri, pub);
		int err_code = Result.getErrorCode(geturl,result);
		check.assertEquals(err_code, 100, "转账dest_address[" + dest_add
				+ "]校验失败");
		
	}
//	@Test
	public void oper_sourceAddValidCheck(){
		//check operation_.source_address()
		sequence_number = Result.seq_num(source_address);
		Object dest_add = TxUtil.createAccount().get("address");
		List opers = TxUtil.opertransfer(type, source_address,asset_type, dest_add, asset_amount, asset_issuer, asset_code);
		String result = SignUtil.unnormal_tx(opers, source_address, fee,
				sequence_number, metadata, pri, pub);
		int err_code = Result.getErrorCode(geturl,result);
		check.assertEquals(err_code, 0, "转账dest_address[" + dest_add
				+ "]校验失败");
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void asset_typeCheck(){
		//check ERRCODE_ASSET_INVALID
		sequence_number = Result.seq_num(source_address);
		int asset_type = 2;
			List opers = TxUtil.opertransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					sequence_number, metadata, pri, pub);
			int err_code = Result.getErrorCode(geturl,result);
			check.assertEquals(err_code, 4, "转账asset_type[" + asset_type
					+ "]校验失败");
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void asset_issuerCheck(){
		int asset_type = 1;
		sequence_number = Result.seq_num(source_address);
		Object[] asset_issuers = {-1,"abc","!@#","",null};
		for (Object asset_issuer : asset_issuers) {
			List opers = TxUtil.opertransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					sequence_number, metadata, pri, pub);
			int err_code = Result.getErrorCode(geturl,result);
			check.assertEquals(err_code, 4, "转账asset_issuer[" + asset_issuer
					+ "]校验失败");
		}
	}
//	@Test
	public void asset_issuerNotEqualSourceAddCheck(){
		int asset_type = 1;
		sequence_number = Result.seq_num(source_address);
			List opers = TxUtil.opertransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					sequence_number, metadata, pri, pub);
			int err_code = Result.getErrorCode(geturl,result);
			check.assertEquals(err_code, 104, "转账asset_issuer[" + asset_issuer
					+ "]校验失败");
	}
	
//	@Test
	public void asset_codeinValidCheck(){
		int asset_type = 1;
		sequence_number = Result.seq_num(source_address);
//		Object[] asset_codes = {-1,0,"qqqqqqqq"};  //这几种类型没有做校验，checkvalid是校验通过的
		Object[] asset_codes = {"",null};
		for (Object asset_code : asset_codes) {
			List opers = TxUtil.opertransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					sequence_number, metadata, pri, pub);
			int err_code = Result.getErrorCode(geturl,result);
			check.assertEquals(err_code, 4, "转账asset_code[" + asset_code
					+ "]校验失败");
		}
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void asset_amountinValidCheck(){
		String balance = Result.getBalanceInAcc(source_address);
		Object asset_amount = "-1";
//		Object asset_amount = balance;
			List opers = TxUtil.opertransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code);
			sequence_number = Result.seq_num(source_address);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					sequence_number, metadata, pri, pub);
			int err_code = Result.getErrorCode(geturl,result);
			check.assertEquals(err_code, 4, "转账asset_amount[" + asset_amount
					+ "]失败");
	}
//	@Test
	public void asset_amountNotEnoughCheck(){
		sequence_number = Result.seq_num(source_address);
		String balance = Result.getBalanceInAcc(source_address);
			List opers = TxUtil.opertransfer(type, asset_type, dest_address, balance, asset_issuer, asset_code);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					sequence_number, metadata, pri, pub);
			int err_code = Result.getErrorCode(geturl,result);
			check.assertEquals(err_code, 100, "转账asset_amount[" + asset_amount
					+ "]失败");
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void details_amountCheck(){
		sequence_number = Result.seq_num(source_address);
		Object start = 0;
		Object length = 31536000;
		Object ext = "abcd";
		int asset_type = 1;
		Object[] amounts = {-1,0,"abc","!@#","",null,6};
		for (Object amount : amounts) {
			List details = TxUtil.details(amount, start, length, ext);
			List opers = TxUtil.opertransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code,details);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					sequence_number, metadata, pri, pub);
			int err_code = Result.getErrorCode(geturl,result);
			check.assertEquals(err_code, 4, "转账details_amount[" + amount
					+ "]校验失败");
		}
		
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void details_startCheck(){
		sequence_number = Result.seq_num(source_address);
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
					sequence_number, metadata, pri, pub);
			int err_code = Result.getErrorCode(geturl,result);
			check.assertEquals(err_code, 4, "转账details_start[" + start
					+ "]校验失败");
		}
		
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void details_lengthCheck(){
		sequence_number = Result.seq_num(source_address);
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
					sequence_number, metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 4, "转账detatils_length[" + length
					+ "]校验失败");
		}
		
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void details_extCheck(){
		sequence_number = Result.seq_num(source_address);
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
					sequence_number, metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 4, "转账detatils_ext[" + ext
					+ "]校验失败");
		}
		
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void feeCheck(){
		sequence_number = Result.seq_num(source_address);
		Object[] fees = {-1,0,999,"abc","!@#","",null};
		for (Object fee : fees) {
			List opers = TxUtil.opertransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					sequence_number, metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 4, "转账fee[" + fee + "]校验失败");
		}
		
	}
	
//	@Test(priority = 4)
	@SuppressWarnings("rawtypes")
	public void source_addressCheck(){
		sequence_number = Result.seq_num(source_address);
		String address = APIUtil.generateAcc().get("address");
		Object[] source_adds = {-1,0,"abc","!@#","",null,address};
		for (Object source_address : source_adds) {
			List opers = TxUtil.opertransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					sequence_number, metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 4, "转账source_address["
					+ source_address
					+ "]校验失败");
		}
		
	}
	
//	@Test(priority = 5)
	@SuppressWarnings("rawtypes")
	public void private_keyCheck(){
		sequence_number = Result.seq_num(source_address);
		Object pri1 = TxUtil.createAccount().get("private_key");
		Object pri2 = APIUtil.generateAcc().get("private_key");

		Object[] pri_keys = { pri1, pri2 };
		// Object[] pri_keys = {-1,10,"abc","!@#","",null,pri};
		for (Object private_key : pri_keys) {
			String pri = private_key.toString();
			List opers = TxUtil.opertransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					sequence_number, metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 4, "转账private_key[" + pri + "]校验失败");
			
		}
		check.result("转账交易校验成功");
	}
}
