package utils;

import java.io.IOException;
import java.net.UnknownHostException;
import java.util.List;

import javax.net.ssl.SSLException;
import javax.net.ssl.SSLHandshakeException;

import net.sf.json.JSONObject;

import org.apache.http.HttpEntity;
import org.apache.http.HttpEntityEnclosingRequest;
import org.apache.http.HttpRequest;
import org.apache.http.HttpResponse;
import org.apache.http.NoHttpResponseException;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.HttpRequestRetryHandler;
import org.apache.http.client.config.RequestConfig;
import org.apache.http.client.methods.CloseableHttpResponse;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.client.protocol.HttpClientContext;
import org.apache.http.config.Registry;
import org.apache.http.config.RegistryBuilder;
import org.apache.http.conn.ConnectTimeoutException;
import org.apache.http.conn.socket.ConnectionSocketFactory;
import org.apache.http.conn.socket.LayeredConnectionSocketFactory;
import org.apache.http.conn.socket.PlainConnectionSocketFactory;
import org.apache.http.conn.ssl.SSLConnectionSocketFactory;
import org.apache.http.entity.StringEntity;
import org.apache.http.impl.client.CloseableHttpClient;
import org.apache.http.impl.client.HttpClients;
import org.apache.http.impl.conn.PoolingHttpClientConnectionManager;
import org.apache.http.protocol.HttpContext;
import org.apache.http.util.EntityUtils;

import com.google.gson.JsonObject;

import base.ErrorHandler;
import base.TestBase;

public class HttpPool extends TestBase {

	private RequestConfig requestConfig = null;
	private static int maxTotal = 200;
	private static int maxPerRoute = 50;
	private static int reTryTimes = 3;
	private static final String ENCODING = "utf-8";

	public HttpPool() {
		requestConfig = RequestConfig.custom()
				.setConnectionRequestTimeout(5000)
				.setConnectTimeout(6000)
				.setSocketTimeout(5000).build();
	}
	
	public static CloseableHttpClient getClientFromHttpPool() {
		ConnectionSocketFactory plainsf = PlainConnectionSocketFactory
				.getSocketFactory();
		LayeredConnectionSocketFactory sslsf = SSLConnectionSocketFactory
				.getSocketFactory();
		Registry<ConnectionSocketFactory> registry = RegistryBuilder
				.<ConnectionSocketFactory> create().register("http", plainsf)
				.register("https", sslsf).build();
		PoolingHttpClientConnectionManager cm = new PoolingHttpClientConnectionManager(
				registry);
		// 线程池最大连接数
		cm.setMaxTotal(maxTotal);
		// 每个站点最大连接数
		cm.setDefaultMaxPerRoute(maxPerRoute);

		HttpRequestRetryHandler httpRequestRetryHandler = new HttpRequestRetryHandler() {
			@Override
			public boolean retryRequest(IOException exception,
					int executionCount, HttpContext context) {
				if (executionCount >= reTryTimes) {// 如果已经重试了n次，就放弃
					return false;
				}
				if (exception instanceof NoHttpResponseException) {// 如果服务器丢掉了连接，那么就重试
					return true;
				}
				if (exception instanceof SSLHandshakeException) {// 不要重试SSL握手异常
					return false;
				}

				if (exception instanceof UnknownHostException) {// 目标服务器不可达
					return false;
				}
				if (exception instanceof ConnectTimeoutException) {// 连接被拒绝
					return false;
				}
				if (exception instanceof SSLException) {// ssl握手异常
					return false;
				}

				HttpClientContext clientContext = HttpClientContext
						.adapt(context);
				HttpRequest request = clientContext.getRequest();
				// 如果请求是幂等的，就再次尝试
				if (!(request instanceof HttpEntityEnclosingRequest)) {
					return true;
				}
				return false;
			}
		};

		return HttpClients.custom().setConnectionManager(cm)
				.setRetryHandler(httpRequestRetryHandler).build();
	}

	public static String doGet(String url) {
		String result = get(baseUrl + url);
		return result;

	}

	public static String doGet(String url, String key, Object value) {
		String url1 = baseUrl + url + "?" + key + "=" + value;
		System.out.println(url1);
		String result = get(url1);
		return result;
	}
	
	public static String doGet(String baseurl,String url, String key, Object value) {
		String url1 = baseurl + url + "?" + key + "=" + value;
//		System.out.println(url);
		String result = get(url1);
		return result;
	}
	
	

	/**
	 * get请求中需要处理延迟的
	 * 
	 * @param url1
	 * @param key
	 * @param value
	 * @return
	 */
	public static String doGetDelay(String url, String key, Object value) {
		String url1 = baseUrl + url + "?" + key + "=" + value;
//		System.out.println(url);
		String result = get(url1);
		int error_code = Result.getErrorCode(result);
		if (error_code == 4) {
			for (int i = 0; i < timeout; i++) {
				APIUtil.wait(1);
				String re = get(url1);
				error_code = Result.getErrorCode(re);
				if (error_code == 0) {
					return re;
				}
			}
		}
		return result;
	}

