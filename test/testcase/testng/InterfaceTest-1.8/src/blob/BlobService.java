package blob;

import java.nio.ByteBuffer;
import java.util.List;
import org.apache.commons.codec.binary.Hex;

import com.google.protobuf.ByteString;

import base.TestBase;
import cn.bubi.blockchain.adapter.Message;
import cn.bubi.common.acc.BubiInfo;
import cn.bubi.common.util.Tools;
import model.Account;
import model.Operation;
import model.Signer;
import model.Threshold;

public class BlobService extends TestBase{
	
//	private static Long init_balance ;
//	private static int fee;
//	private static RedisUtil redis = RedisUtil.getRu();
//	private static BlockChainProcess blockChainProcess = new BlockChainProcess();
//	private static final Logger logger = LoggerFactory.getLogger(BlobService.class);
	
	public static TransactionBlob getTransactionBlob(List<Operation> operations,String metadata,Account srcadd,List<BubiInfo> createingAccs) throws Exception {
		// 构建blob
		Message.Transaction.Builder builder = Message.Transaction.newBuilder();
		if (!Tools.isNull(metadata)) {
			builder.setMetadata(ByteString.copyFromUtf8(Hex.encodeHexString(metadata.getBytes())));
		}
		builder.setSourceAddress(srcadd.getAddress());
		//bubi coin
		int sumFee = operations.size()*fee;
		builder.setFee(sumFee);
		builder.setSequenceNumber(srcadd.seqence);
		
		//set account need to be created
		if(!Tools.isNullByList(createingAccs)){
//			ConcurrentMap<String, Object> createingAccMap = new ConcurrentHashMap<String, Object>();
			Operation o = null;
			for (BubiInfo acc : createingAccs) {
//				if(!createingAccMap.containsKey(acc.getBubiAddress())){
					o = new Operation();
					o.setInit_balance(init_balance);
					o.setDest_address(acc.getBubiAddress());
					operations.add(0,o);
//					createingAccMap.put(acc.getBubiAddress(), acc);
//				}
				
			}
		}
		
		for (Operation opt : operations) {
			try{
				buildTransaction(builder,opt);
			}catch(Exception e){
				e.printStackTrace();
			}
		}
		ByteBuffer bytes = ByteBuffer.wrap(builder.build().toByteArray());

		TransactionBlob transactionBlob = new TransactionBlob(bytes);
		return transactionBlob;
	}
	
	public static TransactionBlob getTransactionBlob(Operation operations,String metadata,Account srcadd,BubiInfo createingAccs) throws Exception {
		// 构建blob
		Message.Transaction.Builder builder = Message.Transaction.newBuilder();
		if (!Tools.isNull(metadata)) {
			builder.setMetadata(ByteString.copyFromUtf8(Hex.encodeHexString(metadata.getBytes())));
		}
		builder.setSourceAddress(srcadd.getAddress());
		//代币
		builder.setFee(fee);
		builder.setSequenceNumber(srcadd.seqence);
		
		//set account need to be created
//			ConcurrentMap<String, Object> createingAccMap = new ConcurrentHashMap<String, Object>();
			Operation o = new Operation();
					o.setInit_balance(init_balance);
					o.setDest_address(createingAccs.getBubiAddress());
			try{
				buildTransaction(builder,o);
			}catch(Exception e){
				e.printStackTrace();
			}
		ByteBuffer bytes = ByteBuffer.wrap(builder.build().toByteArray());

		TransactionBlob transactionBlob = new TransactionBlob(bytes);
		return transactionBlob;
	}
	
	private static void buildTransaction(Message.Transaction.Builder builder,Operation opt) throws Exception{
		Message.Operation.Builder operation = builder.addOperationsBuilder();
		Integer optType = opt.getType();
		operation.setType(Message.Operation.Type.valueOf(optType));
		if (!Tools.isNull(opt.getMetadata())) {
			operation.setMetadata(ByteString.copyFrom(opt.getMetadata().getBytes("UTF-8")));
		}
		String opSourceAddress = opt.getSource_address();
		if (!Tools.isNull(opSourceAddress)) {
			operation.setSourceAddress(opSourceAddress);
		}
		//创建账户
		if(optType == 0){
			createAccount(operation,opt);
		}else if(optType == 1){//转账
			transfer(operation,opt);
		}else if(optType == 2){//发行资产
			issue(operation,opt);
		}else if(optType == 3){ //提交或取消一个order
			
		}else if(optType == 4){//set account property
			setAccount(operation,opt);
		}else if(optType == 5){//init transfer
			grantasset(operation,opt);
		}
		
	}
	
