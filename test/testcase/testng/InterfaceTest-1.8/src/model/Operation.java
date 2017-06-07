package model;

import java.util.List;

import com.google.protobuf.ByteString;

import cn.bubi.blockchain.adapter.Message;
import model.asset.Detail;



public class Operation {
	private Integer type;
	private String dest_address;
	private String source_address;
	private Long init_balance;
	private Integer asset_type;
	private String asset_issuer;
	private String asset_code;
	private String asset_unit;
	private Long asset_amount;
	private String metadata;
	private String account_metadata;
	private Threshold threshold;
	private List<Detail> details;
	private List<Signer> signers;
	private List<Input> inputs;
	private List<Output> outputs;
	private String asset_detailed;
	private Message.AssetProperty.Type assetProperty_type;
	private String tran_metadata;
	private String record_id;
	private String record_ext;
	private String record_address;
	private String publickey;
	private Long seq;
	private Integer fee;
	private String tran_source_add;
	private TimeRange timeRange;
	public enum Type {
		/**
		 * 创建账号
		 * type = 0
		 */
        CREATE(0),
        /**
         * 转账
         * type = 1
         */
        TRADE(1),
        /**
         * 发行资产
         * type = 2
         */
        ISSUEASSET(2),
        /**
         * 设置订单
         */
        ORDER(3),
        /**
         * 
         */
        SETACCOUNTCFG(4),
        GRANTASSET(5),
        /**
         * 信息凭证记录
         */
        RECORDINFO(6)
        ;
        private Integer type;
        Type (Integer type) {
            this.type = type;
        }
        public Integer getValue() {
            return this.type;
        }
    }
	
	public void setPublickey(String publickey){
		this.publickey = publickey;
	}
	
	public String getPublickey(){
		return publickey;
	}
	public void setTimeRange(TimeRange timeRange){
		this.timeRange = timeRange;
	}
	public TimeRange getTimeRange(){
		return timeRange;
	}
	public void setTranSource_add(String tran_source_add){
		this.tran_source_add = tran_source_add;
	}
	
	public String getTranSource_add(){
		return tran_source_add;
	}
	
	
	
	public void setFee(Integer fee){
		this.fee = fee;
	}
	
	public Integer getFee(){
		return fee;
	}
	
	public void setSequence_num(Long seq){
		this.seq = seq;
	}
	public Long getSequencenum(){
		return seq;
	}
	public void setRecord_id(String record_id){
		this.record_id = record_id;
	}
	
	public String getRecored_id(){
		return record_id;
	}
	public void setRecord_address(String record_address){
		this.record_address = record_address;
	}
	
	public String getRecored_address(){
		return record_address;
	}
	public void setRecord_ext(String record_ext){
		this.record_ext = record_ext;
	}
	
	public String getRecored_ext(){
		return record_ext;
	}
	
	public void setAPtype(Message.AssetProperty.Type type){
		this.assetProperty_type = type;
	}
	
	public Message.AssetProperty.Type getAPtype(){
		return assetProperty_type;
	}
	
	public void setTransaction_metadata(String metadata){
		this.tran_metadata = metadata;
	}
	
	public String getTransaction_metadata(){
		return tran_metadata;
	}
	
	public List<Detail> getDetails() {
		return details;
	}
	public void setDetails(List<Detail> details) {
		this.details = details;
	}
	
	public String getDest_address() {
		return dest_address;
	}
	public void setDest_address(String dest_address) {
		this.dest_address = dest_address;
	}
	
	public void setAsset_detailed(String asset_detailed){
		this.asset_detailed = asset_detailed;
	}
	
	public ByteString getAsset_detailed(){
		return ByteString.copyFrom(asset_detailed.getBytes());
	}
	public Long getInit_balance() {
		return init_balance;
	}
	public void setInit_balance(Long init_balance) {
		this.init_balance = init_balance;
	}
	public Integer getAsset_type() {
		return asset_type;
	}
	public void setAsset_type(Integer asset_type) {
		this.asset_type = asset_type;
	}
	public String getAsset_issuer() {
		return asset_issuer;
	}
	public void setAsset_issuer(String asset_issuer) {
		this.asset_issuer = asset_issuer;
	}
	public String getAsset_code() {
		return asset_code;
	}
	public void setAsset_code(String asset_code) {
		this.asset_code = asset_code;
	}
	public String getAsset_unit() {
		return asset_unit;
	}
	public void setAsset_unit(String asset_unit) {
		this.asset_unit = asset_unit;
	}
	public Long getAsset_amount() {
		return asset_amount;
	}
	public void setAsset_amount(Long asset_amount) {
		this.asset_amount = asset_amount;
	}
//	public List<Detail> getDetails() {
//		return details;
//	}
//	public void setDetails(List<Detail> details) {
//		this.details = details;
//	}
	public Integer getType() {
		return type;
	}
	public void setType(Integer type) {
		this.type = type;
	}
	public List<Input> getInputs() {
		return inputs;
	}
	public void setInputs(List<Input> inputs) {
		this.inputs = inputs;
	}
	public List<Output> getOutputs() {
		return outputs;
	}
	public void setOutputs(List<Output> outputs) {
		this.outputs = outputs;
	}
	public String getMetadata() {
		return metadata;
	}
	public void setMetadata(String metadata) {
		this.metadata = metadata;
	}
	public String getSource_address() {
		return source_address;
	}
	public void setSource_address(String source_address) {
		this.source_address = source_address;
	}
	public Threshold getThreshold() {
		return threshold;
	}
	public void setThreshold(Threshold threshold) {
		this.threshold = threshold;
	}
	public List<Signer> getSigners() {
		return signers;
	}
	public void setSigners(List<Signer> signers) {
		this.signers = signers;
	}
	public String getAccount_metadata() {
		return account_metadata;
	}
	public void setAccount_metadata(String account_metadata) {
		this.account_metadata = account_metadata;
	}
	
}
