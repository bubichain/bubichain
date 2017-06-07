package model;

public class FromTh {

	private String address;
	private int index;
	private String hash;
	
	public void setAddress(Account acc){
		this.address = acc.getAddress();
	}
	public String getAddress(){
		return address;
	}
	public void setIndex(Integer index){
		this.index = index;
	}
	public int getIndex(){
		return index;
	}
	public void setHash(String hash){
		this.hash = hash;
	}
	public String getHash(){
		return hash;
	}
}
