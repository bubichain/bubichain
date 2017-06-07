package test;

import java.math.BigInteger;
import java.util.ArrayList;
import java.util.List;

import org.testng.annotations.Test;

import base.TestBase;
import cn.bubi.blockchain.adapter.Message.Operation.Type;
import model.Account;
import model.Operation;
import model.Threshold;
import newop.Transaction;

@Test
public class CreateAccountTest extends TestBase{
	Transaction tran = new Transaction();
	
//	@Test
	public void create_normal(){
		tran.createAccountOne(genesis);
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 0,"创建账户交易失败");
	}
	
//	@Test
	public void create_threshold(){
		Operation oper = new Operation();
		Account a = tran.createAccountOne(genesis);
		Threshold threshold = new Threshold();
		threshold.setHigh_threshold(2);
		threshold.setLow_threshold(2);
		threshold.setMed_threshold(2);
		threshold.setMaster_weight(2);
		oper.setThreshold(threshold);
		oper.setTransaction_metadata("metadatatest");
		tran.createAccount1(a, oper);
		int error_code =Transaction.getErrorCode();
		check.assertEquals(error_code, 0,"创建账户交易失败");
	}
//	@Test
	public void create_threshold_High(){
		Operation oper = new Operation();
		Account a = tran.createAccountOne(genesis);
		Threshold threshold = new Threshold();
		threshold.setHigh_threshold(-2);
		threshold.setLow_threshold(2);
		threshold.setMed_threshold(2);
		threshold.setMaster_weight(2);
		oper.setThreshold(threshold);
		oper.setTransaction_metadata("metadatatest");
		tran.createAccount1(a, oper);
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 2,"create account high threshold check failed");
	}
//	@Test
	public void create_threshold_Med(){
		Operation oper = new Operation();
		Account a = tran.createAccountOne(genesis);
		Threshold threshold = new Threshold();
		threshold.setHigh_threshold(2);
		threshold.setLow_threshold(2);
		threshold.setMed_threshold(-2);
		threshold.setMaster_weight(2);
		oper.setThreshold(threshold);
		oper.setTransaction_metadata("metadatatest");
		tran.createAccount1(a, oper);
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 2,"create account med threshold check failed");
	}
//	@Test
	public void create_threshold_Low(){
		Operation oper = new Operation();
		Account a = tran.createAccountOne(genesis);
		Threshold threshold = new Threshold();
		threshold.setHigh_threshold(2);
		threshold.setLow_threshold(-2);
		threshold.setMed_threshold(2);
		threshold.setMaster_weight(2);
		oper.setThreshold(threshold);
		oper.setTransaction_metadata("metadatatest");
		tran.createAccount1(a, oper);
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 2,"create account low threshold check failed");
	}
//	@Test
	public void create_threshold_Master(){
		Operation oper = new Operation();
		Account a = tran.createAccountOne(genesis);
		Threshold threshold = new Threshold();
		threshold.setHigh_threshold(2);
		threshold.setLow_threshold(2);
		threshold.setMed_threshold(2);
		threshold.setMaster_weight(-2);
		oper.setThreshold(threshold);
		oper.setTransaction_metadata("metadatatest");
		tran.createAccount1(a, oper);
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 2,"create account master weight check failed");
	}
	
//	@Test
	public void create_signer(){
		Account a1 = tran.createAccountOne(genesis);
		Account a2 = tran.createAccountOne(genesis);
		tran.createSignerOld(genesis, a1, 3, a2, 3);
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 0,"create account with signers check failed");
	}
//	@Test
	public void create_signer_weight(){
		Account a1 = tran.createAccountOne(genesis);
		Account a2 = tran.createAccountOne(genesis);
		tran.createSignerOld(genesis, a1, 3, a2, -1);
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 2,"create account signer weight check failed");
	}
	//	@Test
	public void createMuti(){
		List<Operation> oper_list = new ArrayList<>();
		Operation op1 = new Operation();
		op1.setDest_address(Account.generateAccount().getAddress());
		Operation op2 = new Operation();
		op2.setDest_address(Account.generateAccount().getAddress());
		Operation op3 = new Operation();
		op3.setDest_address(Account.generateAccount().getAddress());
		oper_list.add(op1);
		oper_list.add(op2);
		oper_list.add(op3);
		tran.createAccount(genesis, oper_list);
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 0,"create muti_account check failed ");
	}
