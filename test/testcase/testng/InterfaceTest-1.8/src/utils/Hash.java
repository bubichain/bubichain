package utils;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

import org.apache.commons.codec.binary.Hex;

public class Hash {

	
	public static String getHash(byte[] buffer) {
		MessageDigest md=null;
		String hash ="";
		try {
			md = MessageDigest.getInstance("SHA-256");
			md.update(buffer);
			hash=Hex.encodeHexString( md.digest());
		} catch (NoSuchAlgorithmException e) {
			throw new RuntimeException(e.getMessage(), e);
		}
		
		return hash;
	}
}
