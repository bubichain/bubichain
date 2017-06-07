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
public class InitTransferTest extends TestBase{

	Object dest_address = TxUtil.createAccount().get("address");
	@SuppressWarnings("rawtypes")
	Map acc = TxUtil.createAccount();
	Object source_address = acc.get("address");
	String pri = acc.get("private_key").toString();
	Object pub = acc.get("public_key");
	Object type = 5;
	Object asset_type = 1;
	Object asset_code = "abc";
	Object asset_issuer = source_address;
	int asset_amount = 10;
	String metadata = "abcd";
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void inittransferCheck(){
		// 1.资产发行，无details
		int type_ = 2;
		List opers = TxUtil.operIssue(type_, asset_type, source_address,
				asset_code, asset_amount);
		String result = SignUtil.tx(opers, source_address, fee,
				metadata, pri, pub);
		//2.初始化转账
		if (Result.getErrorCode(result)==0) {
			List opers_ = TxUtil.operInitTransfer(type, asset_type, dest_address,
					asset_amount, asset_issuer, asset_code);
			String result_ = SignUtil.tx(opers_, source_address, fee,
					metadata, pri, pub);
			int err_code_ = Result.getErrorCode(result_);
			check.assertEquals(err_code_, 0, "初始化转账校验失败");
		}
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void typeCheck(){
		Object[] types = {20,-1,"abc","!@#","",null};
		for (Object type : types) {
			List opers = TxUtil.operInitTransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertNotEquals(err_code, 0, "初始化转账type[" + type + "]校验失败");
		}
//		check.result("初始化转账校验成功");
	}

//	@Test
	@SuppressWarnings("rawtypes")
	public void dest_addressCheck(){

		Object[] dest_adds = {0,-1,"abc","!@#","",null};
		for (Object dest_address : dest_adds) {
			List opers = TxUtil.operInitTransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertNotEquals(err_code, 0, "初始化转账dest_address["
					+ dest_address
					+ "]校验失败");
		}
		
	}

	/**
	 * 初始化转账给不存在的目标账户，目标账户不会被创建 这个功能在2.0上去掉了，所以修改测试用例-20161206
	 */
	public void NonDestAddressCheck() {
		System.out.println("=====如果账户未被创建，初始化转账不会自动创建账户=====");
		Object dest_a = APIUtil.generateAcc().get("address");
		List opers = TxUtil.operInitTransfer(type, asset_type, dest_a,
				asset_amount, asset_issuer, asset_code);
		String result = SignUtil.unnormal_tx(opers, source_address, fee,
				 metadata, pri, pub);
		int err_code = Result.getErrorCode(result);
		check.assertNotEquals(err_code, 0, "初始化转账给未创建的账户转账校验失败");
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void asset_typeCheck(){
		Object[] asset_types = {-1,"abc","!@#","",null};
		for (Object asset_type : asset_types) {
			List opers = TxUtil.operInitTransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertNotEquals(err_code, 0, "初始化转账asset_type[" + asset_type
					+ "]校验失败");
		}
		
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void asset_issuerCheck(){

		Object dest_a = APIUtil.generateAcc().get("address");
		Object[] asset_issuers = {-1,"abc","!@#","",null,dest_a};
		for (Object asset_issuer : asset_issuers) {
			List opers = TxUtil.operInitTransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertNotEquals(err_code, 0, "初始化转账asset_issuer["
					+ asset_issuer
					+ "]校验失败");
		}
		
	}
//	@Test
	public void asset_issuerNoneCheck(){
		//issue 
		Object asset_issuer = null;
			List opers = TxUtil.operInitTranNoissuer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 2, "初始化转账asset_issuer["
					+ asset_issuer
					+ "]校验失败");
		
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void asset_codeCheck(){

		Object[] asset_codes = {-1,0,"qq","",null};
		for (Object asset_code : asset_codes) {
			List opers = TxUtil.operInitTransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertNotEquals(err_code, 0, "初始化转账asset_code[" + asset_code
					+ "]校验失败");
		}
		
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void asset_amountCheck(){
		Object[] asset_amounts = {-1,0,"abc","!@#","",null,1000000};
		for (Object asset_amount : asset_amounts) {
			List opers = TxUtil.operInitTransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
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
			List details = TxUtil.details(amount, start, length, ext);
			List opers = TxUtil.operInitTransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code,details);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
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
			List details = TxUtil.details(amount, start, length, ext);
			List opers = TxUtil.operInitTransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code,details);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
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
			List details = TxUtil.details(amount, start, length, ext);
			List opers = TxUtil.operInitTransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code,details);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
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
			List details = TxUtil.details(amount, start, length, ext);
			List opers = TxUtil.operInitTransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code,details);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertNotEquals(err_code, 0, "初始化转账details_ext校验失败");
		}
		
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void feeCheck(){
		Object[] fees = {-1,0,999,"abc","!@#","",null};
		for (Object fee : fees) {
			List opers = TxUtil.operInitTransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertNotEquals(err_code, 0, "初始化转账fee校验失败");
		}
		
	}
	
//	 @Test
	@SuppressWarnings("rawtypes")
	public void source_addressCheck(){
		
		String address = APIUtil.generateAcc().get("address");
		Object[] source_adds = {-1,0,"abc","!@#",address};
		for (Object source_address : source_adds) {
			List opers = TxUtil.operInitTransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertNotEquals(err_code, 0, "初始化转账source_address校验失败");
		}

	}
	
//	 @Test
	@SuppressWarnings("rawtypes")
	public void private_keyCheck(){
		Object pri1 = TxUtil.createAccount().get("private_key");
		Object pri2 = APIUtil.generateAcc().get("private_key");
		Object[] pri_keys = { pri1, pri2 };
		for (Object private_key : pri_keys) {
			String pri = private_key.toString();
			List opers = TxUtil.operInitTransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					 metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 4, "初始化转账private_key校验失败");
		}

	}
}
