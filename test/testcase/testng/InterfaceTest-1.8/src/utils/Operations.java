package utils;

import java.util.LinkedHashMap;
import java.util.Map;

import com.google.protobuf.ByteString;

import cn.bubi.blockchain.adapter.BlockChainAdapter;
import cn.bubi.blockchain.adapter.Message;
import cn.bubi.blockchain.adapter.Message.Operation.Type;
import cn.bubi.common.acc.BubiInfo;
import model.Account;
import model.AssetProperty;

public class Operations {
	
	private static Long initBalance;
	
	public void execute(){
		
	}
	
	public Map<String , Type> issueAsset(Account account, AssetProperty assetproperty, Message.AssetProperty.Type type, long amount, BlockChainAdapter bbc){
		Message.AssetProperty.Builder assetProperty = Message.AssetProperty.newBuilder();
		assetProperty.setType(type);
		assetProperty.setCode(assetproperty.code);
		assetProperty.setIssuer(assetproperty.issuer);
		
		Message.Asset.Builder asset_b = Message.Asset.newBuilder();
		asset_b.setAmount(amount);
		asset_b.setProperty(assetProperty);
		
		Message.OperationIssueAsset.Builder opis = Message.OperationIssueAsset.newBuilder();
		opis.setAsset(asset_b);
		Message.Operation.Builder opb = Message.Operation.newBuilder();
		opb.setType(Message.Operation.Type.ISSUE_ASSET);
		opb.setIssueAsset(opis);

		Message.Transaction.Builder txb = Message.Transaction.newBuilder();
		txb.setSourceAddress(account.getAddress());
		txb.setSequenceNumber(account.seqence);
		txb.setFee(0);
		txb.addOperations(opb);

		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		txeb.setTransaction(txb);
		
		Message.Signature.Builder s = Message.Signature.newBuilder();
		byte[] buffer = txb.build().toByteArray();
		try {
			s.setPublicKey(account.getPub_key());
			s.setSignData(ByteString.copyFrom(SignUtil.sign(buffer, account.getPri_key())));
		} catch (Exception e) {
			e.printStackTrace();
		}
		txeb.addSignatures(s);
		Map<String , Type> map_hash = new LinkedHashMap<>();
		String hash = new Hash().getHash(buffer);
		map_hash.put(hash, Message.Operation.Type.ISSUE_ASSET);
		// use zero mq
		while(!bbc.Send(Message.ChainMessageType.CHAIN_SUBMITTRANSACTION_VALUE, txeb.build().toByteArray())){
			System.out.println("发行资产交易发送失败");
			try {
				Thread.sleep(100);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
//			bbc.Stop();
		}
			System.out.println("发行资产交易发送成功");
			
		return map_hash;
	}
	
	public Map<String, Type> initPayment(Account account, Account dest, Long amount, String assetcode, String assetissuer, Message.AssetProperty.Type type, BlockChainAdapter bbc){
		Message.AssetProperty.Builder assetproperty = Message.AssetProperty.newBuilder();
		assetproperty.setCode(assetcode);
		assetproperty.setIssuer(assetissuer);
		assetproperty.setType(type);
		
		Message.Asset.Builder asset = Message.Asset.newBuilder();
		asset.setAmount(amount);
		asset.setProperty(assetproperty);
		
		Message.OperationInitPayment.Builder oppb = Message.OperationInitPayment.newBuilder();
		// get account from redis server
		oppb.setAsset(asset);
		oppb.setDestAddress(dest.getAddress());
		Message.Operation.Builder opb = Message.Operation.newBuilder();
		opb.setType(Message.Operation.Type.INIT_PAYMENT);
		opb.setInitPayment(oppb);

		Message.Transaction.Builder txb = Message.Transaction.newBuilder();
		txb.setSourceAddress(account.getAddress());
		txb.setSequenceNumber(account.seqence);
		txb.setFee(0);
		txb.addOperations(opb);

		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		txeb.setTransaction(txb);
		
		Message.Signature.Builder s = Message.Signature.newBuilder();
		byte[] buffer = txb.build().toByteArray();
		try {
			s.setPublicKey(account.getPub_key());
			s.setSignData(ByteString.copyFrom(SignUtil.sign(buffer, account.getPri_key())));
		} catch (Exception e) {
			e.printStackTrace();
		}
		txeb.addSignatures(s);
		Map<String , Type> map_hash = new LinkedHashMap<>();
		String hash = new Hash().getHash(buffer);
		map_hash.put(hash, Message.Operation.Type.INIT_PAYMENT);
		// use zero mq
		while(!bbc.Send(Message.ChainMessageType.CHAIN_SUBMITTRANSACTION_VALUE, txeb.build().toByteArray())){
			System.out.println("发送失败");
		}
			System.out.println("发送成功");
		return map_hash;
	}
	
	private void issueAsset(Account account, AssetProperty assetproperty, Message.AssetProperty.Type type, long amount){
		Message.AssetProperty.Builder assetProperty = Message.AssetProperty.newBuilder();
		assetProperty.setType(type);
		assetProperty.setCode(assetproperty.code);
		assetProperty.setIssuer(assetproperty.issuer);
		
		Message.Asset.Builder asset_b = Message.Asset.newBuilder();
		asset_b.setAmount(amount);
		asset_b.setProperty(assetProperty);
		
		Message.OperationIssueAsset.Builder opis = Message.OperationIssueAsset.newBuilder();
		opis.setAsset(asset_b);
		Message.Operation.Builder opb = Message.Operation.newBuilder();
		opb.setType(Message.Operation.Type.ISSUE_ASSET);
		opb.setIssueAsset(opis);

		Message.Transaction.Builder txb = Message.Transaction.newBuilder();
		txb.setSourceAddress(account.getAddress());
		txb.setSequenceNumber(account.seqence);
		txb.setFee(0);
		txb.addOperations(opb);

		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		txeb.setTransaction(txb);
		
		Message.Signature.Builder s = Message.Signature.newBuilder();
		byte[] buffer = txb.build().toByteArray();
		try {
			s.setPublicKey(account.getPub_key());
			s.setSignData(ByteString.copyFrom(SignUtil.sign(buffer, account.getPri_key())));
		} catch (Exception e) {
			e.printStackTrace();
		}
		txeb.addSignatures(s);
		// use zero mq
//		while(!bbc.Send(Message.ChainMessageType.CHAIN_SUBMITTRANSACTION_VALUE, txeb.build().toByteArray())){
//			System.out.println("发送失败");
//		}
//			System.out.println("发送成功");
	}

	public void createAccount(Account srcAcc, BubiInfo destAcc) {

		Message.OperationCreateAccount.Builder opcb = Message.OperationCreateAccount.newBuilder();

		opcb.setDestAddress(destAcc.getBubiAddress()); // read from file to accelerate
		opcb.setInitBalance(0);
		Message.Operation.Builder opb = Message.Operation.newBuilder();
		opb.setType(Message.Operation.Type.CREATE_ACCOUNT);
		opb.setCreateAccount(opcb);

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
		// use zero mq
//		while(!bbc.Send(Message.ChainMessageType.CHAIN_SUBMITTRANSACTION_VALUE, txeb.build().toByteArray())){
//			System.out.println("发送失败");
//		}
//			System.out.println("发送成功");
	}
	
	public Map<String , Type> createAccount(Account srcAcc, BubiInfo destAcc, BlockChainAdapter bbc) {
		
		Message.OperationCreateAccount.Builder opcb = Message.OperationCreateAccount.newBuilder();

		opcb.setDestAddress(destAcc.getBubiAddress()); // read from file to accelerate
		opcb.setInitBalance(0);
		Message.Operation.Builder opb = Message.Operation.newBuilder();
		opb.setType(Message.Operation.Type.CREATE_ACCOUNT);
		opb.setCreateAccount(opcb);

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
		
		Map<String , Type> map_hash = new LinkedHashMap<>();
		String hash = new Hash().getHash(buffer);
		map_hash.put(hash, Message.Operation.Type.CREATE_ACCOUNT);
		
		
		// use zero mq
		while(!bbc.Send(Message.ChainMessageType.CHAIN_SUBMITTRANSACTION_VALUE, txeb.build().toByteArray())){
			System.out.println("创建账号交易发送失败");
			try {
				Thread.sleep(100);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}
		System.out.println(txeb.toString());
		System.out.println("创建账号交易发送成功");
		return map_hash;
	}
	
//	private static void createAccount(Message.Operation.Builder operation,Operation opt){
//		Message.OperationCreateAccount.Builder operationCreateAccount = Message.OperationCreateAccount.newBuilder();
//		operationCreateAccount.setDestAddress(opt.getDest_address());
//		operationCreateAccount.setInitBalance(initBalance);
//		
//		Message.AccountThreshold.Builder accountThreshold = Message.AccountThreshold.newBuilder();
//		Threshold threshold = opt.getThreshold();
//		if(!Tools.isNull(threshold)){
//			accountThreshold.setMasterWeight(threshold.getMaster_weight());
//			accountThreshold.setLowThreshold(threshold.getLow_threshold());
//			accountThreshold.setMedThreshold(threshold.getMed_threshold());
//			accountThreshold.setHighThreshold(threshold.getHigh_threshold());
//			operationCreateAccount.setThresholds(accountThreshold);
//		}
//		if (!Tools.isNull(opt.getMetadata())) {
//			operationCreateAccount.setAccountMetadata(ByteString.copyFromUtf8(opt.getMetadata()));
//		}
////		List<Signer> signers = opt.getSigners();
////		if(!Tools.isNull(signers)){
////			for(Signer signer : signers){
////				Message.Signer.Builder sign = Message.Signer.newBuilder();
////				sign.setAddress(signer.getAddress());
////				sign.setWeight(signer.getWeight());
////				operationCreateAccount.addSigners(sign);
////			}
////		}
//		operation.setCreateAccount(operationCreateAccount);
//	}
	
	private static void operations(Account srcAcc, BubiInfo destAcc,Iterable<? extends Message.Operation> operations, Message.Operation op, Long initbalance){
		Type type = op.getType();
		Message.Operation.Builder opb = Message.Operation.newBuilder();
		for (Message.Operation oper : operations) {
//			Message.OperationCreateAccount.Builder opcb = Message.OperationCreateAccount.newBuilder();
//			opcb.setDestAddress(destAcc.getBubiAddress()); // read from file to accelerate
//			opcb.setInitBalance(initbalance);
//			 List<E> list = new ArrayList<E>();
			opb.setType(Message.Operation.Type.CREATE_ACCOUNT);
//			opb.setCreateAccount(oper);
		}
		switch (type) {
		case CREATE_ACCOUNT:
			
			break;
		case ISSUE_ASSET:
//			opb.setType(Message.Operation.Type.ISSUE_ASSET);
//			opb.setIssueAsset(opb);
			break;
		case PAYMENT:
			
			break;

		default:
			break;
		}
		
		Message.Transaction.Builder txb = Message.Transaction.newBuilder();
		txb.setSourceAddress(srcAcc.getAddress());
		txb.setSequenceNumber(srcAcc.seqence);
		txb.setFee(0);
		txb.addOperations(op);
//		txb.addAllOperations(values);
		
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
		
	}
}
