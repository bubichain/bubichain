package test;

import org.testng.annotations.Test;

import base.TestBase;
import cn.bubi.blockchain.adapter.Message.AssetProperty.Type;
import model.Account;
import model.Operation;
import model.Threshold;
import newop.Transaction;

@Test
public class OperationTest extends TestBase{
	Transaction tran = new Transaction();
	/**
	 * 多个operation组合（2个），如创建账户，发行资产组合
	 */
	
	//创建账户转账组合
//	@Test
	public void create_payment(){
		Account a = tran.createAccountOne(genesis);
		Operation op1 = new Operation();
		op1.setDest_address(Account.generateAccount().getAddress());
		op1.setType(0);
		Operation op2 = new Operation();
		op2.setDest_address(tran.createAccountOne(genesis).getAddress());
		op2.setType(1);
		op2.setAsset_issuer(a.getAddress());
		op2.setAsset_amount(100L);
		tran.submit(a, op1, op2);
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 0,"create and payment check failed");
	}
	//创建账户发行唯一资产
//	@Test
	public void create_issueUnique(){
		Account a = tran.createAccountOne(genesis);
		Operation op1 = new Operation();
		op1.setDest_address(Account.generateAccount().getAddress());
		op1.setType(0);
		Operation op2 = new Operation();
		op2.setType(7);
		op2.setAsset_issuer(a.getAddress());
		op2.setAsset_amount(100L);
		op2.setAsset_code("123");
		op2.setAsset_detailed("1123");
		tran.submit(a, op1, op2);
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 0,"create and issueUnique checkfailed");
	}
	//创建账户转移唯一资产
//	@Test
	public void create_paymentUnique(){
		Account a = tran.createAccWithUniqueAsset(genesis);
		Operation op1 = new Operation();
		op1.setDest_address(Account.generateAccount().getAddress());
		op1.setType(0);
		op1.setInit_balance(base_reserve);
		Operation op2 = new Operation();
		op2.setType(8);
		op2.setAsset_issuer(a.getAddress());
		op2.setAsset_code("123");
		op2.setAPtype(Type.NATIVE);
		op2.setDest_address(tran.createAccountOne(genesis).getAddress());
		tran.submit(a, op1, op2);
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 0,"create and payment unique check failed");
	}
	//创建账户设置属性
//	@Test
	public void create_setOption(){
		Account a = tran.createAccountOne(genesis);
		Operation op1 = new Operation();
		op1.setDest_address(Account.generateAccount().getAddress());
		op1.setType(0);
		op1.setInit_balance(base_reserve);
		Operation op2 = new Operation();
		op2.setType(4);
		Threshold threshold = new Threshold();
		threshold.setHigh_threshold(2);
		threshold.setMed_threshold(2);
		threshold.setLow_threshold(2);
		threshold.setMaster_weight(2);
		op2.setThreshold(threshold);
		tran.submit(a, op1, op2);
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 0,"create and set option check failed");
	}
	//创建账户创建存证
//	@Test
	public void create_evidence(){
		Account a = tran.createAccountOne(genesis);
		Operation op1 = new Operation();
		op1.setDest_address(Account.generateAccount().getAddress());
		op1.setType(0);
		op1.setInit_balance(base_reserve);
		Operation op2 = new Operation();
		op2.setType(9);
		op2.setRecord_ext("1111");
		op2.setRecord_id("123");
		tran.submit(a, op1, op2);
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 0,"create and evidence check failed");
	}
	//创建账户供应链操作，验证operation为供应链时只能有一个operation
//	@Test
	public void create_production(){
		Account a = tran.createAccountOne(genesis);
		Operation op1 = new Operation();
		op1.setDest_address(Account.generateAccount().getAddress());
		op1.setType(0);
		op1.setInit_balance(base_reserve);
		Operation op2 = new Operation();
		op2.setType(6);
		op2.setInputs(tran.createInput());
		op2.setOutputs(tran.createOutput(tran.createAccountOne(genesis).getAddress(), "112233"));
		tran.submit(a, op1, op2);
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 2,"create and production check failed");
	}
	
	//创建账号发行资产
//	@Test
	public void create_issue(){
		Account a = tran.createAccountOne(genesis);
		Operation op1 = new Operation();
		op1.setDest_address(Account.generateAccount().getAddress());
		op1.setType(0);
		op1.setInit_balance(base_reserve);
		Operation op2 = new Operation();
		op2.setType(2);
		op2.setAsset_issuer(a.getAddress());
		op2.setAsset_amount(100L);
		op2.setAsset_code("1234"+ System.currentTimeMillis());
		tran.submit(a, op1, op2);
		int error_code =  Transaction.getErrorCode();
		check.assertEquals(error_code, 0,"create and issue check failed");
	}
	//发行资产转账
