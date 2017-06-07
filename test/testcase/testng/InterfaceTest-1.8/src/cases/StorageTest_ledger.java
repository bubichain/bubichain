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

//存证交易验证
public class StorageTest_ledger extends TestBase{
	String geturl = get_Url2;
	int type = 9;
	Map acc = TxUtil.createAccount();
	Object source_address = acc.get("address");
	String pri = acc.get("private_key").toString();
	Object pub = acc.get("public_key");
//	String s_address = ledger ;
//	String s_key = led_pri;
	String record_id = "1234";
	String record_ext = "1234";
	long sequence_number = Result.seq_num(source_address);
	String metadata = "1234";
	
	@Test
	public void storageCheck(){
		//发表原声明
		sequence_number = Result.seq_num(source_address);
		List opers = TxUtil.operStorage(type, record_id, record_ext);
		String response = SignUtil.tx(opers, source_address, fee, sequence_number, metadata, pri, pub);
		int error_code = Result.getErrorCode(geturl,response);
		check.assertEquals(error_code, 0, "存证交易出错");
	}

	// 重复发表原声明失败
	public void storageReCheck() {
		sequence_number = Result.seq_num(source_address);
		List opers = TxUtil.operStorage(type, record_id, record_ext);
		String response = SignUtil.tx(opers, source_address, fee,
				sequence_number, metadata, pri, pub);
		int error_code = Result.getErrorCode(geturl,response);
		check.assertEquals(error_code, 4, "原声明重复发起验证失败");
	}

	@Test
	public void appendCheck() {
		//验证追加声明成功
		Map acc = TxUtil.createAccount();
		Object source_address = acc.get("address");
		String pri = acc.get("private_key").toString();
		Object pub = acc.get("public_key");
		sequence_number = Result.seq_num(source_address);
		String record_id = "1234";
		String record_ext = "1234";
		Object record_address = source_address;
		String metadata = "1234";
		List opers = TxUtil.operStorage(type, record_id, record_ext); // 发表存证
		String response = SignUtil.tx(opers, source_address, fee,
				sequence_number, metadata, pri, pub);
		int error_code = Result.getErrorCode(geturl,response);
		if (error_code == 0) { // 发表存证成功后再进行追加操作
			sequence_number = Result.seq_num(source_address);
			List opers_ = TxUtil.operStorage(type, record_id, record_ext,
					record_address);
			String response_ = SignUtil.tx(opers_, source_address, fee,
					sequence_number, metadata, pri, pub);
			int error_code_ = Result.getErrorCode(geturl,response);
			check.assertEquals(error_code_, 0, "追加存证交易出错");
		} else {
			check.fail("发表存证失败");
		}

	}

//	@Test
//	public void typeCheck(){
//		sequence_number = Result.seq_num(source_address);
//		Object[] types = {-1,10,"abc","!@#","",null};
//		for (Object type : types) {
//			List opers = TxUtil.operStorage(type, record_id, record_ext);
//			String response = SignUtil.unnormal_tx(opers, source_address, fee,
//					sequence_number, metadata, pri, pub);
//			int error_code = Result.getErrorCode(geturl,response);
//			check.assertEquals(error_code, 4, "存证交易type[" + type + "]校验出错");
//		}
//	}
	
//	@Test
	public void recordIdCheck(){
		sequence_number = Result.seq_num(source_address);
		Object[] record_ids = {"0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef1","",null};
		for (Object record_id : record_ids) {
			List opers = TxUtil.operStorage(type, record_id, record_ext);
			String response = SignUtil.unnormal_tx(opers, source_address, fee,
					sequence_number, metadata, pri, pub);
			int error_code = Result.getErrorCode(geturl,response);
			check.assertEquals(error_code, 4, "存证交易record_id[" + record_id
					+ "]校验出错");
		}
	}
	

//	@Test
	public void recordExtCheck(){
		long sequence_number = Result.seq_num(source_address);
		Object[] record_exts = {-1,0,"abc","!@#","qq","",null};
		for (Object record_ext : record_exts) {
			List opers = TxUtil.operStorage(type, record_id, record_ext);
			String response = SignUtil.unnormal_tx(opers, source_address, fee,
					sequence_number, metadata, pri, pub);
			int error_code = Result.getErrorCode(geturl,response);
			check.assertEquals(error_code, 4, "存证交易record_ext[" + record_ext
					+ "]校验出错");
		}
	}
	
//	@Test
	public void recordAddressCheck(){
		long sequence_number = Result.seq_num(source_address);
		Object[] record_adds = {-1,0,"abc","!@#","12","",null};
		for (Object record_add : record_adds) {
			List opers = TxUtil.operStorage(type, record_id, record_ext,record_add);
			String response = SignUtil.unnormal_tx(opers, source_address, fee,
					sequence_number, metadata, pri, pub);
			int error_code = Result.getErrorCode(geturl,response);
			check.assertEquals(error_code, 4, "存证交易record_add[" + record_add
					+ "]校验出错");
		}
		
	}
	
//	@Test
	public void sourceAddressCheck(){
		long sequence_number = Result.seq_num(source_address);
		Object[] source_adds = {-1,10,"abc","!@#","",null};
		for (Object source_address : source_adds) {
			List opers = TxUtil.operStorage(type, record_id, record_ext);
			String response = SignUtil.unnormal_tx(opers, source_address, fee,
					sequence_number, metadata, pri, pub);
			int error_code = Result.getErrorCode(geturl,response);
			check.assertEquals(error_code, 4, "存证交易source_address["
					+ source_address
					+ "]校验出错");
		}
	}
	
//	@Test
	public void privateKeyCheck(){
		long sequence_number = Result.seq_num(source_address);
		Object pri1 = TxUtil.createAccount().get("private_key");
		Object pri2 = APIUtil.generateAcc().get("private_key");

		Object[] pri_keys = { pri1, pri2 };
		// Object[] pri_keys = {-1,10,"abc","!@#","",null};
		for (Object s_key : pri_keys) {
			String pri = s_key.toString();
			List opers = TxUtil.operStorage(type, record_id, record_ext);
			String response = SignUtil.unnormal_tx(opers, source_address, fee,
					sequence_number, metadata, pri, pub);
			int error_code = Result.getErrorCode(geturl,response);
			check.assertEquals(error_code, 4, "存证交易private_key[" + pri
					+ "]校验出错");
		}
		check.result("存证交易校验通过");
	}
}
