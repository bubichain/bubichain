package utils;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

import org.apache.http.NameValuePair;
import org.apache.http.client.entity.UrlEncodedFormEntity;
import org.apache.http.message.BasicNameValuePair;
import org.apache.http.util.EntityUtils;

public class CommonUtil {

	/**
	 * 判断字符串是否为空
	 * 
	 * @param s
	 * @return 如果字符串为空或者字符串去除首尾空格为空字符串则返回true,反之返回false
	 */
	public static boolean isEmpty(String s) {
		if (s == null || s.length() == 0) {
			return true;
		}
		return false;
	}
	
	/**
	 * 判断map是否为空
	 * 
	 * @param map
	 * @return 如果map==null或者map.size()==0则返回true,反之返回false
	 */
	@SuppressWarnings("all")
	public static boolean isEmpty(Map map) {
		if (map == null || map.size() == 0) {
			return true;
		}
		return false;
	}
	
	/***
	 * 判断list是否为空
	 * 
	 * @param list
	 *            list对象
	 * @return 如果list==null或者list.size==则返回true,反之返回false
	 */
	@SuppressWarnings("all")
	public static boolean isEmpty(List list) {
		if (list == null || list.size() == 0) {
			return true;
		}
		return false;
	}
	
//	public static String map2UrlParams(Map<String, Object> map, String charset) {
//		if (isEmpty(map)) {
//			return null;
//		}
//		StringBuilder sb = new StringBuilder();
//		for (Entry<String, Object> entry : map.entrySet()) {
//			if (!isEmpty(entry.getValue().toString())) {
//				String key = entry.getKey();
//				try {
//					String value = URLEncoder.encode(entry.getValue()
//							.toString(), charset);
//					sb.append("&" + key + "=" + value);
//				} catch (Exception e) {
//					ErrorHandler.stopRunning(e, "格式化请求参数错误:"+key+" - "+entry.getValue()+" ");
//				}
//			}
//		}
//		if (sb.length() > 0) {
//			return sb.substring(1);
//		}
//		return null;
//	}
	
	public static String map2UrlParams(Map<String, Object> map, String charset) {
		if (isEmpty(map)) {
			return null;
		}
		
		List<NameValuePair> pairs = new ArrayList<NameValuePair>(map.size());

		for (Entry<String, Object> entry : map.entrySet()) {
			if (!isEmpty(entry.getValue().toString())) {
				pairs.add(new BasicNameValuePair(entry.getKey(),entry.getValue().toString()));			
			}
		}
		try {
			return EntityUtils.toString(new UrlEncodedFormEntity(pairs, charset));
		} catch (Exception e) {
			return null;
		}
	}
	
	
}
