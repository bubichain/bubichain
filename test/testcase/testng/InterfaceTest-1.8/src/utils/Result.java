package utils;
import base.ErrorHandler;
import base.Log;
import base.TestBase;
import model.Address;
import net.sf.json.JSONArray;
import net.sf.json.JSONNull;
import net.sf.json.JSONObject;

import java.util.ArrayList;
import java.util.List;

import com.google.gson.JsonArray;
import com.google.gson.JsonObject;
import com.google.gson.JsonParser;
import com.google.gson.JsonSyntaxException;


public class Result extends TestBase{
	
	public static String getAddress(String result){
		String value = "address";
		String address = getResultTh(result, value);
		return address;
	}
	
	/**
	 * 获取供应链溯源depth大于等于1时的地址信息
	 * @param result
	 * @return
	 */
	public static String getAddInFrom(String result){
		JsonArray from = getResultTh1(result, "from");
		String address = from.get(0).getAsJsonObject().get("address").getAsString();
		return address;
	} 
	
	public static String getPrivatekey(String result){
		String value = "private_key";
		String private_key = getResultTh(result, value);
		return private_key;
	}
	
	public static String getPublickey(String result){
		String value = "public_key";
		String private_key = getResultTh(result, value);
		return private_key;
	}
	
	/**
	 * 从getAssetUnique 结果中取得asset_code
	 * @param result
	 * @return
	 */
	public static String getasset_code(String result){
		String asset_code = null;
		try {
			JsonObject data = new JsonParser().parse(result).getAsJsonObject();
			JsonArray resultjson = (JsonArray) new JsonParser().parse(data.get("result").toString());
			if(resultjson.size()>0){
				String re = resultjson.get(0).toString();
				JsonObject jsonObject = new JsonParser().parse(re).getAsJsonObject();
				asset_code = jsonObject.get("asset_code").getAsString();
			}
		} catch (JsonSyntaxException e) {
			Log.error(result);
		}
		return asset_code;
	}
	
	/**
	 * 获取交易记录中transaction的个数
	 * @return
	 */
	public static int getTranSize(String result){
		JsonObject data = new JsonParser().parse(result).getAsJsonObject();
		JsonObject resultjson = (JsonObject) new JsonParser().parse(data.get("result").toString());
		JsonArray trans = (JsonArray) new JsonParser().parse(resultjson.get("transactions").toString());
		return trans.size();
	}
	
	/**
	 * 获取asset_amount
	 * 
	 * @param result
	 * @return
	 */

	public static int getasset_amount(String result) {
		int asset_amount = 0;
		JsonObject data = new JsonParser().parse(result).getAsJsonObject();
		JsonObject resultjson = (JsonObject) new JsonParser().parse(data.get(
				"result").toString());
		JsonArray resultjson1 = (JsonArray) new JsonParser().parse(resultjson
				.get("transactions").toString());

		if (resultjson1.size() > 0) {
			String re = resultjson1.get(0).toString();
			JsonObject jsonObject = new JsonParser().parse(re)
					.getAsJsonObject();
			JsonArray resultjson2 = (JsonArray) new JsonParser()
					.parse(jsonObject.get("operations").toString());
			if (resultjson2.size() > 0) {
				String re1 = resultjson2.get(0).toString();
				JsonObject jsonObject2 = new JsonParser().parse(re1)
						.getAsJsonObject();
				asset_amount = jsonObject2.get("asset_amount").getAsInt();
			}
		}

		return asset_amount;

	}

	/**
	 * 获取base_fee
	 * 
	 * @param result
	 * @return
	 */
	public static int getbase_fee(String result) {
		JsonObject data = new JsonParser().parse(result).getAsJsonObject();
		JsonObject re = (JsonObject) new JsonParser().parse(data.get("result")
				.toString());
		int base_fee = re.get("base_fee").getAsInt();
		return base_fee;
	}

	/**
	 * 获取base_reserve
	 * 
	 * @param result
	 * @return
	 */
	public static int getbase_reserve(String result) {
		JsonObject data = new JsonParser().parse(result).getAsJsonObject();
		JsonObject re = (JsonObject) new JsonParser().parse(data.get("result")
				.toString());
		int base_reserve = re.get("base_reserve").getAsInt();
		return base_reserve;
	}

	/**
	 * 获取metadata
	 * 
	 * @param address
	 * @return
	 */
	public static String getMetadata(Object address) {
		String accountInfo = getAccInfo(address);
		JsonObject re = new JsonParser().parse(accountInfo).getAsJsonObject();
		JsonObject result = (JsonObject) new JsonParser().parse(re
				.get("result").toString());
		String metadata = result.get("metadata").getAsString();
		return metadata;
	}