//	@Test
	public void issue_payment(){
		Account a = tran.createAccountOne(genesis);
		Operation op1 = new Operation();
		op1.setType(2);
		op1.setAsset_issuer(a.getAddress());
		op1.setAsset_amount(100L);
		op1.setAsset_code("1234"+ System.currentTimeMillis());
		Operation op2 = new Operation();
		op2.setDest_address(tran.createAccountOne(genesis).getAddress());
		op2.setType(1);
		op2.setAsset_issuer(a.getAddress());
		op2.setAsset_amount(100L);
		tran.submit(a, op1, op2);
		int error_code =  Transaction.getErrorCode();
		check.assertEquals(error_code, 0,"issue and payment check failed");
	}
	//发行资产设置属性
//	@Test
	public void issue_setOption(){
		Account a = tran.createAccountOne(genesis);
		Operation op1 = new Operation();
		op1.setType(2);
		op1.setAsset_issuer(a.getAddress());
		op1.setAsset_amount(100L);
		op1.setAsset_code("1234"+ System.currentTimeMillis());
		Operation op2 = new Operation();
		op2.setType(4);
		Threshold threshold = new Threshold();
		threshold.setHigh_threshold(2);
		threshold.setMed_threshold(2);
		threshold.setLow_threshold(2);
		threshold.setMaster_weight(2);
		op2.setThreshold(threshold);
		tran.submit(a, op1, op2);
		int error_code =  Transaction.getErrorCode();
		check.assertEquals(error_code, 0,"issue and set option check failed");
	}
	//发行资产创建存证
//	@Test
	public void issue_evidence(){
		Account a = tran.createAccountOne(genesis);
		Operation op1 = new Operation();
		op1.setType(2);
		op1.setAsset_issuer(a.getAddress());
		op1.setAsset_amount(100L);
		op1.setAsset_code("1234"+ System.currentTimeMillis());
		Operation op2 = new Operation();
		op2.setType(9);
		op2.setRecord_ext("1111");
		op2.setRecord_id("123");
		tran.submit(a, op1, op2);
		int error_code =  Transaction.getErrorCode();
		check.assertEquals(error_code, 0,"issue and evidence check failed");
	}
	//发行资产发行唯一资产
//	@Test
	public void issue_issueUnique(){
		Account a = tran.createAccountOne(genesis);
		Operation op1 = new Operation();
		op1.setType(2);
		op1.setAsset_issuer(a.getAddress());
		op1.setAsset_amount(100L);
		op1.setAsset_code("1234"+ System.currentTimeMillis());
		Operation op2 = new Operation();
		op2.setType(7);
		op2.setAsset_issuer(a.getAddress());
		op2.setAsset_amount(100L);
		op2.setAsset_code("123");
		op2.setAsset_detailed("1123");
		tran.submit(a, op1, op2);
		int error_code =  Transaction.getErrorCode();
		check.assertEquals(error_code, 0,"issue and issue unique check failed");
	}
	//发行资产创建供应链
	/*
	 * 验证供应链操作，operation只能有一个，operation.size大于2，交易失败
	 */
//	@Test
	public void issue_production(){
		Account a = tran.createAccountOne(genesis);
		Operation op1 = new Operation();
		op1.setType(2);
		op1.setAsset_issuer(a.getAddress());
		op1.setAsset_amount(100L);
		op1.setAsset_code("1234"+ System.currentTimeMillis());
		Operation op2 = new Operation();
		op2.setType(6);
		op2.setInputs(tran.createInput());
		op2.setOutputs(tran.createOutput(tran.createAccountOne(genesis).getAddress(), "112233"));
		tran.submit(a, op1, op2);
		int error_code =  Transaction.getErrorCode();
		check.assertEquals(error_code, 2,"issue and production check failed");
	}
	//发行资产转移唯一资产
//	@Test
	public void issue_paymentUnique(){
		Account a = tran.createAccWithUniqueAsset(genesis);
		Operation op1 = new Operation();
		op1.setType(2);
		op1.setAsset_issuer(a.getAddress());
		op1.setAsset_amount(100L);
		op1.setAsset_code("1234"+ System.currentTimeMillis());
		Operation op2 = new Operation();
		op2.setType(8);
		op2.setAsset_issuer(a.getAddress());
		op2.setAsset_code("123");
		op2.setAPtype(Type.NATIVE);
		op2.setDest_address(tran.createAccountOne(genesis).getAddress());
		tran.submit(a, op1, op2);
		int error_code =  Transaction.getErrorCode();
		check.assertEquals(error_code, 0,"issue and payment unique check failed");
	}
	/**
	 * sequence先后顺序校验
	 * 先发送一个seq=2的交易，再发送一个seq=1的交易
	 * 验证两个交易都执行成功，不会出现超时现象
	 */
