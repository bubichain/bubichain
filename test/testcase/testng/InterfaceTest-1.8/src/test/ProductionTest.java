package test;

import java.math.BigInteger;
import java.util.ArrayList;
import java.util.List;

import org.testng.annotations.Test;

import base.TestBase;
import cn.bubi.blockchain.adapter.Message.Operation.Type;
import model.Account;
import model.Input;
import model.Operation;
import model.Output;
import newop.Transaction;

@Test
public class ProductionTest extends TestBase{
	Operation oper = new Operation();
	Transaction tran = new Transaction();
//	@Test
	public void testProduction(){
		Account a = tran.createAccountOne(genesis);
		Account b = tran.createAccountOne(genesis);
		List<Input> inputs = new ArrayList<>();
		List<Output> outputs = new ArrayList<>();
		Output output = new Output();
		output.setAddress(b.getAddress());
		output.setMetadata("1234");
		outputs.add(output);
		oper.setOutputs(outputs);
		oper.setInputs(inputs);
		tran.production(a, oper);
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 0,"production check failed");
	}
	/*
	 * 验证带有input的供应链且签名错误的交易提示信息正确
	 */
//	@Test   //曾出现sql语句异常的问题，bug已修复
	public void testProduction_withInput(){
		Account a = tran.createAccountOne(genesis);
		oper.setOutputs(tran.createOutputs());
		oper.setInputs(tran.createInputs(a));
		tran.production(a, oper);
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 93,"production check failed");
	}
	/*
	 * 验证带有input的供应链且签名正确的交易成功
	 */
//	@Test  
	public void testProductionwithInput(){
		Account a = tran.createAccountOne(genesis);
		Account b = tran.createAccountOne(genesis);
		oper.setOutputs(tran.createOutputs());
		oper.setInputs(tran.createInputs(a,b));
		tran.production(a, b, oper);
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 0,"production check failed");
	}
	
	/*
	 * 验证供应链交易，缺少operation交易失败
	 */
//	@Test
	public void production_operation_not_exist(){
		Account a1 = tran.createAccountOne(genesis);
		tran.noOperation(a1, Type.PRODUCTION);	//create account
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 2,"production no operation check failed");
	}
	/*
	 * 对供应链交易中hash值非法校验
	 */
//	@Test
	public void production_input_hash(){
		Account a1 = tran.createAccountOne(genesis);
		Account a2 = tran.createAccountOne(genesis);
		List<Input> inputs = new ArrayList<>();
		Input input = new Input();
		input.setHash("1234");
		input.setIndex(0);
		input.setMetadata("1234");
		inputs.add(input);
		List<Output> outputs = new ArrayList<>();
		Output output = new Output();
		output.setAddress(a2.getAddress());
		output.setMetadata("1234");
		outputs.add(output);
		oper.setOutputs(outputs);
		oper.setInputs(inputs);
		a1.seqence = Account.getSeq(a1);
		tran.production(a1, oper);
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 2,"production input hash check failed");
	}
	/*
	 * 验证供应链交易，index值非法校验正确
	 */
//	@Test
	public void production_input_index(){
		Account a1 = tran.createAccountOne(genesis);
		Account a2 = tran.createAccountOne(genesis);
		List<Input> inputs = new ArrayList<>();
		Input input = new Input();
		input.setHash("a46dc6a33e9dd71a024d4a304279230b");
		input.setIndex(-1);
		input.setMetadata("1234");
		inputs.add(input);
		List<Output> outputs = new ArrayList<>();
		Output output = new Output();
		output.setAddress(a2.getAddress());
		output.setMetadata("1234");
		outputs.add(output);
		oper.setOutputs(outputs);
		oper.setInputs(inputs);
		a1.seqence = Account.getSeq(a1);
		tran.production(a1, oper);
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 2,"production input index check failed");
	}
//	@Test
	public void production_input_metadata(){
		Account a1 = tran.createAccountOne(genesis);
		Account a2 = tran.createAccountOne(genesis);
		List<Input> inputs = new ArrayList<>();
		Input input = new Input();
		input.setHash(Transaction.txInfo.getHash());
		input.setIndex(0);
		BigInteger bigInteger = new BigInteger("1024");
		BigInteger a = bigInteger.pow(1024).pow(1024);
		input.setMetadata(a.toString());
		inputs.add(input);
		List<Output> outputs = new ArrayList<>();
		Output output = new Output();
		output.setAddress(a2.getAddress());
		output.setMetadata("1234");
		outputs.add(output);
		oper.setOutputs(outputs);
		oper.setInputs(inputs);
		a1.seqence = Account.getSeq(a1);
		tran.production(a1, oper);
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 2,"production input index check failed");
	}
//	@Test
	public void production_output_address(){
		Account a1 = tran.createAccountOne(genesis);
		Account a2 = tran.createAccountOne(genesis);
		List<Input> inputs = new ArrayList<>();
		Input input = new Input();
		input.setHash("a46dc6a33e9dd71a024d4a304279230b5c30086ca884f1e5d358f9615a34aaa6");
		input.setIndex(1);
		input.setMetadata("1234");
//		inputs.add(input);
		List<Output> outputs = new ArrayList<>();
		Output output = new Output();
		output.setAddress("1234");
		output.setMetadata("1234");
		outputs.add(output);
		oper.setOutputs(outputs);
		oper.setInputs(inputs);
		a1.seqence = Account.getSeq(a1);
		tran.production(a1, oper);
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 2,"production output address check failed");
	}
//	@Test
	public void production_output_size(){
		Account a1 = tran.createAccountOne(genesis);
		Account a2 = tran.createAccountOne(genesis);
		List<Input> inputs = new ArrayList<>();
		Input input = new Input();
		input.setHash(Transaction.txInfo.getHash());
		input.setIndex(0);
		input.setMetadata("1234");
//		inputs.add(input);
		List<Output> outputs = new ArrayList<>();
		oper.setOutputs(outputs);
		oper.setInputs(inputs);
		a1.seqence = Account.getSeq(a1);
		tran.production(a1, oper);
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 2,"production output size check failed");
	}
}
