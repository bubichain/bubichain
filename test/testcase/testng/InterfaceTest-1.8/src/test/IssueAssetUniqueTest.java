package test;

import java.math.BigInteger;

import org.testng.annotations.Test;

import base.TestBase;
import cn.bubi.blockchain.adapter.Message.Operation.Type;
import model.Account;
import model.Operation;
import model.TxInfo;
import newop.Transaction;
import utils.APIUtil;

@Test
public class IssueAssetUniqueTest extends TestBase{
	Transaction tran = new Transaction();
//	@Test
	public void issueAssetUniqueTest() {
		Operation opt = new Operation();
		Account a1 = tran.createAccountOne(genesis);
		opt.setAsset_issuer(a1.getAddress());
		opt.setAsset_code("asset_code");
		opt.setAsset_detailed("asset_detailedtest");
		tran.issueAssetUnique(a1, opt);
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 0, "发行唯一资产校验失败");
	}
//	@Test
	public void uniqueAsset_already_exist(){
		Operation opt = new Operation();
		Account a1 = tran.createAccountOne(genesis);
			opt.setAsset_issuer(a1.getAddress());
			opt.setAsset_code("asset_code");
			opt.setAsset_detailed("asset_detailedtest");
			tran.issueAssetUnique(a1,opt);
			APIUtil.wait(1);
			tran.issueAssetUnique(a1,opt);
			int error_code = Transaction.getErrorCode();
			check.assertEquals(error_code, 3, "重复发行唯一资产校验失败");
	}
//	@Test
	public void uniqueAsset_assetissuer() {
		Operation opt = new Operation();
		Account a1 = tran.createAccountOne(genesis);
		opt.setAsset_issuer(Account.generateAccount().getAddress());
		opt.setAsset_code("asset_code");
		opt.setAsset_detailed("asset_detailedtest");
		tran.issueAssetUnique(a1, opt); // first issue
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 92, "唯一资产发行者不存在时校验失败");
	}
//	@Test
	public void issueUnique_operation_not_exist(){
		Account a1 = tran.createAccountOne(genesis);
		tran.noOperation(a1, Type.ISSUE_UNIQUE_ASSET);	//create account
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 2,"issue unique asset no operation check failed");
	}
	
//	@Test
//	public void issueunique_assetdetailed() {
//		srcAcc.acc(led_acc, led_pri, led_pub, url_getAccInfo); // get leddger
//		Account a1 = tran.createAccount(srcAcc);
//		opt.setAsset_issuer(a1.getAddress());
//		opt.setAsset_code("asset_code");
//		opt.setAsset_detailed("1");
//		a1.seqence = Account.getSeq(a1);
//		tran.issueAssetUnique(a1, opt); // first issue
//		int error_code = Transaction.txInfo.getError_code();
//		check.assertEquals(error_code, 92, "issue unique asset detailed check failed");
//	}
	
//	@Test
	public void issueAssetUnique_type() {
		Operation opt = new Operation();
		Account a1 = tran.createAccountOne(genesis);
		opt.setAsset_issuer(a1.getAddress());
		opt.setAsset_code("asset_code");
		opt.setAsset_detailed("asset_detailedtest");
		opt.setAsset_type(1);
		tran.issueAssetUnique(a1, opt);
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 2, "issue unique asset type check failed");
	}
	
//	@Test
	public void issueAssetUnique_asset_code() {
		Operation opt = new Operation();
		Account a1 = tran.createAccountOne(genesis);
		opt.setAsset_issuer(a1.getAddress());
		opt.setAsset_code("1234567890abcdefghijklmnopqrstuvwxyz1234567890abcdefghijklmnopqrstu");
		opt.setAsset_detailed("asset_detailedtest");
		tran.issueAssetUnique(a1, opt);
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 2, "issue unique asset code size greater than 64 check failed");
	}
//	@Test
	public void issueAssetUnique_asset_issuer_invalid() {
		Operation opt = new Operation();
		Account a1 = tran.createAccountOne(genesis);
		opt.setAsset_issuer("123");
		opt.setAsset_code("123");
		opt.setAsset_detailed("asset_detailedtest");
		tran.issueAssetUnique(a1, opt);
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 2, "issue unique asset issuer invalid check failed");
	}
//	@Test
	public void issueAssetUnique_asset_detailed() {
		Operation opt = new Operation();
		Account a1 = tran.createAccountOne(genesis);
		opt.setAsset_issuer(a1.getAddress());
		opt.setAsset_code("123");
		BigInteger bigInteger = new BigInteger("1024");
		BigInteger a = bigInteger.pow(1024).pow(1024);
		opt.setAsset_detailed(a.toString());
		tran.issueAssetUnique(a1, opt);
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 2, "issue unique asset code size greater than 64 check failed");
	}
}