	/**
	 * get请求中需要处理延迟的
	 * 
	 * @param url1
	 * @param key
	 * @param value
	 * @return
	 */
	public static String doGetDelay(String url1, String key1, Object value1,
			String key2, Object value2) {
		String url = baseUrl + url1 + "?" + key1 + "=" + value1 + "&" + key2
				+ "=" + value2;
//		System.out.println(url);
		String result = get(url);
		int error_code = Result.getErrorCode(result);
		if (error_code == 4) {
			for (int i = 0; i < timeout; i++) {
				APIUtil.wait(1);
				String re = get(url);
				error_code = Result.getErrorCode(re);
				if (error_code == 0) {
					return re;
				}
			}
		}
		return result;
	}

	public static String doGet(String url1, String key1, Object value1,
			String key2, Object value2) {
		String url = baseUrl + url1 + "?" + key1 + "=" + value1 + "&" + key2
				+ "=" + value2;
		 System.out.println(url);
		String result = get(url);
		return result;

	}

	public static String get(String url) {
		String result = "";
		String charset = "utf-8";
		HttpGet httpGet = new HttpGet(url);
		try {
			CloseableHttpResponse response = getClientFromHttpPool().execute(
					httpGet);
			int statusCode = response.getStatusLine().getStatusCode();
			if (statusCode != 200) {
				httpGet.abort();
				ErrorHandler.stopRunning("响应状态异常：" + statusCode);
			}
			HttpEntity entity = response.getEntity();
			result = EntityUtils.toString(entity, charset);
			EntityUtils.consume(entity);
			response.close();
		} catch (Exception e) {
			ErrorHandler.stopRunning(e, "发送http-get请求异常");
		}
		return result;
	}

	public static String doPost(String url1, JSONObject jsonObject) {
		String url = baseUrl + url1;
		System.out.println("posturl: " + url);
		String result = post(url, jsonObject);
		return result;
	}
	
	public static String doPost(String baseurl,String url1, JSONObject jsonObject) {
		String url = baseurl + url1;
		String result = post(url, jsonObject);
		return result;
	}

	public static String doPost(List list) {
		String url = baseUrl + "submitTransaction";
		JSONObject items = JSONObject.fromObject(list);
		String result = post(url, items);
		return result;
	}

	public static String doPost(JSONObject jsonObject) {
		String url = baseUrl + "getTransactionBlob";
		String result = post(url, jsonObject);
		return result;
	}

	public static String post(String url, Object jsonObject) {
		HttpPost httpPost = new HttpPost(url);
		CloseableHttpClient client = getClientFromHttpPool();
		String respContent = null;
		StringEntity entity = new StringEntity(jsonObject.toString(), "utf-8");// 解决中文乱码问题
		entity.setContentEncoding("UTF-8");
		entity.setContentType("application/json");
		httpPost.setEntity(entity);

		HttpResponse resp;
		try {
			resp = client.execute(httpPost);
			if (resp.getStatusLine().getStatusCode() == 200) {
				HttpEntity he = resp.getEntity();
				respContent = EntityUtils.toString(he, "UTF-8");
			}
		} catch (Exception e) {
			ErrorHandler.stopRunning(e, "发送http-post请求异常");
		}
		return respContent;
		// try {
		// CloseableHttpResponse response = getClientFromHttpPool().execute(
		// httpPost);
		//
		// int statusCode = response.getStatusLine().getStatusCode();
		// if (statusCode != 200) {
		// httpPost.abort();
		// ErrorHandler.stopRunning("响应状态异常：" + statusCode);
		// }
		// HttpEntity entity = response.getEntity();
		// if (entity != null) {
		// if (charset == null) {
		// result = EntityUtils.toString(entity, ENCODING);
		// } else {
		// result = EntityUtils.toString(entity, charset);
		// }
		// }
		// EntityUtils.consume(entity);
		// response.close();
		//
		// StringEntity entity = new StringEntity(jsonObject.toString(),
		// "utf-8");
		// httpPost.setEntity(entity);
		// response = httpClient.execute(httpPost);
		// HttpEntity entity2 = response.getEntity();
		// result = EntityUtils.toString(entity2);
		//
		// HttpEntity entity = response.getEntity();
		// result = EntityUtils.toString(entity, charset);
		// EntityUtils.consume(entity);
		// response.close();
		// } catch (Exception e) {
		// ErrorHandler.stopRunning(e, "发送http-post请求异常");
		// }
		// return result;
	}
}