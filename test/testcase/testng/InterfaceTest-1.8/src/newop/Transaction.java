package newop;

import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import com.google.protobuf.ByteString;
import com.google.protobuf.InvalidProtocolBufferException;

import base.Log;
import base.TestBase;
import cn.bubi.blockchain.adapter.BlockChainAdapter;
import cn.bubi.blockchain.adapter.BlockChainAdapterProc;
import cn.bubi.blockchain.adapter.Message;
import cn.bubi.blockchain.adapter.Message.ChainTxStatus;
import cn.bubi.blockchain.adapter.Message.CloseTimeRange;
import cn.bubi.blockchain.adapter.Message.Operation.Type;
import cn.bubi.common.util.Tools;
import model.Account;
import model.Input;
import model.Operation;
import model.Output;
import model.Signer;
import model.Threshold;
import model.TxInfo;
import model.asset.Detail;
import utils.APIUtil;
import utils.GetAddress;
import utils.Hash;
import utils.HexUtil;
import utils.HttpUtil;
import utils.Result;
import utils.SignUtil;

public class Transaction extends TestBase{
	public static TxInfo txInfo = new TxInfo();
	public static TxInfo sendinfo = new TxInfo();
	public static Map<String, TxInfo> tx_send_map = new LinkedHashMap<>();
	public static Map<String, TxInfo> tx_recv_map = new LinkedHashMap<>();
	
	public  Transaction(){
		System.out.println(push_ip+":"+push_port + "       >>>>>>>>"+pull_ip+":"+pull_port);
		bbc.AddChainMethod(Message.ChainMessageType.CHAIN_TX_STATUS_VALUE,new BlockChainAdapterProc() {
			
			@Override
			public void ChainMethod(byte[] msg, int length) {
				txInfo = callback(msg, length);
			}
		});
		bbc.AddChainMethod(Message.ChainMessageType.CHAIN_STATUS_VALUE,new BlockChainAdapterProc(){
			@Override
			public void ChainMethod (byte[] msg, int length) {
				OnChainHello(msg, length);
			}
		});
		
//		bbc.Start();
		System.out.println("start to test chainadapter");
	}
	
	public  Transaction(BlockChainAdapter bbc){
		bbc.AddChainMethod(Message.ChainMessageType.CHAIN_PEER_MESSAGE_VALUE, new BlockChainAdapterProc() {
			
			@Override
			public void ChainMethod(byte[] msg, int length) {
				peerMessage(msg, length);
			}
		});
		bbc.Start();
		System.out.println("start to test peer message");
	}
	
	public TxInfo callback( byte[] msg, int length){
		String hash = "";
		try {
			Message.ChainTxStatus chainTx = Message.ChainTxStatus.parseFrom(msg);
			String errorCode  = chainTx.getErrorCode().toString();
//			System.out.println(chainTx.toString());
			switch (chainTx.getStatus().getNumber()) {
			case ChainTxStatus.TxStatus.COMPLETE_VALUE:
				txInfo.setHash(chainTx.getTxHash());
				txInfo.setSourceAdd(chainTx.getSourceAddress());
				txInfo.setRecv_Source_seq(chainTx.getSourceAccountSeq());
				txInfo.setRecv_NewAcc_seq(chainTx.getNewAccountSeq());
				txInfo.setError_code(chainTx.getErrorCode().getNumber());
				tx_recv_map.put(chainTx.getTxHash(), txInfo);
				hash = chainTx.getTxHash();
//				System.out.println(chainTx.toString());
				System.out.println("交易成功 ,hash:"+hash );
				break;
			case ChainTxStatus.TxStatus.FAILURE_VALUE:
				txInfo.setHash(chainTx.getTxHash());
				txInfo.setSourceAdd(chainTx.getSourceAddress());
				txInfo.setRecv_Source_seq(chainTx.getSourceAccountSeq());
				txInfo.setRecv_NewAcc_seq(chainTx.getNewAccountSeq());
				txInfo.setError_code(chainTx.getErrorCode().getNumber());
				tx_recv_map.put(chainTx.getTxHash(), txInfo);
//				System.out.println(chainTx.toString());
				Log.error("交易失败："+errorCode+" 错误信息: " + chainTx.getErrorDesc());
				break;
			case ChainTxStatus.TxStatus.PENDING_VALUE:
				break;
			case ChainTxStatus.TxStatus.CONFIRMED_VALUE:
				break;
			case ChainTxStatus.TxStatus.UNDEFINED_VALUE:
				break;
			default:
				break;
			}
		} catch (Exception e) {
			Log.error("接受交易结果出错",e);
		}
		return txInfo;
	}
	
	private TxInfo peerMessage(byte[] msg, int length){
		try {
			Message.ChainPeerMessage chainPeerMessage = Message.ChainPeerMessage.parseFrom(msg);
			txInfo.setPeerMessage(chainPeerMessage.getData().toString());
			System.out.println("receive: " + chainPeerMessage);
		} catch (InvalidProtocolBufferException e) {
			e.printStackTrace();
		}
		return txInfo;
	}
	
	private void OnChainHello(byte[] msg, int length) {
		try {
			cn.bubi.blockchain.adapter.Message.ChainStatus chain_status = cn.bubi.blockchain.adapter.Message.ChainStatus.parseFrom(msg);
			System.out.println("=================receive hello info============");
			System.out.println(chain_status);
			String add = GetAddress.getDesAddress(push_ip,19333);
			String self_add = chain_status.getSelfAddr();
			check.assertEquals(add, self_add, "发送消息地址不一致");
			System.out.println("done========");
			bbc.Stop();
		} catch (InvalidProtocolBufferException e) {
			e.printStackTrace();
		}
	}

	public void sendTransacionEvn(Account srcAcc, Message.Operation.Builder opb){
		Message.Transaction.Builder txb = Message.Transaction.newBuilder();
		txb.setSourceAddress(srcAcc.getAddress());
		txb.setSequenceNumber(srcAcc.seqence);
		txb.setFee(0);
		txb.addOperations(opb);
		
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		txeb.setTransaction(txb);
		
		Message.Signature.Builder s = Message.Signature.newBuilder();
		byte[] buffer = txb.build().toByteArray();
		try {
			s.setPublicKey(srcAcc.getPub_key());
			s.setSignData(ByteString.copyFrom(SignUtil.sign(buffer, srcAcc.getPri_key())));
		} catch (Exception e) {
			e.printStackTrace();
		}
		txeb.addSignatures(s);
		bbc.Send(Message.ChainMessageType.CHAIN_SUBMITTRANSACTION_VALUE, txeb.build().toByteArray());
	}
	
	public Map<String, Object> buildTransaction(Account srcAcc, Message.Operation.Builder opb){
		Message.Transaction.Builder txb = Message.Transaction.newBuilder();
		txb.setSourceAddress(srcAcc.getAddress());
		txb.setSequenceNumber(srcAcc.seqence);
		txb.setFee(0);
		txb.addOperations(opb);
		
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		txeb.setTransaction(txb);
		
		Message.Signature.Builder s = Message.Signature.newBuilder();
		byte[] buffer = txb.build().toByteArray();
		Map<String, Object> map = new LinkedHashMap<>();
		String hash = Hash.getHash(buffer);
		map.put("hash", hash);
		map.put(hash, txeb);
		try {
			s.setPublicKey(srcAcc.getPub_key());
			s.setSignData(ByteString.copyFrom(SignUtil.sign(buffer, srcAcc.getPri_key())));
		} catch (Exception e) {
			e.printStackTrace();
		}
		txeb.addSignatures(s);
		return map;
	}
	
	public void issue(Account srcAcc){
		List<Operation> oper_list = new ArrayList<>();
		Operation oper = new Operation();
		oper.setAsset_code("123456");
		oper.setAsset_amount(100L);
		oper.setType(2);
		oper.setAsset_issuer(srcAcc.getAddress());
		oper_list.add(oper);
		excute(srcAcc, oper_list);
	}
	public void issue1(Account srcAcc){
		List<Operation> oper_list = new ArrayList<>();
		Operation oper = new Operation();
		oper.setAsset_code("123456");
		oper.setAsset_amount(100L);
		oper.setType(2);
		oper.setAsset_issuer(srcAcc.getAddress());
		oper_list.add(oper);
		excute1(srcAcc, oper_list);
	}
	