	/**
	 * 获取master_weight
	 * 
	 * @param result
	 * @return
	 */
	public static int getThresholdTh(String result, String value) {
		int master_weight = 0;
		JsonObject item = new JsonParser().parse(result).getAsJsonObject();
		JsonObject res = (JsonObject) new JsonParser().parse(item.get("result")
				.toString());
		// System.out.println(result);
		JsonArray transactions = (JsonArray) new JsonParser().parse(res.get(
				"transactions").toString());
		if (transactions.size() > 0) {
			String tran = transactions.get(0).toString();
			// APIUtil.wait(2);
			JsonObject transac = new JsonParser().parse(tran).getAsJsonObject();
			JsonArray opers = (JsonArray) new JsonParser().parse(transac.get(
					"operations").toString());
			String oper = opers.get(0).toString();
			// APIUtil.wait(2);
			JsonObject op = new JsonParser().parse(oper).getAsJsonObject();
			JsonObject threshold = (JsonObject) new JsonParser().parse(op.get(
					"threshold").toString());
			master_weight = threshold.get(value).getAsInt();

		}
		return master_weight;
	}

	/**
	 * 从getAssetUnique 结果中取得asset_issuer
	 * 
	 * @param result
	 * @return
	 */
	public static String getasset_issuer(String result){
		String asset_issuer = null;
		JsonObject data = new JsonParser().parse(result).getAsJsonObject();
		JsonArray resultjson = (JsonArray) new JsonParser().parse(data.get("result").toString());
		if(resultjson.size()>0){
			String re = resultjson.get(0).toString();
			JsonObject jsonObject = new JsonParser().parse(re).getAsJsonObject();
			asset_issuer = jsonObject.get("asset_issuer").getAsString();
//			System.out.println("hash: " +hash);
		}
		
		return asset_issuer;
	}
	
	/**
	 * 从getAssetUnique 、获取区块高度结果中取得ledger_seq
	 * @param result
	 * @return
	 */
	public static int getledger_seq(String result){
		
		int ledger_seq = 000;
		JsonObject data = new JsonParser().parse(result).getAsJsonObject();
		JsonArray resultjson = (JsonArray) new JsonParser().parse(data.get("result").toString());
		if(resultjson.size()>0){
			String re = resultjson.get(0).toString();
			JsonObject jsonObject = new JsonParser().parse(re).getAsJsonObject();
			ledger_seq = jsonObject.get("ledger_seq").getAsInt();
		}
		return ledger_seq;
	}
	
	/**
	 * getLedger返回的结果中，解析ledger_seq的值
	 * @param result
	 * @return
	 */
	public static int getledger_seqDefault(String result){
		JsonObject data = new JsonParser().parse(result).getAsJsonObject();
		JsonObject re1 = (JsonObject) new JsonParser().parse(data.get("result").toString());
		int ledger_seq = re1.get("ledger_seq").getAsInt();
		return ledger_seq;
	}
	
	public static String getTx_count(String result){
		String value = "tx_count";
		String re = getResultTh(result, value);
		return re;
	}
	/**
	 * 从getAssetUnique结果中取得ledger_seq
	 * @param result  getAssetUnique返回的结果
	 * @param i   要取第几组值的ledger_seq，i从0开始
	 * @return
	 */
	public static int getledger_seq(String result,int i){
		
		int ledger_seq = 000;
		JsonObject data = new JsonParser().parse(result).getAsJsonObject();
		JsonArray resultjson = (JsonArray) new JsonParser().parse(data.get("result").toString());
		if(resultjson.size()>0){
			String re = resultjson.get(i).toString();
			JsonObject jsonObject = new JsonParser().parse(re).getAsJsonObject();
			ledger_seq = jsonObject.get("ledger_seq").getAsInt();
//			System.out.println("hash: " +hash);
		}
		return ledger_seq;
	}
	/**
	 * 获取默认的区块高度
	 * @return
	 */
	public static int getDefaultLedgerHeight(){
		String transaction = "getLedger";
		String result = HttpPool.doGet(transaction);
		APIUtil.wait(2);
//		System.out.println("getledger=="+result);
		int ledgerHeight = Result.getledger_seqDefault(result);
		return ledgerHeight;
	}
	
	public static int getLedgerBySeq(int seq){
		String transaction = "getLedger";
		String key = "seq";
		int value = seq;
//		System.out.println("value=="+value);
		String result = HttpPool.doGet(transaction, key, value);
//		System.out.println("seq ledger result= " + result);
		int ledgerHeight = Result.getledger_seqDefault(result);
		return ledgerHeight;
	}
	/**
	 * 根据address，查询账户信息中的ledger_seq
	 * @param address
	 * @return
	 */
	public static String getledger_seqFromAddress(Object address){
		String result = Result.getAccInfo(address);
		String tx_seq = getTx_seqFromResult(result);
		return tx_seq;
	}
	
	public static String getPre_ledger_seqFromAddress(Object address) {
		String result = Result.getAccInfo(address);
		String tx_seq = getPreledger_seqFromResult(result);
		return tx_seq;
	}

	/**
	 * 通过获取账户信息的返回结果，得到tx_seq
	 * @param result
	 * @return
	 */
	public static String getTx_seqFromResult(String result){
		try {
			String value = "tx_seq";
			String tx_seq = getResultTh(result, value);
			return tx_seq;
		} catch (Exception e) {
			Log.error(e.toString());
			return result;
		}
	}
	/**
	 * 根据address，查询账户信息中的ledger_seq
	 * @param address
	 * @return
	 */
	public static String getAccTx_seq(Object address){
		String accinfo = getAccInfo(address);
		String tx_seq =getTx_seqFromResult(accinfo);
		return tx_seq;
	}
	
