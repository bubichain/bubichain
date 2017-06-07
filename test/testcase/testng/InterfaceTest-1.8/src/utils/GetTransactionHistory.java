package utils;
import java.io.IOException;

import net.sf.json.JSONObject;

import org.apache.http.HttpEntity;
import org.apache.http.HttpResponse;
import org.apache.http.client.HttpClient;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.impl.client.DefaultHttpClient;
import org.apache.http.util.EntityUtils;
import org.testng.annotations.Test;

import com.google.gson.Gson;
import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonParser;


public class GetTransactionHistory {

	public static void main(String[] args) {

//		String baseUrl = "http://bubichain-qa.chinacloudapp.cn:19333";
//		String transaction = "getTransactionHistory";
//		String key = "hash";
//		String value = "81bcd3c6463a9edc3b07aa9b62a80c8164c09efb6ba2e2090dcc3a2d157599fb";
//		
//		String item  = getResponse(baseUrl, transaction, key, value);
////		System.out.println(item);
//		String error_code = getErrorCode(item);
//		System.out.println("hash = " + value);
//		System.out.println("error_code : " + error_code);
		
	}
	/**
	 * getTransactionHistory ≤‚ ‘
	 * @param item
	 * @return
	 */
	@Test
	public void getTransactionHistoryTest(){
		String transaction = "getTransactionHistory";
		String key = "hash";
		String value = "ed2e54c2f9fdef84d443750e872e8f3fe0a851912e3f8abcf7eddc4f81213854";
		
		String item  =HttpUtil.getResponse(transaction, key, value);
		System.out.println("item" + item);
	}

}