	public void setoption(Account a1, Integer w1, Integer w2, Integer high, Integer low, Integer med, Integer mas){
		Account srcAcc = new Account();
		srcAcc.acc(led_acc, led_pri, led_pub, url_getAccInfo);  //get leddger 
		Account a2 = createAccountOne(srcAcc);
		Account a3 = createAccountOne(srcAcc);
		setOption(a1, a2, 6, a3, 6, 10, 10, 10, 10); 
	}
	public void issue(Account srcAcc, Account a1, Account a2, Operation opt){
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		List<Operation> operations = new ArrayList<>();
		opt.setType(2);
		opt.setAsset_issuer(srcAcc.getAddress());
		operations.add(opt);
		txeb =buildTransactionEnvSigners(srcAcc, a1, a2, operations);
		send(txeb);
	}
	/**
	 * 两个联名账户发行资产，只有联名账号进行签名
	 * @param srcAcc
	 * @param a1
	 * @param a2
	 */
	public void issue(Account srcAcc, Account a1, Account a2){
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		List<Operation> operations = new ArrayList<>();
		Operation oper = new Operation();
		oper.setAsset_code("123456");
		oper.setAsset_amount(100L);
		oper.setType(2);
		oper.setAsset_issuer(srcAcc.getAddress());
		operations.add(oper);
		txeb =buildTransactionEnvSigners(srcAcc, a1, a2, operations);
		send(txeb);
	}
	/**
	 * 一个联名账号进行发行资产操作
	 * @param srcAcc
	 * @param a1
	 */
	public void issue(Account srcAcc, Account a1){
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		List<Operation> operations = new ArrayList<>();
		Operation oper = new Operation();
		oper.setAsset_code("123456");
		oper.setAsset_amount(100L);
		oper.setType(2);
		oper.setAsset_issuer(srcAcc.getAddress());
		operations.add(oper);
		txeb =buildTransactionEnvSigners(srcAcc, a1, operations);
		send(txeb);
	}
	
	public void createAccount(Account srcAcc, Account a1, Account a2){
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		List<Operation> operations = new ArrayList<>();
		Operation oper = new Operation();
		oper.setDest_address(Account.generateAccount().getAddress());
		oper.setInit_balance(base_reserve);
		oper.setType(0);
		operations.add(oper);
		txeb =buildTransactionEnvSigners(srcAcc, a1, a2, operations);
		send(txeb);
	}
	
	public void issueUnique(Account srcAcc, Account a1, Account a2){
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		List<Operation> operations = new ArrayList<>();
		Operation oper = new Operation();
		oper.setAsset_issuer(srcAcc.getAddress());
		oper.setType(7);
		oper.setAsset_code("123Uni");
		oper.setAsset_detailed("1234detail");
		operations.add(oper);
		txeb =buildTransactionEnvSigners(srcAcc, a1, a2, operations);
		send(txeb);
	}
	
	public void initPayment(Account srcAcc, Account a1, Account a2){
		Operation issue_oper = new Operation();
		issue_oper.setAsset_code("123456");
		issue_oper.setAsset_amount(100L);
		issue_oper.setType(2);
		issue_oper.setAsset_issuer(srcAcc.getAddress());
		issue(srcAcc, issue_oper);   //发行资产，资产是未初始化的状态
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		List<Operation> operations = new ArrayList<>();
		Operation oper = new Operation();
		Account a =createAccountOne(genesis);  //创建目标地址
		oper.setDest_address(a.getAddress());
		oper.setAsset_issuer(srcAcc.getAddress());
		oper.setAsset_code("123456");
		oper.setAsset_amount(100L);
		oper.setType(5);
		operations.add(oper);
		txeb =buildTransactionEnvSigners(srcAcc, a1, a2, operations);
		send(txeb);
	}
	public void uniquePayment(Account srcAcc, Account a1, Account a2){
		Operation issueUnique_oper = new Operation();
		issueUnique_oper.setAsset_issuer(srcAcc.getAddress());
		issueUnique_oper.setAsset_code("asset_code");
		issueUnique_oper.setAsset_detailed("asset_detailedtest");
		issueUnique_oper.setType(7);
		issueAssetUnique(srcAcc, issueUnique_oper);
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		List<Operation> operations = new ArrayList<>();
		Operation oper = new Operation();
		Account a =createAccount(genesis);
		oper.setDest_address(a.getAddress());
		oper.setAsset_issuer(srcAcc.getAddress());
		oper.setAsset_code("asset_code");
		oper.setAsset_amount(100L);
		oper.setType(8);
		operations.add(oper);
		txeb =buildTransactionEnvSigners(srcAcc, a1, a2, operations);
		send(txeb);
	}
	public void evidence(Account srcAcc, Account a1, Account a2){
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		List<Operation> operations = new ArrayList<>();
		Operation oper = new Operation();
		oper.setRecord_ext("record_ext");
		oper.setRecord_id("recordid");
		oper.setType(Type.RECORD_VALUE);
		operations.add(oper);
		txeb =buildTransactionEnvSigners(srcAcc, a1, a2, operations);
		send(txeb);
	}
	public void setOption(Account srcAcc, Account a1, Account a2){
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		List<Operation> operations = new ArrayList<>();
		Operation oper = new Operation();
		Threshold threshold = new Threshold();
		threshold.setHigh_threshold(10);
		threshold.setLow_threshold(10);
		threshold.setMed_threshold(10);
		threshold.setMaster_weight(10);
		oper.setThreshold(threshold);
		oper.setType(Type.SET_OPTIONS_VALUE);
		operations.add(oper);
		txeb =buildTransactionEnvSigners(srcAcc, a1, a2, operations);
		send(txeb);
	}
	
	public List<Account> createAccountwithSigners(Integer w1, Integer w2){
		List<Account> accounts = new ArrayList<>();
		Account srcAcc = createAccountOne(genesis);
		Account a2 = createAccountOne(genesis);
		Account a3 = createAccountOne(genesis);
		setOption(srcAcc, a2, w1, a3, w2, 10, 10, 10, 10); // 设置a2,a3为srcAcc的联名账户srcAcc.threshold=10
		accounts.add(0, srcAcc);
		accounts.add(a2);
		accounts.add(a3);
		return accounts;
	}
	public List<Account> createAccountwithSigners(Integer w1, Integer w2, Integer high, Integer l, Integer m, Integer mas){
		List<Account> accounts = new ArrayList<>();
		Account srcAcc = createAccountOne(genesis);
		Account a2 = createAccountOne(genesis);
		Account a3 = createAccountOne(genesis);
		accounts.add(0, srcAcc);
		accounts.add(a2);
		accounts.add(a3);
		return accounts;
	}
	/**
	 * 创建一个联名账号，只有一个signer
	 * @param high
	 * @param low
	 * @param med
	 * @param master_weight
	 * @param weight
	 * @return
	 */
	public List<Account> createAccountwithSigners(Integer high, Integer low, Integer med, Integer master_weight, Integer weight){
		List<Account> accounts = new ArrayList<>();
		Account srcAcc = createAccountOne(genesis);
		Account a2 = createAccountOne(genesis);
		setOption(srcAcc, a2, high, low, med, master_weight, weight); // 设置a2为srcAcc的联名账户srcAcc.threshold=10
		accounts.add(0, srcAcc);
		accounts.add(a2);
		return accounts;
	}
	
	public void issue(Account srcAcc, Operation operation) {
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		List<Operation> operations = new ArrayList<>();
		operation.setType(2);
		operation.setAsset_issuer(srcAcc.getAddress());
		operations.add(operation);
		txeb = buildTransactionEnv(srcAcc, operations);
		send(txeb);
	}
	
	public void issue1(Account src, List<Operation> operations){
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		buildTransactionEnv(src,operations);
	}
	
	public Account initissue(Account srcAcc) {
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		Account a = createAccount(srcAcc);
		System.out.println("new account,a.address: " + a.getAddress());
		Operation op = new Operation();
		op.setType(2);
		op.setAsset_issuer(a.getAddress());
		op.setAsset_code("123");
		op.setAsset_amount(100000L);
		List<Operation> operations = new ArrayList<>();
		a.seqence = Account.getSeq(a);
		issue(a,op);
		return a;
	}
	
