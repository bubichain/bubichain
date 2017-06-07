package utils;
import java.io.IOException;

import net.sf.json.JSONObject;

import org.apache.http.HttpEntity;
import org.apache.http.HttpResponse;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.HttpClient;
import org.apache.http.client.methods.CloseableHttpResponse;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.entity.StringEntity;
import org.apache.http.impl.client.CloseableHttpClient;
import org.apache.http.impl.client.DefaultHttpClient;
import org.apache.http.impl.client.HttpClients;
import org.apache.http.util.EntityUtils;

import base.TestBase;



public class HttpUtil extends TestBase{
	

	/**
	 * get请求，一个参数（比如hello交易）
	 * @param transaction
	 * @return
	 */
	public  static String  doget(String transaction){
		String url = baseUrl + transaction;
		System.out.println(url);
		CloseableHttpClient client = HttpClients.createDefault();
		HttpGet httpGet = new HttpGet(url); 
		String entityString = null ;
		//执行get请求
		CloseableHttpResponse response;
		try {
			response = client.execute(httpGet);
			HttpEntity entity = response.getEntity();
//			System.out.println(EntityUtils.toString(entity));
			entityString = EntityUtils.toString(entity);
			response.close();
		} catch (ClientProtocolException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
		return entityString;
	}

	public static String doget(String url, String transaction) {
		String url1 = url + transaction;
//		System.out.println(url1);
		CloseableHttpClient client = HttpClients.createDefault();
		HttpGet httpGet = new HttpGet(url1);
		String entityString = null;
		// 执行get请求
		CloseableHttpResponse response;
		try {
			response = client.execute(httpGet);
			HttpEntity entity = response.getEntity();
			// System.out.println(EntityUtils.toString(entity));
			entityString = EntityUtils.toString(entity);
			response.close();
		} catch (ClientProtocolException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
		return entityString;
	}
	
	public  static String  doget(String baseurl,String transaction ,String key,Object value){
		String url = baseurl + transaction + "?" + key + "=" + value;
//		System.out.println("geturl: " + url);
		CloseableHttpClient client = HttpClients.createDefault();
		HttpGet httpGet = new HttpGet(url); 
		String entityString = null ;
		//执行get请求
		CloseableHttpResponse response;
		try {
			response = client.execute(httpGet);
//			APIUtil.wait(2);
			HttpEntity entity = response.getEntity();
			entityString = EntityUtils.toString(entity);
			response.close();
		} catch (ClientProtocolException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
		return entityString;
	}
	
	public  static String  dogetTh(String baseurl,String transaction ){
		String url = baseurl + transaction ;
//		System.out.println("geturl: " + url);
		CloseableHttpClient client = HttpClients.createDefault();
		HttpGet httpGet = new HttpGet(url); 
		String entityString = null ;
		//执行get请求
		CloseableHttpResponse response;
		try {
			response = client.execute(httpGet);
//			APIUtil.wait(2);
			HttpEntity entity = response.getEntity();
			entityString = EntityUtils.toString(entity);
			response.close();
		} catch (ClientProtocolException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
		return entityString;
	}
	/**
	 * get请求，请求路径，需要设置一组请求参数
	 * @param transaction
	 * @param key
	 * @param value
	 * @return
	 */
	public  static String  doget(String transaction ,String key,Object value){
		
		String url = baseUrl + transaction + "?" + key + "=" + value;
//		System.out.println("geturl: "+url);
		CloseableHttpClient client = HttpClients.createDefault();
		HttpGet httpGet = new HttpGet(url); 
		String entityString = null ;
		//执行get请求
		CloseableHttpResponse response;
		try {
			response = client.execute(httpGet);
//			APIUtil.wait(2);
			HttpEntity entity = response.getEntity();
			entityString = EntityUtils.toString(entity);
			response.close();
		} catch (ClientProtocolException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
		return entityString;
	}

	/**
	 * get请求，请求路径，需要设置两组请求参数
	 * @param transaction
	 * @param key1
	 * @param value1
	 * @param key2
	 * @param value2
	 * @return
	 */
	public  static String doget(String transaction ,String key1,String value1,String key2,String value2){
	
	String url = baseUrl + transaction + "?" + key1 + "=" + value1 + "&"+key2 + "=" + value2;
	System.out.println("url: " + url);
	CloseableHttpClient client = HttpClients.createDefault();
	HttpGet httpGet = new HttpGet(url); 
	String entityString = null ;
	//执行get请求
	CloseableHttpResponse response;
	try {
		response = client.execute(httpGet);
		HttpEntity entity = response.getEntity();
		entityString = EntityUtils.toString(entity);
		response.close();
		
	} catch (ClientProtocolException e) {
		e.printStackTrace();
	} catch (IOException e) {
		e.printStackTrace();
	}
	return entityString;
}
	/**
	 * post发送json数据，请求都是submitTransaction
	 * @param jsonObject
	 * @return
	 */
	/**
	 * 通过url发送post请求，得到getTransactionHistory的响应
	 * @param 
	 * @param transaction
	 * @param key
	 * @param value
	 * @return
	 */
	public static String getResponse(String transaction,String key , String value){
		String baseUrl = "http://bubichain-qa.chinacloudapp.cn:19333/";
		String url = baseUrl + transaction + "?" + key + "=" + value ;
		String responseData = null ;
		@SuppressWarnings({ "resource", "deprecation" })
		HttpClient httpClient = new DefaultHttpClient();
		HttpGet httpGet = new HttpGet(url);
		try {
			HttpResponse httpResponse = httpClient.execute(httpGet);
			HttpEntity entity = httpResponse.getEntity();
			responseData = EntityUtils.toString(entity);
//			System.out.println("responseData: " + responseData);
		} catch (IOException e) {
			e.printStackTrace();
		}
		return responseData;
	}
	public static String dopost(JSONObject jsonObject){
		String url = baseUrl + "submitTransaction";
		System.out.println("post_url: " + url );
		String result = null;
		@SuppressWarnings({ "resource", "deprecation" })
		HttpClient httpClient = new DefaultHttpClient();
		
		HttpPost httpPost = new HttpPost(url);
		
		StringEntity entity = new StringEntity(jsonObject.toString(),"utf-8");
		httpPost.setEntity(entity);
		HttpResponse response;
		try {
			response = httpClient.execute(httpPost);
			HttpEntity entity2 = response.getEntity();
			result = EntityUtils.toString(entity2);
//			System.out.println(result);
		} catch (ClientProtocolException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
		return result;
	}
	
	public static String get(String url) {
		CloseableHttpClient client = HttpClients.createDefault();
		HttpGet httpGet = new HttpGet(url);
		String entityString = null;
		// 执行get请求
		CloseableHttpResponse response;
		try {
			response = client.execute(httpGet);
			HttpEntity entity = response.getEntity();
			// System.out.println(EntityUtils.toString(entity));
			entityString = EntityUtils.toString(entity);
			response.close();
		} catch (ClientProtocolException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
		return entityString;
	}
	
	/**
	 * 手动设置url，发送post请求
	 * 
	 * @param url1
	 * @param jsonObject
	 * @return
	 */
	public static String dopost(String url1, JSONObject jsonObject) {
		String url = baseUrl + url1;
		 System.out.println("postUrl: " + url);
		String result = null;
		@SuppressWarnings({ "resource", "deprecation" })
		HttpClient httpClient = new DefaultHttpClient();

		HttpPost httpPost = new HttpPost(url);

		StringEntity entity = new StringEntity(jsonObject.toString(), "utf-8");
		httpPost.setEntity(entity);
		HttpResponse response;
		try {
			response = httpClient.execute(httpPost);
			HttpEntity entity2 = response.getEntity();
			result = EntityUtils.toString(entity2);
			// System.out.println(result);
		} catch (ClientProtocolException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
		return result;
	}
	
	public static String dopost(String baseurl,String url1, JSONObject jsonObject) {
		String url = baseurl + url1;
//		System.out.println(url);
		String result = null;
		@SuppressWarnings({ "resource", "deprecation" })
		HttpClient httpClient = new DefaultHttpClient();
		HttpPost httpPost = new HttpPost(url);
		StringEntity entity = new StringEntity(jsonObject.toString(), "utf-8");
		httpPost.setEntity(entity);
		HttpResponse response;
		try {
			response = httpClient.execute(httpPost);
			HttpEntity entity2 = response.getEntity();
			result = EntityUtils.toString(entity2);
//			 System.out.println(result);
		} catch (ClientProtocolException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
	
		return result;
	}
	
	public static String dopost(String baseurl,String url1, String jsonObject) {
		String url = baseurl + url1;
//		System.out.println(url);
		String result = null;
		@SuppressWarnings({ "resource", "deprecation" })
		HttpClient httpClient = new DefaultHttpClient();
		HttpPost httpPost = new HttpPost(url);
		StringEntity entity = new StringEntity(jsonObject.toString(), "utf-8");
		httpPost.setEntity(entity);
		HttpResponse response;
		try {
			response = httpClient.execute(httpPost);
			HttpEntity entity2 = response.getEntity();
			result = EntityUtils.toString(entity2);
//			 System.out.println(result);
		} catch (ClientProtocolException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
	
		return result;
	}
	
	public static String dopostGetTransactionBlob(JSONObject jsonObject){
		String url = "getTransactionBlob";
		String blob = HttpPool.doPost(url, jsonObject);
		return blob;
	}
	
}