	public static String getAccTx_seq(String url,Object address){
		String accinfo = getAccInfo(url,address);
		String tx_seq =getTx_seqFromResult(accinfo);
		return tx_seq;
	}
	
	public static String getledger_seqFromgetLedger(String result) {
		JsonObject resultJsonObject = new JsonParser().parse(result)
				.getAsJsonObject();
		// System.out.println("resultJsonObject"+resultJsonObject);
		JsonObject jsonObject = (JsonObject) new JsonParser()
				.parse(resultJsonObject.get("result").toString());
		// System.out.println(jsonObject);
		String tx_seq = jsonObject.get("ledger_seq").getAsString();
		return tx_seq;
	}

	public static String getPreledger_seqFromResult(String result) {
		JsonObject resultJsonObject = new JsonParser().parse(result)
				.getAsJsonObject();
		// System.out.println("resultJsonObject"+resultJsonObject);
		JsonObject jsonObject = (JsonObject) new JsonParser()
				.parse(resultJsonObject.get("result").toString());
		// System.out.println(jsonObject);
		String tx_seq = jsonObject.get("previous_ledger_seq").getAsString();
		return tx_seq;
	}
	
	/**
	 * 通过账户地址，得到当前的seq_num，并且+1了
	 * @param address
	 * @return
	 */
	public static long seq_num(Object address){
		try {
			String accInfo = HttpUtil.get(baseUrl + "getAccount?address=" + address);
			long seq_num = Long.parseLong(Result.getTx_seqFromResult(accInfo))+1;

//			String sequence_num = Result.getAccTx_seq(address);
//			long seq_num = Long.parseLong(sequence_num)+1;
			return seq_num;
		} catch (NumberFormatException e) {
			Log.error("获取seq失败的地址： " + address.toString());
			return 9999;
		}
		
	}
	
		
	public static long seq_num(String url,Object address){
		String sequence_num = Result.getAccTx_seq(url,address);
		long seq_num = Long.parseLong(sequence_num)+1;
		return seq_num;
	}
	/**
	 * 通过结果获取result中json的个数
	 * @param result
	 * @return
	 */
	public static int getResultSize(String result){
		try {
			JsonObject data = new JsonParser().parse(result).getAsJsonObject();
			JsonArray jsonArray = data.get("result").getAsJsonArray();
			int reSize = jsonArray.size();
			return reSize;
		} catch (JsonSyntaxException e) {
			Log.error(result);
			return 9999;
		}
	}
	
	/**
	 * 获取账户信息，入参是object
	 * @param value
	 * @return
	 */
	public static String getAccInfo(Object address){
		String transaction = "getAccount";
		String key = "address";
		Object value = address;
		String accountInfo = HttpUtil.doget(get_Url,transaction, key, value);
		int error_code = Result.getoutErrCodeFromGet(accountInfo);
		
		if(error_code!=0){
			if (error_code == 4) {
				for (int i = 0; i < timeout; i++) {
					APIUtil.wait(1);
					String accountInfo1 = HttpPool.doGetDelay(transaction, key,
							value);
					int error_code1 = Result.getoutErrCodeFromGet(accountInfo1);
					if (error_code1 == 0) {
						return accountInfo1;
					}
				}
			}
			return accountInfo;
		}
		return accountInfo;
	}
	
	public static String getAccInfo(String baseurl,Object address){
		String transaction = "getAccount";
		String key = "address";
		Object value = address;
		String accountInfo = HttpUtil.doget(baseurl,transaction, key, value);
		int error_code = Result.getoutErrCodeFromGet(accountInfo);
		
		if(error_code!=0){
			if (error_code == 4) {
				for (int i = 0; i < timeout; i++) {
					APIUtil.wait(1);
					String accountInfo1 = HttpPool.doGetDelay(transaction, key,
							value);
					int error_code1 = Result.getoutErrCodeFromGet(accountInfo1);
					if (error_code1 == 0) {
						return accountInfo1;
					}
				}
			}
			return accountInfo;
		}
		return accountInfo;
	}
	
	//获取账户的metadata_version
	public static int getMetadata_version(Object address){
		try {
			String accountInfo = getAccInfo(address);
			JsonObject re = new JsonParser().parse(accountInfo).getAsJsonObject();
			JsonObject result = (JsonObject) new JsonParser().parse(re.get("result").toString());
			int metadata_version = result.get("metadata_version").getAsInt();
			return metadata_version;
		} catch (JsonSyntaxException e) {
			Log.error(address.toString());
			return 9999;
		}
	}
	

