package utils;

import java.io.UnsupportedEncodingException;
import java.security.InvalidKeyException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.PrivateKey;
import java.security.Signature;
import java.security.SignatureException;
import java.util.List;

import net.i2p.crypto.eddsa.EdDSAEngine;
import net.i2p.crypto.eddsa.EdDSAPrivateKey;
import net.i2p.crypto.eddsa.spec.EdDSANamedCurveTable;
import net.i2p.crypto.eddsa.spec.EdDSAParameterSpec;
import net.i2p.crypto.eddsa.spec.EdDSAPrivateKeySpec;
import net.sf.json.JSONObject;
import base.CheckPoint;
import base.Log;
import base.TestBase;
import cn.bubi.tools.acc.Sign;

import com.google.gson.JsonObject;
import com.google.gson.JsonParser;

import freemarker.debug.Breakpoint;

public class SignUtil extends TestBase{
	static int time = 6;// timeout;
	/**
	 * 提交签名
	 * @param operations
	 * @param source_address
	 * @param fee
	 * @param sequence_number
	 * @param metadata
	 * @param pri
	 * @param pub
	 * @return
	 */
	@SuppressWarnings("rawtypes")
	public static String  tx(List operations,Object source_address, Object fee, Object sequence_number, Object metadata,String pri,Object pub){
		JSONObject tran = TxUtil.tran_json(source_address, fee, sequence_number, metadata, operations);
//		 System.out.println("transaction =" + tran + "\n");
		String blobresult = SignUtil.getUnSignBlobResult(tran);
		int err = Result.getoutErrCodeFromGet(blobresult);
		if (err ==0) {
			String blobString = SignUtil.getTranBlobsString(blobresult);
			String signdata;
			String result = null;
			try {
				signdata = Sign.priKeysign(blobString, pri);
				List signatures = TxUtil.signatures(signdata, pub);
				JSONObject jsonObject = TxUtil.itemJsonObject(signatures, blobString);
				result = HttpPool.doPost("submitTransaction",jsonObject);
				return result;
			} catch (Exception e) {
				e.printStackTrace();
			}
			if(err!=0){
				System.out.println("获取blob出错，error_code: " + err
						+ " err_desc: "
						+ Result.getErrDescFromGetResult(blobresult));
			}
		}
		return blobresult;
	}
	public static String  tx1(List operations,Object source_address, Object fee, Object metadata,String pri,Object pub){
		Object sequence_number = Result.seq_num(source_address);
		JSONObject tran = TxUtil.tran_json(source_address, fee, sequence_number, metadata, operations);
//		 System.out.println("transaction =" + tran + "\n");
		String blobresult = SignUtil.getUnSignBlobResult(tran);
		int err = Result.getoutErrCodeFromGet(blobresult);
		if (err ==0) {
			String blobString = SignUtil.getTranBlobsString(blobresult);
			String signdata;
			String result = null;
			try {
				signdata = Sign.priKeysign(blobString, pri);
				List signatures = TxUtil.signatures(signdata, pub);
				JSONObject jsonObject = TxUtil.itemJsonObject(signatures, blobString);
				result = HttpPool.doPost("submitTransaction",jsonObject);
				return result;
			} catch (Exception e) {
				e.printStackTrace();
			}
			if(err!=0){
				System.out.println("获取blob出错，error_code: " + err
						+ " err_desc: "
						+ Result.getErrDescFromGetResult(blobresult));
			}
		}
		return blobresult;
	}
	public static String  tx(List operations,Object source_address, Object fee, Object metadata,String pri,Object pub){
		APIUtil.wait(close_time);
		Object sequence_number = Result.seq_num(source_address);
		JSONObject tran = TxUtil.tran_json(source_address, fee, sequence_number, metadata, operations);
//		 System.out.println("transaction =" + tran + "\n");
		String blobresult = SignUtil.getUnSignBlobResult(tran);
		int err = Result.getoutErrCodeFromGet(blobresult);
		if (err ==0) {
			String blobString = SignUtil.getTranBlobsString(blobresult);
			String signdata;
			String result = null;
			try {
				signdata = Sign.priKeysign(blobString, pri);
				List signatures = TxUtil.signatures(signdata, pub);
				JSONObject jsonObject = TxUtil.itemJsonObject(signatures, blobString);
				result = HttpPool.doPost("submitTransaction",jsonObject);
				return result;
			} catch (Exception e) {
				e.printStackTrace();
			}
			if(err!=0){
				System.out.println("获取blob出错，error_code: " + err
						+ " err_desc: "
						+ Result.getErrDescFromGetResult(blobresult));
			}
		}
		return blobresult;
	}
	
