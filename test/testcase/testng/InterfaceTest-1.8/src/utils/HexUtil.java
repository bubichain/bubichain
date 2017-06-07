package utils;

import java.io.UnsupportedEncodingException;

import org.apache.commons.codec.DecoderException;
import org.apache.commons.codec.binary.Hex;

import com.google.protobuf.ByteString;

import cn.bubi.common.util.Tools;

public class HexUtil {

	public static void main(String[] args) {
		String hash = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
//		String toBinString = HexStringToBinaryString(hash);
//		System.out.println(toBinString);
	}
	
	public static byte[] HexStringToBinaryByteArray(String hex) {
		final byte[] binary = new byte[hex.length() / 2];
		 for (int i = 0;  i < hex.length(); i += 2) {
			 byte c1 = (byte)Character.digit(hex.charAt(i), 16);
			 byte c2 = (byte)Character.digit(hex.charAt(i + 1), 16);
			 binary[i / 2] = (byte) ((c1 << 4) | c2);
		 }
		 return binary;
	}
	
	public static String toBinStr(String hash){
		StringBuffer sb = new StringBuffer();
		for (int i = 0; i < hash.length() / 2; i++) {
			String a1 = hash.substring(i * 2, i * 2 + 2); // 取前两位
			String a2 = a1.substring(0, 1);  // 取两位中第一位
			String str =  padLeft(a2);   //前面补位0
			String a2_ = hexString2binaryString(str);  //十六进制转二进制
//			System.out.println("a2_: " + a2_);
			String a22 = Integer.toBinaryString( Integer.parseInt(a2_, 2));
			String a3 = a1.substring(1, 2);    // 取两位中第二位
			String str2 =  padRight(a3);   //前面补位0 
			String a3_ = hexString2binaryString(str2);  //十六进制转二进制
//			System.out.println("a3_: " + a3_);
			int b1 = Integer.parseInt(a2_, 2);  //转成二进制int
			int b2 = Integer.parseInt(a3_, 2);
			int b3_ = b1 | b2;
			String n_s = Integer.toBinaryString(b3_);
			String nb = padRight(n_s,8);
			
//			System.out.println("$$$: " + n_s);
			sb.append(nb);
//			System.out.println(n_s+ " ");
//			System.out.println(nb + "***");
		}
//		System.out.println("sb: " + sb);
		return sb.toString();
	}
	 
	 /**
	  * 左补位，右对齐
	  * @param oriStr  原字符串
	  * @param len  目标字符串长度
	  * @param alexin  补位字符
	  * @return  目标字符串
	  */
	public static String padLeft(String oriStr, int len, char alexin) {
		String str = "";
		int strlen = oriStr.length();
		if (strlen < len) {
			for (int i = 0; i < len - strlen; i++) {
				str = str + alexin;
			}
		}
		str = oriStr + str;
		return str;
	}
	
	public static String padLeft(String oriStr, int len) {
		String str = "";
		int strlen = oriStr.length();
		if (strlen < len) {
			for (int i = 0; i < len - strlen; i++) {
				str = str + 0;
			}
		}
		str = oriStr + str;
		return str;
	}
	
	public static String padLeft(String oriStr) {
		String str = "";
		int strlen = oriStr.length();
		if (strlen < 2) {
			for (int i = 0; i < 2 - strlen; i++) {
				str = str + 0;
			}
		}
		str = oriStr + str;
		return str;
	}
	
	 /**
	  * 右补位，左对齐
	  * @param oriStr  原字符串
	  * @param len  目标字符串长度
	  * @param alexin  补位字符
	  * @return  目标字符串
	  */
	public String padRight(String oriStr, int len, char alexin) {
		String str = "";
		int strlen = oriStr.length();
		if (strlen < len) {
			for (int i = 0; i < len - strlen; i++) {
				str = str + alexin;
			}
		}
		str = str + oriStr;
		return str;
	}
	public static String padRight(String oriStr, int len) {
		String str = "";
		int strlen = oriStr.length();
		if (strlen < len) {
			for (int i = 0; i < len - strlen; i++) {
				str = str + 0;
			}
		}
		str = str + oriStr;
		return str;
	}
	public static String padRight(String oriStr) {
		String str = "";
		int strlen = oriStr.length();
		if (strlen < 2) {
			for (int i = 0; i < 2 - strlen; i++) {
				str = str + 0;
			}
		}
		str = str + oriStr;
		return str;
	}
	
	/** 
	 * 字符串转换成十六进制字符串 
	 */  
	  
	public static String str2HexStr(String str) { 
		try{
			if(Tools.isNull(str)){return "";}
		    char[] chars = "0123456789abcdef".toCharArray();  
		    StringBuilder sb = new StringBuilder("");  
		    byte[] bs = str.getBytes("utf-8");  
		    int bit;  
		    for (int i = 0; i < bs.length; i++) {  
		        bit = (bs[i] & 0x0f0) >> 4;  
		        sb.append(chars[bit]);  
		        bit = bs[i] & 0x0f;  
		        sb.append(chars[bit]);  
		    }  
		    return sb.toString();  
		}catch(Exception e){
			e.printStackTrace();
		}
		return "";
	}  
	
	public static ByteString toByteString(String a){
		return ByteString.copyFrom(a.getBytes());
	}
	public static ByteString toByte(byte[] bytes){
		return ByteString.copyFrom(bytes);
	}
	
	/** 
	 * 十六进制转换字符串 
	 */  
	  
	public static String hexStr2Str(String hexStr) {
		
		if(Tools.isNull(hexStr)){return "";}
	    String str = "0123456789abcdef";  
	    char[] hexs = hexStr.toCharArray();  
	    byte[] bytes = new byte[hexStr.length() / 2];  
	    int n;  
	    for (int i = 0; i < bytes.length; i++) {  
	        n = str.indexOf(hexs[2 * i]) * 16;  
	        n += str.indexOf(hexs[2 * i + 1]);  
	        bytes[i] = (byte) (n & 0xff);  
	    }  
	    try {
			return new String(bytes,"utf-8");
		} catch (UnsupportedEncodingException e) {
			e.printStackTrace();
		}
	    return "";
	}  
	  // 将字符串转换成二进制字符串，以空格相隔  
	public static String StrToBinstr(String str) {
		char[] strChar = str.toCharArray();
		String result = "";
		for (int i = 0; i < strChar.length; i++) {
			result += Integer.toBinaryString(strChar[i]) + " ";
		}
		return result;
	}

	/**
	 * 二进制转十六进制
	 * @param bString
	 * @return
	 */
	 public static String binaryString2hexString(String bString)
		{
			if (bString == null || bString.equals("") || bString.length() % 8 != 0)
				return null;
			StringBuffer tmp = new StringBuffer();
			int iTmp = 0;
			for (int i = 0; i < bString.length(); i += 4)
			{
				iTmp = 0;
				for (int j = 0; j < 4; j++)
				{
					iTmp += Integer.parseInt(bString.substring(i + j, i + j + 1)) << (4 - j - 1);
				}
				tmp.append(Integer.toHexString(iTmp));
			}
			return tmp.toString();
		}
	 
	 /**
	  * 十六进制转二进制
	  * @param hexString
	  * @return
	  */
	 public static String hexString2binaryString(String hexString)
		{
			if (hexString == null || hexString.length() % 2 != 0)
				return null;
			String bString = "", tmp;
			for (int i = 0; i < hexString.length(); i++)
			{
				tmp = "0000"
						+ Integer.toBinaryString(Integer.parseInt(hexString
								.substring(i, i + 1), 16));
				bString += tmp.substring(tmp.length() - 4);
			}
			return bString;
		}
}
