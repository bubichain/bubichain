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
public class UniIssueTransferTest extends TestBase{

	//转移唯一资产
	int type = 8;
	Map acc = TxUtil.createAccount(); // 创建一个账户，发行唯一资产
	Object source_address = acc.get("address");
	String pri = acc.get("private_key").toString();
	Object pub = acc.get("public_key");
	Object dest_address = TxUtil.createAccount().get("address");
	Object address = APIUtil.generateAcc().get("address");
	String asset_code = "abcd";
	Object asset_issuer = source_address;
//	long sequence_number = Result.seq_num(source_address);
	String metadata = "1234";
	String asset_detailed = "1234";

	@SuppressWarnings("rawtypes")
//	 @Test
	public void uniIssueTransferCheck() {
		Account srcAcc = TxUtil.createNewAccount();
		TxUtil.issueUnique(srcAcc, asset_code, asset_detailed);
		List opers = TxUtil.operUniIssueTransfer(type, dest_address, srcAcc.getAddress(), asset_code);
		String result = SignUtil.tx(opers, srcAcc.getAddress(), fee, metadata, srcAcc.getPri_key(),
				srcAcc.getPub_key());
		int err_code = Result.getErrorCode(result);
		check.assertEquals(err_code, 0, "转移唯一资产出错");
	}

//	 @Test
	public void uniIssueTransferwithoutIssueCheck() {
		 Map acc = TxUtil.createAccount(); // 创建一个账户，发行唯一资产
			Object source_address = acc.get("address");
			String pri = acc.get("private_key").toString();
			Object pub = acc.get("public_key");
		List opers = TxUtil.operUniIssueTransfer(type, dest_address,
				asset_issuer, asset_code);
		String result = SignUtil.tx(opers, source_address, fee,
				metadata, pri, pub);
		int err_code = Result.getErrorCode(result);
		check.assertEquals(err_code, 104, "没有唯一资产进行转账，校验失败");
	}

//	 @Test
	public void typeCheck(){
		Object[] types = {0,20,-1,"abc","!@#","",null};
		for (Object type : types) {
			List opers = TxUtil.operUniIssueTransfer(type, dest_address, asset_issuer, asset_code);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int error_code = Result.getErrorCode(result);
			check.assertNotEquals(error_code, 0, "转移资产type[" + type + "]校验出错");
			
		}
	}
	
//	@Test
	public void asset_typeCheck(){
		Object[] asset_types = {0,20,-1,"abc","!@#","",null};
		for (Object asset_type : asset_types) {
			List opers = TxUtil.operUniIssueTransfer(type, asset_type,dest_address, asset_issuer, asset_code);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 2, "转移唯一资产asset_type[" + asset_type + "]校验失败");
		}
	}
	
	public void dest_addressCheck(){
		Object[] dest_adds = {0,-1,"abc","!@#","",null};
		for (Object dest_address : dest_adds) {
			List opers = TxUtil.operUniIssueTransfer(type, dest_address, asset_issuer, asset_code);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					 metadata, pri, pub);
			int error_code = Result.getErrorCode(result);
			check.assertNotEquals(error_code, 0, "转移资产dest_address["
					+ dest_address
					+ "]校验出错");
		}
	}
//	@Test
	public void dest_addressEqualSourceAddCheck(){
		//验证唯一资产转账的目的地址不能是自己
		int type_ = 7; // 发行唯一资产的类型
		List opers_ = TxUtil.operUniIssue(type_, asset_issuer, asset_code,
				asset_detailed);
		String result_ = SignUtil.tx(opers_, source_address, fee,
				 metadata, pri, pub);
		int error_code = Result.getErrorCode(result_);
		if(error_code==0){
			List opers = TxUtil.operUniIssueTransfer(type, source_address, asset_issuer, asset_code);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 4, "转移资产dest_address["
					+ dest_address
					+ "]校验出错");
		}else {
			System.out.println("唯一资产发行失败");
		}
	}
	
	public void asset_issuerCheck(){
		Object[] asset_issuers = { -1, "abc", "!@#", "", null, address };
		for (Object asset_issuer : asset_issuers) {
			List opers = TxUtil.operUniIssueTransfer(type, dest_address, asset_issuer, asset_code);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int error_code = Result.getErrorCode(result);
			check.assertNotEquals(error_code, 0, "转移资产asset_issuer["
					+ asset_issuer
					+ "]校验出错");
			
		}
	}
	
	public void asset_codeCheck(){
		Object[] asset_codes = { -1, 0, "qqqqqqqq", null };
		for (Object asset_code : asset_codes) {
			List opers = TxUtil.operUniIssueTransfer(type, dest_address, asset_issuer, asset_code);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int error_code = Result.getErrorCode(result);
			check.assertNotEquals(error_code, 0, "转移唯一资产asset_code["
					+ asset_code
					+ "]校验失败");
		}
	}
	
//	@Test
	public void feeCheck(){
		Object[] fees = {-1,0,fee-1,"abc","!@#","",null};
		for (Object fee : fees) {
			List opers = TxUtil.operUniIssueTransfer(type, dest_address, asset_issuer, asset_code);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int error_code = Result.getErrorCode(result);
			check.assertNotEquals(error_code, 0, "转移唯一资产fee[" + fee + "]校验出错");
		}
	}
	
//	@Test
	public void source_addressCheck(){
		Object[] source_adds = {-1,0,"abc","!@#",address};
		for (Object source_address : source_adds) {
			List opers = TxUtil.operUniIssueTransfer(type, dest_address, asset_issuer, asset_code);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int error_code = Result.getErrorCode(result);
			check.assertNotEquals(error_code, 0, "转移唯一资产source_address["
					+ source_address + "]校验出错");
		}
	}
	
	// @Test
	public void private_keyCheck(){
		Object pri1 = TxUtil.createAccount().get("private_key");
		Object pri2 = APIUtil.generateAcc().get("private_key");

		Object[] private_keys = { pri1, pri2 };
		for (Object private_key : private_keys) {
			String pri = private_key.toString();
			List opers = TxUtil.operUniIssueTransfer(type, dest_address, asset_issuer, asset_code);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int error_code = Result.getErrorCode(result);
			check.assertNotEquals(error_code, 0, "转移唯一资产private_key[" + pri
					+ "]校验出错");
		}
	}
}