		/**
		 * 获取hash
		 * {
  "results": [
    {
      "error_code": 0,
      "hash": "b29dfe229415e5b4710019fb25054c8644292a51674dc244ea3ee0817f3e0425"
    }
  ],
  "success_count": 1
}
		 * @param result POST请求返回的结果
		 * @return 返回hash的JsonObject对象
		 */
	public static String getHash(String result){
		String hash = null;
		try {
			JsonObject data = new JsonParser().parse(result).getAsJsonObject();
			JsonArray resultjson = (JsonArray) new JsonParser().parse(data.get("results").toString());
				String re = resultjson.get(0).toString();
				JsonObject jsonObject = new JsonParser().parse(re).getAsJsonObject();
				hash = jsonObject.get("hash").getAsString();
				return hash;
		} catch (JsonSyntaxException e) {
			Log.error(result);
			return result;
		}
	}
	

	
	/**
	 * 获取get请求中result下的字段值
	 * 
	 * @param response
	 * @param value
	 * @return
	 */
	public static String getResultTh(String response, String value) {
		try {
			JsonObject re = new JsonParser().parse(response).getAsJsonObject();
			JsonObject result = (JsonObject) new JsonParser().parse(re.get("result").toString());
			String result_ = result.get(value).getAsString();
			return result_;
		} catch (JsonSyntaxException e) {
			Log.error(response);
			return response;
		}
	}
	
	public static String getConsensusTh(String response, String value) {
		try {
			JsonObject re = new JsonParser().parse(response).getAsJsonObject();
			
			JsonObject result1 = (JsonObject) new JsonParser().parse(re.get("result").toString());
			JsonObject result_1 = result1.get("consensus_value").getAsJsonObject();
			String result = result_1.get(value).getAsString();
			return result;
		} catch (JsonSyntaxException e) {
			Log.error(response);
			return response;
		}
	}
	//获取result返回值中某个字段返回是数组的情况
	public static JsonArray getResultTh1(String response, String value) {
			JsonObject re = new JsonParser().parse(response).getAsJsonObject();
			JsonObject result = (JsonObject) new JsonParser().parse(re.get("result").toString());
			System.out.println("返回结果： " + result);
			//需要增加一个异常处理，如果返回结果不是数组，该如何处理
			JsonArray result_ = result.get(value).getAsJsonArray();
			return result_;
	}
	
	/**
	 * 获取资产排行返回的结果
	 * @param response
	 * @param value
	 * @return
	 */
	public static String getAssetRankTh(String response, String value) {
		try {
			JsonObject re = new JsonParser().parse(response).getAsJsonObject();
			JsonObject result = (JsonObject) new JsonParser().parse(re.get("result").toString());
			JsonArray result_ = result.get(value).getAsJsonArray();
//			System.out.println("addresss"+result_);
			return result_.toString();
		} catch (JsonSyntaxException e) {
			Log.error(response);
			return response;
		}
	}
	
	/**
	 * 获取资产排行查询返回的所有地址信息
	 * @param response
	 * @param value
	 * @return
	 */
	public static List<Address> getAssetRankAdds(String response){
		List<Address> addrs = new ArrayList<>();
		String result = getAssetRankTh(response, "addresss");
		JSONArray  jsonArray = JSONArray.fromObject(result);
		List<JSONObject> addresslist = (List)jsonArray;
		for (int i = 0; i < jsonArray.size(); i++) {
			Address add = new Address();
			JSONObject addressJson = addresslist.get(i);
//			System.out.println("addressJson: " + addressJson);
			String address = addressJson.get("address").toString();
			Long amount = Long.parseLong(addressJson.get("amount").toString());
			Integer number = Integer.parseInt(addressJson.get("number").toString());
			add.setAddress(address);
			add.setAmount(amount);
			add.setNumber(number);
			addrs.add(add);
		}
		return addrs;
	}
	
	public static int getNumberByAddress(Address addr){
		addr.setAddress(addr.getAddress());
		return addr.getNumber();
	}
	
	/**
	 * 获取资产排行返回结果个数
	 * @param response
	 * @return
	 */
	public static Integer getAssetRankResultSize(String response){
		APIUtil.wait(2);
		String result = getAssetRankTh(response, "addresss");
		return JSONArray.fromObject(result).size();
	}
	
//	public static Integer getNumber(Account acc){
//		
//	}
	/**
	 * 获取get返回结果中的result所有内容
	 * @param response
	 * @return
	 */
	public static String getResultTh(String response) {
			JsonObject re = new JsonParser().parse(response).getAsJsonObject();
			Object o = re.get("result");
			if(o instanceof JSONNull){
				System.out.println("Is empty null");
				return null;
			}else{
				System.out.println("is String null");
				return "null";
			}
	}

	/**
	 * 通过账户地址获取账户余额
	 * 
	 * @param address
	 * @return
	 */
	public static String getBalanceInAcc(Object address){
		String accinfo = Result.getAccInfo(address);
		String balance = "";
		if (accinfo.contains("balance") ) {
			balance = Result.getBalanceInResponse(accinfo);
		}else {
			ErrorHandler.continueRunning("address["+address+"]获取账户信息有误");
		}
		return balance;
	}	
	
