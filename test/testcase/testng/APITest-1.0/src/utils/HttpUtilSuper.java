package utils;


import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

import javax.net.ssl.SSLContext;

import net.sf.json.JSONObject;

import org.apache.http.HttpEntity;
import org.apache.http.NameValuePair;
import org.apache.http.client.config.RequestConfig;
import org.apache.http.client.entity.UrlEncodedFormEntity;
import org.apache.http.client.methods.CloseableHttpResponse;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.config.Registry;
import org.apache.http.config.RegistryBuilder;
import org.apache.http.conn.socket.ConnectionSocketFactory;
import org.apache.http.conn.socket.PlainConnectionSocketFactory;
import org.apache.http.conn.ssl.NoopHostnameVerifier;
import org.apache.http.conn.ssl.SSLConnectionSocketFactory;
import org.apache.http.conn.ssl.TrustStrategy;
import org.apache.http.entity.StringEntity;
import org.apache.http.impl.client.CloseableHttpClient;
import org.apache.http.impl.client.HttpClientBuilder;
import org.apache.http.impl.conn.PoolingHttpClientConnectionManager;
import org.apache.http.message.BasicNameValuePair;
import org.apache.http.ssl.SSLContextBuilder;
import org.apache.http.util.EntityUtils;

import base.ErrorHandler;


public class HttpUtilSuper {

	private static CloseableHttpClient httpClient = null;
	private static final String ENCODING = "utf-8";
	private static final int timeout = 3000;
	private static  RequestConfig requestConfig = null; 
	
	static{
		//设置全局配置-链接超时时长
		requestConfig = RequestConfig.custom()
				 		// 依次是代理地址，代理端口号，协议类型
//						.setProxy(new HttpHost("127.0.0.1", 8080, "http"))
						.setConnectionRequestTimeout(timeout)
						.setConnectTimeout(timeout)    
						.setSocketTimeout(timeout).build();

		//获取一个忽略SSL证书验证的http client对象,可以同时兼容https协议
		httpClient = getIgnoreSSLCertificateHttpClient();	
	}
	
	@SuppressWarnings("deprecation")
	public static CloseableHttpClient getIgnoreSSLCertificateHttpClient(){

		SSLContext sslContext = null;
		try {
			sslContext = new SSLContextBuilder().loadTrustMaterial(null, new TrustStrategy() {
		    @Override
		    public boolean isTrusted(final X509Certificate[] arg0, final String arg1)
		      throws CertificateException {
		      //永远返回验证通过
		      return true;
		    }
		  }).build();
		} catch (Exception e) {
		  ErrorHandler.stopRunning(e, "创建http client失败!");
		}
		SSLConnectionSocketFactory sslSocketFactory = new SSLConnectionSocketFactory(sslContext,
		  new NoopHostnameVerifier());
		Registry<ConnectionSocketFactory> socketFactoryRegistry = RegistryBuilder
		  .<ConnectionSocketFactory> create()
		  .register("http", PlainConnectionSocketFactory.getSocketFactory())
		  .register("https", sslSocketFactory).build();
		PoolingHttpClientConnectionManager connMgr = new PoolingHttpClientConnectionManager(socketFactoryRegistry);
		return HttpClientBuilder.create().setSslcontext(sslContext).setConnectionManager(connMgr)
		  .build();
	}
	
	public static String doGet(String url){
		return get(url, null, null);
	}

	public static String doGet(String url, String charset){
		return get(url, null, charset);
	}
	
	public static String doGet(String url, Map<String, Object> params){	
		url += "?" + CommonUtil.map2UrlParams(params, ENCODING);
		return get(url, null, null);		
	}
	
	public static String doGet(String url, Map<String, Object> headers, String charset){	
		return get(url, headers, charset);		
	}
	
	public static String doGet(String url, Map<String, Object> headers, Map<String, Object> params, String charset){	
		url += "?" + CommonUtil.map2UrlParams(params, ENCODING);
		return get(url, headers, null);		
	}
	
	public static String doPost(String url, String requestStr, String charset){
		return post(url, null, requestStr, null, charset);		
	}

	public static String doPost(String url, Map<String, Object> headers, String requestStr, String charset){
		return post(url, headers, requestStr, null, charset);			
	}
	
	public static String doPost(String url, Map<String,Object> params, String charset){
		return post(url, null, null, params, charset);		
	}
	
	public static String doPost(String url, Map<String, Object> headers, Map<String,Object> params, String charset){
		return post(url, headers, null, params, charset);			
	}
	
	
	private static String get(String url, Map<String, Object> headers, String charset){
		String result = "";
		try {
			//统一通过url字符串创建get方法实体
			HttpGet httpGet = new HttpGet(url);
			//加载全局环境配置
			httpGet.setConfig(requestConfig);
			//如果设置了请求报文头,加载处理
			if(!CommonUtil.isEmpty(headers)){
				for(Entry<String, Object> entry : headers.entrySet()){
					httpGet.addHeader(entry.getKey(),  entry.getValue().toString());
		         }
			}			
			
			//创建CloseableHttpResponse 而不使用HttpResponse接口,是为了调用close()关闭链接
			
			Date start_date=new Date();
			CloseableHttpResponse response = httpClient.execute(httpGet);
			Date end_date=new Date();
			long responseTime = end_date.getTime()-start_date.getTime();
			int statusCode = response.getStatusLine().getStatusCode();
			if (statusCode != 200) {
				//注销链接
				httpGet.abort();
				ErrorHandler.stopRunning("响应状态异常："+statusCode);
		    }
			HttpEntity entity = response.getEntity();
			if (entity != null){
				//未指定编码格式的默认utf-8
				if(charset == null){
					result = EntityUtils.toString(entity, ENCODING);
				}else{
					result = EntityUtils.toString(entity, charset);
				}
		    }				
			
			//响应实体是input流对象，使用后要注销相当于关闭流,减少io开销
	        EntityUtils.consume(entity);
	        //关闭链接
	        response.close();
		} catch (Exception e) {
			ErrorHandler.stopRunning(e, "发送http-get请求异常");
		}			
		return result;
	}
	
