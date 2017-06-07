package test;

import java.math.BigInteger;

import org.testng.annotations.Test;

import base.TestBase;
import cn.bubi.blockchain.adapter.Message.Operation.Type;
import model.Account;
import model.Operation;
import newop.Transaction;
import utils.APIUtil;

@Test
public class RecordTest extends TestBase{

	Transaction tran = new Transaction();
	
//	@Test
	public void testRecord(){
		Operation opt = new Operation();
		Account a1 = tran.createAccountOne(genesis);
		opt.setRecord_ext("1234");
		opt.setRecord_id("11");
		tran.record(a1, opt);
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 0, "存证交易验证失败");
	}
	
//	@Test
	public void record_operation_not_exist(){
		Account a1 = tran.createAccountOne(genesis);
		tran.noOperation(a1, Type.RECORD);	//create account
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 2,"record no operation check failed");
	}
	
	/**
	 * 验证存证id已存在无法重复创建成功
	 */
//	@Test
	public void testRecord_idunique(){
		Account a1 = tran.createAccountOne(genesis);
		Operation opt = new Operation();
		opt.setRecord_ext("1234");
		opt.setRecord_id("11");
		tran.record(a1, opt);
		Operation opt1 = new Operation();
		opt1.setRecord_ext("1234");
		opt1.setRecord_id("11");
		tran.record(a1, opt1);
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 3, "存证交易验证失败");  //ERRCODE_ALREADY_EXIST
	}
	/**
	 * 验证追加存证功能正常
	 */
//	@Test
	public void testRecordAppend(){
		Operation opt = new Operation();
		Account a1 = tran.createAccountOne(genesis);
		a1.seqence = Account.getSeq(a1);
		opt.setRecord_ext("1234");
		opt.setRecord_id("11");
		tran.record(a1, opt);   //original record
		opt.setRecord_address(a1.getAddress());
		a1.seqence = Account.getSeq(a1);
		tran.record(a1, opt);   //append record
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 0, "追加存证交易验证失败");
	}
//	@Test
	public void testRecord_metadata(){
		Operation opt = new Operation();
		Account a1 = tran.createAccountOne(genesis);
		opt.setRecord_ext("1234");
		opt.setRecord_id("11");
		opt.setMetadata("op_metadata");
		tran.record(a1, opt);   //original record
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 0, "存证设置transaction metadata 交易失败");
	}
//	@Test
	public void testTransaction_metadata(){
		Operation opt = new Operation();
		Account a1 = tran.createAccountOne(genesis);
		opt.setRecord_ext("1234");
		opt.setRecord_id("11");
		opt.setMetadata("op_metadata");
		opt.setTransaction_metadata("tran_metadata");
		tran.record(a1, opt);   //original record
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 0, "存证设置transaction metadata 交易失败");
	}
//	@Test
	public void testRecord_address_notexist(){
		Operation opt = new Operation();
		Account a1 = tran.createAccountOne(genesis);
		a1.seqence = Account.getSeq(a1);
		opt.setRecord_ext("1234");
		opt.setRecord_id("11");
		tran.record(a1, opt);   //original record
		opt.setRecord_address(Account.generateAccount().getAddress());
		tran.record(a1, opt);   //original record
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 4, "存证设置不存在的地址验证失败");  //ERRCODE_NOT_EXIST
	}
	
//	@Test
	public void testRecord_address_invalid(){
		Operation opt = new Operation();
		Account a1 = tran.createAccountOne(genesis);
		opt.setRecord_ext("1234");
		opt.setRecord_id("11");
		tran.record(a1, opt);   //original record
		opt.setRecord_address("123");
		tran.record(a1, opt);   //original record
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 2, "record address check failed");  //ERRCODE_NOT_EXIST
	}
//	@Test
	public void testRecord_ext(){
		Operation opt = new Operation();
		Account a1 = tran.createAccountOne(genesis);
		BigInteger bigInteger = new BigInteger("1024");
		BigInteger a = bigInteger.pow(1024).pow(1024);
		opt.setRecord_ext(a.toString());
		opt.setRecord_id("11");
		tran.record(a1, opt);   //original record
		opt.setRecord_address("123");
		tran.record(a1, opt);   //original record
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 2, "record ext Max size check failed");  //ERRCODE_NOT_EXIST
	}
//	@Test
	public void testRecord_address(){
		Operation opt = new Operation();
		Account a1 = tran.createAccountOne(genesis);
		BigInteger bigInteger = new BigInteger("1024");
		BigInteger a = bigInteger.pow(1024).pow(1024);
		opt.setRecord_ext(a.toString());
		opt.setRecord_id("11");
		opt.setRecord_address("123");
		tran.record(a1, opt);   //original record
		opt.setRecord_address("123");
		tran.record(a1, opt);   //original record
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 2, "record address check failed");  //ERRCODE_NOT_EXIST
	}
	
//	@Test
	public void record_id(){
		Operation opt = new Operation();
		Account a1 = tran.createAccountOne(genesis);
		opt.setRecord_ext("1234");
		opt.setRecord_id("1234567890abcdefghijklmnopqrstuvwxyz1234567890abcdefghijklmnopqrstu");
		tran.record(a1, opt);
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 2, "record id size greater than 64 check failed");
	}
}