	/**
	 * 获取账户信息中的余额
	 * 返回为string  如果是创始账号，余额太大，long都显示不对
	 * @param response
	 * @return
	 */
	public static String getBalanceInResponse(String response){
		String value = "balance";
		String balance =getResultTh(response, value);
		return balance;
	}
	/**
	 * 获取区块信息-获取ledger_version
	 * @param response
	 * @return
	 */
	public static String getLedgerVersion(String response){
		
		JsonObject item1 = new JsonParser().parse(response).getAsJsonObject();
		JsonObject result = (JsonObject) new JsonParser().parse(item1.get("result").toString());
		String ledger_version = result.get("ledger_version").getAsString();
		return ledger_version;
	}
	
	
	/**
	 * 获取供应链溯源中的address
	 * @param response
	 * @return
	 */
	public static String getSourceAdd(String response){
		
		JsonObject item1 = new JsonParser().parse(response).getAsJsonObject();
		
//		System.out.println("供应链溯源item1: " + item1);
		
		JsonObject result = (JsonObject) new JsonParser().parse(item1.get("result").toString());
		
//		System.out.println("供应链溯源result: " + result);
		
		String address = result.get("address").getAsString();
		return address;
	}
	
	/**
	 * 供应链溯源，通过hash查询
	 * 
	 * @param hash
	 * @return
	 */
	public static String getSources(String hash){
		APIUtil.wait(close_time);
		String result = Result.getResult("getSources","hash",hash);
		return result;
	}

	public static String getSources(String hash, String depth) {
		String re = getResult("getSources", "hash", hash, "depth", depth);
		return re;
	}

	public static String getResult(String transaction, String key1,
			String value1, String key2, String value2) {
		String result = HttpPool.doGet(transaction, key1, value1, key2, value2);
		int error_code = Result.getoutErrCodeFromGet(result);
		if (error_code != 0) {
			if (error_code == 4) {
				for (int i = 0; i < timeout; i++) {
					APIUtil.wait(1);
					String re = HttpPool.doGet(transaction, key1, value1, key2,
							value2);
					int err_code = Result.getoutErrCodeFromGet(re);
					if (err_code == 0) {
						return re;
					}
				}
			}
			return result;
		}
		return result;
	}
	//直接封装 
	public static String getResult(String transaction,String key,String value){
		String result = HttpPool.doGet(transaction, key, value);
		int error_code = Result.getoutErrCodeFromGet(result);
		if (error_code !=0) {
			if (error_code == 4) {
				for (int i = 0; i < timeout; i++) {
					APIUtil.wait(1);
					String re = HttpPool.doGet(transaction, key, value);
					int err_code = Result.getoutErrCodeFromGet(re);
					if (err_code == 0) {
						return re;
					}
				}
			}
			return result;
		}
		return result;
	}
	
	
	public static String getErrDescFromGetResult(String response){
		
		JsonObject data = new JsonParser().parse(response).getAsJsonObject();
		String err_desc = data.get("error_desc").getAsString();
		return err_desc;
		
	}
	
	public static int getErrorCode(String result){
		if (result.contains("results")) {
			int e1 = Result.getErrCodeFromPost(result);
			if (e1 == 0) {
				String hash = Result.getHash(result);
				APIUtil.wait(1);
				String re = Result.getTranHisByHash(hash);
				int e2 = Result.getoutErrCodeFromGet(re); // 交易记录外层的error_code
				if (e2 == 0) {
					int e3 = Result.getErcInTranHistory(re);
					return e3;
				} else if (e2 == 4) {
					for (int i = 0; i < timeout; i++) {
						APIUtil.wait(1);
						re = Result.getTranHisByHash(hash);
						e2 = Result.getoutErrCodeFromGet(re);
						if (e2 == 0) {
							int e4 = Result.getErcInTranHistory(re);
							return e4;
						}
					}
					Log.error("交易记录为空，error_code: " + e2 +" hash: " + hash);
					return e2;
				} else {
					return e2;
				}
			}
			return e1;
		} else {
			int e2 = Result.getoutErrCodeFromGet(result);
			return e2;
		}

	}
	
	public static int getErrorCode(String url,String result){
		if (result.contains("results")) {
			int e1 = Result.getErrCodeFromPost(result);
			if (e1 == 0) {
				String hash = Result.getHash(result);
				System.out.println("hash: " + hash);
				String re = Result.getTranHisByHash(url,hash);
				int e2 = Result.getoutErrCodeFromGet(re); // 交易记录外层的error_code
				if (e2 == 0) {
					int e3 = Result.getErcInTranHistory(re);
					return e3;
				} else if (e2 == 4) {
					for (int i = 0; i < timeout; i++) {
						APIUtil.wait(1);
						re = Result.getTranHisByHash(url,hash);
						e2 = Result.getoutErrCodeFromGet(re);
						if (e2 == 0) {
							int e4 = Result.getErcInTranHistory(re);
							return e4;
						}
					}
					Log.error("交易记录为空，error_code: " + e2 +" hash: " + hash);
					return e2;
				} else {
					return e2;
				}
			}
			return e1;
		} else {
			int e2 = Result.getoutErrCodeFromGet(result);
			return e2;
		}

	}
	