	public void production(Account srcAcc, Operation operation){
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		List<Operation> operations = new ArrayList<>();
		operation.setType(6);
		operations.add(operation);
		txeb = buildTransactionEnv(srcAcc, operations);
		bbc.Send(Message.ChainMessageType.CHAIN_SUBMITTRANSACTION_VALUE, txeb.build().toByteArray());
		APIUtil.wait(5);
	}
	public void production(Account srcAcc, Account dest_add, Operation operation){
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		List<Operation> operations = new ArrayList<>();
		operation.setType(6);
		operations.add(operation);
		txeb = buildTransactionEnvSigners1(srcAcc, dest_add, operations);
		bbc.Send(Message.ChainMessageType.CHAIN_SUBMITTRANSACTION_VALUE, txeb.build().toByteArray());
		APIUtil.wait(5);
	}
	
	public List<Output> createOutputs(){
		List<Output> outputs = new ArrayList<>();
		outputs.add(createOutput());
		return outputs;
	}
	
	public List<Input> createInputs(Account srcAcc){
		List<Input> inputs = new ArrayList<>();
		inputs.add(createInput(srcAcc));
		return inputs;
	}
	
	public List<Input> createInputs(Account srcAcc, Account dest_add){
		List<Input> inputs = new ArrayList<>();
		inputs.add(createInput(srcAcc, dest_add));
		return inputs;
	}
	
	private Output createOutput(){
		Account a = createAccountOne(genesis);
		Output output = new Output();
		output.setAddress(a.getAddress());
		output.setMetadata("outputMeta");
		return output;
	}
	
	private Input createInput(Account srcAcc){
		Account b = createAccountOne(genesis);
		Operation oper = new Operation();
		List<Input> inputs = new ArrayList<>();
		List<Output> outputs = new ArrayList<>();
		Output output = new Output();
		output.setAddress(b.getAddress());
		output.setMetadata("1234");
		outputs.add(output);
		oper.setOutputs(outputs);
		oper.setInputs(inputs);
		production(srcAcc, oper);
		String hash = sendinfo.getHash();
		Input input = new Input();
		input.setHash(hash);
		input.setIndex(0);
		input.setMetadata("inputmeta1");
		inputs.add(input);
		return input;
	}
	
	private Input createInput(Account srcAcc, Account out_des){
		Operation oper = new Operation();
		List<Input> inputs = new ArrayList<>();
		List<Output> outputs = new ArrayList<>();
		Output output = new Output();
		output.setAddress(out_des.getAddress());
		output.setMetadata("1234");
		outputs.add(output);
		oper.setOutputs(outputs);
		oper.setInputs(inputs);
		production(srcAcc, oper);
		String hash = sendinfo.getHash();
		Input input = new Input();
		input.setHash(hash);
		input.setIndex(0);
		input.setMetadata("inputmeta1");
		inputs.add(input);
		return input;
	}
	
	public void payment(Account src, Account dest, Operation operation){
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		List<Operation> operations = new ArrayList<>();
		operation.setType(1);
		operation.setAsset_issuer(src.getAddress());
		operation.setDest_address(dest.getAddress());
		operations.add(operation);
		txeb = buildTransactionEnv(src, operations);
		send(txeb);
	}
	public void paymentIOU(Account src, Account dest, Operation operation){
		Operation op1 = new Operation();
		op1.setAsset_amount(100L);
		op1.setAsset_code("abcd");
		issue(src, op1);
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		List<Operation> operations = new ArrayList<>();
		operation.setType(1);
		operation.setAsset_issuer(src.getAddress());
		operation.setDest_address(dest.getAddress());
		operations.add(operation);
		txeb = buildTransactionEnv(src, operations);
		send(txeb);
	}
	
	public void paymentUnique(Account src, Account dest, Operation operation){
		Operation opt = new Operation();
		opt.setAsset_issuer(src.getAddress());
		opt.setAsset_code("asset_code");
		opt.setAsset_detailed("asset_detailedtest");
		issueAssetUnique(src, opt);
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		List<Operation> operations = new ArrayList<>();
		operation.setType(8);
		if (operation.getAsset_issuer() == null) {
			operation.setAsset_issuer(src.getAddress());
		}
		operation.setDest_address(dest.getAddress());
		operations.add(operation);
		txeb = buildTransactionEnv(src, operations);
		send(txeb);
	}
	
	private void excute(Account srcAcc, List<Operation> operations){
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		txeb = buildTransactionEnv(srcAcc, operations);
		send(txeb);
	}
	private void excute1(Account srcAcc, List<Operation> operations){
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		txeb = buildTransactionEnv1(srcAcc, operations);
		send(txeb);
	}
	
	private void excute(Account srcAcc, Operation operation){
		List<Operation> oper_list = new ArrayList<>();
		oper_list.add(operation);
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		txeb = buildTransactionEnv(srcAcc, oper_list);
		send(txeb);
	}
	
	public void createAccount(Account srcAcc, Operation operation){
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		List<Operation> operations = new ArrayList<>();
		operations.add(operation);
		operation.setType(0);
		operation.setDest_address(Account.generateAccount().getAddress());
		txeb = buildTransactionEnv(srcAcc, operations);
		send(txeb);
	}
	
	public void initpayment(Account srcAcc, Account dest, Operation operation){
		Operation oper = new Operation();
		oper.setAsset_code("123");
		oper.setAsset_amount(100L);
		issue(srcAcc, oper);
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		List<Operation> operations = new ArrayList<>();
		operations.add(operation);
		operation.setType(5);
		txeb = buildTransactionEnv(srcAcc, operations);
		send(txeb);
	}
	public void createAccount(Account srcAcc, List<Operation> operations){
		srcAcc.seqence = Account.getSeq(srcAcc);
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		for (Operation oper : operations) {
			oper.setType(0);
		}
		txeb = buildTransactionEnv(srcAcc, operations);
		send(txeb);
	}
	
	public Account createAccount1(Account srcAcc, Operation operation){
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		Account a = Account.generateAccount();
		operation.setType(0);
		if (operation.getDest_address()!=null) {
			operation.setDest_address(operation.getDest_address());
		}else{
			operation.setDest_address(a.getAddress());
		}
		txeb = buildTransactionEnv(srcAcc, operation);
		send(txeb);
		return a;
	}
	
	public void createSignerOld(Account srcAcc, Account a1, Integer w1, Account a2, Integer w2){
		srcAcc.seqence = Account.getSeq(srcAcc);
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		Account a = Account.generateAccount();
		Operation operation = new Operation();
		Signer signer1 = new Signer();					//set signers
		signer1.setAddress(a1.getAddress());
		signer1.setWeight(w1);
		Signer signer2 = new Signer();
		signer2.setAddress(a2.getAddress());
		signer2.setWeight(w2);
		List<Signer> signer_list = new ArrayList<>();
		signer_list.add(signer1);
		signer_list.add(signer2);
		operation.setSigners(signer_list);
		operation.setType(0);
		if (operation.getDest_address()!=null) {
			operation.setDest_address(operation.getDest_address());
		}else{
			operation.setDest_address(a.getAddress());
		}
		txeb = buildTransactionEnv(srcAcc, operation);
		send(txeb);
	}
	//暂时没用
	private void createSigner(Account srcAcc, Account a1, Integer w1, Account a2, Integer w2){
		srcAcc.seqence = Account.getSeq(srcAcc);
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		Operation operation = new Operation();
		Signer signer1 = new Signer();					//set signers
		signer1.setAddress(a1.getAddress());
		signer1.setWeight(w1);
		Signer signer2 = new Signer();
		signer2.setAddress(a2.getAddress());
		signer2.setWeight(w2);
		List<Signer> signer_list = new ArrayList<>();
		signer_list.add(signer1);
		signer_list.add(signer2);
		operation.setSigners(signer_list);
		operation.setType(0);
		txeb = buildTransactionEnv(srcAcc, operation);
		send(txeb);
	}
	
	
	public void noOperation(Account srcAcc,Type type ){
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		txeb = buildTransactionEnv_noOperation(srcAcc,type);
		send(txeb);
	}
	
	public void invalidPublickey(Account srcAcc,Operation operation, Type type, String publickey){
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		operation.setAsset_issuer(srcAcc.getAddress());
		setType(type, operation);
		txeb = buildTransactionEnvPublickey(srcAcc,operation, type, publickey);
		send(txeb);
	}
	
