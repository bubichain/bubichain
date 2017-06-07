package utils;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.net.URLEncoder;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Set;

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



public class HttpUtil extends TestBase
{
	

	/**
	 * get请求，一个参数（比如hello交易）
	 * @param transaction
	 * @return
	 */
	
	public static String dogetApi(String baseurl,String apipath,LinkedHashMap<String, String> keyvalues)
	{
		String result = null;
		String url = baseurl + apipath;
		
		do
		{
			if (keyvalues==null)
				break;
			if(keyvalues.size()==0)
				break;
			url=url+"?";
			for (Iterator it =  keyvalues.keySet().iterator();it.hasNext();)
		   {
				Object key = it.next();
			    try {
					url+= URLEncoder.encode((String)key,"utf-8") +"=" + URLEncoder.encode(keyvalues.get(key),"utf-8") + "&";
				} catch (UnsupportedEncodingException e) {
					// TODO 自动生成的 catch 块
					e.printStackTrace();
				}  
		   }		
		
			url = url.substring(0,url.length()-1);//discard the last &
			
		}while (false);
		System.out.println( "posturl:"+url);
		System.out.println( "body:");
		
		long ct1 = System.currentTimeMillis();
		String t = String.valueOf(ct1);
		System.out.println("send at："+t);
		
		result = HttpPool.getapi(url);
		System.out.println( "response:");
		JSONObject jsonResult = new JSONObject();	
		jsonResult = JSONObject.fromObject(result);
		if(jsonResult!=null)
		{
			 Iterator iterator = jsonResult.keys();
			 while(iterator.hasNext()){
				 String key = (String) iterator.next();
				 System.out.println("\""+key+"\""+":" +"\""+jsonResult.getString(key)+"\"");
			 }		      
		}
		long ct2 = System.currentTimeMillis();
		t = String.valueOf(ct2);
		System.out.println("receiv at："+t);
		t = String.valueOf(ct2-ct1);
		System.out.println("cost time："+t+"ms");
		return result;
	}
	
	
	
	public static String dopostApi(String baseurl,String apipath,LinkedHashMap<String, String> keyvalues,JSONObject jsonObject)
	{
		String result = null;
		String url = baseurl + apipath;
		
		do
		{
			if(keyvalues.size()==0)
				break;
			url=url+"?";
			for (Iterator it =  keyvalues.keySet().iterator();it.hasNext();)
		   {
		    Object key = it.next();
		    try {
				url+= URLEncoder.encode((String)key,"utf-8") +"=" + URLEncoder.encode(keyvalues.get(key),"utf-8") + "&";
			} catch (UnsupportedEncodingException e) {
				// TODO 自动生成的 catch 块
				e.printStackTrace();
			}  
		   }		
		
			url = url.substring(0,url.length()-1);//discard the last &
			
		}while (false);
		System.out.println( "posturl:"+url);
		System.out.println( "body:");
		if(jsonObject!=null)
		{
			 Iterator iterator = jsonObject.keys();
			 while(iterator.hasNext()){
				 String key = (String) iterator.next();
				 System.out.println("\""+key+"\""+":" +"\""+jsonObject.getString(key)+"\"");
			 }		      
		}
		long ct1 = System.currentTimeMillis();
		String t = String.valueOf(ct1);
		System.out.println("send at："+t);
		
		result = HttpPool.postapi(url,jsonObject);
		System.out.println( "response:");
		JSONObject jsonResult = new JSONObject();	
		jsonResult = JSONObject.fromObject(result);
		if(jsonResult!=null)
		{
			 Iterator iterator = jsonResult.keys();
			 while(iterator.hasNext()){
				 String key = (String) iterator.next();
				 System.out.println("\""+key+"\""+":" +"\""+jsonResult.getString(key)+"\"");
			 }		      
		}
		long ct2 = System.currentTimeMillis();
		t = String.valueOf(ct2);
		System.out.println("receiv at："+t);
		t = String.valueOf(ct2-ct1);
		System.out.println("cost time："+t+"ms");
		return result;
	}
	
}
