package model;

public class Input {
	private String hash;
	private Integer index;
	private String metadata;
	private byte[] hash_input;
	
	public String getHash() {
		return hash;
	}
	public void setHash(String hash) {
		this.hash = hash;
	}
	public Integer getIndex() {
		return index;
	}
	public void setIndex(Integer index) {
		this.index = index;
	}
	public byte[] getHash_Input() {
		return hash_input;
	}
	public void setHash_Input(byte[] hash_input) {
		this.hash_input = hash_input;
	}
	public String getMetadata() {
		return metadata;
	}
	public void setMetadata(String metadata) {
		this.metadata = metadata;
	}
}