	private void setType(Type type, Operation operation){
		if (type == Type.CREATE_ACCOUNT) {
			operation.setType(0);
		}
		if (type == Type.ISSUE_ASSET) {
			operation.setType(2);
		}
		if (type == Type.PAYMENT) {
			operation.setType(1);
		}
		if (type == Type.INIT_PAYMENT) {
			operation.setType(5);
		}
		if (type == Type.ISSUE_UNIQUE_ASSET) {
			operation.setType(7);
		}
		if (type == Type.PAYMENT_UNIQUE_ASSET) {
			operation.setType(8);
		}
		if (type == Type.RECORD) {
			operation.setType(9);
		}
		if (type == Type.SET_OPTIONS) {
			operation.setType(4);
		}
		if (type == Type.PRODUCTION) {
			operation.setType(6);
		}
	}
	public void sequceCheck(Account srcAcc,Operation operation, Type type){
		srcAcc.seqence = Account.getSeq(srcAcc);
		if (type == Type.CREATE_ACCOUNT) {
			operation.setType(0);
		}
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		txeb = buildTransactionEnvseq(srcAcc,operation, type);
		send(txeb);
	}
	
	public void submit(Account srcAcc, Operation op1, Operation op2){
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		List<Operation> operations = new ArrayList<>();
		operations.add(0, op1);
		operations.add(op2);
		txeb = buildTransactionEnv(srcAcc,operations);
		send(txeb);
	}
	public void submit(Account a1, Account a2, Operation op1, Operation op2){
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		List<Operation> operations = new ArrayList<>();
		operations.add(0, op1);
		operations.add(op2);
		txeb = buildTransactionEnv(a1, a2, operations);
		send(txeb);
	}
	public void submitSigners(Account srcAcc, Account a1, Operation op1, Operation op2){
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		List<Operation> operations = new ArrayList<>();
		operations.add(0, op1);
		operations.add(op2);
		txeb = buildTransactionEnvSigners1(srcAcc, a1, operations);
		send(txeb);
	}
	
	public void feeCheck(Account srcAcc,Operation operation, Type type){
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		txeb = buildTransactionEnvFee(srcAcc,operation, type);
		send(txeb);
	}
	public void operationMetadataCheck(Account srcAcc,Operation operation, Type type){
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		txeb = buildTransactionEnv(srcAcc,operation, type);
		send(txeb);
	}
	public void operationSourceCheck(Account srcAcc,Operation operation, Type type){
		if (type == Type.CREATE_ACCOUNT) {
			operation.setType(0);
		}
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		txeb = buildTransactionEnv(srcAcc,operation, type);
		send(txeb);
	}
	
	public void issueAssetUnique(Account srcAcc, Operation operation){
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		operation.setType(7);
		txeb = buildTransactionEnv(srcAcc, operation);
		send(txeb);
	}
	
	private void send(Message.TransactionEnv.Builder txeb){
		bbc.Send(Message.ChainMessageType.CHAIN_SUBMITTRANSACTION_VALUE, txeb.build().toByteArray());
		APIUtil.wait(6);
	}
	
	public void record(Account srcAcc, Operation operation){
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		operation.setType(9);
		txeb = buildTransactionEnv(srcAcc, operation);
		send(txeb);
	}
	
	public List<Account> createAccount(Account srcAcc, Integer account_num){
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		List<Operation> oper_list = new ArrayList<>();
		List<Account> acc_list = new ArrayList<>();
		for (int i = 0; i < account_num; i++) {
			Operation opt = new Operation();
			opt.setType(0);
			Account a = Account.generateAccount();
			opt.setDest_address(a.getAddress());
			acc_list.add(a);
			oper_list.add(opt);
		}
		txeb = buildTransactionEnv(srcAcc, oper_list);
		bbc.Send(Message.ChainMessageType.CHAIN_SUBMITTRANSACTION_VALUE, txeb.build().toByteArray());
		APIUtil.wait(3);
		return acc_list;
	}
	
	public Account createAccount(Account srcAcc){
		Account dest_address = Account.generateAccount();
		Operation operation = new Operation();
		operation.setDest_address(dest_address.getAddress());
		operation.setType(Type.CREATE_ACCOUNT_VALUE);
		operation.setInit_balance(base_reserve+init_balance*2);
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		List<Operation> operations = new ArrayList<>();
		operations.add(operation);
		txeb = buildTransactionEnv(srcAcc, operations);
		send(txeb);
		return dest_address;
	}
	
	public Account createAccountOne(Account srcAcc){
		Account dest_address = createAccount(srcAcc);
		return dest_address;
	}
	
	public List<Input> createInput(String hash, Integer index, String metadata){
		List<Input> inputs = new ArrayList<>();
		if (hash!=null && index!=null && metadata !=null) {
			Input input = new Input();
			input.setHash(hash);
			input.setIndex(index);
			input.setMetadata(metadata);
			inputs.add(input);
		}else{
			System.out.println("传入参数有误");
		}
		return inputs;
	}
	
	public List<Input> createInput(){
		List<Input> inputs = new ArrayList<>();
		return inputs;
	}
	
	public List<Output> createOutput(String address, String metadata){
		List<Output> outputs = new ArrayList<>();
		Output output = new Output();
		output.setAddress(address);
		output.setMetadata(metadata);
		outputs.add(output);
		return outputs;
	}
	
	public Account createAccWithUniqueAsset(Account srcAcc){
		Account a = createAccountOne(genesis);
		Operation op1 = new Operation();
		op1.setDest_address(Account.generateAccount().getAddress());
		op1.setType(0);
		op1.setInit_balance(init_balance+base_reserve);
		Operation op2 = new Operation();
		op2.setType(7);
		op2.setAsset_issuer(a.getAddress());
		op2.setAsset_amount(100L);
		op2.setAsset_code("123");
		op2.setAsset_detailed("1123");
		submit(a, op1, op2);
		return a;
	}
	
	public void setOption(Account account, Integer High_th, Integer Low_th, Integer Med_th, Integer Mas_w){
		Operation op = new Operation();
		Threshold threshold = new Threshold();
		threshold.setHigh_threshold(High_th);
		threshold.setLow_threshold(Low_th);
		threshold.setMed_threshold(Med_th);
		threshold.setMaster_weight(Mas_w);
		op.setThreshold(threshold);
		op.setType(Type.SET_OPTIONS_VALUE);
		excute(account, op);
	}
	public void setOption(Account account, String metadata){
		Operation op = new Operation();
		op.setType(Type.SET_OPTIONS_VALUE);
		op.setMetadata(metadata);
		excute(account, op);
	}
	
	public void setOptionWithThreshold(Account account, String metadata){
		Operation op = new Operation();
		op.setType(Type.SET_OPTIONS_VALUE);
		op.setMetadata(metadata);
		Threshold threshold = new Threshold();
		threshold.setHigh_threshold(10);
		threshold.setLow_threshold(10);
		threshold.setMed_threshold(10);
		threshold.setMaster_weight(10);
		threshold.setMetadata(metadata);
		op.setThreshold(threshold);
		excute(account, op);
	}
	
	public void setOption(Account account, List<Signer> signers){
		Operation op = new Operation();
		op.setSigners(signers);
		op.setType(Type.SET_OPTIONS_VALUE);
		excute(account, op);
	}
	public void setOption(Account account, Account a1, Integer w1, Account a2, Integer w2,  Integer high_threshold, Integer low_threshold, Integer med_threshold, Integer master_weight){
		account.seqence = Account.getSeq(account);
		List<Signer> signers = new ArrayList<>();
		Signer s1 = new Signer();
		s1.setAddress(a1.getAddress());
		s1.setWeight(w1);
		Signer s2 = new Signer();
		s2.setAddress(a2.getAddress());
		s2.setWeight(w2);
		signers.add(s1);
		signers.add(s2);
		Threshold threshold = new Threshold();
		threshold.setHigh_threshold(high_threshold);
		threshold.setLow_threshold(low_threshold);
		threshold.setMed_threshold(med_threshold);
		threshold.setMaster_weight(master_weight);
		Operation op = new Operation();
		op.setSigners(signers);
		op.setType(Type.SET_OPTIONS_VALUE);
		op.setThreshold(threshold);
		excute(account, op);
	}
	/**
	 * 创建一个联名账号带一个signer
	 * @param account
	 * @param high_threshold
	 * @param low_threshold
	 * @param med_threshold
	 * @param master_weight
	 * @param weight   signer的weight
	 */
	public void setOption(Account account, Account a1, Integer high_threshold, Integer low_threshold, Integer med_threshold, Integer master_weight, Integer weight){
		List<Signer> signers = new ArrayList<>();
		Signer s1 = new Signer();
		s1.setAddress(a1.getAddress());
		s1.setWeight(weight);
		Threshold threshold = new Threshold();
		threshold.setHigh_threshold(high_threshold);
		threshold.setLow_threshold(low_threshold);
		threshold.setMed_threshold(med_threshold);
		threshold.setMaster_weight(master_weight);
		Operation op = new Operation();
		signers.add(s1);
		op.setSigners(signers);
		op.setType(Type.SET_OPTIONS_VALUE);
		op.setThreshold(threshold);
		excute(account, op);
	}
	
