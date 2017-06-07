package utils;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.Locale;

public class DateHandler {

	public static String getTimeStamp() {
		return getFormatDate(new Date(), "yyyyMMddHHmmssSS");
	}

	public static String getNowDay() {
		return getFormatDate(new Date(), "yyyy-MM-dd");
	}
	 public static String TimeStamp2Date(String timestampString) {
//	        if (TextUtils.isEmpty(formats))
	         String  formats = "yyyy-MM-dd HH:mm:ss";
	        Long timestamp = Long.parseLong(timestampString) * 1000;
	        String date = new SimpleDateFormat(formats, Locale.CHINA).format(new Date(timestamp));
	        return date;
	    }
	public static String getSimpleDay(){
		Date date = new Date();
		SimpleDateFormat sd = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss SSS");
		return sd.format(date);
	}

	public static String getForwardDay(String date, int increment) {
		Calendar AddDay = Calendar.getInstance();
		AddDay.setTime(ParseDate(date, "yyyy-MM-dd"));
		AddDay.add(Calendar.DATE, increment);
		return getFormatDate(AddDay.getTime(), "yyyy-MM-dd");
	}

	private static String getFormatDate(Date date, String reg) {
		SimpleDateFormat dateFormat = new SimpleDateFormat(reg);
		return dateFormat.format(date);
	}

	private static Date ParseDate(String date, String reg) {
		SimpleDateFormat dateFormat = new SimpleDateFormat(reg);
		try {
			return dateFormat.parse(date);
		} catch (ParseException e) {
			e.printStackTrace();
			return null;
		}
	}
}