//	@Test
	public void create_transaction_metadata(){
		Operation oper = new Operation();
		Account a1 = tran.createAccountOne(genesis);
		oper.setTransaction_metadata("q");
		tran.createAccount1(a1,oper);	//create account
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 0,"创建账户transaction metadata 校验失败 ");
	}
//	@Test
	public void create_transaction_metadata_Max(){
		Operation oper = new Operation();
		Account a1 = tran.createAccountOne(genesis);
		BigInteger bigInteger = new BigInteger("1024");
		BigInteger a = bigInteger.pow(1024).pow(1024);
		oper.setTransaction_metadata(a.toString());
		tran.createAccount1(a1,oper);	//create account
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 2,"创建账户transaction metadata 十六进制校验失败 ");
	}
//	@Test
	public void create_operation_not_exist(){
		Account a = tran.createAccountOne(genesis);
		tran.noOperation(a, Type.CREATE_ACCOUNT);	//create account
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 2,"create account no operation check failed");
	}
//	@Test
	public void create_invalidpublickey(){
		Operation oper = new Operation();
		Account a = tran.createAccountOne(genesis);
		oper.setDest_address(Account.generateAccount().getAddress());
		oper.setType(0);
		tran.invalidPublickey(a, oper, Type.CREATE_ACCOUNT, "123");	//create account
		int e =Transaction.getErrorCode();
		check.assertEquals(e, 90,"create account invalid publickey check failed");
	}
//	@Test
	public void create_illegalpublickey(){
		Operation oper = new Operation();
		Account a = tran.createAccountOne(genesis);
		oper.setDest_address(Account.generateAccount().getAddress());
		oper.setType(0);
		tran.invalidPublickey(a, oper, Type.CREATE_ACCOUNT, Account.generateAccount().getPub_key());	//create account
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 93,"create account illegal publickey check failed");
	}
//	@Test
	public void create_sequence(){
		Operation oper = new Operation();
		Account a = tran.createAccountOne(genesis);
		oper.setDest_address(Account.generateAccount().getAddress());
		oper.setSequence_num(-1L);
		tran.sequceCheck(a, oper, Type.CREATE_ACCOUNT);	//create account
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 2,"create account sequence must greater than 0 check failed");
	}
//	@Test
//	public void create_fee_nagative(){
//		Operation oper = new Operation();
//		srcAcc.acc(led_acc, led_pri, led_pub, url_getAccInfo); //get genesis count
//		oper.setDest_address(Account.generateAccount().getAddress());
//		oper.setFee(1<<3);
//		tran.feeCheck(srcAcc, oper, Type.CREATE_ACCOUNT);	//create account
//		int e = Transaction.txInfo.getError_code();
//		check.assertEquals(e, 111,"create account fee must greater than 0 check failed");
//	}
//	@Test
	public void create_fee_zero(){
		Operation oper = new Operation();
		Account a = tran.createAccountOne(genesis);
		oper.setDest_address(Account.generateAccount().getAddress());
		oper.setFee(0);
		oper.setType(0);
		tran.feeCheck(a, oper, Type.CREATE_ACCOUNT);	//create account
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 111,"create account fee must greater than 0 check failed");
	}
	
//	@Test
	public void create_accountmeta(){
		Operation oper = new Operation();
		Account a = tran.createAccountOne(genesis);
		oper.setAccount_metadata("1234");
		oper.setDest_address(Account.generateAccount().getAddress());
		oper.setType(0);
//		srcAcc.seqence = Account.getSeq(srcAcc);
		tran.operationMetadataCheck(a, oper, Type.CREATE_ACCOUNT);  
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 0,"create account operation metadata check failed");
	}