	/**
	 * 获取error_code test
	 * 如果是get请求，外层的error_code；如果是0，去查交易记录
	 * 如果是post请求，直接返回error_code
	 * @param result
	 * @return
	 */
	public static int getErrorCodeFromMutiQuery(String result) {
		// if (result.contains("results")) {
			int e1 = Result.getErrCodeFromPost(result);
			if (e1 == 0) {
			// String hash = Result.getHash(result);
			// String re = Result.getTranHisByHash(hash);
			// int e2 = Result.getoutErrCodeFromGet(re); // 交易记录外层的error_code
			// if (e2 == 0) {
			// int e3 = Result.getErcInTranHistory(re);
			// return e3;
			// } else if (e2 == 4) {
			// for (int i = 0; i < time; i++) {
			// APIUtil.wait(1);
			// re = Result.getTranHisByHash(hash);
			// e2 = Result.getoutErrCodeFromGet(re);
			// if (e2 == 0) {
			// int e4 = Result.getErcInTranHistory(re);
			// return e4;
			// }
			// }
			// return e2;
			// } else {
			// return e2;
			// }
			}
			return e1;
		// } else {
		// int e2 = Result.getoutErrCodeFromGet(result);
		// return e2;
		// }

	}
	
	/**
	 * post请求后返回的error_code
	 * @param result
	 * @return
	 */
	
	
	public static int getErrCodeFromPost(String result){
		int error_code = 9999;
		JsonObject data = new JsonParser().parse(result).getAsJsonObject();
		JsonArray resultjson = (JsonArray) new JsonParser().parse(data.get("results").toString());
		if(resultjson.size()>0){
			String re = resultjson.get(0).toString();
			JsonObject jsonObject = new JsonParser().parse(re).getAsJsonObject();
			error_code = jsonObject.get("error_code").getAsInt();
			return error_code;
		}else {
			Log.error("Get error_code failed from post-request");
			return error_code;
		}
	}
	/**
	 * 发送多个transaction，results会返回多个error_code，可以任意取其中一个
	 * @param result
	 * @param i
	 * @return
	 */
	public static int getErrCodeFromPost(String result,int i){
		int error_code = 9999;
		JsonObject data = new JsonParser().parse(result).getAsJsonObject();
		JsonArray resultjson = (JsonArray) new JsonParser().parse(data.get("results").toString());
		if(resultjson.size()>0){
			String re = resultjson.get(i).toString();
			JsonObject jsonObject = new JsonParser().parse(re).getAsJsonObject();
			error_code = jsonObject.get("error_code").getAsInt();
			return error_code;
		}else {
			System.out.println("获取错误码出错");
			return error_code;
		}
	}
	
	/**
	 * 获取blob返回中的error_code
	 * @param result
	 * @return
	 */
	public static int getErrCodeFromBlob(String result) {
		JsonObject data = new JsonParser().parse(result).getAsJsonObject();
		int err_code = data.get("error_code").getAsInt();
		return err_code;
	}
	
	/**
	 * 获取get请求外层的error_code
	 * {
		  "error_code": 4,
		  "result": null
		}
	 * @param result
	 * @return
	 */
	
	public static int getoutErrCodeFromGet(String result){
		String value = "error_code";
		int err_code = parseErrorCodeFromResult(result, value);
		return err_code;
	}
	/**
	 * getConsensusInfo 结果返回type
	 * @param result
	 * @return
	 */
	public static String getTypeFromConInfo(String result){
		String value = "type";
		String type = parseStringFromResult(result, value);
		return type;
	}
	
	public static int parseErrorCodeFromResult(String result,String value){
		try {
			JsonObject data = new JsonParser().parse(result).getAsJsonObject();
			int err_code = data.get(value).getAsInt();
			return err_code;
		} catch (JsonSyntaxException e) {
			Log.error(result);
		}
		return 9999;
	}
	
	public static String parseStringFromResult(String result,String value){
		JsonObject data = new JsonParser().parse(result).getAsJsonObject();
		String err_code = data.get(value).getAsString();
		return err_code;
	}
	
	/**
	 * 从hello结果中获取值
	 * @param result
	 * @param value
	 * @return
	 */
	public static String gethelloTh(String result,String value){
		JsonObject data = new JsonParser().parse(result).getAsJsonObject();
		String re = data.get(value).getAsString();
		return re;
	}
	
	/**
	 * 获取get请求内层的error_code
	 * {
  "error_code": 0,
  "result": {
    "address": "bubiV8i7Jp4ANkaugTRcvqRatYcpKRkqfVxQYAsS",
    "error_code": 0,
    "hash": "b34c3724fe25b09bccafc0b8a927a94f2e0c8090d141f4d6e318ed2c2d538d18"
  }
}
	 * @param result
	 * @return
	 */
	public static int getinErrCodeFromGet(String result){
		JsonObject data = new JsonParser().parse(result).getAsJsonObject();
		JsonObject re1 = (JsonObject) new JsonParser().parse(data.get("result").toString());
		int error_code = re1.get("error_code").getAsInt();
		return error_code;
	}
	
	/**
	 * 获取modules中值
	 * @param result
	 * @param key1
	 * @param key2
	 * @return
	 */
	public static String getModulesTh(String result, String key1, String key2) {
		JsonObject data = new JsonParser().parse(result).getAsJsonObject();
//		System.out.println("data: " + data);
		JsonObject re = (JsonObject) new JsonParser().parse(data.get(key1)
				.toString());
//		System.out.println("re: " + re);
		String result_ = re.get(key2).getAsString();
		return result_;
	}