	public static byte[] sign(byte blobArr[], String srcPrivateKey) throws Exception {
		String privateSrcKey = srcPrivateKey;
		byte privSrcArr[] = cn.bubi.common.util.Base58.decode(privateSrcKey);
		byte[] privArr = new byte[32];
		System.arraycopy(privSrcArr, 4, privArr, 0, privArr.length);
		Signature sgr = new EdDSAEngine(MessageDigest.getInstance("SHA-512"));
		EdDSAParameterSpec spec = EdDSANamedCurveTable.getByName("ed25519-sha-512");
		EdDSAPrivateKeySpec privKey = new EdDSAPrivateKeySpec(privArr, spec);
		PrivateKey sKey = new EdDSAPrivateKey(privKey);
		sgr.initSign(sKey);
		sgr.update(blobArr);
		return sgr.sign();
	}
	
	/**
	 * 有timerange
	 * @param operations
	 * @param source_address
	 * @param fee
	 * @param sequence_number
	 * @param metadata
	 * @param timeRange
	 * @param pri
	 * @param pub
	 * @return
	 */
	public static String  tx(List operations,Object source_address, Object fee, Object sequence_number, Object metadata,JSONObject timeRange,String pri,Object pub){
		JSONObject tran = TxUtil.tran_json(source_address, fee, sequence_number, metadata, operations,timeRange);
		String blobresult = SignUtil.getUnSignBlobResult(tran);
		int err = Result.getoutErrCodeFromGet(blobresult);
		if (err ==0) {
			String blobString = SignUtil.getTranBlobsString(blobresult);
			String signdata;
			String result = null;
			try {
				signdata = Sign.priKeysign(blobString, pri);
				List signatures = TxUtil.signatures(signdata, pub);
				JSONObject jsonObject = TxUtil.itemJsonObject(signatures, blobString);
				result = HttpPool.doPost("submitTransaction",jsonObject);
				return result;
			} catch (Exception e) {
				e.printStackTrace();
			}
			if(err!=0){
				System.out.println("获取blob出错，error_code: " + err
						+ " err_desc: "
						+ Result.getErrDescFromGetResult(blobresult));
			}
		}
		return blobresult;
		
	}
	public static String  tx2(List operations,Object source_address, Object fee,  Object metadata,JSONObject timeRange,String pri,Object pub){
		Object sequence_number = Result.seq_num(source_address);
		JSONObject tran = TxUtil.tran_json(source_address, fee, sequence_number, metadata, operations,timeRange);
		String blobresult = SignUtil.getUnSignBlobResult(tran);
		int err = Result.getoutErrCodeFromGet(blobresult);
		if (err ==0) {
			String blobString = SignUtil.getTranBlobsString(blobresult);
			String signdata;
			String result = null;
			try {
				signdata = Sign.priKeysign(blobString, pri);
				List signatures = TxUtil.signatures(signdata, pub);
				JSONObject jsonObject = TxUtil.itemJsonObject(signatures, blobString);
				result = HttpPool.doPost("submitTransaction",jsonObject);
				return result;
			} catch (Exception e) {
				e.printStackTrace();
			}
			if(err!=0){
				System.out.println("获取blob出错，error_code: " + err
						+ " err_desc: "
						+ Result.getErrDescFromGetResult(blobresult));
			}
		}
		return blobresult;
		
	}
	

