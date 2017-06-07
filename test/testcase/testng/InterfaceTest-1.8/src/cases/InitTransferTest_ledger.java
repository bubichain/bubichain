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
//初始化转账测试
public class InitTransferTest_ledger extends TestBase{
	String geturl = get_Url2;
	@SuppressWarnings("rawtypes")
	Object dest_address = TxUtil.createAccount1().get("address");
	
	Map acc = TxUtil.createAccount1();
	Object source_address = acc.get("address");
	String pri = acc.get("private_key").toString();
	Object pub = acc.get("public_key");
	Object type = 5;
	Object asset_type = 1;
	Object asset_code = "abc";
	Object asset_issuer = source_address;
	int asset_amount = 10;
	String metadata = "abcd";
	long sequence_number = Result.seq_num(geturl,source_address);
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void inittransferCheck(){
		// 1.资产发行，无details
		int type_ = 2;
		List opers = TxUtil.operIssue(type_, asset_type, source_address,
				asset_code, asset_amount);
		long sequence_number = Result.seq_num(source_address);
		String result = SignUtil.tx(opers, source_address, fee,
				sequence_number, metadata, pri, pub);
		//2.初始化转账
		if (Result.getErrorCode(geturl, result)==0) {
			List opers_ = TxUtil.operInitTransfer(type, asset_type, dest_address,
					asset_amount, asset_issuer, asset_code);
			sequence_number = Result.seq_num(source_address);
			String result_ = SignUtil.tx(opers_, source_address, fee,
					sequence_number, metadata, pri, pub);
			// System.out.println("初始化转账：" + result_);
			int err_code_ = Result.getErrorCode(geturl,result_);
			check.assertEquals(err_code_, 0, "初始化转账校验失败");
		}
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void dest_addressCheck(){

		Object[] dest_adds = {0,-1,"abc","!@#","",null};
		for (Object dest_address : dest_adds) {
			List opers = TxUtil.operInitTransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code);
			sequence_number = Result.seq_num(source_address);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					sequence_number, metadata, pri, pub);
			int err_code = Result.getErrorCode(geturl,result);
			check.assertNotEquals(err_code, 0, "初始化转账dest_address["
					+ dest_address
					+ "]校验失败");
		}
		
	}

	/**
	 * 初始化转账给不存在的目标账户，目标账户不会被创建 这个功能在2.0上去掉了，所以修改测试用例-20161206
	 */
//	@Test
	public void NonDestAddressCheck() {
		System.out.println("=====如果账户未被创建，初始化转账不会自动创建账户=====");
		Object dest_a = APIUtil.generateAcc().get("address");
		List opers = TxUtil.operInitTransfer(type, asset_type, dest_a,
				asset_amount, asset_issuer, asset_code);
		sequence_number = Result.seq_num(source_address);
		String result = SignUtil.unnormal_tx(opers, source_address, fee,
				sequence_number, metadata, pri, pub);
		int err_code = Result.getErrorCode(geturl,result);
		check.assertNotEquals(err_code, 0, "初始化转账给未创建的账户转账校验失败");
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void asset_issuerCheck(){

		Object dest_a = APIUtil.generateAcc().get("address");
		Object[] asset_issuers = {-1,"abc","!@#","",null,dest_a};
		for (Object asset_issuer : asset_issuers) {
			sequence_number = Result.seq_num(source_address);
			List opers = TxUtil.operInitTransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					sequence_number, metadata, pri, pub);
			int err_code = Result.getErrorCode(geturl,result);
			check.assertNotEquals(err_code, 0, "初始化转账asset_issuer["
					+ asset_issuer
					+ "]校验失败");
		}
		
	}