	public static String getErrCodeFromGetAsString(String result){
		JsonObject data = new JsonParser().parse(result).getAsJsonObject();
		String err_code = data.get("error_code").getAsString();
		return err_code;
		
		
	}
	/**
	 * 通过record_address查询存证信息
	 * @param address
	 * @return
	 */
	public static String getStorageByAdd(String address){
		String transaction = "getRecord";
		String key = "record_address";
		String response = HttpPool.doGet(transaction, key, address);
		return response;
	}
	
	public static String getrecordParticipant(String address){
		String response = getStorageByAdd(address);
		JsonObject jsonObject = new JsonParser().parse(response).getAsJsonObject();
		String participant = jsonObject.get("result").getAsString();
		return participant;
	}
	
	
	/**
	 * 获取getTransactionHistory的errorcode
	 * 
	 * @param response
	 * @return
	 */
	public static int getErcInTranHistory(String response) {
		JsonObject item = new JsonParser().parse(response).getAsJsonObject();
		JsonObject result = (JsonObject) new JsonParser().parse(item.get(
				"result").toString());
		JsonArray transactions = (JsonArray) new JsonParser().parse(result.get(
				"transactions").toString());
		String tran = transactions.get(0).toString();
		JsonObject transac = new JsonParser().parse(tran).getAsJsonObject();
		int error_code = transac.get("error_code").getAsInt();
		return error_code;
	}

	/**
	 * 从交易记录中取transaction下面的值
	 * @param response
	 * @param value
	 * @return
	 */
	public static String getTranthInTranHistory(String response,String value){
		String result1 = null;
		JsonObject item = new JsonParser().parse(response).getAsJsonObject();
//		System.out.println("item==="+item);
		JsonObject result = (JsonObject) new JsonParser().parse(item.get("result").toString());
//		System.out.println(result);
		JsonArray transactions = (JsonArray) new JsonParser().parse(result.get("transactions").toString());
		if(transactions.size()>0){
			String tran = transactions.get(0).toString();
			// APIUtil.wait(2);
			JsonObject transac = new JsonParser().parse(tran).getAsJsonObject();
			result1 = transac.get(value).getAsString();
			
		}else{
			System.out.println("交易记录返回值为空");
		}
		return result1;
//		System.out.println("=error_code=="+error_code);
		
	}
	
	/**
	 * 获取历史记录中operation里的signers下的字段信息
	 * 
	 * @param response
	 * @param value
	 * @return
	 */
	public static String getOperSignerthInTranHistory(String response,String value){
		String dest_address = null;
		JsonObject item = new JsonParser().parse(response).getAsJsonObject();
		JsonObject result = (JsonObject) new JsonParser().parse(item.get(
				"result").toString());
		JsonArray transactions = (JsonArray) new JsonParser().parse(result.get(
				"transactions").toString());
		if (transactions.size() > 0) {
			String tran = transactions.get(0).toString();
			JsonObject transac = new JsonParser().parse(tran).getAsJsonObject();
			JsonArray opers = (JsonArray) new JsonParser().parse(transac.get(
					"operations").toString());
			String oper = opers.get(0).toString();
			JsonObject op = new JsonParser().parse(oper).getAsJsonObject();
			JsonArray signers = (JsonArray) new JsonParser().parse(op.get(
					"signers").toString());
			String signer = signers.get(0).toString();
			JsonObject address = new JsonParser().parse(signer)
					.getAsJsonObject();
			String add = address.get(value).getAsString();
			return add;

		} else {
			System.out.println("交易记录返回值为空");
		}
		return dest_address;
	}

	/**
	 * 获取账户信息中signers或者threshold里的字段信息
	 * 
	 * @param response
	 * @param value
	 * @return
	 */
	public static String getThInAccountInfo(String response, String key,
			String value) {
		String dest_address = null;
		JsonObject item = new JsonParser().parse(response).getAsJsonObject();
		// System.out.println("item==="+item);
		JsonObject result = (JsonObject) new JsonParser().parse(item.get(
				"result").toString());
		// System.out.println(result);
		JsonArray signers = (JsonArray) new JsonParser().parse(result.get(
key)
				.toString());
		if (signers.size() > 0) {
			String signer = signers.get(0).toString();
			JsonObject address = new JsonParser().parse(signer)
					.getAsJsonObject();
			String add = address.get(value).getAsString();
			return add;

		}
		return dest_address;
	}

	/**
	 * 获取交易记录operations里threhold里的值
	 * @param response
	 * @param value
	 * @return
	 */
	public static String getOperThresthInTranHistory(String response,String value){
		String dest_address = null;
		JsonObject item = new JsonParser().parse(response).getAsJsonObject();
		JsonObject result = (JsonObject) new JsonParser().parse(item.get(
				"result").toString());
		JsonArray transactions = (JsonArray) new JsonParser().parse(result.get(
				"transactions").toString());
		if (transactions.size() > 0) {
			String tran = transactions.get(0).toString();
			JsonObject transac = new JsonParser().parse(tran).getAsJsonObject();
			JsonArray opers = (JsonArray) new JsonParser().parse(transac.get(
					"operations").toString());
			String oper = opers.get(0).toString();
			JsonObject op = new JsonParser().parse(oper).getAsJsonObject();
			JsonObject thresholds = new JsonParser().parse(
					op.get("threshold").toString()).getAsJsonObject();
			String threshold = thresholds.get(value).getAsString();
			return threshold;

		} else {
			System.out.println("交易记录返回值为空");
		}
		return dest_address;
	}
	