	/**
	 * 提交签名数据包含错误字段
	 * @param operations
	 * @param source_address
	 * @param fee
	 * @param sequence_number
	 * @param metadata
	 * @param pri
	 * @param pub
	 * @return
	 */
	@SuppressWarnings("rawtypes")
	public static String  unnormal_tx(List operations,Object source_address, Object fee, Object sequence_number, Object metadata,String pri,Object pub){
		JSONObject tran = TxUtil.tran_json(source_address, fee, sequence_number, metadata, operations);
//		System.out.println("tran: " + tran);
		String blobr = SignUtil.getUnSignBlobResult(tran);
		int err = Result.getoutErrCodeFromGet(blobr);
		if (err ==0) {
			String blob1 = SignUtil.getTranBlobsString(blobr);
			String signdata;
			String result = null;
			try {
				signdata = Sign.priKeysign(blob1, pri);
				
				List signatures = TxUtil.signatures(signdata, pub);
				JSONObject jsonObject = TxUtil.itemJsonObject(signatures, blob1);
				result = HttpPool.doPost("submitTransaction", jsonObject);
				return result;
			} catch (Exception e) {
				e.printStackTrace();
				new CheckPoint().isFailed("私钥签名失败");
			}
		}
		return blobr;
	}
	@SuppressWarnings("rawtypes")
	public static String  unnormal_tx(List operations,Object source_address, Object fee, Object metadata,String pri,Object pub){
//		System.out.println("!!!source_address: "+source_address);
		Object sequence_number = Result.seq_num(source_address);
		JSONObject tran = TxUtil.tran_json(source_address, fee, sequence_number, metadata, operations);
//		System.out.println("tran: " + tran);
		String blobr = SignUtil.getUnSignBlobResult(tran);
		int err = Result.getoutErrCodeFromGet(blobr);
		if (err ==0) {
			String blob1 = SignUtil.getTranBlobsString(blobr);
			String signdata;
			String result = null;
			try {
				signdata = Sign.priKeysign(blob1, pri);
				
				List signatures = TxUtil.signatures(signdata, pub);
				JSONObject jsonObject = TxUtil.itemJsonObject(signatures, blob1);
				result = HttpPool.doPost("submitTransaction", jsonObject);
				return result;
			} catch (Exception e) {
				e.printStackTrace();
				new CheckPoint().isFailed("私钥签名失败");
			}
		}
		return blobr;
	}
	
