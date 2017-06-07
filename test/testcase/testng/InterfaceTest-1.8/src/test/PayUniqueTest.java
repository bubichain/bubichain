package test;

import org.testng.annotations.Test;

import base.TestBase;
import cn.bubi.blockchain.adapter.Message.Operation.Type;
import model.Account;
import model.Operation;
import newop.Transaction;
import utils.APIUtil;

@Test
public class PayUniqueTest extends TestBase{

	Transaction tran = new Transaction();
	String asset_code = "asset_code";
//	@Test
	public void testPaymentUnique(){
		Account a1 = tran.createAccountOne(genesis);
		Account a2 = tran.createAccountOne(genesis);
		Operation opt1 = new Operation();
		opt1.setAsset_issuer(a1.getAddress());
		opt1.setAsset_code(asset_code);
		opt1.setDest_address(a2.getAddress());
		tran.paymentUnique(a1, a2, opt1);
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 0, "payment unique asset check failed");
	}
//	@Test
	public void paymentUnique_asset_type(){
		Account a1 = tran.createAccountOne(genesis);
		Account a2 = tran.createAccountOne(genesis);
		Operation opt = new Operation();
		opt.setAsset_issuer(a1.getAddress());
		opt.setAsset_code(asset_code);
		opt.setAsset_type(1);
		a1.seqence = Account.getSeq(a1);
		APIUtil.wait(1);
		tran.paymentUnique(a1, a2, opt);
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 2, "payment unique asset type check failed");
	}
//	@Test
	public void paymentUnique_asset_code(){
		Account a1 = tran.createAccountOne(genesis);
		Account a2 = tran.createAccountOne(genesis);
		Operation opt = new Operation();
		opt.setAsset_issuer(a1.getAddress());
		opt.setAsset_code("1234567890abcdefghijklmnopqrstuvwxyz1234567890abcdefghijklmnopqrstu");
		tran.paymentUnique(a1, a2, opt);
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 2, "payment unique asset code greater than 64 check failed");
	}
	/**
	 * 验证唯一资产转账，发行者和原地址不一致报错
	 */
//	@Test
	public void paymentUnique_asset_issuer(){
		Account a1 = tran.createAccountOne(genesis);
		Account a2 = tran.createAccountOne(genesis);
		Operation opt = new Operation();
		opt.setAsset_issuer("123");
		opt.setAsset_code(asset_code);
		tran.paymentUnique(a1, a2, opt);
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 2, "payment unique asset issuer check failed");
	}
//	@Test
	public void paymentUnique_operation_not_exist(){
		Account a1 = tran.createAccountOne(genesis);
		tran.noOperation(a1, Type.PAYMENT_UNIQUE_ASSET);	//create account
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 2,"payment unique_ no operation check failed");
	}
	/**
	 * 验证唯一资产转账，目标地址等于原地址交易失败
	 */
//	@Test
	public void paymentUnique_destaddress(){
		Account a1 = tran.createAccountOne(genesis);
//		Account a2 = tran.createAccountOne(genesis);
		Operation opt2 = new Operation();
		opt2.setAsset_issuer(a1.getAddress());
		opt2.setAsset_code(asset_code);
		opt2.setDest_address(a1.getAddress());
		tran.paymentUnique(a1, a1, opt2);
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code,101, "payment unique asset dest_address check failed");
	}
}