//	@Test
	public void sequence_check(){
		Account a1 = tran.createAccountOne(genesis);
		tran.issue1(a1);   //issue
		tran.issue(a1);   //issue
		int error_code1 =  Transaction.getErrorCode();
		int error_code2 =  Transaction.getErrorCode();
		check.assertEquals(error_code1, 0, "大序号交易成功");
		check.assertEquals(error_code2, 0, "小序号交易成功");
	}
	//转账，发行资产
//	@Test
	public void payment_issue(){
		Account a = tran.createAccountOne(genesis);
		Operation op1 = new Operation();
		op1.setDest_address(tran.createAccountOne(genesis).getAddress());
		op1.setType(1);
		op1.setAsset_issuer(a.getAddress());
		op1.setAsset_amount(100L);
		Operation op2 = new Operation();
		op2.setType(2);
		op2.setAsset_issuer(a.getAddress());
		op2.setAsset_amount(100L);
		op2.setAsset_code("1234"+ System.currentTimeMillis());
		tran.submit(a, op1, op2);
		int error_code =  Transaction.getErrorCode();
		check.assertEquals(error_code, 0,"payment and issue check failed");
	}
	//转账，发行唯一资产
//	@Test
	public void payment_issueUnique(){
		Account a = tran.createAccountOne(genesis);
		Operation op1 = new Operation();
		op1.setDest_address(tran.createAccountOne(genesis).getAddress());
		op1.setType(1);
		op1.setAsset_issuer(a.getAddress());
		op1.setAsset_amount(100L);
		Operation op2 = new Operation();
		op2.setType(7);
		op2.setAsset_issuer(a.getAddress());
		op2.setAsset_amount(100L);
		op2.setAsset_code("123");
		op2.setAsset_detailed("1123");
		tran.submit(a, op1, op2);
		int error_code =  Transaction.getErrorCode();
		check.assertEquals(error_code, 0,"payment and issue unique check failed");
	}
	//转账，创建账户
//	@Test
	public void payment_create(){
		Account a = tran.createAccountOne(genesis);
		Operation op1 = new Operation();
		op1.setDest_address(tran.createAccountOne(genesis).getAddress());
		op1.setType(1);
		op1.setAsset_issuer(a.getAddress());
		op1.setAsset_amount(100L);
		Operation op2 = new Operation();
		op2.setDest_address(Account.generateAccount().getAddress());
		op2.setType(0);
		tran.submit(a, op1, op2);
		int error_code =  Transaction.getErrorCode();
		check.assertEquals(error_code, 0,"payment and create check failed");
	}
	//转账设置属性
//	@Test
	public void payment_setOption(){
		Account a = tran.createAccountOne(genesis);
		Operation op1 = new Operation();
		op1.setDest_address(tran.createAccountOne(genesis).getAddress());
		op1.setType(1);
		op1.setAsset_issuer(a.getAddress());
		op1.setAsset_amount(100L);
		Operation op2 = new Operation();
		op2.setType(4);
		Threshold threshold = new Threshold();
		threshold.setHigh_threshold(2);
		threshold.setMed_threshold(2);
		threshold.setLow_threshold(2);
		threshold.setMaster_weight(2);
		op2.setThreshold(threshold);
		tran.submit(a, op1, op2);
		int error_code =  Transaction.getErrorCode();
		check.assertEquals(error_code, 0,"payment and create check failed");
	}
	//转账存证
//	@Test
	public void payment_evidence(){
		Account a = tran.createAccountOne(genesis);
		Operation op1 = new Operation();
		op1.setDest_address(tran.createAccountOne(genesis).getAddress());
		op1.setType(1);
		op1.setAsset_issuer(a.getAddress());
		op1.setAsset_amount(100L);
		Operation op2 = new Operation();
		op2.setType(9);
		op2.setRecord_ext("1111");
		op2.setRecord_id("123");
		tran.submit(a, op1, op2);
		int error_code =  Transaction.getErrorCode();
		check.assertEquals(error_code, 0,"payment and evidence check failed");
	}
	//创建、发行，两个签名
//	@Test
	public void create_issuer_signers(){
		Account a1 = tran.createAccountOne(genesis);
		Account a2 = Account.generateAccount();
		Operation op1 = new Operation();
		op1.setDest_address(a2.getAddress());
		op1.setType(0);
		op1.setSource_address(a1.getAddress());
		Operation op2 = new Operation();
		op2.setType(2);
		op2.setAsset_issuer(a2.getAddress());
		op2.setAsset_amount(100L);
		op2.setAsset_code("1234"+ System.currentTimeMillis());
		op2.setSource_address(a2.getAddress());
		tran.submitSigners(a1, a2, op1, op2);
		int error_code =  Transaction.getErrorCode();
		check.assertEquals(error_code, 0,"create and issue check failed");
	}
	
}
