package cases;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import org.testng.annotations.Test;

import cn.bubi.tools.acc.Sign;
import net.sf.json.JSONObject;
import utils.APIUtil;
import utils.Result;
import utils.SignUtil;
import utils.TxUtil;
import base.TestBase;

@Test
public class SubmitTxTest_ledger extends TestBase{
	String geturl = get_Url2;
	/*
	 * verify signatures is exist
	 */
//	@Test
	public void signaturesCheck(){
		String tran_blob = null;
		JSONObject item = TxUtil.itemBlobonly(tran_blob);
		JSONObject items = TxUtil.tx(item);
		System.out.println(items);
		String result = TxUtil.txPost(items);
		int err_code = Result.getErrorCode(geturl,result);
		check.assertEquals(err_code, 2,"signatures 为空校验失败");
	}
	/*
	 * verify transaction blob must be Hex
	 */
//	@Test
	public void transaction_blobCheck(){
		List signature = new ArrayList();
		String tranblob = "qq";
		JSONObject item = TxUtil.item(signature,tranblob);
		JSONObject items = TxUtil.tx(item);
		String result = TxUtil.txPost(items);
		int err_code = Result.getErrorCode(geturl,result);
		check.assertEquals(err_code, 2,"transaction_blob 校验失败");
	}
	/*
	 * invalid sign_data
	 */
//	@Test
	public void sign_dataCheck(){
		String sign_data = "275*-a525386cf5410ca";
		String public_key = APIUtil.generateAcc().get("public_key");
		List signature = TxUtil.signatures(sign_data, public_key);
		String tranblob = "1231";
		JSONObject item = TxUtil.item(signature,tranblob);
		JSONObject items = TxUtil.tx(item);
		String result = TxUtil.txPost(items);
		int err_code = Result.getErrorCode(geturl,result);
		check.assertEquals(err_code, 2,"signatures 为空校验失败");
	}
	
	/*
	 * verify public_key
	 */
	@Test
	public void public_keyCheck(){
		//get correct sign_data and blob by issueTx;
		int type = 2;
		int asset_type = 1;
		Object asset_issuer = led_acc;
		String asset_code = "abc" ;
		int asset_amount = 100;
		String metadata = "abcd";
		String source_address = led_acc;
		long sequence_number = Result.seq_num(led_acc);
		List opers = TxUtil.operIssue(type, asset_type, asset_issuer, asset_code, asset_amount);		
		JSONObject tran = TxUtil.tran_json(source_address, fee, sequence_number, metadata, opers);
		String blobresult = SignUtil.getUnSignBlobResult(tran);
		String blobString = SignUtil.getTranBlobsString(blobresult);
		String sign_data;
		try {
			sign_data = Sign.priKeysign(blobString, led_pri);
			String public_key = "aa";
			List signature = TxUtil.signatures(sign_data, public_key);
			String tranblob = "1231";
			JSONObject item = TxUtil.item(signature,blobString);
			JSONObject items = TxUtil.tx(item);
			String result = TxUtil.txPost(items);
			int err_code = Result.getErrorCode(geturl,result);
			check.assertEquals(err_code, 2,"无效的public_key校验失败");
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
}