//	@Test
	public void create_oper_sourceaddress(){
		Operation oper = new Operation();
		Account a = tran.createAccountOne(genesis);
		oper.setSource_address(a.getAddress());
		oper.setDest_address(Account.generateAccount().getAddress());
		tran.operationSourceCheck(a, oper, Type.CREATE_ACCOUNT);  
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 0,"create account operation sourceaddress check failed");
	}
	
//	@Test
	public void create_oper_src_invalid(){
		Account a = tran.createAccountOne(genesis);
		Operation oper = new Operation();
		oper.setSource_address(Account.generateAccount().getAddress());
		oper.setDest_address(Account.generateAccount().getAddress());
		tran.operationSourceCheck(a, oper, Type.CREATE_ACCOUNT);  
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 2,"create account operation source address invalid check failed");
	}
//	@Test
	public void create_oper_src_not_exist(){
		Operation oper = new Operation();
		Account a = tran.createAccountOne(genesis);
		oper.setSource_address("123");
		oper.setDest_address(Account.generateAccount().getAddress());
		tran.operationSourceCheck(a, oper, Type.CREATE_ACCOUNT);  
		int e =Transaction.getErrorCode();
		check.assertEquals(e, 2,"create account operation source address not exist check failed");
	}
//	@Test
	public void create_oper_src_not_self(){
		Operation oper = new Operation();
		Account a = tran.createAccountOne(genesis);
		Account b = tran.createAccountOne(genesis);
		oper.setSource_address(b.getAddress());   //set operation source address as another user
		oper.setDest_address(Account.generateAccount().getAddress());
		tran.operationSourceCheck(a, oper, Type.CREATE_ACCOUNT);  
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 93,"create account operation source address check failed");
	}
//	@Test
	public void create_oper_metadata_Max(){
		Operation oper = new Operation();
		Account a = tran.createAccountOne(genesis);
		oper.setSource_address(a.getAddress());   //set operation source address as another user
		oper.setDest_address(Account.generateAccount().getAddress());
		BigInteger bigInteger = new BigInteger("1024");
		BigInteger a1 = bigInteger.pow(1024).pow(1024);
		oper.setMetadata(a1.toString());
		tran.operationSourceCheck(a, oper, Type.CREATE_ACCOUNT);  
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 2,"create account operation metadata_Max check failed");
	}
//	@Test
	public void dest_address_exist(){
		Operation oper = new Operation();
		Account a1 = tran.createAccountOne(genesis);
		Account a2 = tran.createAccountOne(genesis);
		oper.setDest_address(a2.getAddress());
		tran.createAccount1(a1, oper);  
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 102,"create account exist dest_address check failed");
	}
//	@Test
	public void dest_address_invalid(){
		Operation oper = new Operation();
		Account a = tran.createAccountOne(genesis);
		oper.setDest_address("123");
		tran.createAccount1(a, oper);  
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 2,"create account invalid dest_address check failed");
	}
//	@Test
	public void dest_address_equal_source(){
		Operation oper = new Operation();
		Account a = tran.createAccountOne(genesis);
		oper.setDest_address(a.getAddress());
		tran.createAccount1(a, oper);  
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 101,"create account dest_address equal sour_address check failed");
	}
//	@Test
	public void init_balance_low(){
		Operation oper = new Operation();
		Account a = tran.createAccountOne(genesis);
		oper.setInit_balance(1L);
		tran.createAccount1(a, oper);  
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 100,"create account low init_balance check failed");
	}
//	@Test
	public void init_balance_0(){
		Operation oper = new Operation();
		Account a = tran.createAccountOne(genesis);
		oper.setInit_balance(0L);
		tran.createAccount1(a, oper);  
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 100,"create account low init_balance check failed");
	}
//	@Test
	public void init_balance_negative(){
		Operation oper = new Operation();
		Account a = tran.createAccountOne(genesis);
		oper.setInit_balance(-1L);
		tran.createAccount1(a, oper);  
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 2,"create account negative init_balance check failed");
	}
	
}
