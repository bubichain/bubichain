package model;

public class Transfer {
	private String address;
	private Long amount;
	
	public void setAddress(Account account){
		this.address = account.getAddress();
	}
	public String getAddress(){
		return address;
	}
	public void setAmount(Long amount){
		this.amount = amount;
	}
	public Long getAmount(){
		return amount;
	}
}
