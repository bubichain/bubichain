package cases;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import net.sf.json.JSONObject;

import org.testng.annotations.Test;

import utils.APIUtil;
import utils.HttpPool;
import utils.Result;
import utils.SignUtil;
import utils.TxUtil;
import base.TestBase;
import model.Account;

@Test
public class IssueTest extends TestBase{
	
	int type = 2;
	int asset_type = 1;
	Map acc = TxUtil.createAccount();
	Object address = acc.get("address");
	String pri = acc.get("private_key").toString();
	Object pub = acc.get("public_key");
	Object asset_issuer = address;
	String asset_code = "abc" ;
	int asset_amount = 100;
	Object source_address = address;
	String metadata = "abcd";

//	 @Test
	@SuppressWarnings("rawtypes")
	public void issue(){
		 Account srcAcc = TxUtil.createNewAccount();
		List opers = TxUtil.operIssue(type, asset_type, srcAcc.getAddress(), asset_code, asset_amount);		
		String result = SignUtil.tx(opers, srcAcc.getAddress(), fee,metadata, srcAcc.getPri_key(), srcAcc.getPub_key());
		int err_code = Result.getErrorCode(result);
		check.assertEquals(err_code, 0, "资产发行失败");
	}
	
//	@Test
	public void operationSizeCheck(){
		//Operation size is zero
		List opers = new ArrayList<>();
		String result = SignUtil.tx(opers, source_address, fee, metadata, pri, pub);
		int err_code = Result.getErrorCode(result);
		check.assertEquals(err_code, 4, "资产发行operation为0 校验失败");
	}
	