	private static String post(String url, Map<String, Object> headers, String requestStr, Map<String,Object> params, String charset){
		
		String result = "";		
		if(!CommonUtil.isEmpty(charset)){
			charset = ENCODING;
		}
		
		try {
			//创建post方法实体
			HttpPost httpPost = new HttpPost(url);
			//加载全局环境配置
			httpPost.setConfig(requestConfig);
			//如果设置了请求报文头,加载处理
			if(!CommonUtil.isEmpty(headers)){
				for(Entry<String, Object> entry : headers.entrySet()){
					httpPost.addHeader(entry.getKey(),  entry.getValue().toString());
		        }
			}	
			
			//如果使用的是从字符串实体,直接装载实体
			if(!CommonUtil.isEmpty(requestStr)){
				StringEntity strEntity = new StringEntity(requestStr, charset);
				httpPost.setEntity(strEntity);
			}
			
			//如果使用的是Map键值对,转换处理,拼接报文实体
			if(!CommonUtil.isEmpty(params)){
				List<NameValuePair> pairs = new ArrayList<NameValuePair>(params.size());
                for(Map.Entry<String,Object> entry : params.entrySet()){
                    String value = entry.getValue().toString();
                    if(!CommonUtil.isEmpty(value)){
                        pairs.add(new BasicNameValuePair(entry.getKey(), value));
                    }
                }
                
                if(!CommonUtil.isEmpty(pairs)){
                	httpPost.setEntity(new UrlEncodedFormEntity(pairs, charset));
                }
			}
						
			CloseableHttpResponse response = httpClient.execute(httpPost);
			int statusCode = response.getStatusLine().getStatusCode();
			if (statusCode != 200) {
				httpPost.abort();
				ErrorHandler.stopRunning("响应状态异常："+statusCode);
		    }
			HttpEntity entity = response.getEntity();
			if (entity != null){
				if(charset == null){
					result = EntityUtils.toString(entity, ENCODING);
				}else{
					result = EntityUtils.toString(entity, charset);
				}
		    }				
	        EntityUtils.consume(entity);
	        response.close();
			
		} catch (Exception e) {
			ErrorHandler.stopRunning(e, "发送http-post请求异常");
		}
		return result;		
	}

	private static String post(String url, JSONObject jsonObject) {

		String result = "";
		// if(!CommonUtil.isEmpty(charset)){
		// charset = ENCODING;
		// }

		try {
			// 创建post方法实体
			HttpPost httpPost = new HttpPost(url);
			// 加载全局环境配置
			httpPost.setConfig(requestConfig);
			// 如果设置了请求报文头,加载处理
			// if(!CommonUtil.isEmpty(headers)){
			// for(Entry<String, Object> entry : headers.entrySet()){
			// httpPost.addHeader(entry.getKey(), entry.getValue().toString());
			// }
			// }

			// 如果使用的是从字符串实体,直接装载实体
			// if(!CommonUtil.isEmpty(requestStr)){
			// StringEntity strEntity = new StringEntity(requestStr, charset);
			// httpPost.setEntity(strEntity);
			// }

			// 如果使用的是Map键值对,转换处理,拼接报文实体
			// if(!CommonUtil.isEmpty(params)){
			// List<NameValuePair> pairs = new
			// ArrayList<NameValuePair>(params.size());
			// for(Map.Entry<String,Object> entry : params.entrySet()){
			// String value = entry.getValue().toString();
			// if(!CommonUtil.isEmpty(value)){
			// pairs.add(new BasicNameValuePair(entry.getKey(), value));
			// }
			// }

			// if(!CommonUtil.isEmpty(pairs)){
			// httpPost.setEntity(new UrlEncodedFormEntity(pairs, charset));
			// }
			// }

			CloseableHttpResponse response = httpClient.execute(httpPost);
			int statusCode = response.getStatusLine().getStatusCode();
			if (statusCode != 200) {
				httpPost.abort();
				ErrorHandler.stopRunning("响应状态异常：" + statusCode);
			}
			HttpEntity entity = response.getEntity();
			// if (entity != null){
			// if(charset == null){
			// result = EntityUtils.toString(entity, ENCODING);
			// }else{
			// result = EntityUtils.toString(entity, charset);
			// }
			// }
			EntityUtils.consume(entity);
			response.close();

		} catch (Exception e) {
			ErrorHandler.stopRunning(e, "发送http-post请求异常");
		}
		return result;
	}

}
