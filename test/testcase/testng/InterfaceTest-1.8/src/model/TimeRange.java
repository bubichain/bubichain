package model;

public class TimeRange {

	private Long max;
	private Long min;
	
	public void setMaxTime(Long max){
		this.max = max;
	}
	public Long getMaxTime(){
		return max;
	}
	public void setMinTime(Long min){
		this.min = min;
	}
	public Long getMinTime(){
		return min;
	}
}