	public void tranMetadataCheck(){
		List opers = TxUtil.operIssue(type, asset_type, asset_issuer, asset_code, asset_amount);		
		String result = SignUtil.tx(opers, source_address, fee,1048580, pri, pub);
		int err_code = Result.getErrorCode(result);
		check.assertEquals(err_code, 2, "资产发行transaction metadata校验失败");
	}
//	@Test
	public void min_timeinValidCheck(){
		String response = HttpPool.doGet("getLedger");
		String close_time = Result.getConsensusTh(response, "close_time");
		Long max_time = Long.parseLong(close_time)+10000;
		Object[] time = {-1,"aa"};
		for (Object min_time : time) {
			JSONObject time_range = TxUtil.timeRange(min_time, max_time);
			List opers = TxUtil.operIssue(type, asset_type, asset_issuer, asset_code, asset_amount);		//ledger发行未初始化资产
			String result = SignUtil.tx(opers, source_address, fee, metadata,time_range, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 2,"非法min_time["+min_time+"]校验失败");
		}
	}
	
//	@Test
	public void max_timeinValidCheck(){
		String response = HttpPool.doGet("getLedger");
		String close_time = Result.getConsensusTh(response, "close_time");
		Long min_time = Long.parseLong(close_time)-10000;
		Object[] time = {-1,"aa"};
		for (Object max_time : time) {
			JSONObject time_range = TxUtil.timeRange(min_time, max_time);
			List opers = TxUtil.operIssue(type, asset_type, asset_issuer, asset_code, asset_amount);		//ledger发行未初始化资产
			String result = SignUtil.tx(opers, source_address, fee, metadata,time_range, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 2,"非法max_time["+max_time+"]校验失败");
		}
	}
	
//	@Test
	public void time_rangeValidCheck() {
		Account srcAcc = TxUtil.createNewAccount();
		String response = HttpPool.doGet("getLedger");
		String close_time = Result.getConsensusTh(response, "close_time");
		Long min_time = Long.parseLong(close_time) - 20000000;
		Long max_time = Long.parseLong(close_time) * 2;
		JSONObject time_range = TxUtil.timeRange(min_time, max_time);
		List opers = TxUtil.operIssue(type, asset_type, srcAcc.getAddress(), asset_code, asset_amount); // ledger发行未初始化资产
		String result = SignUtil.tx2(opers, srcAcc.getAddress(), fee, metadata, time_range, srcAcc.getPri_key(),
				srcAcc.getPub_key());
		int err_code = Result.getErrorCode(result);
		check.assertEquals(err_code, 0, "timerange校验失败");
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void typeCheck(){
		Object[] types = {0,20,-1,"abc","!@#","",null};
		for (Object type : types) {
			List opers = TxUtil.operIssue(type, asset_type, asset_issuer, asset_code, asset_amount);		//ledger发行未初始化资产
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					 metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 2, "资产发行type[" + type + "]校验失败");
		}
	}
	
//	@Test
	@SuppressWarnings({ "rawtypes" })
	public void asset_typeCheck(){
		Object[] asset_types = {0,20,-1,"abc","!@#","",null};
		for (Object asset_type : asset_types) {
			List opers = TxUtil.operIssue(type, asset_type, asset_issuer, asset_code, asset_amount);		//ledger发行未初始化资产
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 2, "资产发行asset_type[" + asset_type
					+ "]校验失败");
		}
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void asset_issuerCheck(){
		Object[] asset_issuers = {0,20,-1,"abc","!@#","",null};
		for (Object asset_issuer : asset_issuers) {
			List opers = TxUtil.operIssue(type, asset_type, asset_issuer, asset_code, asset_amount);		//ledger发行未初始化资产
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 2, "资产发行asset_issuer["
					+ asset_issuer + "]校验失败");
		}
		
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void asset_codevalidCheck(){
		int asset_amount = 100;
		int type = 2 ;
		int asset_type = 1;
		//预期成功
		Object[] asset_codes = {0,20,"abc","!@#","1234567890abcdef"};
		for (Object asset_code : asset_codes) {
			List opers = TxUtil.operIssue(type, asset_type, asset_issuer, asset_code, asset_amount);		//ledger发行未初始化资产
			String result = SignUtil.tx(opers, source_address, fee,
					 metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.equals(err_code, 0,"资产发行asset_code["+asset_code+"]校验失败");
		}
		
	}
	
	public void asset_codeinvalidCheck(){
		//预期失败
				Object[] asset_codes2 = {"",null,"1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef1"};
				for (Object asset_code : asset_codes2) {
					List opers = TxUtil.operIssue(type, asset_type, asset_issuer, asset_code, asset_amount);		//ledger发行未初始化资产
					String result = SignUtil.unnormal_tx(opers, source_address, fee,
							 metadata, pri, pub);
					int err_code = Result.getErrorCode(result);
					check.assertEquals(err_code, 2, "资产发行asset_code[" + asset_code
							+ "]校验失败");
				}
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void asset_amountCheck(){
		Object[] asset_amounts = {0,"",null,"abc","!@#"};
		for (Object asset_amount : asset_amounts) {
			List opers = TxUtil.operIssue(type, asset_type, asset_issuer, asset_code, asset_amount);		//ledger发行未初始化资产
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					 metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 2, "资产发行asset_amount["
					+ asset_amount + "]校验失败");
		}
		
	}
	
//	 @Test
	@SuppressWarnings("rawtypes")
	public void details_amountCheck(){
		String asset_code = "abc";
		int asset_amount = 10;
		//预期失败
		Object start = 0;
		Object length = -1;
		Object ext = "abcd";
		Object[] detatils_amounts = {0,-1,"",null,"abc","!@#",asset_amount+1};
		for (Object detatils_amount : detatils_amounts) {
			List detail = TxUtil.details(detatils_amount, start, length, ext);
			List opers = TxUtil.operIssue(type, asset_type, asset_issuer,
					asset_code, asset_amount, detail); // ledger发行未初始化资产
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 2, "资产发行detatils_amount["
					+ detatils_amount + "]校验失败");
		}
	}
	
	// @Test
	@SuppressWarnings("rawtypes")
	public void details_startCheck(){
		// long s = System.currentTimeMillis()/1000-10000 ;
		//预期失败
		Object detatils_amount = 100;
		Object length = -1;
		Object ext = "abcd";
		Object[] detatils_starts = { -1, "", null, "abc", "!@#" };
		for (Object detatils_start : detatils_starts) {
			List detail = TxUtil.details(detatils_amount, detatils_start,
					length, ext);
			List opers = TxUtil.operIssue(type, asset_type, asset_issuer,
					asset_code, asset_amount, detail); // ledger发行未初始化资产
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 2, "资产发行detatils_start["
					+ detatils_start + "]校验失败");
		}
	}
	
	// @Test
	@SuppressWarnings("rawtypes")
	public void details_lengthCheck(){
		Object detatils_amount = 100;
		Object start = 0;
		Object ext = "7777";
		Object[] detatils_lengths = { "", null, "abc", "!@#" };
		for (Object detatils_length : detatils_lengths) {
			List detail = TxUtil.details(detatils_amount, start,
					detatils_length, ext);
			List opers = TxUtil.operIssue(type, asset_type, asset_issuer,
					asset_code, asset_amount, detail); // ledger发行未初始化资产
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 2, "资产发行detatils_length["
					+ detatils_length + "]校验失败");
		}
		
	}
	
