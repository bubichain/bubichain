package cases;

import java.util.List;
import java.util.Map;

import org.testng.annotations.Test;

import utils.APIUtil;
import utils.Result;
import utils.SignUtil;
import utils.TxUtil;
import base.TestBase;

//发行唯一资产 
@Test
public class UniIssueTest_ledger extends TestBase{
	String geturl = get_Url2;
	int type = 7;
	@SuppressWarnings("rawtypes")
	Map acc = TxUtil.createAccount();
	Object source_address = acc.get("address");
	String pri = acc.get("private_key").toString();
	Object pub = acc.get("public_key");
	String asset_code = "abc" ;
	String asset_issuer = led_acc; 
	String asset_detailed = "1234";
	String s_address = led_acc;
	String s_key = led_pri;
	long sequence_number = Result.seq_num(source_address);
	String metadata = "1234";

	public void uniIssueCheck(){
		Object asset_issuer = source_address;
		List opers = TxUtil.operUniIssue(type, asset_issuer, asset_code, asset_detailed);
		sequence_number = Result.seq_num(source_address);
		String result = SignUtil.tx(opers, source_address, fee, sequence_number, metadata, pri, pub);
		int err_code = Result.getErrorCode(geturl,result);
		check.assertEquals(err_code, 0, "发行唯一资产失败");
		
	}
	
//	@Test
	public void asset_typeCheck(){
		sequence_number = Result.seq_num(source_address);
		Object[] asset_types = {0,20,-1,"abc","!@#","",null};
		for (Object asset_type : asset_types) {
			List opers = TxUtil.operUniIssue(type, asset_type,asset_issuer, asset_code, asset_detailed);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					sequence_number, metadata, pri, pub);
			int err_code = Result.getErrorCode(geturl,result);
			check.assertEquals(err_code, 2, "发行唯一资产asset_type[" + asset_type + "]校验失败");
		}
	}
//	@Test
	public void asset_issuerCheck(){
		sequence_number = Result.seq_num(source_address);
		Object[] asset_issuers = {-1,"abc","!@#","",null};
		for (Object asset_issuer : asset_issuers) {
			List opers = TxUtil.operUniIssue(type, asset_issuer, asset_code, asset_detailed);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					sequence_number, metadata, pri, pub);
			int err_code = Result.getErrorCode(geturl,result);
			check.assertEquals(err_code, 4, "发行唯一资产asset_issuer["
					+ asset_issuer
					+ "]校验失败");
		}
	}
	
//	@Test
	public void asset_codeCheck(){
		sequence_number = Result.seq_num(source_address);
		Object[] asset_codes = {-1,0,"qqqqqqqq","",null};
		for (Object asset_code : asset_codes) {
			List opers = TxUtil.operUniIssue(type, asset_issuer, asset_code, asset_detailed);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					sequence_number, metadata, pri, pub);
			int err_code = Result.getErrorCode(geturl,result);
			check.assertEquals(err_code, 4, "发行唯一资产asset_code[" + asset_code
					+ "]校验失败");
		}
	}
	
//	@Test
	public void asset_detailedCheck(){
		sequence_number = Result.seq_num(source_address);
		Object[] asset_detaileds = {"abc","qq",0,-1,"",null};
		for (Object asset_detailed : asset_detaileds) {
			List opers = TxUtil.operUniIssue(type, asset_issuer, asset_code, asset_detailed);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					sequence_number, metadata, pri, pub);
			int err_code = Result.getErrorCode(geturl,result);
			check.assertEquals(err_code, 4, "发行唯一资产asset_detailed["
					+ asset_detailed + "]校验失败");
		}
	}
	
//	@Test
	public void feeCheck(){
		sequence_number = Result.seq_num(source_address);
		Object[] fees = {-1,0,999,"abc","!@#","",null};
		for (Object fee : fees) {
			List opers = TxUtil.operUniIssue(type, asset_issuer, asset_code, asset_detailed);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					sequence_number, metadata, pri, pub);
			int err_code = Result.getErrorCode(geturl,result);
			check.assertEquals(err_code, 4, "发行唯一资产fee[" + fee + "]校验失败");
		}
	}
	
	@Test(priority = 4)
	public void source_addressCheck(){
		sequence_number = Result.seq_num(source_address);
		Object[] source_adds = {-1,0,"abc","!@#","",null};
		for (Object source_address : source_adds) {
			List opers = TxUtil.operUniIssue(type, asset_issuer, asset_code, asset_detailed);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					sequence_number, metadata, pri, pub);
			int err_code = Result.getErrorCode(geturl,result);
			check.assertEquals(err_code, 4, "发行唯一资产source_address["
					+ source_address + "]校验失败");
		}
	}
	
	public void private_keyCheck(){
		sequence_number = Result.seq_num(source_address);
		Object pri1 = TxUtil.createAccount().get("private_key");
		Object pri2 = APIUtil.generateAcc().get("private_key");

		Object[] pri_keys = { pri1, pri2 };
		for (Object pri_key : pri_keys) {
			String pri = pri_key.toString();
			List opers = TxUtil.operUniIssue(type, asset_issuer, asset_code,
					asset_detailed);
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					sequence_number, metadata, pri, pub);
			int err_code = Result.getErrorCode(geturl,result);
			check.assertEquals(err_code, 4, "发行唯一资产[" + pri_key + "]失败");
		}

		// 下面的入参项适用于手动传私钥。本身sign.jar就会报java.lang.RuntimeException: Not a Base58
		// Object[] pri_keys = {-1,0,"abc","!@#","",null};
		check.result("发行唯一资产校验成功");
	}

	
}