	/**
	 * 创建账户
	 * @param operation
	 * @param opt
	 */
	private static void createAccount(Message.Operation.Builder operation,Operation opt){
		Message.OperationCreateAccount.Builder operationCreateAccount = Message.OperationCreateAccount.newBuilder();
		operationCreateAccount.setDestAddress(opt.getDest_address());
		operationCreateAccount.setInitBalance(init_balance);
		
		Message.AccountThreshold.Builder accountThreshold = Message.AccountThreshold.newBuilder();
		Threshold threshold = opt.getThreshold();
		if(!Tools.isNull(threshold)){
			accountThreshold.setMasterWeight(threshold.getMaster_weight());
			accountThreshold.setLowThreshold(threshold.getLow_threshold());
			accountThreshold.setMedThreshold(threshold.getMed_threshold());
			accountThreshold.setHighThreshold(threshold.getHigh_threshold());
			operationCreateAccount.setThresholds(accountThreshold);
		}
		if (!Tools.isNull(opt.getMetadata())) {
			operationCreateAccount.setAccountMetadata(ByteString.copyFromUtf8(opt.getMetadata()));
		}
		List<Signer> signers = opt.getSigners();
		if(!Tools.isNull(signers)){
			for(Signer signer : signers){
				Message.Signer.Builder sign = Message.Signer.newBuilder();
				sign.setAddress(signer.getAddress());
				sign.setWeight(signer.getWeight());
				operationCreateAccount.addSigners(sign);
			}
		}
		operation.setCreateAccount(operationCreateAccount);
	}
	/**
	 * 转账
	 * @param operation
	 * @param opt
	 */
	private static void transfer(Message.Operation.Builder operation,Operation opt){
		Message.OperationPayment.Builder operationPayment = Message.OperationPayment.newBuilder();
		operationPayment.setDestAddress(opt.getDest_address());
		Message.Asset.Builder asset = Message.Asset.newBuilder();
		Message.AssetProperty.Builder assetProperty = Message.AssetProperty.newBuilder();
		assetProperty.setType(Message.AssetProperty.Type.IOU);
		assetProperty.setIssuer(opt.getAsset_issuer());
		assetProperty.setCode(opt.getAsset_code());
		asset.setProperty(assetProperty);
		asset.setAmount(opt.getAsset_amount());
		
//		if(!Tools.isNull(opt.getDetails())){
//			for(Detail detail : opt.getDetails()){
//				Message.Detail.Builder detailBuilder  = Message.Detail.newBuilder();
//				detailBuilder.setAmount(detail.getAmount());
//				detailBuilder.setExt(detail.getExt());
//				detailBuilder.setStart(detail.getStart());
//				detailBuilder.setLength(detail.getLength());
//				asset.addDetails(detailBuilder);
//			}
//		}
		operationPayment.setAsset(asset);
		operation.setPayment(operationPayment);
	}
	/**
	 * 发行
	 * @param operation
	 * @param opt
	 */
	private static void issue(Message.Operation.Builder operation,Operation opt){
		Message.OperationIssueAsset.Builder operationMint = Message.OperationIssueAsset.newBuilder();
		Message.Asset.Builder asset = Message.Asset.newBuilder();
		Message.AssetProperty.Builder assetProperty = Message.AssetProperty.newBuilder();
		assetProperty.setType(Message.AssetProperty.Type.IOU);
		assetProperty.setIssuer(opt.getAsset_issuer());
		assetProperty.setCode(opt.getAsset_code());
		asset.setProperty(assetProperty);
		asset.setAmount(opt.getAsset_amount());
		operationMint.setAsset(asset);
		operation.setIssueAsset(operationMint);
	}
	/**
	 * set account property
	 * @param operation
	 * @param opt
	 */
	private static void setAccount(Message.Operation.Builder operation,Operation opt){
		Message.OperationSetOptions.Builder operationSetOptions = Message.OperationSetOptions.newBuilder();
		if(!Tools.isNull(opt.getAccount_metadata())){
			ByteString metadata = ByteString.copyFromUtf8(opt.getAccount_metadata());
			operationSetOptions.setAccountMetadata(metadata);
		}
		Threshold threshold = opt.getThreshold();
		if(!Tools.isNull(threshold)){
			operationSetOptions.setMasterWeight(threshold.getMaster_weight());
			operationSetOptions.setLowThreshold(threshold.getLow_threshold());
			operationSetOptions.setMedThreshold(threshold.getMed_threshold());
			operationSetOptions.setHighThreshold(threshold.getHigh_threshold());
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
		operation.setSetoptions(operationSetOptions);
	}
	/**
	 * init transfer
	 * @param operation
	 * @param opt
	 */
	private static void grantasset(Message.Operation.Builder operation,Operation opt){
		Message.OperationInitPayment.Builder operationInitPayment = Message.OperationInitPayment.newBuilder();
		operationInitPayment.setDestAddress(opt.getDest_address());
		Message.Asset.Builder asset = Message.Asset.newBuilder();
		Message.AssetProperty.Builder assetProperty = Message.AssetProperty.newBuilder();
		assetProperty.setType(Message.AssetProperty.Type.IOU);
		assetProperty.setIssuer(opt.getAsset_issuer());
		assetProperty.setCode(opt.getAsset_code());
		asset.setProperty(assetProperty);
		asset.setAmount(opt.getAsset_amount());
		
//		if(!Tools.isNull(opt.getDetails())){
//			for(Detail detail : opt.getDetails()){
//				Message.Detail.Builder detailBuilder  = Message.Detail.newBuilder();
//				detailBuilder.setAmount(detail.getAmount());
//				detailBuilder.setExt(detail.getExt());
//				detailBuilder.setStart(detail.getStart());
//				detailBuilder.setLength(detail.getLength());
//				asset.addDetails(detailBuilder);
//			}
//		}
		operationInitPayment.setAsset(asset);
		operation.setInitPayment(operationInitPayment);
	}
}
