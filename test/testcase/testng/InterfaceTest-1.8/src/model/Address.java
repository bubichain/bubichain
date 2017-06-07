package model;

public class Address {

	private String address;
	private Long amount;
	private Integer number;
	public  void setAddress(String address){
		this.address = address;
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
	public void setNumber(Integer number){
		this.number = number;
	}
	public Integer getNumber(){
		return number;
	}
	
	public String toString(){
		return "address:" + address + " number: " + number + "\n";
	}
}
