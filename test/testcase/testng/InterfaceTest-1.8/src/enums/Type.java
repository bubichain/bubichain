package enums;

public enum Type {

	NATIVE(0,"布比"),
	IOU (1,"用户发行资产"),
	UNIQUE(2,"唯一资产");
	
	private Integer code;
	private String desc;
	
	private Type (int code , String desc) {
		this.code = code;
		this.desc = desc;
	}
}
