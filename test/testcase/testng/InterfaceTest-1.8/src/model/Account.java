package model;

import java.util.List;

import base.TestBase;
import cn.bubi.common.acc.AccountFactory;
import cn.bubi.common.acc.BubiInfo;
import utils.APIUtil;
import utils.HttpUtil;
import utils.Result;

public class Account extends TestBase{

	private String address;
	private String pub_key;
	private String priv_key;
	public long seqence;
	private Threshold threshold;
	private List<Signer> signers;
	private String metadata;
	private Long init_balance;
	
	public Account(){}
	public Account(String address, String pri, String pub){
		this.address = address;
		this.priv_key = pri;
		this.pub_key = pub;
	}
	public Long getInit_balance() {
		return init_balance;
	}
	public void setInit_balance(Long init_balance) {
		this.init_balance = init_balance;
	}
	
	public void setMetada(String metadata){
		this.metadata = metadata;
	}
	
	public String getMetada(){
		return metadata;
	}
	
	public List<Signer> getSigners() {
		return signers;
	}
	public void setSigners(List<Signer> signers) {
		this.signers = signers;
	}
	
	public void setThreshold(Threshold threshold){
		this.threshold = threshold;
	}
	
	public Threshold getThreshold(){
		return threshold;
	}
	
//	public void setSequence(String hash){
//		this.seqence = ZMQhandle.map1.get(hash).getNewAccountSeq()+1;
//	}
//	
//	public Long getSequence(){
//		return seqence;
//	}
	
	public void setAddress(String address){
		this.address = address;
	}
	
	public String getAddress(){
		return address;
	}
	
	public void setPub_key(String pub_key){
		this.pub_key = pub_key;
	}
	
	public String getPub_key(){
		return pub_key;
	}
	
	public void setPri_key(String pri_key){
		this.priv_key = pri_key;
	}
	
	public String getPri_key(){
		return priv_key;
	}
	
	public Account acc(String add, String pri, String pub, String url1){
		this.address = add;
		this.priv_key = pri;
		this.pub_key = pub;
		String accInfo = HttpUtil.get(url1 + "getAccount?address=" + add);
		this.seqence = Long.parseLong(Result.getTx_seqFromResult(accInfo))+1;
		return new Account();
	} 
	
	public Long getSeq(Account account, String url){
		String accInfo =HttpUtil.get(url + "getAccount?address=" + account.getAddress());
		account.seqence = Long.parseLong(Result.getTx_seqFromResult(accInfo))+1;
		return account.seqence;
	}
	
	public static Long getSeq(Account account){
		System.out.println("account info: " + account.getAddress());
		String accInfo =HttpUtil.get(url_getAccInfo + "getAccount?address=" + account.getAddress());
		APIUtil.wait(1);
		account.seqence = Long.parseLong(Result.getTx_seqFromResult(accInfo))+1;
		return account.seqence;
	}
	
	public static String getAccountInfo(Account account){
		String accInfo =HttpUtil.get(url_getAccInfo + "getAccount?address=" + account.getAddress());
		return accInfo;
	}
	
	public static Account generateAccount(){
		Account account = new Account();
		BubiInfo bubiInfo = AccountFactory.generateBubiInfo();
		account.setAddress(bubiInfo.getBubiAddress());
		account.setPri_key(bubiInfo.getPriKey());
		account.setPub_key(bubiInfo.getPubKey());
		return account;
	}
	
}