	public Account createAccountUnite(Account acount1, Account account2){
		return null;
	}
	
	/**
	 * 多操作
	 * @param srcAcc
	 * @param operations
	 * @return
	 */
	private static Message.TransactionEnv.Builder buildTransactionEnv(Account srcAcc, List<Operation> operations) {
		srcAcc.seqence = Account.getSeq(srcAcc);
		Message.Transaction.Builder txb = Message.Transaction.newBuilder();
		for (Operation operation : operations) {
			Message.Operation.Builder op = Message.Operation.newBuilder();
			op = operation(operation);
			txb.addOperations(op);
		}
		txb.setSourceAddress(srcAcc.getAddress());
		txb.setSequenceNumber(srcAcc.seqence);
		txb.setFee(operations.size() * fee);
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		txeb.setTransaction(txb);

		Message.Signature.Builder s = Message.Signature.newBuilder();
		byte[] buffer = txb.build().toByteArray();
		try {
			s.setPublicKey(srcAcc.getPub_key());
			s.setSignData(ByteString.copyFrom(SignUtil.sign(buffer, srcAcc.getPri_key())));
		} catch (Exception e) {
			e.printStackTrace();
		}
		txeb.addSignatures(s);
//		System.out.println(txeb);
//		System.out.println("hash: " + Hash.getHash(buffer));
		sendinfo.setHash(Hash.getHash(buffer));
		sendinfo.setSend_source_seq(srcAcc.seqence);
		sendinfo.setTranEnv(txeb);
		tx_send_map.put(Hash.getHash(buffer), sendinfo);
		return txeb;
	}
	/**
	 * 此交易序号大一
	 * @param srcAcc
	 * @param operations
	 * @return
	 */
	private static Message.TransactionEnv.Builder buildTransactionEnv1(Account srcAcc, List<Operation> operations) {
		srcAcc.seqence = Account.getSeq(srcAcc)+1;
		Message.Transaction.Builder txb = Message.Transaction.newBuilder();
		for (Operation operation : operations) {
			Message.Operation.Builder op = Message.Operation.newBuilder();
			op = operation(operation);
			txb.addOperations(op);
		}
		txb.setSourceAddress(srcAcc.getAddress());
		txb.setSequenceNumber(srcAcc.seqence);
		txb.setFee(operations.size() * fee); 
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		txeb.setTransaction(txb);
		
		Message.Signature.Builder s = Message.Signature.newBuilder();
		byte[] buffer = txb.build().toByteArray();
		try {
			s.setPublicKey(srcAcc.getPub_key());
			s.setSignData(ByteString.copyFrom(SignUtil.sign(buffer, srcAcc.getPri_key())));
		} catch (Exception e) {
			e.printStackTrace();
		}
		txeb.addSignatures(s);
//		System.out.println(txeb);
//		System.out.println("hash: " + Hash.getHash(buffer));
		sendinfo.setHash(Hash.getHash(buffer));
		sendinfo.setSend_source_seq(srcAcc.seqence);
		sendinfo.setTranEnv(txeb);
		tx_send_map.put(Hash.getHash(buffer), sendinfo);
		return txeb;
	}
	private static Message.TransactionEnv.Builder buildTransactionEnv(Account srcAcc, Account a1, List<Operation> operations){
		Message.Transaction.Builder txb = Message.Transaction.newBuilder();
		for (Operation operation : operations) {
			Message.Operation.Builder op = Message.Operation.newBuilder();
			op = operation(operation);
			txb.addOperations(op);
		}
		txb.setSourceAddress(srcAcc.getAddress());
		txb.setSequenceNumber(srcAcc.seqence);
		txb.setFee(operations.size()*fee);
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		txeb.setTransaction(txb);
		
		Message.Signature.Builder s = Message.Signature.newBuilder();
		byte[] buffer = txb.build().toByteArray();
		try {
			s.setPublicKey(srcAcc.getPub_key());
			s.setSignData(ByteString.copyFrom(SignUtil.sign(buffer, srcAcc.getPri_key())));
		} catch (Exception e) {
			e.printStackTrace();
		}
		txeb.addSignatures(s);
//		System.out.println(txeb);
//		System.out.println("hash: " + Hash.getHash(buffer));
		sendinfo.setHash(Hash.getHash(buffer));
		sendinfo.setSend_source_seq(srcAcc.seqence);
		sendinfo.setTranEnv(txeb);
		return txeb;
		
	}
	
	private static Message.TransactionEnv.Builder buildTransactionEnv(Account srcAcc, Operation operation) {
		srcAcc.seqence = Account.getSeq(srcAcc);
		Message.Transaction.Builder txb = Message.Transaction.newBuilder();
		Message.Operation.Builder op = Message.Operation.newBuilder();
		op = operation(operation);
		txb.addOperations(op);
		txb.setSourceAddress(srcAcc.getAddress());
		txb.setSequenceNumber(srcAcc.seqence);
		if (operation.getTimeRange() != null) {
			CloseTimeRange.Builder closetime = CloseTimeRange.newBuilder();
			closetime.setMinTime(operation.getTimeRange().getMinTime());
			closetime.setMaxTime(operation.getTimeRange().getMaxTime());
			txb.setCloseTimeRange(closetime);
		}
		if (operation.getFee() != null) {
			txb.setFee(operation.getFee());
		} else {
			txb.setFee(fee);
		}
		if (operation.getTransaction_metadata() != null) {
			txb.setMetadata(HexUtil.toByteString(operation.getTransaction_metadata()));
		}
		if (operation.getTranSource_add() != null) {
			txb.setSourceAddress(operation.getTranSource_add());
		}
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		txeb.setTransaction(txb);

		Message.Signature.Builder s = Message.Signature.newBuilder();
		byte[] buffer = txb.build().toByteArray();
		try {
			s.setPublicKey(srcAcc.getPub_key());
			s.setSignData(ByteString.copyFrom(SignUtil.sign(buffer, srcAcc.getPri_key())));
		} catch (Exception e) {
			e.printStackTrace();
		}
		txeb.addSignatures(s);
//		System.out.println(txeb);
//		System.out.println("hash: " + Hash.getHash(buffer));
		sendinfo.setHash(Hash.getHash(buffer));
		sendinfo.setSend_source_seq(srcAcc.seqence);
		sendinfo.setTranEnv(txeb);
		return txeb;
	}
	private static Message.TransactionEnv.Builder buildTransactionEnv(Account srcAcc, Operation operation,Message.Operation.Type oper_type){
		srcAcc.seqence = Account.getSeq(srcAcc);
		Message.Transaction.Builder txb = Message.Transaction.newBuilder();
		Message.Operation.Builder op = Message.Operation.newBuilder();
		op = operation(operation);
		txb.addOperations(op);
		txb.setSourceAddress(srcAcc.getAddress());
		txb.setSequenceNumber(srcAcc.seqence);
		txb.setFee(fee);
		if (operation.getTransaction_metadata()!=null) {
			txb.setMetadata(HexUtil.toByteString(operation.getTransaction_metadata()));
		}
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		txeb.setTransaction(txb);
		
		Message.Signature.Builder s = Message.Signature.newBuilder();
		byte[] buffer = txb.build().toByteArray();
		try {
			s.setPublicKey(srcAcc.getPub_key());
			s.setSignData(ByteString.copyFrom(SignUtil.sign(buffer, srcAcc.getPri_key())));
		} catch (Exception e) {
			e.printStackTrace();
		}
		txeb.addSignatures(s);
//		System.out.println(txeb);
//		System.out.println("hash: " + Hash.getHash(buffer));
		sendinfo.setHash(Hash.getHash(buffer));
		sendinfo.setSend_source_seq(srcAcc.seqence);
		sendinfo.setTranEnv(txeb);
		tx_send_map.put(Hash.getHash(buffer), sendinfo);
		return txeb;
		
	}
	
