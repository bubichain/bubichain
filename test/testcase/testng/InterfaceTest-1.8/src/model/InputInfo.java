package model;

public class InputInfo {

	private String hash;
	private Account account;
	private Account srcAcc;
	private Account firstLevel;
	private Account secondLevel;
	private Account threeLevel;
	
	public void setHash(String hash){
		this.hash = hash;
	}
	public String getHash(){
		return hash;
	}
	public void setOutputAccount(Account account){
		this.account = account;
	}
	public Account getOutputAccount(){
		return account;
	}
	public void setSrcAcc(Account srcAcc){
		this.srcAcc = srcAcc;
	}
	public Account getSrcAcc(){
		return srcAcc;
	}
	public void setFirstLevel(Account account){
		this.firstLevel = account;
	}
	public Account getFirstLevel(){
		return firstLevel;
	}
	public void setSecondLevel(Account account){
		this.secondLevel = account;
	}
	public Account getSecondLevel(){
		return secondLevel;
	}
	public void setThreeLevel(Account account){
		this.threeLevel = account;
	}
	public Account getThreeLevel(){
		return threeLevel;
	}
	
}
