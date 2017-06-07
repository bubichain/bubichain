package model.asset;

public class Detail {
	private Long amount;
	private Long start;
	private Long length;
	private String ext;
	private String ext_hash;
	public Long getAmount() {
		return amount;
	}
	public void setAmount(Long amount) {
		this.amount = amount;
	}
	public Long getStart() {
		return start;
	}
	public void setStart(Long start) {
		this.start = start;
	}
	public Long getLength() {
		return length;
	}
	public void setLength(Long length) {
		this.length = length;
	}
	public String getExt() {
		return ext;
	}
	public void setExt(String ext) {
		this.ext = ext;
	}
	public String getExt_hash() {
		return ext_hash;
	}
	public void setExt_hash(String ext_hash) {
		this.ext_hash = ext_hash;
	}
	
	
}