	/**
	 * 联合签名账户a1, a2发起签名操作
	 * @param srcAcc
	 * @param a1
	 * @param a2
	 * @param operations
	 * @return
	 */
	private static Message.TransactionEnv.Builder buildTransactionEnvSigners(Account srcAcc, Account a1, Account a2, List<Operation> operations){
		List<Account> signers = new ArrayList<>();
		signers.add(a1);
		signers.add(a2);
		return buildTransactionEnvSigners(srcAcc, signers, operations);
	}
	/**
	 * 源账户发起交易，多个signers进行签名的操作
	 * @param srcAcc
	 * @param accounts
	 * @param operations
	 * @return
	 */
	private static Message.TransactionEnv.Builder buildTransactionEnvSigners(Account srcAcc, List<Account> accounts, List<Operation> operations){
		srcAcc.seqence = Account.getSeq(srcAcc);
		Message.Transaction.Builder txb = Message.Transaction.newBuilder();
		for (Operation operation : operations) {
			Message.Operation.Builder op = Message.Operation.newBuilder();
			op = operation(operation);
			txb.addOperations(op);
		}
		txb.setSourceAddress(srcAcc.getAddress());
		txb.setSequenceNumber(srcAcc.seqence);
		txb.setFee(operations.size()*fee);
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		txeb.setTransaction(txb);
		byte[] buffer = txb.build().toByteArray();
		for (Account account : accounts) {
			Message.Signature.Builder signer = Message.Signature.newBuilder();
			signer.setPublicKey(account.getPub_key());
			try {
				signer.setSignData(ByteString.copyFrom(SignUtil.sign(buffer, account.getPri_key())));
			} catch (Exception e) {
				e.printStackTrace();
			}
			txeb.addSignatures(signer);
		}
		Message.Signature.Builder s2 = Message.Signature.newBuilder();
		System.out.println(txeb);
//		System.out.println("hash: " + Hash.getHash(buffer));
		sendinfo.setHash(Hash.getHash(buffer));
		sendinfo.setSend_source_seq(srcAcc.seqence);
		sendinfo.setTranEnv(txeb);
		tx_send_map.put(Hash.getHash(buffer), sendinfo);
		return txeb;
	}
	/**
	 * 一个联合签名账号进行签名的操作
	 * @param srcAcc
	 * @param a1
	 * @param operations
	 * @return
	 */
	private static Message.TransactionEnv.Builder buildTransactionEnvSigners(Account srcAcc, Account a1, List<Operation> operations){
		List<Account> signers = new ArrayList<>();
		signers.add(a1);
		return buildTransactionEnvSigners(srcAcc, signers, operations);
	}
	
	private static Message.Operation.Builder operation(Operation operation){
		Message.Operation.Builder op = Message.Operation.newBuilder();
		int type = operation.getType();
//		System.out.println("操作类型： "+type);
		switch (type) {
		case Message.Operation.Type.CREATE_ACCOUNT_VALUE:	//createaccount
			createAccount(op, operation);
			break;
		case Message.Operation.Type.ISSUE_ASSET_VALUE:
			issue(op, operation);
			break;
		case Message.Operation.Type.PAYMENT_VALUE:
			transfer(op, operation);
			break;
		case Message.Operation.Type.ISSUE_UNIQUE_ASSET_VALUE:
			issueAssetUnique(op, operation);
			break;
		case Message.Operation.Type.PAYMENT_UNIQUE_ASSET_VALUE:
			transferUnique(op, operation);
			break;
		case Message.Operation.Type.PRODUCTION_VALUE:
			production(op, operation);
			break;
		case Message.Operation.Type.INIT_PAYMENT_VALUE:
			inittransfer(op, operation);
			break;
		case Message.Operation.Type.SET_OPTIONS_VALUE:
			setOption(op, operation);
			break;
		case Message.Operation.Type.RECORD_VALUE:
			record(op, operation);
			break;
		default:
			break;
		}
		return op;
	}
	
	/**
	 * 源账户和联名账户一起签名的操作
	 * @param srcAcc
	 * @param a1
	 * @param operations
	 * @return
	 */
	private static Message.TransactionEnv.Builder buildTransactionEnvSigners1(Account srcAcc, Account a1, List<Operation> operations){
		srcAcc.seqence = Account.getSeq(srcAcc);
		Message.Transaction.Builder txb = Message.Transaction.newBuilder();
		for (Operation operation : operations) {
			Message.Operation.Builder op = Message.Operation.newBuilder();
			op = operation(operation);
			txb.addOperations(op);
		}
		txb.setSourceAddress(srcAcc.getAddress());
		txb.setSequenceNumber(srcAcc.seqence);
		txb.setFee(operations.size()*fee);
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		txeb.setTransaction(txb);
		
		Message.Signature.Builder s1 = Message.Signature.newBuilder();
		Message.Signature.Builder s2 = Message.Signature.newBuilder();
		byte[] buffer = txb.build().toByteArray();
		try {
			s1.setPublicKey(a1.getPub_key());
			s2.setPublicKey(srcAcc.getPub_key());
			s1.setSignData(ByteString.copyFrom(SignUtil.sign(buffer, a1.getPri_key())));
			System.out.println("a1签名后的："+s1.toString());
			s2.setSignData(ByteString.copyFrom(SignUtil.sign(buffer, srcAcc.getPri_key())));
			
			System.out.println("a2签名后的："+s2.toString());
		} catch (Exception e) {
			e.printStackTrace();
		}
		txeb.addSignatures(s1);
		txeb.addSignatures(s2);
		System.out.println(txeb);
//		System.out.println("hash: " + Hash.getHash(buffer));
		sendinfo.setHash(Hash.getHash(buffer));
		sendinfo.setSend_source_seq(srcAcc.seqence);
		sendinfo.setTranEnv(txeb);
		return txeb;
	}
	
	public static Integer getErrorcode(String hash){
		String result = HttpUtil.doget(url_getAccInfo, "getTransactionHistory", "hash", hash);
		int error_code = Result.getErrorCode(result);
		return error_code;
	}
	
	public static Integer getErrorCode(){
		System.out.println(Transaction.sendinfo.getHash());
		Integer error_code = Transaction.tx_recv_map.get(Transaction.sendinfo.getHash()).getError_code();
		return error_code;
	}
	

	private static Message.TransactionEnv.Builder buildTransactionEnvPublickey(Account srcAcc, Operation operation,Message.Operation.Type oper_type, String publickey){
		srcAcc.seqence = Account.getSeq(srcAcc);
		Message.Transaction.Builder txb = Message.Transaction.newBuilder();
		Message.Operation.Builder op = Message.Operation.newBuilder();
		op = operation(operation);
		txb.addOperations(op);
		txb.setSourceAddress(srcAcc.getAddress());
		txb.setSequenceNumber(srcAcc.seqence);
		txb.setFee(fee);
		if (operation.getTransaction_metadata()!=null) {
			txb.setMetadata(HexUtil.toByteString(operation.getTransaction_metadata()));
		}
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		txeb.setTransaction(txb);
		
		Message.Signature.Builder s = Message.Signature.newBuilder();
		byte[] buffer = txb.build().toByteArray();
		try {
			s.setPublicKey(publickey);
			s.setSignData(ByteString.copyFrom(SignUtil.sign(buffer, srcAcc.getPri_key())));
		} catch (Exception e) {
			e.printStackTrace();
		}
		txeb.addSignatures(s);
//		System.out.println(txeb);
//		System.out.println("hash: " + Hash.getHash(buffer));
		sendinfo.setHash(Hash.getHash(buffer));
		sendinfo.setSend_source_seq(srcAcc.seqence);
		sendinfo.setTranEnv(txeb);
		tx_send_map.put(Hash.getHash(buffer), sendinfo);
		return txeb;
		
	}
	