	/**
	 * 抽象出获取交易记录operations里的数组里的值
	 * 
	 * @param response
	 * @param key
	 * @param value
	 * @return
	 */
	public static String getOperThInTranHistory(String response, String key,
			String value) {
		String dest_address = null;
		JsonObject item = new JsonParser().parse(response).getAsJsonObject();
		// System.out.println("item==="+item);
		JsonObject result = (JsonObject) new JsonParser().parse(item.get(
				"result").toString());
		// System.out.println(result);
		JsonArray transactions = (JsonArray) new JsonParser().parse(result.get(
				"transactions").toString());
		if (transactions.size() > 0) {
			String tran = transactions.get(0).toString();
			// APIUtil.wait(2);
			JsonObject transac = new JsonParser().parse(tran).getAsJsonObject();
			JsonArray opers = (JsonArray) new JsonParser().parse(transac.get(
					"operations").toString());
			String oper = opers.get(0).toString();
			// APIUtil.wait(2);
			JsonObject op = new JsonParser().parse(oper).getAsJsonObject();
			JsonArray signers = (JsonArray) new JsonParser().parse(op.get(key)
					.toString());
			String signer = signers.get(0).toString();
			JsonObject address = new JsonParser().parse(signer)
					.getAsJsonObject();
			String add = address.get(value).getAsString();
			return add;

		} else {
			System.out.println("交易记录返回值为空");
		}
		return dest_address;
	}

	/**
	 * 抽象出来的方法，从交易记录的operation里取值
	 * 
	 * @param response
	 * @return
	 */
	public static String getOperthInTranHistory(String response,String value){
		String dest_address = null;
		JsonObject item = new JsonParser().parse(response).getAsJsonObject();
//		System.out.println("item==="+item);
		JsonObject result = (JsonObject) new JsonParser().parse(item.get("result").toString());
//		System.out.println(result);
		JsonArray transactions = (JsonArray) new JsonParser().parse(result.get("transactions").toString());
		if(transactions.size()>0){
			String tran = transactions.get(0).toString();
			JsonObject transac = new JsonParser().parse(tran).getAsJsonObject();
			JsonArray opers = (JsonArray) new JsonParser().parse(transac.get("operations").toString());
			String oper = opers.get(0).toString();
			JsonObject op = new JsonParser().parse(oper).getAsJsonObject();
			dest_address = op.get(value).getAsString();
			if (dest_address == null) {
				return null;
			}
			dest_address = op.get(value).getAsString();
			
		}else{
			System.out.println("交易记录返回值为空");
		}
		return dest_address;
		
	}
	/**
	 * 获取交易记录transactions里面的ledger_seq
	 * @param response
	 * @return
	 */
	public static String getLed_seqInTranHistory(String response){
		String ledger_seq = "";
		JsonObject item = new JsonParser().parse(response).getAsJsonObject();
		JsonObject result = (JsonObject) new JsonParser().parse(item.get("result").toString());
		JsonArray transactions = (JsonArray) new JsonParser().parse(result.get("transactions").toString());
		if(transactions.size()>0){
			String tran = transactions.get(0).toString();
			APIUtil.wait(2);
			JsonObject transac = new JsonParser().parse(tran).getAsJsonObject();
			ledger_seq = transac.get("ledger_seq").getAsString();
			
		}else{
			System.out.println("交易记录返回值为空");
		}
		return ledger_seq;
	}
	/**
	 * 通过hash查询交易记录，得到返回信息
	 * @param hash
	 * @return
	 */
	public static String getTranHisByHash(String hash){
		String transaction = "getTransactionHistory";
		String key = "hash";
		String response = HttpUtil.doget(get_Url,transaction, key, hash);
		int errorcode = Result.getErrorCode(response);
		if (errorcode==0) {
			
		}
		return response;
	}
	
	public static String getTranHisByHash(String url,String hash){
		String transaction = "getTransactionHistory";
		String key = "hash";
		String response = HttpUtil.doget(url,transaction, key, hash);
		return response;
	}
	
	public static String getTranHisByAdd(Object address){
		String transaction = "getTransactionHistory";
		String key = "address";
		Object value = address;
		APIUtil.wait(2);
		String response = HttpPool.doGetDelay(transaction, key, value);
		return response;
	}
	
	/**
			 * {
		  "error_code": 4,
		  "result": {
		    "total_count": 0,
		    "transactions": []
		  }
		}
	 * @param value
	 * @return
	 */
	public static int getTotalCount(String result){
		JsonObject data = new JsonParser().parse(result).getAsJsonObject();
		JsonObject resultJsonObject = (JsonObject) new JsonParser().parse(data.get("result").toString());
		int totalCount = resultJsonObject.get("total_count").getAsInt();
		return totalCount;
	}
	
	public static int getErrCodeForAccinfo(Object address){
		String response = getAccInfo(address);
		JsonObject jsonObject = new JsonParser().parse(response).getAsJsonObject();
		int error_code = jsonObject.get("error_code").getAsInt();
		return error_code;
	}
	
	

}


