package utils;

import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import com.google.protobuf.ByteString;
import com.google.protobuf.InvalidProtocolBufferException;
import base.Log;
import base.TestBase;
import blob.TransactionBlob;
import cn.bubi.blockchain.adapter.BlockChainAdapterProc;
import cn.bubi.blockchain.adapter.Message;
import cn.bubi.blockchain.adapter.Message.AssetProperty;
import cn.bubi.blockchain.adapter.Message.ChainTxStatus;
import cn.bubi.blockchain.adapter.Message.Signer;
import cn.bubi.common.acc.BubiInfo;
import cn.bubi.common.util.Tools;
import model.Account;
import model.TxInfo;
import thread.CacheCondition;
import thread.ThreadCondition;

public class ZMQhandle extends TestBase{

	public static Map<String, TxInfo> map = new LinkedHashMap<>();
	private Map<String, TxInfo> map_send = new LinkedHashMap<>();
	private Object object = new Object();
	public static TxInfo env = new TxInfo();
	
	public  ZMQhandle(){
		bbc.AddChainMethod(Message.ChainMessageType.CHAIN_TX_STATUS_VALUE,new BlockChainAdapterProc() {
			
			@Override
			public void ChainMethod(byte[] msg, int length) {
				env = callback(msg, length, object);
			}
		});
		bbc.AddChainMethod(Message.ChainMessageType.CHAIN_STATUS_VALUE,new BlockChainAdapterProc(){
			@Override
			public void ChainMethod (byte[] msg, int length) {
				OnChainHello(msg, length);
			}
		});
		//bbc.Start();
//		System.out.println("启动");
		synchronized (object) {
			try {
				object.wait(1000);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}
	}
	
	
	private void OnChainHello(byte[] msg, int length) {
		try {
			cn.bubi.blockchain.adapter.Message.ChainStatus chain_status = cn.bubi.blockchain.adapter.Message.ChainStatus.parseFrom(msg);
			System.out.println("=================receive hello info============");
			System.out.println(chain_status);
			String add = GetAddress.getDesAddress(pull_ip,19333);
			String self_add = chain_status.getSelfAddr();
			check.assertEquals(add, self_add, "发送消息地址不一致");
			System.out.println("done========");
			bbc.Stop();
		} catch (InvalidProtocolBufferException e) {
			e.printStackTrace();
		}
	}
	
	public static Long getNewAccountSeq(String hash){
		return map.get(hash).getRecv_NewAcc_seq()+1;
	}
	
	public static Long getSourceAccountSeq(String hash){
		return map.get(hash).getRecv_Source_seq()+1;
	}

	public void execute(TransactionBlob tranBlob, List<Account> signer, Account srcadd, Message.Operation opb) throws Exception{
		try{
			Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
			Message.Transaction.Builder txb = Message.Transaction.newBuilder();
			txb.setSourceAddress(srcadd.getAddress());
			txb.setSequenceNumber(srcadd.seqence);
			txb.setFee(0);
			txb.addOperations(opb);
//			txb.addAllOperations(values)
			txeb.setTransaction(txb);
			Message.Signature.Builder signBuilder = Message.Signature.newBuilder();
			byte[] buffer = txeb.build().toByteArray();
			if (!Utils.isNullByList(signer)) {
					for (Account a : signer) {
						signBuilder.setPublicKey(a.getPub_key());
						signBuilder.setSignData(ByteString.copyFrom(SignUtil.sign(buffer, a.getPri_key())));
					}
				}
			bbc.Send(Message.ChainMessageType.CHAIN_SUBMITTRANSACTION_VALUE, txeb.build().toByteArray());
			} catch (Exception e) {
				e.printStackTrace();
		}
	} 
	
	public String create(Account srcAcc, Integer fee, BubiInfo destAcc ){
		Message.OperationCreateAccount.Builder opcb = Message.OperationCreateAccount.newBuilder();
		opcb.setDestAddress(destAcc.getBubiAddress()); 
		opcb.setInitBalance(init_balance);
		
		Message.Operation.Builder opb = Message.Operation.newBuilder();
		opb.setType(Message.Operation.Type.CREATE_ACCOUNT);
		opb.setCreateAccount(opcb);

		Message.Transaction.Builder txb = Message.Transaction.newBuilder();
		txb.setSourceAddress(srcAcc.getAddress());
		txb.setSequenceNumber(srcAcc.seqence);
		txb.setFee(fee);
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
//		Map<String , Object> map_hash = new LinkedHashMap<>();
		String hash = new Hash().getHash(buffer);
		bbc.Send(Message.ChainMessageType.CHAIN_SUBMITTRANSACTION_VALUE, txeb.build().toByteArray());
		return hash;
	}
	
	public TxInfo create(Account srcAcc, Integer fee, Account destAcc ){
		Message.OperationCreateAccount.Builder opcb = Message.OperationCreateAccount.newBuilder();
		opcb.setDestAddress(destAcc.getAddress()); 
		if (srcAcc.getInit_balance()!=null) {
			opcb.setInitBalance(srcAcc.getInit_balance());
		}else{
			opcb.setInitBalance(init_balance);
		}
		if (destAcc.getMetada()!=null) {
			opcb.setAccountMetadata(HexUtil.toByteString(destAcc.getMetada()));
		}
		if (srcAcc.getThreshold() !=null) {
			Message.AccountThreshold.Builder threhold = Message.AccountThreshold.newBuilder();
			threhold.setHighThreshold(srcAcc.getThreshold().getHigh_threshold());
			threhold.setLowThreshold(srcAcc.getThreshold().getLow_threshold());
			threhold.setMedThreshold(srcAcc.getThreshold().getMed_threshold());
			threhold.setMasterWeight(srcAcc.getThreshold().getMaster_weight());
			opcb.setThresholds(threhold);
		}
		System.out.println(srcAcc.getSigners());
		if (srcAcc.getSigners()!=null) {
			Signer.Builder signer = Signer.newBuilder();
			int list_size = srcAcc.getSigners().size();
			for (int i = 0;i<list_size;i++) {
				signer.setAddress(srcAcc.getSigners().get(i).getAddress());
				signer.setWeight(srcAcc.getSigners().get(i).getWeight());
				opcb.setSigners(i, signer);
			}
		}
		
		Message.Operation.Builder opb = Message.Operation.newBuilder();
		opb.setType(Message.Operation.Type.CREATE_ACCOUNT);
		opb.setCreateAccount(opcb);
		Message.Transaction.Builder txb = Message.Transaction.newBuilder();
		txb.setSourceAddress(srcAcc.getAddress());
		txb.setSequenceNumber(srcAcc.seqence);
		txb.setFee(fee);
		txb.addOperations(opb);
		System.out.println(txb);
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		txeb.setTransaction(txb);
		
		Message.Signature.Builder s = Message.Signature.newBuilder();
		byte[] buffer = txb.build().toByteArray();
		TxInfo send = new TxInfo();
		send.setHash(Hash.getHash(buffer));
		send.setSend_Optype(Message.Operation.Type.CREATE_ACCOUNT);
		send.setSend_source_seq(srcAcc.seqence);
		try {
			s.setPublicKey(srcAcc.getPub_key());
			s.setSignData(ByteString.copyFrom(SignUtil.sign(buffer, srcAcc.getPri_key())));
		} catch (Exception e) {
			e.printStackTrace();
		}
		txeb.addSignatures(s);
		bbc.Send(Message.ChainMessageType.CHAIN_SUBMITTRANSACTION_VALUE, txeb.build().toByteArray());
		APIUtil.wait(5);
		return send;
	}
	
//	public TxInfo create(Account srcAcc, Integer fee, Account destAcc ){
//		Message.OperationCreateAccount.Builder opcb = Message.OperationCreateAccount.newBuilder();
//		opcb.setDestAddress(destAcc.getAddress()); 
//		opcb.setInitBalance(init_balance);
//		
//		Message.Operation.Builder opb = Message.Operation.newBuilder();
//		opb.setType(Message.Operation.Type.CREATE_ACCOUNT);
//		opb.setCreateAccount(opcb);
//
//		Message.Transaction.Builder txb = Message.Transaction.newBuilder();
//		txb.setSourceAddress(srcAcc.getAddress());
//		txb.setSequenceNumber(srcAcc.seqence);
//		txb.setFee(fee);
//		txb.addOperations(opb);
//		System.out.println(txb);
//		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
//		txeb.setTransaction(txb);
//		
//		Message.Signature.Builder s = Message.Signature.newBuilder();
//		byte[] buffer = txb.build().toByteArray();
//		TxInfo send = new TxInfo();
//		send.setHash(Hash.getHash(buffer));
//		send.setSend_Optype(Message.Operation.Type.CREATE_ACCOUNT);
//		send.setSend_source_seq(srcAcc.seqence);
//		try {
//			s.setPublicKey(srcAcc.getPub_key());
//			s.setSignData(ByteString.copyFrom(SignUtil.sign(buffer, srcAcc.getPri_key())));
//		} catch (Exception e) {
//			e.printStackTrace();
//		}
//		txeb.addSignatures(s);
//		bbc.Send(Message.ChainMessageType.CHAIN_SUBMITTRANSACTION_VALUE, txeb.build().toByteArray());
//		APIUtils.wait(5);
//		return send;
//	}
	
	public Map<String, Object> createtrace(Account srcAcc, Integer fee, Account destAcc ){
		Message.OperationCreateAccount.Builder opcb = Message.OperationCreateAccount.newBuilder();
		opcb.setDestAddress(destAcc.getAddress()); 
		opcb.setInitBalance(init_balance);
		
		Message.Operation.Builder opb = Message.Operation.newBuilder();
		opb.setType(Message.Operation.Type.CREATE_ACCOUNT);
		opb.setCreateAccount(opcb);

		Message.Transaction.Builder txb = Message.Transaction.newBuilder();
		txb.setSourceAddress(srcAcc.getAddress());
		txb.setSequenceNumber(srcAcc.seqence);
		txb.setFee(fee);
		txb.addOperations(opb);
		System.out.println(txb);
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
		Map<String, Object> map = new LinkedHashMap<>();
		map.put("hash", Hash.getHash(buffer));
		map.put("source_add", srcAcc.getAddress());
		map.put("dest_add", destAcc.getAddress());
		map.put("source_seq", srcAcc.seqence);
		map.put("operation", Message.Operation.Type.CREATE_ACCOUNT.toString());
		bbc.Send(Message.ChainMessageType.CHAIN_SUBMITTRANSACTION_VALUE, txeb.build().toByteArray());
//		APIUtils.wait(3);
		return map;
	}
	
	public TxInfo production(Account account, Account dest, String metadata){
		TxInfo send = new TxInfo();
		Message.Output.Builder output = Message.Output.newBuilder();
		output.setAddress(dest.getAddress());
		output.setMetadata(ByteString.copyFrom(metadata.getBytes()));
		Message.OperationProduction.Builder oppc = Message.OperationProduction.newBuilder();
//		oppc.setInputs(0, input);
		oppc.addOutputs(output);
//		oppc.setOutputs(0, output);
		Message.Operation.Builder opb = Message.Operation.newBuilder();
		opb.setType(Message.Operation.Type.PRODUCTION);
		opb.setProduction(oppc);

		Message.Transaction.Builder txb = Message.Transaction.newBuilder();
		txb.setSourceAddress(account.getAddress());
		txb.setSequenceNumber(account.seqence);
		txb.setFee(fee);
		txb.addOperations(opb);
		System.out.println(txb);
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		txeb.setTransaction(txb);
		
		Message.Signature.Builder s = Message.Signature.newBuilder();
		byte[] buffer = txb.build().toByteArray();
		send.setHash(Hash.getHash(buffer));
		send.setSend_Optype(Message.Operation.Type.PRODUCTION);
		send.setSend_source_seq(account.seqence);
		try {
			s.setPublicKey(account.getPub_key());
			s.setSignData(ByteString.copyFrom(SignUtil.sign(buffer, account.getPri_key())));
		} catch (Exception e) {
			e.printStackTrace();
		}
		txeb.addSignatures(s);
		bbc.Send(Message.ChainMessageType.CHAIN_SUBMITTRANSACTION_VALUE, txeb.build().toByteArray());
		APIUtil.wait(5);
		return send;
	}

//	public Map<String, SendEnv> issue(Account srcAcc, String asset_code, Long asset_amount){
//		Operation opt = new Operation();
//		opt.setAsset_code(asset_code);
//		opt.setAsset_amount(asset_amount);
//		opt.setAsset_issuer(srcAcc.getAddress());
//		map_send = issueOperation(srcAcc, fee, opt);
//		return map_send;
//	}
	
	public TxInfo issue(Account srcAcc, String asset_code, Long asset_amount){
		
		Message.AssetProperty.Builder assetProperty = Message.AssetProperty.newBuilder();
		assetProperty.setCode(asset_code);
		assetProperty.setIssuer(srcAcc.getAddress());
		assetProperty.setType(AssetProperty.Type.IOU);
		Message.Asset.Builder asset_b = Message.Asset.newBuilder();
		asset_b.setAmount(asset_amount);
		asset_b.setProperty(assetProperty);
		
		Message.OperationIssueAsset.Builder opis = Message.OperationIssueAsset.newBuilder();
		opis.setAsset(asset_b);
		Message.Operation.Builder opb = Message.Operation.newBuilder();
		opb.setType(Message.Operation.Type.ISSUE_ASSET);
		opb.setIssueAsset(opis);

		Message.Transaction.Builder txb = Message.Transaction.newBuilder();
		txb.setSourceAddress(srcAcc.getAddress());
		txb.setSequenceNumber(srcAcc.seqence);
		txb.setFee(fee);
		txb.addOperations(opb);
		System.out.println("发送issue：" + txb);

		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		txeb.setTransaction(txb);
		
		Message.Signature.Builder s = Message.Signature.newBuilder();
		byte[] buffer = txb.build().toByteArray();
		String hash = new Hash().getHash(buffer);
		Map<String, Object> map = new LinkedHashMap<>();
		TxInfo send = new TxInfo();
		send.setIssueHash(hash);
		send.setSourceAdd(srcAcc.getAddress());
		send.setSend_Optype(Message.Operation.Type.ISSUE_ASSET);
		send.setSend_source_seq(srcAcc.seqence);
		map.put("hash", hash);
		System.out.println(send.getSend_Optype() + " hash: " + hash);
		try {
			s.setPublicKey(srcAcc.getPub_key());
			s.setSignData(ByteString.copyFrom(SignUtil.sign(buffer, srcAcc.getPri_key())));
		} catch (Exception e) {
			e.printStackTrace();
		}
		txeb.addSignatures(s);
		bbc.Send(Message.ChainMessageType.CHAIN_SUBMITTRANSACTION_VALUE, txeb.build().toByteArray());
		APIUtil.wait(5);
		return send;
	}
	
	public TxInfo paymentUnique(Account account, Account dest, String asset_code){
		TxInfo send = new TxInfo();
		Message.AssetProperty.Builder assetproperty = Message.AssetProperty.newBuilder();
		assetproperty.setCode(asset_code);
		assetproperty.setIssuer(account.getAddress());
		assetproperty.setType(Message.AssetProperty.Type.UNIQUE);
		
		Message.OperationPaymentUniqueAsset.Builder opua1 = Message.OperationPaymentUniqueAsset.newBuilder();
		opua1.setDestAddress(dest.getAddress());
		opua1.setAssetPro(assetproperty);
		Message.Operation.Builder opb = Message.Operation.newBuilder();
		opb.setType(Message.Operation.Type.PAYMENT_UNIQUE_ASSET);
		opb.setPaymentUniqueAsset(opua1);

		Message.Transaction.Builder txb = Message.Transaction.newBuilder();
		txb.setSourceAddress(account.getAddress());
		txb.setSequenceNumber(account.seqence);
		txb.setFee(fee);
		txb.addOperations(opb);
		System.out.println(txb);
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		txeb.setTransaction(txb);
		
		Message.Signature.Builder s = Message.Signature.newBuilder();
		byte[] buffer = txb.build().toByteArray();
		send.setSend_Optype(Message.Operation.Type.PAYMENT_UNIQUE_ASSET);
		send.setSend_source_seq(account.seqence);
		send.setHash(Hash.getHash(buffer));
		try {
			s.setPublicKey(account.getPub_key());
			s.setSignData(ByteString.copyFrom(SignUtil.sign(buffer, account.getPri_key())));
		} catch (Exception e) {
			e.printStackTrace();
		}
		txeb.addSignatures(s);
		bbc.Send(Message.ChainMessageType.CHAIN_SUBMITTRANSACTION_VALUE, txeb.build().toByteArray());
		APIUtil.wait(5);
		return send;
	}
	
	public TxInfo issueUnique(Account srcAcc, String asset_code, String asset_detailed){
		TxInfo txinfo = new TxInfo();
		Message.AssetProperty.Builder assetproperty = Message.AssetProperty.newBuilder();
		assetproperty.setCode(asset_code);
		assetproperty.setIssuer(srcAcc.getAddress());
		assetproperty.setType(Message.AssetProperty.Type.UNIQUE);
		
		Message.UniqueAsset.Builder ua = Message.UniqueAsset.newBuilder();
		ua.setDetailed(ByteString.copyFrom(asset_detailed.getBytes()));
		ua.setProperty(assetproperty);
		
		Message.OperationIssueUniqueAsset.Builder opua = Message.OperationIssueUniqueAsset.newBuilder();
		opua.setAsset(ua);
		Message.Operation.Builder opb = Message.Operation.newBuilder();
		opb.setType(Message.Operation.Type.ISSUE_UNIQUE_ASSET);
		opb.setIssueUniqueAsset(opua);

		Message.Transaction.Builder txb = Message.Transaction.newBuilder();
		txb.setSourceAddress(srcAcc.getAddress());
		txb.setSequenceNumber(srcAcc.seqence);
		txb.setFee(0);
		txb.addOperations(opb);
		
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		txeb.setTransaction(txb);
		
		Message.Signature.Builder s = Message.Signature.newBuilder();
		byte[] buffer = txb.build().toByteArray();
		String hash = Hash.getHash(buffer);
		txinfo.setSend_Optype(Message.Operation.Type.ISSUE_UNIQUE_ASSET);
		txinfo.setSend_source_seq(srcAcc.seqence);
		txinfo.setHash(hash);
		try {
			s.setPublicKey(srcAcc.getPub_key());
			s.setSignData(ByteString.copyFrom(SignUtil.sign(buffer, srcAcc.getPri_key())));
		} catch (Exception e) {
			e.printStackTrace();
		}
		txeb.addSignatures(s);
		
		bbc.Send(Message.ChainMessageType.CHAIN_SUBMITTRANSACTION_VALUE, txeb.build().toByteArray());
		APIUtil.wait(5);
		return txinfo;
	}
	
	public static void isCreate(Map map){
		if (ZMQhandle.map.containsKey(map.get("hash"))) {
			System.out.println("成功创建账户： " + map.get("dest_add"));
		}else {
			System.out.println("账户创建失败： " + map.get("dest_add") + " hash: " + map.get("hash"));
		}
	}
	
	public Map<String, TxInfo> transfer(Account srcAcc, Account dest, Long amount, String assetcode, Message.AssetProperty.Type type){
		Message.AssetProperty.Builder assetproperty = Message.AssetProperty.newBuilder();
		assetproperty.setCode(assetcode);
		assetproperty.setIssuer(srcAcc.getAddress());
		assetproperty.setType(type);
		map_send = transferOperation(srcAcc, dest, assetproperty, amount);
		return map_send;
	}
	
	public Map<String, TxInfo> transferOperation(Account srcAcc, Account dest, Message.AssetProperty.Builder assetproperty, Long amount){
		
		
		Message.Asset.Builder asset = Message.Asset.newBuilder();
		asset.setAmount(amount);
		asset.setProperty(assetproperty);
		
		Message.OperationPayment.Builder oppb = Message.OperationPayment.newBuilder();
		oppb.setAsset(asset);
		oppb.setDestAddress(dest.getAddress());
		Message.Operation.Builder opb = Message.Operation.newBuilder();
		opb.setType(Message.Operation.Type.PAYMENT);
		opb.setPayment(oppb);

		Message.Transaction.Builder txb = Message.Transaction.newBuilder();
		txb.setSourceAddress(srcAcc.getAddress());
		txb.setSequenceNumber(srcAcc.seqence);
		txb.setFee(fee);
		txb.addOperations(opb);
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		txeb.setTransaction(txb);
		System.out.println("transfer: " + txb);
		Message.Signature.Builder s = Message.Signature.newBuilder();
		byte[] buffer = txb.build().toByteArray();
		String hash = Hash.getHash(buffer);
//		Map<String, Object> map = new LinkedHashMap<>();
		TxInfo send = new TxInfo();
		send.setHash(hash);
		send.setSourceAdd(srcAcc.getAddress());
		send.setSend_Optype(Message.Operation.Type.PAYMENT);
		map_send.put("hash", send);
		System.out.println(send.getSend_Optype() + " hash: " + hash);
		try {
			s.setPublicKey(srcAcc.getPub_key());
			s.setSignData(ByteString.copyFrom(SignUtil.sign(buffer, srcAcc.getPri_key())));
		} catch (Exception e) {
			e.printStackTrace();
		}
		txeb.addSignatures(s);
		bbc.Send(Message.ChainMessageType.CHAIN_SUBMITTRANSACTION_VALUE, txeb.build().toByteArray());
		APIUtil.wait(5);
		return map_send;
	}
	
	public Long getNewSequence(Map map){
		Account account = new Account();
		String hash = (String) map.get("hash");
		return ZMQhandle.getSourceAccountSeq(hash);
	}
	
	public TxInfo callback( byte[] msg, int length, Object obj){
		String hash = "";
		TxInfo env = new TxInfo();
		try {
			Message.ChainTxStatus chainTx = Message.ChainTxStatus.parseFrom(msg);
			String errorCode  = chainTx.getErrorCode().toString();
			System.out.println(chainTx.toString());
			env.setHash(hash);
			env.setSourceAdd(chainTx.getSourceAddress());
			env.setRecv_Source_seq(chainTx.getSourceAccountSeq());
			env.setRecv_NewAcc_seq(chainTx.getNewAccountSeq());
			switch (chainTx.getStatus().getNumber()) {
			case ChainTxStatus.TxStatus.COMPLETE_VALUE:
				hash = chainTx.getTxHash();
				
				map.put(hash, env);
				System.out.println("交易成功 ,hash:"+hash + " source_seq: " + env.getRecv_Source_seq() + " new_acc_seq: " + env.getRecv_NewAcc_seq());
				notifyThread(hash);
				break;
			case ChainTxStatus.TxStatus.FAILURE_VALUE:
				Log.error("交易失败："+errorCode+",status="+chainTx.getStatus()+" ,交易hash:"+hash);
				long sequence = 0l;
				if(chainTx.hasSourceAccountSeq()){
					sequence = chainTx.getSourceAccountSeq();
				}
				String initiatorAddress = chainTx.getSourceAddress();
//				decrSequence(initiatorAddress,sequence);
				notifyThread(hash);
				break;
			case ChainTxStatus.TxStatus.PENDING_VALUE:
				break;
			case ChainTxStatus.TxStatus.CONFIRMED_VALUE:
				break;
			default:
				obj.notifyAll();
				break;
			}
		} catch (Exception e) {
			Log.error("接受交易结果出错",e);
			notifyThread(hash);
		}
		return env;
	}
	
	public void peerMessage(byte[] msg, int length){
		try {
			Message.ChainTxStatus chainTx = Message.ChainTxStatus.parseFrom(msg);
			System.out.println(chainTx.toString());
		} catch (InvalidProtocolBufferException e) {
			e.printStackTrace();
		}
	}
	
	public TxInfo initPayment(Account src, Account dest, String assetcode, Long amount){
		Message.AssetProperty.Builder assetproperty = Message.AssetProperty.newBuilder();
		assetproperty.setCode(assetcode);
		assetproperty.setIssuer(src.getAddress());
		assetproperty.setType(Message.AssetProperty.Type.IOU);
		
		Message.Asset.Builder asset = Message.Asset.newBuilder();
		asset.setAmount(amount);
		asset.setProperty(assetproperty);
		
		Message.OperationInitPayment.Builder oppb = Message.OperationInitPayment.newBuilder();
		oppb.setAsset(asset);
		oppb.setDestAddress(dest.getAddress());
		Message.Operation.Builder opb = Message.Operation.newBuilder();
		opb.setType(Message.Operation.Type.INIT_PAYMENT);
		opb.setInitPayment(oppb);

		Message.Transaction.Builder txb = Message.Transaction.newBuilder();
		txb.setSourceAddress(src.getAddress());
		txb.setSequenceNumber(src.seqence);
		txb.setFee(fee);
		txb.addOperations(opb);
		System.out.println(txb.toString());
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		txeb.setTransaction(txb);
		
		Message.Signature.Builder s = Message.Signature.newBuilder();
		byte[] buffer = txb.build().toByteArray();
		String hash = Hash.getHash(buffer);
		TxInfo send = new TxInfo();
		send.setHash(hash);
		send.setSourceAdd(src.getAddress());
		send.setSend_Optype(Message.Operation.Type.INIT_PAYMENT);
		try {
			s.setPublicKey(src.getPub_key());
			s.setSignData(ByteString.copyFrom(SignUtil.sign(buffer, src.getPri_key())));
		} catch (Exception e) {
			e.printStackTrace();
		}
		txeb.addSignatures(s);
		bbc.Send(Message.ChainMessageType.CHAIN_SUBMITTRANSACTION_VALUE, txeb.build().toByteArray());
		APIUtil.wait(5);
		return send;
	}

	public TxInfo record(Account account, String record_id, String record_ext){
		Message.OperationRecord.Builder opr = Message.OperationRecord.newBuilder();
		opr.setId(record_id);
//		opr.setExt(record_ext.);
		opr.setExt(ByteString.copyFrom(record_ext.getBytes()));
		Message.Operation.Builder opb = Message.Operation.newBuilder();
		opb.setType(Message.Operation.Type.RECORD);
		opb.setRecord(opr);

		Message.Transaction.Builder txb = Message.Transaction.newBuilder();
		txb.setSourceAddress(account.getAddress());
		txb.setSequenceNumber(account.seqence);
		txb.setFee(fee);
		txb.addOperations(opb);
		System.out.println(txb);
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		txeb.setTransaction(txb);
		
		Message.Signature.Builder s = Message.Signature.newBuilder();
		byte[] buffer = txb.build().toByteArray();
		TxInfo send = new TxInfo();
		send.setHash(Hash.getHash(buffer));
		send.setSourceAdd(account.getAddress());
		send.setSend_Optype(Message.Operation.Type.RECORD);
		try {
			s.setPublicKey(account.getPub_key());
			s.setSignData(ByteString.copyFrom(SignUtil.sign(buffer, account.getPri_key())));
		} catch (Exception e) {
			e.printStackTrace();
		}
		txeb.addSignatures(s);
		bbc.Send(Message.ChainMessageType.CHAIN_SUBMITTRANSACTION_VALUE, txeb.build().toByteArray());
		APIUtil.wait(3);
		return send;
	}
	
	public TxInfo setoption(Account src, int highthre, int lowthre, int medthre, int masterw){
		Message.OperationSetOptions.Builder opso = Message.OperationSetOptions.newBuilder();
		opso.setHighThreshold(highthre);
		opso.setLowThreshold(lowthre);
		opso.setMedThreshold(medthre);
		opso.setMasterWeight(masterw);
		Message.Operation.Builder opb = Message.Operation.newBuilder();
		opb.setType(Message.Operation.Type.SET_OPTIONS);
		opb.setSetoptions(opso);

		Message.Transaction.Builder txb = Message.Transaction.newBuilder();
		txb.setSourceAddress(src.getAddress());
		txb.setSequenceNumber(src.seqence);
		txb.setFee(fee);
		txb.addOperations(opb);
		System.out.println(txb);
		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		txeb.setTransaction(txb);
		
		Message.Signature.Builder s = Message.Signature.newBuilder();
		byte[] buffer = txb.build().toByteArray();
		TxInfo send = new TxInfo();
		send.setHash(Hash.getHash(buffer));
		send.setSourceAdd(src.getAddress());
		send.setSend_Optype(Message.Operation.Type.SET_OPTIONS);
		try {
			s.setPublicKey(src.getPub_key());
			s.setSignData(ByteString.copyFrom(SignUtil.sign(buffer, src.getPri_key())));
		} catch (Exception e) {
			e.printStackTrace();
		}
		txeb.addSignatures(s);
		bbc.Send(Message.ChainMessageType.CHAIN_SUBMITTRANSACTION_VALUE, txeb.build().toByteArray());
		APIUtil.wait(3);
		return send;
	}
	/**
	 * 唤醒线程
	 * @param hash
	 */
	private void notifyThread(String hash){
		ThreadCondition threadCondition = CacheCondition.getThreadCondition(hash);
		if(!Tools.isNull(threadCondition)){
			threadCondition.notifyThread();
		}
	}
}
