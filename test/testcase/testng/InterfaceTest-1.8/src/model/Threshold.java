package model;

public class Threshold {
	private Integer master_weight;
	private Integer low_threshold;
	private Integer med_threshold;
	private Integer high_threshold;
	private String metadata;
	private String account_metadata;
	public Threshold(){
	}
	public Threshold(Integer master_weight, Integer low_threshold, Integer med_threshold, Integer high_threshold, String metadata) {
		super();
		this.master_weight = master_weight;
		this.low_threshold = low_threshold;
		this.med_threshold = med_threshold;
		this.high_threshold = high_threshold;
		this.metadata = metadata;
	}
	public Integer getMaster_weight() {
		return master_weight;
	}
	public void setMaster_weight(Integer master_weight) {
		this.master_weight = master_weight;
	}
	public Integer getLow_threshold() {
		return low_threshold;
	}
	public void setLow_threshold(Integer low_threshold) {
		this.low_threshold = low_threshold;
	}
	public Integer getMed_threshold() {
		return med_threshold;
	}
	public void setMed_threshold(Integer med_threshold) {
		this.med_threshold = med_threshold;
	}
	public Integer getHigh_threshold() {
		return high_threshold;
	}
	public void setHigh_threshold(Integer high_threshold) {
		this.high_threshold = high_threshold;
	}
	public String getMetadata() {
		return metadata;
	}
	public void setMetadata(String metadata) {
		this.metadata = metadata;
	}
	public String getAccount_metadata() {
		return account_metadata;
	}
	public void setAccount_metadata(String account_metadata) {
		this.account_metadata = account_metadata;
	}
}