//	 @Test
	@SuppressWarnings("rawtypes")
	public void details_extCheck(){
		Object detatils_amount = 100;
		Object start = 0;
		Object length = -1;
		Object[] detatils_exts = {
				"1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef" };
		for (Object detatils_ext : detatils_exts) {
			List detail = TxUtil.details(detatils_amount, start, length,
					detatils_ext);
			List opers = TxUtil.operIssue(type, asset_type, asset_issuer,
					asset_code, asset_amount, detail); // ledger发行未初始化资产
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					 metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 2, "资产发行detatils_ext["
					+ detatils_ext + "]校验失败");
		}
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void feeinvalidCheck(){
		String asset_code = "abc" ;
		int asset_amount = 10;
		Object[] fees = {-1,"abc","!@#","",null};
		for (Object fee : fees) {
			List opers = TxUtil.operIssue(type, asset_type, asset_issuer, asset_code, asset_amount);		//ledger发行未初始化资产
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 2, "资产发行fee[" + fee + "]校验失败");
		}
	}
	
//	@Test
	public void feeNotEnoughCheck(){
		String asset_code = "abc" ;
		int asset_amount = 10;
		Object[] fees = {0,fee-1};
		for (Object fee : fees) {
			List opers = TxUtil.operIssue(type, asset_type, asset_issuer, asset_code, asset_amount);		//ledger发行未初始化资产
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 111, "资产发行fee[" + fee + "]校验失败");
		}
	}
	
	
//	@Test
	public void opera_metadataCheck(){
		String asset_code = "abc" ;
		int asset_amount = 10;
		Object[] metadatas = {-1,0,999,"abc","!@#",1048579};
		for (Object metadata_ : metadatas) {
			List opers = TxUtil.operIssue(type, asset_type, asset_issuer, asset_code, asset_amount,metadata_,source_address);		//ledger发行未初始化资产
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 2, "operation metadata[" + metadata_ + "]校验失败");
			
		}
		
	}
	
//	@Test
	public void opera_sourceaddressCheck(){
		String asset_code = "abc" ;
		int asset_amount = 10;
		Object[] sourceadds = {-1,0,999,"abc","!@#","",null};
		for (Object sourceadd : sourceadds) {
			List opers = TxUtil.operIssue(type, asset_type, asset_issuer, asset_code, asset_amount,metadata,sourceadd);		//ledger发行未初始化资产
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 2, "operation source_address[" + sourceadd + "]校验失败");
			
		}
		
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void source_addressCheck(){
		Object[] source_adds = {-1,0,"abc","!@#"};
		for (Object source_add : source_adds) {
			List opers = TxUtil.operIssue(type, asset_type, asset_issuer, asset_code, asset_amount);		//ledger发行未初始化资产
			String result = SignUtil.unnormal_tx(opers, source_add, fee,
					 metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 2, "资产发行source_address["
					+ source_add
					+ "]校验失败");
		}
	}
//	@Test	之前预期是103，但是后来查不到交易记录
	public void source_addressinvalidCheck(){
		Object add = APIUtil.generateAddress();
			List opers = TxUtil.operIssue(type, asset_type, asset_issuer, asset_code, asset_amount);		
			String result = SignUtil.unnormal_tx(opers, add, fee,
					 metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 103, "资产发行source_address["
					+ add
					+ "]校验失败");
	}
//	@Test  //之前能返回错误码93，现在查不到交易记录了
	@SuppressWarnings("rawtypes")
	public void private_keyCheck(){
		Object pri1 = TxUtil.createAccount().get("private_key");
		Object pri2 = APIUtil.generateAcc().get("private_key");

		Object[] pri_keys = { pri1, pri2 };
		// Object[] pri_keys = {-1,10,"abc","!@#","",null};
		int asset_amount = 10;
		for (Object pri_key : pri_keys) {
			String pri = pri_key.toString();
			List opers = TxUtil.operIssue(type, asset_type, asset_issuer, asset_code, asset_amount);		//ledger发行未初始化资产
			String result = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 4, "资产发行private_key[" + pri_key
					+ "]校验失败");
		}
	}
	
}