	private static Message.TransactionEnv.Builder buildTransactionEnvseq(Account srcAcc, Operation operation,Message.Operation.Type oper_type){
		Message.Transaction.Builder txb = Message.Transaction.newBuilder();
		Message.Operation.Builder op = Message.Operation.newBuilder();
		op = operation(operation);
		txb.addOperations(op);
		txb.setSourceAddress(srcAcc.getAddress());
		txb.setSequenceNumber(operation.getSequencenum());
		txb.setFee(fee);
		if (operation.getTransaction_metadata()!=null) {
			txb.setMetadata(HexUtil.toByteString(operation.getTransaction_metadata()));
		}
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		txeb.setTransaction(txb);
		
		Message.Signature.Builder s = Message.Signature.newBuilder();
		byte[] buffer = txb.build().toByteArray();
		try {
			s.setPublicKey(srcAcc.getPub_key());
			s.setSignData(ByteString.copyFrom(SignUtil.sign(buffer, srcAcc.getPri_key())));
		} catch (Exception e) {
			e.printStackTrace();
		}
		txeb.addSignatures(s);
//		System.out.println(txeb);
//		System.out.println("hash: " + Hash.getHash(buffer));
		sendinfo.setHash(Hash.getHash(buffer));
		sendinfo.setSend_source_seq(srcAcc.seqence);
		sendinfo.setTranEnv(txeb);
		return txeb;
	}
	
	private static Message.TransactionEnv.Builder buildTransactionEnvFee(Account srcAcc, Operation operation,Message.Operation.Type oper_type){
		srcAcc.seqence = Account.getSeq(srcAcc);
		Message.Transaction.Builder txb = Message.Transaction.newBuilder();
		Message.Operation.Builder op = Message.Operation.newBuilder();
		op = operation(operation);
		txb.addOperations(op);
		txb.setSourceAddress(srcAcc.getAddress());
		txb.setSequenceNumber(srcAcc.seqence);
		txb.setFee(operation.getFee());
		if (operation.getTransaction_metadata()!=null) {
			txb.setMetadata(HexUtil.toByteString(operation.getTransaction_metadata()));
		}
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		txeb.setTransaction(txb);
		
		Message.Signature.Builder s = Message.Signature.newBuilder();
		byte[] buffer = txb.build().toByteArray();
		try {
			s.setPublicKey(srcAcc.getPub_key());
			s.setSignData(ByteString.copyFrom(SignUtil.sign(buffer, srcAcc.getPri_key())));
		} catch (Exception e) {
			e.printStackTrace();
		}
		txeb.addSignatures(s);
//		System.out.println(txeb);
//		System.out.println("hash: " + Hash.getHash(buffer));
		sendinfo.setHash(Hash.getHash(buffer));
		sendinfo.setSend_source_seq(srcAcc.seqence);
		sendinfo.setTranEnv(txeb);
		tx_send_map.put(Hash.getHash(buffer), sendinfo);
		return txeb;
	}
	private static Message.TransactionEnv.Builder buildTransactionEnv_noOperation(Account srcAcc, Message.Operation.Type type){
		srcAcc.seqence = Account.getSeq(srcAcc);
		Message.Transaction.Builder txb = Message.Transaction.newBuilder();
		Message.Operation.Builder op = Message.Operation.newBuilder();
		op.setType(type);
		txb.addOperations(op);
		txb.setSourceAddress(srcAcc.getAddress());
		txb.setSequenceNumber(srcAcc.seqence);
		txb.setFee(fee);
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		txeb.setTransaction(txb);
		
		Message.Signature.Builder s = Message.Signature.newBuilder();
		byte[] buffer = txb.build().toByteArray();
		try {
			s.setPublicKey(srcAcc.getPub_key());
			s.setSignData(ByteString.copyFrom(SignUtil.sign(buffer, srcAcc.getPri_key())));
		} catch (Exception e) {
			e.printStackTrace();
		}
		txeb.addSignatures(s);
//		System.out.println(txeb);
//		System.out.println("hash: " + Hash.getHash(buffer));
		sendinfo.setHash(Hash.getHash(buffer));
		sendinfo.setSend_source_seq(srcAcc.seqence);
		sendinfo.setTranEnv(txeb);
		return txeb;
		
	}
	
	private static void createAccount(Message.Operation.Builder operation,Operation opt){
		Message.OperationCreateAccount.Builder opcb = Message.OperationCreateAccount.newBuilder();
		operation.setType(Message.Operation.Type.CREATE_ACCOUNT);
		opcb.setDestAddress(opt.getDest_address());

		if (opt.getInit_balance()==null) {
			opcb.setInitBalance(init_balance);
		}else{
			opcb.setInitBalance(opt.getInit_balance());
		}
		Message.AccountThreshold.Builder accountThreshold = Message.AccountThreshold.newBuilder();
		Threshold threshold = opt.getThreshold();
		if(!Tools.isNull(threshold)){
			accountThreshold.setMasterWeight(threshold.getMaster_weight());
			accountThreshold.setLowThreshold(threshold.getLow_threshold());
			accountThreshold.setMedThreshold(threshold.getMed_threshold());
			accountThreshold.setHighThreshold(threshold.getHigh_threshold());
			opcb.setThresholds(accountThreshold);
		}
		if (!Tools.isNull(opt.getAccount_metadata())) {
			opcb.setAccountMetadata(ByteString.copyFromUtf8(opt.getAccount_metadata()));
		}
		if (!Tools.isNull(opt.getMetadata())) {
			operation.setMetadata(ByteString.copyFromUtf8(opt.getMetadata()));
		}
		if (!Tools.isNull(opt.getSource_address())) {
			operation.setSourceAddress(opt.getSource_address());
		}
		List<Signer> signers = opt.getSigners();
		if(!Tools.isNull(signers)){
			for(Signer signer : signers){
				Message.Signer.Builder sign = Message.Signer.newBuilder();
				sign.setAddress(signer.getAddress());
				sign.setWeight(signer.getWeight());
				opcb.addSigners(sign);
			}
		}
		operation.setCreateAccount(opcb);
	}
	
	private static void issue(Message.Operation.Builder operation,Operation opt){
		Message.OperationIssueAsset.Builder operationMint = Message.OperationIssueAsset.newBuilder();
		Message.Asset.Builder asset = Message.Asset.newBuilder();
		Message.AssetProperty.Builder assetProperty = Message.AssetProperty.newBuilder();
		if (opt.getAsset_type()!=null) {
			assetProperty.setType(Message.AssetProperty.Type.NATIVE);
		}else {
			assetProperty.setType(Message.AssetProperty.Type.IOU);
		}
		assetProperty.setIssuer(opt.getAsset_issuer());
		assetProperty.setCode(opt.getAsset_code());
		asset.setProperty(assetProperty);
		asset.setAmount(opt.getAsset_amount());
		
		if (opt.getSource_address()!=null) {
			operation.setSourceAddress(opt.getSource_address());
		}
		if (opt.getMetadata() != null) {
			operation.setMetadata(HexUtil.toByteString(opt.getMetadata()));
		}
		if (opt.getDetails()!=null) {
			for(Detail detail : opt.getDetails()){
				Message.Detail.Builder detailBuilder  = Message.Detail.newBuilder();
				detailBuilder.setAmount(detail.getAmount());
				if (detail.getExt()!=null) {
					detailBuilder.setExt(detail.getExt());
				}else{
					detailBuilder.setExt("");
				}
				if (detail.getStart()!=null) {
					detailBuilder.setStart(detail.getStart());
				}else{
					detailBuilder.setStart(0);
				}
				if (detail.getLength()!=null) {
					detailBuilder.setLength(detail.getLength());
				}else{
					detailBuilder.setLength(-1);
				}
				asset.addDetails(detailBuilder);
			}
		}
		operationMint.setAsset(asset);
		operation.setIssueAsset(operationMint);
		operation.setType(Message.Operation.Type.ISSUE_ASSET);
	}
	