	public static int  txGetBlob(List operations,Object source_address, Object fee, Object sequence_number, Object metadata,String pri,Object pub){
		JSONObject tran = TxUtil.tran_json(source_address, fee, sequence_number, metadata, operations);
		System.out.println("transaction ="+tran +"\n");
		APIUtil.wait(2);
//		Map<String, Object> map = new LinkedHashMap<String, Object>();
		String blob1 = SignUtil.getUnSignBlobResult(tran);
		int e1 = Result.getErrCodeFromBlob(blob1);
		if(e1==0){
			System.out.println("transaction_blob： "+blob1);
			String hash = Result.getHash(blob1); //从返回的blob结果中取得hash
			System.out.println("hash==" + hash);
			String signdata;
			String result = null;
			try {
				signdata = Sign.priKeysign(blob1, pri);
				
				List signatures = TxUtil.signatures(signdata, pub);
				
//				System.out.println("signatures:-------"+signatures);
				
				JSONObject jsonObject = TxUtil.itemJsonObject(signatures, blob1);
				
//				System.out.println("POST数据=" + jsonObject);
				
				result = HttpUtil.dopost(jsonObject);
				int e2 = Result.getoutErrCodeFromGet(result);
				return e2;
			} catch (Exception e) {
				e.printStackTrace();
			}
			return e1;
		}else{
			return e1;
		}
		
		
	}
	public static boolean verfySign(JSONObject tran,String pub,String pri){
		String blobre = SignUtil.getUnSignBlobResult(tran);
		String blobStr = SignUtil.getTranBlobsString(blobre);
		String signStr;
		boolean a = false;
		try {
			signStr = Sign.priKeysign(blobStr, pri);
			a = Sign.checkPriSign(blobStr, signStr, pub);
			
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		return a;
	}
	
	/**
	 * 联合签名的签名交易
	 * 
	 * @param operations
	 * @param source_address
	 * @param fee
	 * @param sequence_number
	 * @param metadata
	 * @param pri1
	 * @param pri2
	 * @param pub
	 * @return
	 */
	public static String  tx(List operations,Object source_address, Object fee, Object sequence_number, Object metadata,String pri1,String pri2,Object pub){
		JSONObject tran = TxUtil.tran_json(source_address, fee, sequence_number, metadata, operations);
		String blob1 = SignUtil.getUnSignBlobResult(tran);
		String signdata;
		String result = null;
		try {
			signdata = Sign.priKeysign(blob1, pri1);
			List signatures = TxUtil.signatures(signdata, pub);
			JSONObject jsonObject = TxUtil.itemJsonObject(signatures, blob1);
			result = HttpUtil.dopost(jsonObject);
			return result;
		} catch (Exception e) {
			e.printStackTrace();
		}
		return null;
	}
	
	
	/**
	 * 获取签名后的--冯瑞明
	 * @param private_key
	 * @param jsonObject
	 * @return
	 */
	public static String signdata(String private_key,JSONObject jsonObject){
		//签名
				String seed = private_key; // 私钥
				String message = jsonObject.toString();
				byte[] msg_byte = hexToBytes(message);
				Signature sgr;
				String signdata = null;
				try {
					sgr = new EdDSAEngine(MessageDigest.getInstance("SHA-512"));
					EdDSAParameterSpec spec = EdDSANamedCurveTable.getByName("ed25519-sha-512");
					EdDSAPrivateKeySpec privKey = new EdDSAPrivateKeySpec(seed.getBytes(), spec);
					PrivateKey sKey = new EdDSAPrivateKey(privKey);
					sgr.initSign(sKey);
					sgr.update(msg_byte);
					signdata = new String(msg_byte,"GB2312");
//					System.out.println("signdata: " + signdata);
				} catch (NoSuchAlgorithmException e1) {
					e1.printStackTrace();
				}catch (InvalidKeyException e) {
					e.printStackTrace();
				}catch (SignatureException e) {
					e.printStackTrace();
				}catch (UnsupportedEncodingException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
				return signdata;
				
	}
	
	
	
	public static String HexString( byte[] b) {     
		String hex = null;
		   for (int i = 0; i < b.length; i++) {    
		     hex = Integer.toHexString(b[i] & 0xFF);    
		     if (hex.length() == 1) {    
		       hex = '0' + hex;    
		     }    
		     
		   }    
		   return hex;
		} 
	
	public static byte[] hexToBytes(String s) {
        int len = s.length();
        byte[] data = new byte[len / 2];
        for (int i = 0; i < len; i += 2) {
            data[i / 2] = (byte) ((Character.digit(s.charAt(i), 16) << 4)
                    + Character.digit(s.charAt(i+1), 16));
        }
        return data;
    }
	
	public static String bytesToHex(byte[] raw) {
        if ( raw == null ) {
            return null;
        }
        final StringBuilder hex = new StringBuilder(2 * raw.length);
        for (final byte b : raw) {
            hex.append(Character.forDigit((b & 0xF0) >> 4, 16))
            .append(Character.forDigit((b & 0x0F), 16));
        }
        return hex.toString();
    }
	
	/**
	 * 获取未签名blob的结果
	 * @param jsonObject
	 * @return
	 */
	public static String getUnSignBlobResult(JSONObject jsonObject){
		String result = HttpUtil.dopost(baseUrl,"getTransactionBlob",jsonObject);
		
		int e3 = Result.getoutErrCodeFromGet(result);
		if(e3==0){
			return result;
		}else{
			Log.error("get blob failed");
			return result;
		}
	}
	
	public static String getUnSignBlobResult(String jsonObject){
		String result = HttpUtil.dopost(baseUrl,"getTransactionBlob",jsonObject);
		
		int e3 = Result.getoutErrCodeFromGet(result);
		if(e3==0){
			return result;
		}else{
			Log.error("get blob failed");
			return result;
		}
	}
	
	public static String getUnSignBlobResult1(String baseurl,String url,JSONObject jsonObject){
		String result = HttpUtil.dopost(baseurl,url,jsonObject);
//		String result = HttpClientUtil.httpPostRequest(baseurl, url, jsonObject);
		return result;
	}
	
	/**
	 * 		int e1 = Result.getoutErrCodeFromGet(result);
		int e2=99999;
		if (e1 !=0) {
			for(int i=0;i<60;i++){
				String re = HttpUtil.dopostGetTransactionBlob(jsonObject);
				e2 = Result.getErrCodeFromBlob(re);
				if (e2==0) {
					System.out.println("获取blob成功，blob = " + re);
					return re;
				}
			}
			new CheckPoint().equals(e1, 0, "获取未签名blob失败");
			new CheckPoint().isFailed("获取未签名blob失败");
		}else{
			String hash = SignUtil.getBlobTh(result, "hash");
			System.out.println("hash==" + hash);
			String re = Result.getTranHisByHash(hash);
			int e3 = Result.getoutErrCodeFromGet(re);
			if(e3 !=0){
				String warn = "获取blob出错了";
				return warn;
			}
		}
		
		return result;

	 */
	//backup
//	public static String getUnSignBlob(JSONObject jsonObject){
//		String url = baseUrl + "getTransactionBlob";
//		String result = null;
//		@SuppressWarnings({ "resource", "deprecation" })
//		HttpClient httpClient = new DefaultHttpClient();
//		HttpPost httpPost = new HttpPost(url);
//		StringEntity entity = new StringEntity(jsonObject.toString(),"utf-8");
//		httpPost.setEntity(entity);
//		HttpResponse response;
//		try {
//			response = httpClient.execute(httpPost);
//			HttpEntity entity2 = response.getEntity();
//			APIUtil.wait(2);
//			result = EntityUtils.toString(entity2);
//		} catch (ClientProtocolException e) {
//			e.printStackTrace();
//		} catch (IOException e) {
//			e.printStackTrace();
//		}
//		if(result !=null){
//			String transaction_blob =getTranBlob(result);
//			return transaction_blob;
//		}else{
//			System.out.println("获取未签名blob失败");
//			return null;
//		}
//		
//	}
	
	/**
	 * 获取未签名的blob串
	 * @param result
	 * @return
	 */
	public static String getTranBlobsString(String result){
		JsonObject resultJsonObject = new JsonParser().parse(result).getAsJsonObject();
		String transaction_blob = null;
		if (resultJsonObject.get("result").toString() !=null) {
			JsonObject jsonObject = (JsonObject) new JsonParser().parse(resultJsonObject.get("result").toString());
			transaction_blob = jsonObject.get("transaction_blob").getAsString();
		} else {
			System.out.println("获取blob出错");
		}
		return transaction_blob;
	}
	
	public static String getTranBlobsString1(String result){
		JsonObject resultJsonObject = new JsonParser().parse(result).getAsJsonObject();
		String transaction_blob = null;
		if (resultJsonObject.get("result").toString() !=null) {
			JsonObject jsonObject = (JsonObject) new JsonParser().parse(resultJsonObject.get("result").toString());
			transaction_blob = jsonObject.get("transaction_blob").getAsString();
		} else {
			return result;
		}
		return transaction_blob;
	}
	
	/**
	 * 获取getBlob返回里面的内容
	 * @param result
	 * @param value
	 * @return
	 */
	public static String getBlobTh(String result,String value){
		JsonObject resultJsonObject = new JsonParser().parse(result).getAsJsonObject();
		System.out.println("resultJsonObject======"+resultJsonObject);
		String transaction_blob = null;
		if (resultJsonObject.get("result").toString() !=null) {
			JsonObject jsonObject = (JsonObject) new JsonParser().parse(resultJsonObject.get("result").toString());
			
//			System.out.println("blob jsonobject:" + jsonObject);
			
			transaction_blob = jsonObject.get(value).getAsString();
			
		} else {
			System.out.println("获取blob出错");
		}
		return transaction_blob;
	}

}
