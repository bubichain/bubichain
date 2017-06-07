package model;

import cn.bubi.blockchain.adapter.Message;

public class TxInfo {

	private String hash;
	private String source_address;  //Trading sponsors
	private String dest_address;
	private Long recv_source_seq;
	private Long ledger_seq;
	private String operationType;
	private Long recv_newAcc_seq;
	private String issue_hash;
	private Long send_source_seq;
	public Account a1;
	public Account a2;
	private Integer error_code;
	private String peer_message;
	
	public void setPeerMessage(String peer_message){
		this.peer_message = peer_message;
	}
	
	public String getPeerMessage(){
		return peer_message;
	}
	public void setError_code(Integer errorcode){
		this.error_code = errorcode;
	}
	
	public Integer getError_code(){
		return error_code;
	}
	
	private Message.TransactionEnv.Builder txeb;
	
	public void setTranEnv(Message.TransactionEnv.Builder txeb){
		this.txeb = txeb;
	}
	
	public Message.TransactionEnv.Builder getTranEnv(){
		return txeb;
	}
	
	public Account getSrc_seq(){
		a1.seqence = recv_source_seq +1;
		return a1;
	}
	
	public Account getNew_seq(){
		a2.seqence = recv_newAcc_seq +1;
		return a2;
	}
	
	public void setSend_source_seq(Long seq){
		this.send_source_seq = seq;
	}
	
	public Long getSend_source_seq(){
		return send_source_seq;
	}
	
	public void setIssueHash(String hash){
		this.issue_hash = hash;
	}
	
	public String getIssueHash(){
		return issue_hash;
	}
	
	public void setRecv_Source_seq(Long seq){
		this.recv_source_seq = seq;
	}
	
	public Long getRecv_Source_seq(){
		return recv_source_seq;
	}
	
	public void setRecv_NewAcc_seq(Long seq){
		this.recv_newAcc_seq = seq;
	}
	
	public Long getRecv_NewAcc_seq(){
		return recv_newAcc_seq;
	}
	
	public void setRecv_ledgerSeq(Long seq){
		this.ledger_seq = seq;
	}
	
	public Long getRecv_ledgerSeq(){
		return ledger_seq;
	}
	
	public void setSend_Optype(Message.Operation.Type operationType){
		this.operationType = operationType.toString();
	}
	
	public String getSend_Optype(){
		return operationType;
	}
	
	public void setHash(String hash){
		this.hash = hash;
	}
	
	public String getHash(){
		return hash;
	}
	
	public void setSourceAdd(String sourceAdd){
		this.source_address = sourceAdd;
	}
	
	public String getSourceAdd(){
		return source_address;
	}
}