	private static void transfer(Message.Operation.Builder operation,Operation opt){
		Message.OperationPayment.Builder operationPayment = Message.OperationPayment.newBuilder();
		operationPayment.setDestAddress(opt.getDest_address());
		Message.Asset.Builder asset = Message.Asset.newBuilder();
		Message.AssetProperty.Builder assetProperty = Message.AssetProperty.newBuilder();
		if (opt.getAPtype()!=null) {
			assetProperty.setType(opt.getAPtype());
		}else{
			assetProperty.setType(cn.bubi.blockchain.adapter.Message.AssetProperty.Type.NATIVE);
		}
		if (opt.getAsset_issuer()!=null) {
			assetProperty.setIssuer(opt.getAsset_issuer());
		} else {
			System.out.println("payment has no asset issuer");
		}
		if (opt.getAsset_code()!=null) {
			assetProperty.setCode(opt.getAsset_code());
		}else{
			assetProperty.setCode("defaultcode");
		}
		asset.setProperty(assetProperty);
		asset.setAmount(opt.getAsset_amount());
		
		if (opt.getDetails()!=null) {
			for(Detail detail : opt.getDetails()){
				Message.Detail.Builder detailBuilder  = Message.Detail.newBuilder();
				detailBuilder.setAmount(detail.getAmount());
				if (detail.getExt()!=null) {
					detailBuilder.setExt(detail.getExt());
				}
				if (detail.getStart()!=null) {
					detailBuilder.setStart(detail.getStart());
				}
				if (detail.getLength()!=null) {
					detailBuilder.setLength(detail.getLength());
				}
				asset.addDetails(detailBuilder);
			}
		}
		
		operationPayment.setAsset(asset);
		operation.setPayment(operationPayment);
		operation.setType(Message.Operation.Type.PAYMENT);
	}
	
	private static void issueAssetUnique(Message.Operation.Builder operation,Operation opt){
		Message.OperationIssueUniqueAsset.Builder opiua = Message.OperationIssueUniqueAsset.newBuilder();
		Message.UniqueAsset.Builder asset = Message.UniqueAsset.newBuilder();
		Message.AssetProperty.Builder assetProperty = Message.AssetProperty.newBuilder();
		if (opt.getAsset_type()!=null) {
			assetProperty.setType(Message.AssetProperty.Type.IOU);
		} else {
			assetProperty.setType(Message.AssetProperty.Type.UNIQUE);
		}
		assetProperty.setIssuer(opt.getAsset_issuer());
		assetProperty.setCode(opt.getAsset_code());
		asset.setDetailed(opt.getAsset_detailed());
		asset.setProperty(assetProperty);
		opiua.setAsset(asset);
		operation.setIssueUniqueAsset(opiua);
		operation.setType(Message.Operation.Type.ISSUE_UNIQUE_ASSET);
	}
	
	private static void transferUnique(Message.Operation.Builder operation,Operation opt){
		Message.AssetProperty.Builder assetproperty = Message.AssetProperty.newBuilder();
		assetproperty.setCode(opt.getAsset_code());
		assetproperty.setIssuer(opt.getAsset_issuer());
		if (opt.getAsset_type()!=null) {
			assetproperty.setType(Message.AssetProperty.Type.IOU);
		} else {
			assetproperty.setType(Message.AssetProperty.Type.UNIQUE);
		}
		
		Message.OperationPaymentUniqueAsset.Builder opua1 = Message.OperationPaymentUniqueAsset.newBuilder();
		opua1.setDestAddress(opt.getDest_address());
		opua1.setAssetPro(assetproperty);
		operation.setType(Message.Operation.Type.PAYMENT_UNIQUE_ASSET);
		operation.setPaymentUniqueAsset(opua1);
	}
	
	private static void inittransfer(Message.Operation.Builder operation,Operation opt){
		Message.OperationInitPayment.Builder operationInitPayment = Message.OperationInitPayment.newBuilder();
		operationInitPayment.setDestAddress(opt.getDest_address());
		Message.Asset.Builder asset = Message.Asset.newBuilder();
		Message.AssetProperty.Builder assetProperty = Message.AssetProperty.newBuilder();
		if (opt.getAsset_type()!=null) {
			assetProperty.setType(Message.AssetProperty.Type.NATIVE);
		} else {
			assetProperty.setType(Message.AssetProperty.Type.IOU);
		}
		assetProperty.setIssuer(opt.getAsset_issuer());
		assetProperty.setCode(opt.getAsset_code());
		if (opt.getDetails()!=null) {
			for(Detail detail : opt.getDetails()){
				Message.Detail.Builder detailBuilder  = Message.Detail.newBuilder();
				detailBuilder.setAmount(detail.getAmount());
				if (detail.getExt()!=null) {
					detailBuilder.setExt(detail.getExt());
				}
				if (detail.getStart()!=null) {
					detailBuilder.setStart(detail.getStart());
				}
				if (detail.getLength()!=null) {
					detailBuilder.setLength(detail.getLength());
				}
				asset.addDetails(detailBuilder);
			}
		}
		asset.setProperty(assetProperty);
		asset.setAmount(opt.getAsset_amount());
		operationInitPayment.setAsset(asset);
		operation.setInitPayment(operationInitPayment);
		operation.setType(Message.Operation.Type.INIT_PAYMENT);
	}
	
	private static void production(Message.Operation.Builder operation,Operation opt){
		
		Message.OperationProduction.Builder oppc = Message.OperationProduction.newBuilder();
		List<Input> input_list = opt.getInputs();
		if (input_list.size()>0) {
			for (Input input : input_list) {
				Message.Input.Builder input_b = Message.Input.newBuilder();
				input_b.setIndex(input.getIndex());
				input_b.setHash(HexUtil.toByte(HexUtil.HexStringToBinaryByteArray(input.getHash())));
				input_b.setMetadata(HexUtil.toByteString(input.getMetadata()));
				oppc.addInputs(input_b);
			}
		}
		List<Output> output_list = opt.getOutputs();
		for (Output output : output_list) {
			Message.Output.Builder output_b = Message.Output.newBuilder();
			output_b.setAddress(output.getAddress());
			output_b.setMetadata(HexUtil.toByteString(output.getMetadata()));
			oppc.addOutputs(output_b);
		}
		operation.setType(Message.Operation.Type.PRODUCTION);
		operation.setProduction(oppc);
		
	}
	
	private static void record(Message.Operation.Builder operation,Operation opt){
		Message.OperationRecord.Builder opr = Message.OperationRecord.newBuilder();
		opr.setId(opt.getRecored_id());
		opr.setExt(HexUtil.toByteString(opt.getRecored_ext()));
		if (opt.getRecored_address()!=null) {
			opr.setAddress(opt.getRecored_address());
		}
		Message.Operation.Builder opb = Message.Operation.newBuilder();
		operation.setType(Message.Operation.Type.RECORD);
		operation.setRecord(opr);
		if (opt.getMetadata()!=null) {
			operation.setMetadata(HexUtil.toByteString(opt.getMetadata()));
		}
	}
	
	private static void setOption(Message.Operation.Builder operation,Operation opt){
		operation.setType(Message.Operation.Type.SET_OPTIONS);
		Message.OperationSetOptions.Builder operationSetOptions = Message.OperationSetOptions.newBuilder();
//		if(!Tools.isNull(opt.getMetadata())){
//			ByteString metadata = ByteString.copyFromUtf8(opt.getAccount_metadata());
//			operationSetOptions.setAccountMetadata(HexUtil.toByteString(HexUtil.str2HexStr(opt.getMetadata())));
//			operationSetOptions.setAccountMetadata(HexUtil.toByteString(opt.getMetadata()));
//		}
		Threshold threshold = opt.getThreshold();
		if(!Tools.isNull(threshold)){
			operationSetOptions.setMasterWeight(threshold.getMaster_weight());
			operationSetOptions.setLowThreshold(threshold.getLow_threshold());
			operationSetOptions.setMedThreshold(threshold.getMed_threshold());
			operationSetOptions.setHighThreshold(threshold.getHigh_threshold());
			if (threshold.getMetadata()!=null) {
				operationSetOptions.setAccountMetadata(HexUtil.toByteString(threshold.getMetadata()));
			}
		}
		
		List<Signer> signers = opt.getSigners();
		if(!Tools.isNull(signers)){
			for(Signer signer : signers){
				Message.Signer.Builder sign = Message.Signer.newBuilder();
				sign.setAddress(signer.getAddress());
				sign.setWeight(signer.getWeight());
				operationSetOptions.addSigners(sign);
			}
		}
		if (opt.getMetadata() != null) {
			operation.setMetadata(HexUtil.toByteString(opt.getMetadata()));
		}
		operation.setSetoptions(operationSetOptions);
	}
}