//	@Test
	public void asset_issuerNoneCheck(){
		//issue 
		Object dest_a = APIUtil.generateAcc().get("address");
		Object asset_issuer = null;
			sequence_number = Result.seq_num(source_address);
			List opers = TxUtil.operInitTranNoissuer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					sequence_number, metadata, pri, pub);
			int err_code = Result.getErrorCode(geturl,result);
			check.assertEquals(err_code, 2, "初始化转账asset_issuer["
					+ asset_issuer
					+ "]校验失败");
		
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void asset_codeCheck(){

		Object[] asset_codes = {-1,0,"qq","",null};
		for (Object asset_code : asset_codes) {
			sequence_number = Result.seq_num(source_address);
			List opers = TxUtil.operInitTransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					sequence_number, metadata, pri, pub);
			int err_code = Result.getErrorCode(geturl,result);
			check.assertNotEquals(err_code, 0, "初始化转账asset_code[" + asset_code
					+ "]校验失败");
		}
		
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void asset_amountCheck(){
		sequence_number = Result.seq_num(source_address);
		Object[] asset_amounts = {-1,0,"abc","!@#","",null,1000000};
		for (Object asset_amount : asset_amounts) {
			List opers = TxUtil.operInitTransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					sequence_number, metadata, pri, pub);
			int err_code = Result.getErrorCode(geturl,result);
			check.assertNotEquals(err_code, 0, "初始化转账asset_amount["
					+ asset_amount
					+ "]校验失败");
		}
		
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void details_amountCheck(){
		Object start = 0;
		Object length = 31536000;
		Object ext = "abcd";
		Object[] amounts = {-1,0,"abc","!@#","",null,600000};
		for (Object amount : amounts) {
			sequence_number = Result.seq_num(source_address);
			List details = TxUtil.details(amount, start, length, ext);
			List opers = TxUtil.operInitTransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code,details);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					sequence_number, metadata, pri, pub);
			int err_code = Result.getErrorCode(geturl,result);
			check.assertNotEquals(err_code, 0, "初始化转账details_amount校验失败");
		}
		
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void details_startCheck(){
		Object length = 31536000;
		Object ext = "abcd";
		int amount = 10;
		long s = System.currentTimeMillis()/1000-10000 ;
		Object[] detatils_starts = {-1,"",null,"abc","!@#",s};
		for (Object start : detatils_starts) {
			sequence_number = Result.seq_num(source_address);
			List details = TxUtil.details(amount, start, length, ext);
			List opers = TxUtil.operInitTransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code,details);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					sequence_number, metadata, pri, pub);
			int err_code = Result.getErrorCode(geturl,result);
			check.assertNotEquals(err_code, 0, "初始化转账details_start失败");
		}
		
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void details_lengthCheck(){
		
		Object ext = "abcd";
		int start = 0;
		int amount = 10;
		Object[] detatils_lengths = {-2,"",null,"abc","!@#"};
		for (Object length : detatils_lengths) {
			sequence_number = Result.seq_num(source_address);
			List details = TxUtil.details(amount, start, length, ext);
			List opers = TxUtil.operInitTransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code,details);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					sequence_number, metadata, pri, pub);
			int err_code = Result.getErrorCode(geturl,result);
			check.assertNotEquals(err_code, 0, "初始化转账details_length校验失败");
		}
		
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void details_extCheck(){
		
		int start = 0;
		int amount = 10;
		int length = 31536000;
		Object[] detatils_exts = {-2,"",null,"1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef"};
		for (Object ext : detatils_exts) {
			sequence_number = Result.seq_num(source_address);
			List details = TxUtil.details(amount, start, length, ext);
			List opers = TxUtil.operInitTransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code,details);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					sequence_number, metadata, pri, pub);
			int err_code = Result.getErrorCode(geturl,result);
			check.assertNotEquals(err_code, 0, "初始化转账details_ext校验失败");
		}
		
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void feeCheck(){
		Object[] fees = {-1,0,999,"abc","!@#","",null};
		for (Object fee : fees) {
			sequence_number = Result.seq_num(geturl,source_address);
			List opers = TxUtil.operInitTransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					sequence_number, metadata, pri, pub);
			int err_code = Result.getErrorCode(geturl,result);
			check.assertNotEquals(err_code, 0, "初始化转账fee校验失败");
		}
		
	}
	
	// @Test
	@SuppressWarnings("rawtypes")
	public void source_addressCheck(){
		
		String address = APIUtil.generateAcc().get("address");
		Object[] source_adds = {-1,0,"abc","!@#","",null,address};
		for (Object source_address : source_adds) {
			List opers = TxUtil.operInitTransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code);
			sequence_number = Result.seq_num(geturl,source_address);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					sequence_number, metadata, pri, pub);
			int err_code = Result.getErrorCode(geturl,result);
			check.assertNotEquals(err_code, 0, "初始化转账source_address校验失败");
		}

	}
	
	// @Test
	@SuppressWarnings("rawtypes")
	public void private_keyCheck(){
		Object dest_add = APIUtil.generateAddress();
		Object pri1 = TxUtil.createAccount1().get("private_key");
		Object pri2 = APIUtil.generateAcc().get("private_key");

		Object[] pri_keys = { pri1, pri2 };
		// Object[] pri_keys = {-1,10,"abc","!@#","",null,pri};
		for (Object private_key : pri_keys) {
			String pri = private_key.toString();
			List opers = TxUtil.operInitTransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code);
			sequence_number = Result.seq_num(geturl,source_address);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					sequence_number, metadata, pri, pub);
			int err_code = Result.getErrorCode(geturl,result);
			check.assertNotEquals(err_code, 0, "初始化转账private_key校验失败");
		}

	}
}
