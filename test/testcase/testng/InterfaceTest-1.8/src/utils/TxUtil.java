package utils;

import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import cn.bubi.tools.acc.Sign;
import model.Account;
import model.Input;
import model.InputInfo;
import model.Operation;
import model.Output;
import model.Transfer;

import net.sf.json.JSONArray;
import net.sf.json.JSONObject;
import base.CheckPoint;
import base.ErrorHandler;
import base.Log;
import base.TestBase;

public class TxUtil extends TestBase {
	
	static String metadata = "1234";
	
	public static JSONObject tx(List items) {
		Map<String, Object> map = new LinkedHashMap<>();
		map.put("items", items);
		JSONObject jsonObject = JSONObject.fromObject(map);
		// System.out.println("tx======"+jsonObject + "\n");
		return jsonObject;
	}
	
	public static int getUint8(short s){
        return s & 0x00ff;
    }
	
	public static int getUint16(int i){
        return i & 0x0000ffff;
    }
	
	public static long getUint32(long l){
        return l & 0x00000000ffffffff;
    }

	public static JSONObject items(List operations, JSONObject tran,
			String pri, Object pub) {
		String blob1 = SignUtil.getUnSignBlobResult(tran);
		String blob = SignUtil.getTranBlobsString(blob1);
		String signdata;
		JSONObject jsonObject = null;
		try {
			signdata = Sign.priKeysign(blob1, pri);
			List signatures = TxUtil.signatures(signdata, pub);
			jsonObject = TxUtil.itemJsonObject(signatures, blob);
			return jsonObject;
		} catch (Exception e) {
			e.printStackTrace();
		}
		return jsonObject;
	}
	
	public static JSONObject items(String url,List operations, JSONObject tran,
			String pri, Object pub) {
		String blob1 = SignUtil.getUnSignBlobResult1(url,"getTransactionBlob",tran);
		String blob = SignUtil.getTranBlobsString(blob1);
		String signdata;
		JSONObject jsonObject = null;
		try {
			signdata = Sign.priKeysign(blob1, pri);
			List signatures = TxUtil.signatures(signdata, pub);
			jsonObject = TxUtil.itemJsonObject(signatures, blob);
			return jsonObject;
		} catch (Exception e) {
			e.printStackTrace();
		}
		return jsonObject;
	}

	public static JSONObject tx(JSONObject items) {
		List list = new ArrayList<>();
		list.add(items);
		Map<String, Object> map = new LinkedHashMap<>();
		map.put("items", list);
		JSONObject jsonObject = JSONObject.fromObject(map);
		return jsonObject;
	}

	public static JSONObject items_createAcc() {
		int type = 2;
		Object source_address = led_acc;
		String pri = led_pri;
		Object pub = led_pub;
		int asset_type = 1;
		String asset_code = "abcd";
		int asset_amount = 10;
		long sequence_number = Result.seq_num(source_address);
		List opers = TxUtil.operIssue(type, asset_type, source_address,
				asset_code, asset_amount);
		JSONObject tran = TxUtil.transaction(source_address, opers,
				sequence_number, fee);
		JSONObject items = TxUtil.items(opers, tran, pri, pub);
		return items;
	}

	/**
	 * 发起联合签名的请求，直接返回请求结果
	 * 
	 * @param items
	 * @return
	 */
	public static String tx_cosign(List items) {
		JSONObject tx = TxUtil.tx(items);
		String result = HttpPool.doPost("submitTransaction", tx);
		return result;
	}

	public static String txPost(List items) {
		JSONObject tx = TxUtil.tx(items);
		String result = HttpPool.doPost("submitTransaction", tx);
		return result;
	}

	public static String txPost(JSONObject items) {
		String result = HttpPool.doPost("submitTransaction", items);
		return result;
	}

	/**
	 * 批量操作的请求
	 * 
	 * @param url
	 * @param items
	 * @return
	 */
	public static String txPost(String url, JSONObject items) {
		String result = HttpUtil.dopost(url, items);
		return result;
	}

	/**
	 * 把json转成list
	 * 
	 * @param jsonObject
	 * @return
	 */
	public static List items(JSONObject jsonObject) {
		List operlist = new ArrayList<>();
		operlist.add(jsonObject);
		return operlist;
	}

	public static JSONObject signtx(List operations, JSONObject tran,
			String pri, Object pub) {
		JSONObject jsonObject = null;
		String blobresult = SignUtil.getUnSignBlobResult(tran);
		int err = Result.getoutErrCodeFromGet(blobresult);
		if (err == 0) {
			String blobString = SignUtil.getTranBlobsString(blobresult);
			String signdata;
			try {
				signdata = Sign.priKeysign(blobString, pri);
				List signatures = TxUtil.signatures(signdata, pub);
				jsonObject = TxUtil.itemJsonObject(signatures, blobString);
				System.out.println("jsonObject" + jsonObject);
				return jsonObject;
			} catch (Exception e) {
				e.printStackTrace();
			}
		} else {
			System.out.println("获取blob出错，error_code: " + err + " err_desc: "
					+ Result.getErrDescFromGetResult(blobresult));

		}
		return jsonObject;
	}

	public static JSONObject item(List operations, JSONObject tran, String pri,
			Object pub) {
		JSONObject list = null;
		String blobresult = SignUtil.getUnSignBlobResult(tran);
		int err = Result.getoutErrCodeFromGet(blobresult);
		if (err == 0) {
			String blobString = SignUtil.getTranBlobsString(blobresult);
			String signdata;
			try {
				signdata = Sign.priKeysign(blobString, pri);
				List signatures = TxUtil.signatures(signdata, pub);
				list = TxUtil.item(signatures, blobString);
				return list;
			} catch (Exception e) {
				e.printStackTrace();
			}
		} else {
			System.out.println("获取blob出错，error_code: " + err + " err_desc: "
					+ Result.getErrDescFromGetResult(blobresult));

		}
		return list;
	}

	/**
	 * 批量操作中的get请求
	 * 
	 * @param url
	 * @return
	 */
	public static JSONObject mutiGet(String url) {
		String method = "GET";
		Map<String, Object> map = new LinkedHashMap<String, Object>();
		map.put("url", url);
		map.put("method", method);
		JSONObject jsonObject = JSONObject.fromObject(map);
		return jsonObject;
	}

	/**
	 * 批量操作中的post请求
	 * 
	 * @param items
	 * @return
	 */
	public static JSONObject mutiPost(JSONObject items) {
		String url = "submitTransaction";
		String method = "POST";
		Map<String, Object> map = new LinkedHashMap<String, Object>();
		map.put("url", url);
		map.put("method", method);
		map.put("jsonData", items);
		JSONObject jsonObject = JSONObject.fromObject(map);
		return jsonObject;

	}

	/**
	 * 批量发请求，单个直接发的
	 * 
	 * @param opers
	 * @param tran
	 * @param pri
	 * @param pub
	 * @return
	 */
	public static String mutiPost(List opers, JSONObject tran, String pri,
			Object pub) {
		JSONObject tran1 = signtx(opers, tran, pri, pub);
		JSONObject mutiPost = mutiPost(tran1);
		JSONObject list = tx(mutiPost);
		System.out.println(list);
		String result = HttpUtil.dopost("mutliQuery", list);
		return result;
	}

	/**
	 * 本地签名
	 * 
	 * @param signatures
	 * @param tran_blob
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static List itemlist(List signatures, String tran_blob) {
		Map<String, Object> signmap = new LinkedHashMap<>();
		signmap.put("signatures", signatures);
		signmap.put("transaction_blob", tran_blob);
		JSONObject jsonObject = JSONObject.fromObject(signmap);
		List list = new ArrayList();
		list.add(jsonObject);
		return list;
	}

	public static JSONObject itemJSON(List signatures, String tran_blob) {
		Map<String, Object> signmap = new LinkedHashMap<>();
		signmap.put("signatures", signatures);
		signmap.put("transaction_blob", tran_blob);
		JSONObject jsonObject = JSONObject.fromObject(signmap);
		return jsonObject;
	}
	
	/*
	 * verify signatures is empty
	 */
	public static JSONObject itemsignatureonly(List signatures) {
		Map<String, Object> signmap = new LinkedHashMap<>();
		signmap.put("signatures", signatures);
		JSONObject jsonObject = JSONObject.fromObject(signmap);
		return jsonObject;
	}
	
	public static JSONObject itemTranBlobonly(String tran_blob) {
		Map<String, Object> signmap = new LinkedHashMap<>();
		signmap.put("transaction_blob", tran_blob);
		JSONObject jsonObject = JSONObject.fromObject(signmap);
		return jsonObject;
	}

	/**
	 * 提交签名后的blob
	 * 
	 * @param signatures
	 * @param tran_blob
	 * @return
	 */
	@SuppressWarnings({ "rawtypes" })
	public static JSONObject itemJsonObject(List signatures, String tran_blob) {
		List list = itemlist(signatures, tran_blob);
		Map<String, Object> map = new LinkedHashMap<>();
		map.put("items", list);
		JSONObject jsonObject = JSONObject.fromObject(map);
		return jsonObject;
	}
	
	public static String getPeerNodeAddress(){
			String url = "getPeerNodeAddress";
			String key = "token";
			String value = "bubiokqwer";
			String result = HttpPool.doGet(url, key, value);
			return result;
	}

	/*
	 * verify itemTranBlobonly
	 */
	public static JSONObject itemBlobonly(String tran_blob) {
		JSONObject list = itemTranBlobonly(tran_blob);
		return list;
	}
	/*
	 * verify signatures 
	 */
	public static JSONObject itemSignaturesonly(List signatures) {
		JSONObject list = itemsignatureonly(signatures);
		return list;
	}
	
	public static JSONObject item(List signatures, String tran_blob) {

		JSONObject list = itemJSON(signatures, tran_blob);
		return list;
	}

	/**
	 * 发送的items jsonobject
	 * 
	 * @param items
	 * @return
	 */
	public static JSONObject items(List items) {

		Map<String, Object> map = new LinkedHashMap<>();
		map.put("items", items);
		JSONObject jsonObject = JSONObject.fromObject(map);
		return jsonObject;
	}

	/**
	 * JSONObject muti_tx = TxUtil.items(muti_tran); String result =
	 * HttpPool.doPost("submitTransaction",muti_tx);
	 */
	public static String muti_txPost(List items) {
		JSONObject muti_tx = TxUtil.items(items);
		String result = HttpPool.doPost("submitTransaction", muti_tx);
		return result;
	}

	/**
	 * 本地签名，signatures数据模板
	 * 
	 * @param sign_data
	 * @param public_key
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static List signatures(String sign_data, Object public_key) {
		Map<String, Object> signmap = new LinkedHashMap<>();
		signmap.put("sign_data", sign_data);
		signmap.put("public_key", public_key);
		JSONObject jsonObject = JSONObject.fromObject(signmap);
		List list = new ArrayList();
		list.add(jsonObject);
		return list;
	}

	/**
	 * 两个用户联合签名
	 * 
	 * @param u1
	 * @param u2
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static List signatures(JSONObject u1, JSONObject u2) {
		List list = new ArrayList();
		list.add(u1);
		list.add(u2);
		return list;
	}

	/**
	 * 一个用户签名
	 * 
	 * @param u1
	 * @param u2
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static List signatures(JSONObject u1) {
		List list = new ArrayList();
		list.add(u1);
		return list;
	}

	/**
	 * 联合签名的signature
	 * 
	 * @param tran
	 * @param pri
	 * @param pub
	 * @return
	 */
	public static JSONObject signature(JSONObject tran, Object pri, Object pub) {
		String blobresult = SignUtil.getUnSignBlobResult(tran);
		String blobString = SignUtil.getTranBlobsString(blobresult);
		String signdata;
		JSONObject jsonObject = null;
		try {
			signdata = Sign.priKeysign(blobString, pri.toString());
			Map<String, Object> signmap = new LinkedHashMap<>();
			signmap.put("sign_data", signdata);
			signmap.put("public_key", pub);
			jsonObject = JSONObject.fromObject(signmap);
		} catch (Exception e) {
			e.printStackTrace();
		}
		return jsonObject;
	}

	/**
	 * items
	 * 
	 * @param private_keys
	 * @param transaction
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static List items(JSONArray private_keys, JSONObject transaction) {
		Map<String, Object> iteMap = new LinkedHashMap<String, Object>();
		iteMap.put("private_keys", private_keys);
		iteMap.put("transaction_json", transaction);
		List list = new ArrayList();
		JSONObject jsonObject = JSONObject.fromObject(iteMap);
		list.add(jsonObject);
		return list;
	}

	public JSONArray private_keys(Object key1, Object key2) {
		JSONArray priArray = new JSONArray();
		priArray.add(key1);
		priArray.add(key2);
		return priArray;
	}

	// /**
	// * transaction 默认sequence_number
	// * @param source_address
	// * @param fee
	// * @param operations
	// * @return
	// */
	// @SuppressWarnings({ "rawtypes" })
	// public static JSONObject transaction(Object source_address,Object
	// fee,List operations){
	// Map<String, Object> tranMap = new LinkedHashMap<String, Object>();
	// tranMap.put("source_address", source_address);
	// tranMap.put("fee", fee);
	// tranMap.put("operations", operations);
	// JSONObject jsonObject = JSONObject.fromObject(tranMap);
	// return jsonObject;
	// }

	public static String getBlob(JSONObject jsonObject) {
		String re = SignUtil.getUnSignBlobResult(jsonObject);
		String blob = SignUtil.getTranBlobsString(re);
		return blob;
	}

	/**
	 * transaction 需填写sequence_number
	 * 
	 * @param source_address
	 * @param fee
	 * @param sequence_nub
	 * @param operations
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public List transaction(Object source_address, Object fee,
			Object sequence_nub, List operations) {
		Map<String, Object> tranMap = new LinkedHashMap<String, Object>();
		tranMap.put("source_address", source_address);
		tranMap.put("fee", fee);
		tranMap.put("sequence_mub", sequence_nub);
		tranMap.put("operations", operations);
		List tranlist = new ArrayList<>();
		tranlist.add(tranMap);
		return tranlist;
	}

	/**
	 * transaction 需填写sequence_number
	 * 
	 * @param source_address
	 * @param fee
	 * @param sequence_nub
	 * @param operations
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public List transaction(Object source_address, Object fee,
			Object sequence_nub, JSONArray operations) {
		Map<String, Object> tranMap = new LinkedHashMap<String, Object>();
		tranMap.put("source_address", source_address);
		tranMap.put("fee", fee);
		tranMap.put("sequence_mub", sequence_nub);
		tranMap.put("operations", operations);
		List tranlist = new ArrayList<>();
		tranlist.add(tranMap);
		return tranlist;
	}

	/**
	 * 固定1000手续费
	 * 
	 * @param source_address
	 * @param operations
	 * @return
	 */
	@SuppressWarnings("rawtypes")
	public static JSONObject transaction(Object source_address, List operations) {
		int fee = 1000;
		Map<String, Object> tranMap = new LinkedHashMap<String, Object>();
		tranMap.put("source_address", source_address);
		tranMap.put("fee", fee);
		tranMap.put("operations", operations);
		JSONObject jsonObject = JSONObject.fromObject(tranMap);
		// System.out.println("transactioin======" + jsonObject);
		return jsonObject;
	}

	/**
	 * 获取transaction 的jsonobject对象
	 * 
	 * @param source_address
	 * @param operations
	 * @param seq
	 * @param fee
	 * @return
	 */
	public static JSONObject transaction(Object source_address,
			List operations, long seq, Object fee) {
		Map<String, Object> tranMap = new LinkedHashMap<String, Object>();
		tranMap.put("source_address", source_address);
		tranMap.put("fee", fee);
		tranMap.put("sequence_number", seq);
		tranMap.put("operations", operations);
		JSONObject jsonObject = JSONObject.fromObject(tranMap);
		return jsonObject;
	}

	/**
	 * 创建账户 operaions
	 * 
	 * @param type
	 * @param dest_address
	 * @param init_balance
	 * @param account_matadata
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static List operations(Object type, Object dest_address,
			Object init_balance, Object account_matadata) {
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
		operMap.put("type", type);
		operMap.put("dest_address", dest_address);
		operMap.put("init_balance", init_balance);
		operMap.put("account_matadata", account_matadata);
		JSONObject jsonObject = JSONObject.fromObject(operMap);
		List operlist = new ArrayList<>();
		operlist.add(jsonObject);
		return operlist;
	}

	/**
	 * 创建账户，init_balance
	 * 
	 * @param dest_address
	 * @param init_balance
	 * @param account_matadata
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static List operations(Object dest_address, Object init_balance,
			Object account_matadata) {
		// JSONArray operArray = null;
		int type = 0;
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
		operMap.put("type", type);
		operMap.put("dest_address", dest_address);
		operMap.put("init_balance", init_balance);
		operMap.put("account_matadata", account_matadata);
		JSONObject jsonObject = JSONObject.fromObject(operMap);
		List operlist = new ArrayList();
		operlist.add(jsonObject);
		// System.out.println("operation======"+jsonObject);
		return operlist;
	}

	/**
	 * 转账
	 * 
	 * @param dest_address
	 * @param account_matadata
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static List operations(Object dest_address, Object account_matadata) {
		// JSONArray operArray = null;
		int type = 0;
		int init_balance = 200000;
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
		operMap.put("type", type);
		operMap.put("dest_address", dest_address);
		operMap.put("init_balance", init_balance);
		operMap.put("account_metadata", account_matadata);
		JSONObject jsonObject = JSONObject.fromObject(operMap);
		List operlist = new ArrayList();
		operlist.add(jsonObject);
		// System.out.println("operation======"+jsonObject);
		return operlist;
	}

	/**
	 * 转账交易
	 * 
	 * @param dest_address
	 * @param asset_amount
	 * @param source_address
	 * @param s_key
	 * @return
	 */
	@SuppressWarnings("rawtypes")
	public static String tx_transfer(Object type, Object asset_type,
			Object dest_address, Object asset_amount, Object asset_issuer,
			Object asset_code, Object source_address, Object s_key) {
		JSONArray keys = new JSONArray();
		keys.add(s_key);
		List opers = TxUtil.opertransfer(type, asset_type, dest_address,
				asset_amount, asset_issuer, asset_code);
		JSONObject tran = TxUtil.transaction(source_address, opers);
		List items = TxUtil.items(keys, tran);
		JSONObject jsonObject = TxUtil.tx(items);
		// 发送请求
		String response = HttpUtil.dopost(jsonObject);
		return response;
	}

	/**
	 * 创建账户 tx_createAcc
	 * 
	 * @param type
	 * @param dest_address
	 * @param init_balance
	 * @param account_matadata
	 * @param source_address
	 * @param s_key
	 * @return
	 */
	@SuppressWarnings("rawtypes")
	public static String tx_createAcc(Object type, Object dest_address,	Object init_balance, Object account_matadata,Object source_address, Object s_key) {
		JSONArray keys = new JSONArray();
		keys.add(s_key);
		List opers = TxUtil.operations(type, dest_address, init_balance,
				account_matadata);
		JSONObject tran = TxUtil.transaction(source_address, opers);
		List items = TxUtil.items(keys, tran);
		JSONObject jsonObject = TxUtil.tx(items);
		// 发送请求
		String response = HttpUtil.dopost(jsonObject);
		return response;
	}
	
	public static String  tx_result(List operations,Object source_address, Object fee, Object sequence_number, Object metadata,String pri,Object pub){
		JSONObject tran = TxUtil.tran_json(source_address, fee, sequence_number, metadata, operations);
		String blobresult = SignUtil.getUnSignBlobResult1(baseUrl,"getTransactionBlob",tran);
		if (blobresult.contains("transaction_blob")) {
			String blobString = SignUtil.getTranBlobsString(blobresult);
			String signdata;
			String result = null;
			try {
				signdata = Sign.priKeysign(blobString, pri);
				List signatures = TxUtil.signatures(signdata, pub);
				JSONObject jsonObject = TxUtil.itemJsonObject(signatures, blobString);
				result = HttpUtil.dopost("submitTransaction",jsonObject);
				return result;
			} catch (Exception e) {
				e.printStackTrace();
			}
		}else {
			Log.error("get blob failed");
		}
		return blobresult;
	}
	
	public static String  tx_result(String url,List operations,Object source_address, Object fee, Object sequence_number, Object metadata,String pri,Object pub){
		JSONObject tran = TxUtil.tran_json(source_address, fee, sequence_number, metadata, operations);
		String blobresult = SignUtil.getUnSignBlobResult1(url,"getTransactionBlob",tran);
		if (blobresult.contains("transaction_blob")) {
			String blobString = SignUtil.getTranBlobsString(blobresult);
			String signdata;
			String result = null;
			try {
				signdata = Sign.priKeysign(blobString, pri);
				List signatures = TxUtil.signatures(signdata, pub);
				JSONObject jsonObject = TxUtil.itemJsonObject(signatures, blobString);
				result = HttpUtil.dopost(url,"submitTransaction",jsonObject);
				return result;
			} catch (Exception e) {
				e.printStackTrace();
			}
		}else {
			Log.error("get blob failed");
		}
		return blobresult;
	}

	/**
	 * 创建账户，需要设置门限
	 * 
	 * @param type
	 * @param dest_address
	 * @param init_balance
	 * @param account_matadata
	 * @param source_address
	 * @param s_key
	 * @param master_weight
	 * @param med_threshold
	 * @param low_threshold
	 * @param high_threshold
	 * @return
	 */
	@SuppressWarnings("rawtypes")
	public static String tx_createAcc(Object type, Object dest_address,
			Object init_balance, Object account_matadata,
			Object source_address, Object s_key, Object master_weight,
			Object med_threshold, Object low_threshold, Object high_threshold) {
		JSONArray keys = new JSONArray();
		keys.add(s_key);
		List opers = TxUtil.operations(type, dest_address, init_balance,
				account_matadata);
		JSONObject tran = TxUtil.transaction(source_address, opers);
		List items = TxUtil.items(keys, tran);
		JSONObject jsonObject = TxUtil.tx(items);
		// 发送请求
		String response = HttpUtil.dopost(jsonObject);
		return response;
	}

	/**
	 * 唯一资产发行,默认手续费1000
	 * 
	 * @param type
	 * @param asset_issuer
	 * @param asset_code
	 * @param asset_detailed
	 * @param s_address
	 * @param s_key
	 * @return
	 */
	@SuppressWarnings("rawtypes")
	public static String tx_uniIssue(Object type, Object asset_issuer,
			Object asset_code, Object asset_detailed, Object s_address,
			Object s_key) {
		JSONArray keys = new JSONArray();
		keys.add(s_key);
		List opers = TxUtil.operUniIssue(type, asset_issuer, asset_code,
				asset_detailed); // ledger发行未初始化资产
		JSONObject tran = TxUtil.transaction(s_address, opers);
		List items = TxUtil.items(keys, tran);
		JSONObject jsonObject = TxUtil.tx(items);
		// 发送请求
		String response = HttpUtil.dopost(jsonObject);
		return response;
	}

	/**
	 * 唯一资产发行,需要传fee
	 * 
	 * @param type
	 * @param asset_issuer
	 * @param asset_code
	 * @param asset_detailed
	 * @param fee
	 * @param s_address
	 * @param s_key
	 * @return
	 */
	// @SuppressWarnings("rawtypes")
	// public static String tx_uniIssue(Object type,Object asset_issuer,Object
	// asset_code,Object asset_detailed,Object fee,Object s_address,Object
	// s_key){
	// JSONArray keys = new JSONArray();
	// keys.add(s_key);
	// List opers = TxUtil.operUniIssue(type,asset_issuer, asset_code,
	// asset_detailed); //ledger发行未初始化资产
	// JSONObject tran = TxUtil.transaction(s_address,fee, opers);
	// List items = TxUtil.items(keys, tran);
	// JSONObject jsonObject = TxUtil.tx(items);
	// //发送请求
	// String response = HttpUtil.dopost(jsonObject);
	// return response;
	// }

	/**
	 * 转移唯一资产
	 * 
	 * @param type
	 *            默认是8
	 * @param dest_address
	 * @param asset_issuer
	 * @param asset_code
	 * @param s_address
	 * @param s_key
	 * @return
	 */
	@SuppressWarnings("rawtypes")
	public static String tx_initIssueTran(Object type, Object dest_address,
			Object asset_issuer, Object asset_code, Object s_address,
			Object s_key) {
		JSONArray keys = new JSONArray();
		keys.add(s_key);
		List opers = TxUtil.operUniIssueTransfer(type, dest_address,
				asset_issuer, asset_code); // ledger发行未初始化资产
		JSONObject tran = TxUtil.transaction(s_address, opers);
		List items = TxUtil.items(keys, tran);
		JSONObject jsonObject = TxUtil.tx(items);
		// 发送请求
		String response = HttpUtil.dopost(jsonObject);
		return response;
	}

	/**
	 * 转移唯一资产，需要设置fee
	 * 
	 * @param type
	 *            默认是8
	 * @param dest_address
	 * @param asset_issuer
	 * @param asset_code
	 * @param fee
	 * @param s_address
	 * @param s_key
	 * @return
	 */
	@SuppressWarnings("rawtypes")
	public static String tx_initIssueTran(Object type, Object dest_address,
			Object asset_issuer, Object asset_code, Object fee,
			Object s_address, Object s_key) {
		JSONArray keys = new JSONArray();
		keys.add(s_key);
		List opers = TxUtil.operUniIssueTransfer(type, dest_address,
				asset_issuer, asset_code); // ledger发行未初始化资产
		JSONObject tran = TxUtil.transaction(s_address, opers);
		List items = TxUtil.items(keys, tran);
		JSONObject jsonObject = TxUtil.tx(items);
		// 发送请求
		String response = HttpUtil.dopost(jsonObject);
		return response;
	}

	/**
	 * 存证
	 * 
	 * @param type
	 *            默认为9
	 * @param record_id
	 *            字符串长度范围在1~64
	 * @param record_ext
	 *            值必须是16进制字符串，长度为偶数
	 * @param record_address
	 *            原声明的账号地址,若填写该字段，就是追加声明，若不填写，就是原声明， 追加声明必须要有原声明存在
	 * @param s_address
	 * @param s_key
	 * @return
	 */
	@SuppressWarnings("rawtypes")
	public static String tx_Storage(Object type, Object record_id,
			Object record_ext, Object record_address, Object s_address,
			Object s_key) {
		JSONArray keys = new JSONArray();
		keys.add(s_key);
		List opers = TxUtil.operStorage(type, record_id, record_ext,
				record_address); // ledger发行未初始化资产
		JSONObject tran = TxUtil.transaction(s_address, opers);
		List items = TxUtil.items(keys, tran);
		JSONObject jsonObject = TxUtil.tx(items);
		// 发送请求
		String response = HttpUtil.dopost(jsonObject);
		return response;
	}

	/**
	 * 存证
	 * 
	 * @param type
	 *            默认为9
	 * @param record_id
	 *            字符串长度范围在1~64
	 * @param record_ext
	 *            值必须是16进制字符串，长度为偶数
	 * @param s_address
	 * @param s_key
	 * @return
	 */
	@SuppressWarnings("rawtypes")
	public static String tx_Storage(Object type, Object record_id,
			Object record_ext, Object s_address, Object s_key) {
		JSONArray keys = new JSONArray();
		keys.add(s_key);
		List opers = TxUtil.operStorage(type, record_id, record_ext); // ledger发行未初始化资产
		JSONObject tran = TxUtil.transaction(s_address, opers);
		List items = TxUtil.items(keys, tran);
		JSONObject jsonObject = TxUtil.tx(items);
		// 发送请求
		String response = HttpUtil.dopost(jsonObject);
		return response;
	}

	/**
	 * 供应链，input为空
	 * 
	 * @param type
	 *            默认为6
	 * @param s_add
	 *            源账户
	 * @param s_key
	 *            源账户
	 * @param dest_add
	 *            output
	 * @param metadata
	 *            output
	 * @return
	 */
	@SuppressWarnings("rawtypes")
	public static String tx_supplychain(Object type, Object s_add,
			Object s_key, Object dest_add, Object metadata) {
		JSONArray keys = new JSONArray();
		keys.add(s_key);
		List inputs = new ArrayList();
		List outputs = TxUtil.outputs(dest_add, metadata);
		List opers = TxUtil.operations(type, inputs, outputs);
		JSONObject tran = TxUtil.transaction(s_add, opers);
		List items = TxUtil.items(keys, tran);
		JSONObject jsonObject = TxUtil.tx(items);
		String response = HttpUtil.dopost(jsonObject);
		return response;
	}

	/**
	 * 创建账户交易--获取blob用
	 * 
	 * @param type
	 *            默认是0
	 * @param dest_address
	 * @param account_matadata
	 * @param source_address
	 * @param fee
	 * @param sequence_number
	 * @param metadata
	 * @return
	 */
	@SuppressWarnings("rawtypes")
	public static JSONObject tran_createAcc(Object type, Object dest_address,
			Object account_matadata, Object source_address, Object fee,
			Object sequence_number, Object metadata) {
		List opers = TxUtil.operations(type, dest_address, init_balance,
				account_matadata);
		JSONObject tran = TxUtil.transaction(source_address, opers);
		return tran;
	}

	/**
	 * 所有的tran_json 获取blob用
	 * 
	 * @param source_address
	 * @param fee
	 * @param sequence_number
	 * @param metadata
	 * @param operations
	 * @return
	 */
	public static JSONObject tran_json(Object source_address, Object fee,
			Object sequence_number, Object metadata, List operations) {
		Map<String, Object> tranMap = new LinkedHashMap<String, Object>();
		tranMap.put("source_address", source_address);
		tranMap.put("fee", fee);
		tranMap.put("sequence_number", sequence_number);
		tranMap.put("metadata", metadata);
		tranMap.put("operations", operations);
		JSONObject jsonObject = JSONObject.fromObject(tranMap);
		return jsonObject;
	}
	public static JSONObject tran_json(Object source_address, Object fee,
			Object metadata, List operations) {
		Object sequence_number = Result.seq_num(source_address);
		Map<String, Object> tranMap = new LinkedHashMap<String, Object>();
		tranMap.put("source_address", source_address);
		tranMap.put("fee", fee);
		tranMap.put("sequence_number", sequence_number);
		tranMap.put("metadata", metadata);
		tranMap.put("operations", operations);
		JSONObject jsonObject = JSONObject.fromObject(tranMap);
		return jsonObject;
	}
	
	public static JSONObject tran_json(Object source_address, Object fee,
			Object sequence_number, Object metadata, List operations,JSONObject timeRange) {
		Map<String, Object> tranMap = new LinkedHashMap<String, Object>();
		tranMap.put("source_address", source_address);
		tranMap.put("fee", fee);
		tranMap.put("sequence_number", sequence_number);
		tranMap.put("metadata", metadata);
		tranMap.put("time_range", timeRange);
		tranMap.put("operations", operations);
		JSONObject jsonObject = JSONObject.fromObject(tranMap);
		return jsonObject;
	}

	/**
	 * 创建账户，没有account_metadata
	 * 
	 * @param dest_address
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static List operCreateAccount(Object dest_address) {
		int type = 0;
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
		operMap.put("type", type);
		operMap.put("dest_address", dest_address);
		operMap.put("init_balance", init_balance);
		JSONObject jsonObject = JSONObject.fromObject(operMap);
		List list = new ArrayList<>();
		list.add(jsonObject);
		// System.out.println("operation======"+jsonObject);
		return list;
	}

	/**
	 * 创建账户-operCreateAccount
	 * 
	 * @param type
	 * @param dest_address
	 * @param init_balance
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static List operCreateAccount(Object type, Object dest_address,
			Object init_balance) {
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
		operMap.put("type", type);
		operMap.put("dest_address", dest_address);
		operMap.put("init_balance", init_balance);
		JSONObject jsonObject = JSONObject.fromObject(operMap);
		List operlist = new ArrayList<>();
		operlist.add(jsonObject);
		return operlist;
	}
	
	public static List operCreateAccount1(Object type, Object dest_address,
			Object init_balance,Object source_address) {
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
		operMap.put("type", type);
		operMap.put("dest_address", dest_address);
		operMap.put("init_balance", init_balance);
		operMap.put("source_address", source_address);
		JSONObject jsonObject = JSONObject.fromObject(operMap);
		List operlist = new ArrayList<>();
		operlist.add(jsonObject);
		return operlist;
	}
	
	public static List operCreateAccount1(Object type, 
			Object source_address) {
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
		operMap.put("type", type);
		operMap.put("source_address", source_address);
		JSONObject jsonObject = JSONObject.fromObject(operMap);
		List operlist = new ArrayList<>();
		operlist.add(jsonObject);
		return operlist;
	}

	/**
	 * 创建账户-operCreateAccount
	 * 
	 * @param type
	 * @param dest_address
	 * @param init_balance
	 * @param account_metadata
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static List operCreateAccount(Object type, Object dest_address,
			Object init_balance, Object account_metadata) {
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
		operMap.put("type", type);
		operMap.put("dest_address", dest_address);
		operMap.put("init_balance", init_balance);
		operMap.put("account_metadata", account_metadata);
		JSONObject jsonObject = JSONObject.fromObject(operMap);
		List operlist = new ArrayList<>();
		operlist.add(jsonObject);
		return operlist;
	}
	
	public static JSONObject operCreateAccountjson(Object type, Object dest_address,
			Object init_balance, Object account_metadata) {
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
		operMap.put("type", type);
		operMap.put("dest_address", dest_address);
		operMap.put("init_balance", init_balance);
		operMap.put("account_metadata", account_metadata);
		JSONObject jsonObject = JSONObject.fromObject(operMap);
		return jsonObject;
	}
	
	public static List operCreateAccount1(Object type, Object dest_address,
			Object init_balance, Object account_metadata,Object metadata) {
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
		operMap.put("type", type);
		operMap.put("dest_address", dest_address);
		operMap.put("init_balance", init_balance);
		operMap.put("metadata", metadata);
		operMap.put("account_metadata", account_metadata);
		JSONObject jsonObject = JSONObject.fromObject(operMap);
		List operlist = new ArrayList<>();
		operlist.add(jsonObject);
		return operlist;
	}

	/**
	 * 创建账户 operCreateAccount 有权重
	 * 
	 * @param type
	 * @param dest_address
	 * @param init_balance
	 * @param account_metadata
	 * @param threshold
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static List operCreateAccount(Object type, Object dest_address,
			Object init_balance, Object account_metadata, Object threshold) {
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
		operMap.put("type", type);
		operMap.put("dest_address", dest_address);
		operMap.put("init_balance", init_balance);
		operMap.put("account_metadata", account_metadata);
		operMap.put("threshold", threshold);
		JSONObject jsonObject = JSONObject.fromObject(operMap);
		List operlist = new ArrayList<>();
		operlist.add(jsonObject);
		return operlist;
	}

	/**
	 * 创建账户-operCreateAccount
	 * 
	 * @param type
	 * @param dest_address
	 * @param init_balance
	 * @param account_metadata
	 * @param threshold
	 * @param signers
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static List operCreateAccount(Object type, Object dest_address,
			Object init_balance, Object account_metadata, Object threshold,
			Object signers) {
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
		operMap.put("type", type);
		operMap.put("dest_address", dest_address);
		operMap.put("init_balance", init_balance);
		operMap.put("account_metadata", account_metadata);
		operMap.put("threshold", threshold);
		operMap.put("signers", signers);
		JSONObject jsonObject = JSONObject.fromObject(operMap);
		List operlist = new ArrayList<>();
		operlist.add(jsonObject);
		return operlist;
	}

	// 创建账户
	@SuppressWarnings("rawtypes")
	public synchronized static Map createAccount() {
		String account_metadata = "abcd";
		String metadata = "abcd";
		Object source_address = led_acc;
		Map<String, Object> acc = new LinkedHashMap<String, Object>();
		String pri = led_pri;
		String pub = led_pub;
		Map acc_gen = APIUtil.generateAcc();
		Object dest_add = acc_gen.get("address");
		Object private_key = acc_gen.get("private_key");
		Object public_key = acc_gen.get("public_key");
		List opers = TxUtil.operCreateAccount(0, dest_add, init_balance,account_metadata);
		long sequence_number = Result.seq_num(led_acc);
		String result = tx_result(opers, source_address, fee,sequence_number, metadata, pri, pub);//SignUtil.tx(opers, source_address, fee,sequence_number, metadata, pri, pub);
//		System.out.println(result);
		int err_code = Result.getErrorCode(result);
		if (err_code == 0) {
			acc.put("address", dest_add);
			acc.put("private_key", private_key);
			acc.put("public_key", public_key);
			return acc;
		} else {
			 ErrorHandler.continueRunning("账户创建异常,error_code: " + err_code);
			return acc;
		}
	}
	
	public static Account createNewAccount(){
		String metadata = "abcd";
		Account dest = APIUtil.generateAccount();
//		System.out.println("新账号地址： " + dest.getAddress());
//		System.out.println("新账号pri： " + dest.getPri_key());
//		System.out.println("新账号pub： " + dest.getPub_key());
		List opers = TxUtil.operCreateAccount(0, dest.getAddress(), init_balance);
		long sequence_number = Result.seq_num(led_acc);
		String result = tx_result(opers, led_acc, fee,sequence_number, metadata, led_pri, led_pub);
		int err_code = Result.getErrorCode(result);
		return dest;
	}
	
	public static void createAccount(Operation operation) {
		String account_metadata = "abcd";
		String metadata = "abcd";
		Object source_address = led_acc;
		Map<String, Object> acc = new LinkedHashMap<String, Object>();
		String pri = led_pri;
		String pub = led_pub;
		Map acc_gen = APIUtil.generateAcc();
		Object dest_add = acc_gen.get("address");
		Object private_key = acc_gen.get("private_key");
		Object public_key = acc_gen.get("public_key");
		List opers = TxUtil.operCreateAccount(0, dest_add, init_balance,account_metadata);
		long sequence_number = Result.seq_num(led_acc);
		String result = tx_result(opers, source_address, fee,sequence_number, metadata, pri, pub);//SignUtil.tx(opers, source_address, fee,sequence_number, metadata, pri, pub);
		System.out.println(result);
		
	}
	
	public static Map createAccount1() {
		String account_metadata = "abcd";
		String metadata = "abcd";
		Object source_address = led_acc;
		Map<String, Object> acc = new LinkedHashMap<String, Object>();
		String pri = led_pri;
		String pub = led_pub;
		Map acc_gen = APIUtil.generateAcc();
		Object dest_add = acc_gen.get("address");
		Object private_key = acc_gen.get("private_key");
		Object public_key = acc_gen.get("public_key");
		List opers = TxUtil.operCreateAccount(0, dest_add, init_balance,account_metadata);
		long sequence_number = Result.seq_num(led_acc);
		String result = tx_result(get_Url2,opers, source_address, fee,sequence_number, metadata, pri, pub);//SignUtil.tx(opers, source_address, fee,sequence_number, metadata, pri, pub);
//		System.out.println(result);
		int err_code = Result.getErrorCode(get_Url2,result);
		if (err_code == 0) {
			acc.put("address", dest_add);
			acc.put("private_key", private_key);
			acc.put("public_key", public_key);
			return acc;
		} else {
//			acc = createAccount();
			 ErrorHandler.continueRunning("账户创建异常,error_code: " + err_code);
			return acc;
		}

	}
	
	public static Map createAccount(Object init_balance){
		String account_metadata = "abcd";
		String metadata = "abcd";
		Object source_address = led_acc;
		Map<String, Object> acc = new LinkedHashMap<String, Object>();
		String pri = led_pri;
		String pub = led_pub;
		Map acc_gen = APIUtil.generateAcc();
		Object dest_add = acc_gen.get("address");
		Object private_key = acc_gen.get("private_key");
		Object public_key = acc_gen.get("public_key");
		List opers = TxUtil.operCreateAccount(0, dest_add, init_balance,account_metadata);
		long sequence_number = Result.seq_num(source_address);
		String result = tx_result(opers, source_address, fee,sequence_number, metadata, pri, pub);//SignUtil.tx(opers, source_address, fee,sequence_number, metadata, pri, pub);
		int err_code = Result.getErrorCode(result);
		if (err_code == 0) {
			acc.put("address", dest_add);
			acc.put("private_key", private_key);
			acc.put("public_key", public_key);
			return acc;
		} else {
//			acc = createAccount();
			 ErrorHandler.continueRunning("账户创建异常,error_code: " + err_code);
			return acc;
		}
	}

	/**
	 * 创建账户-带权重的方法
	 * 
	 * @return
	 */
	public static Map createAccount(Object master_weight, Object med_threshold,
			Object low_threshold, Object high_threshold) {
		int type = 0;
		String account_metadata = "abcd";
		String metadata = "abcd";
		Object source_address = led_acc;
		long sequence_number = Result.seq_num(source_address);
		Map<String, Object> acc = new LinkedHashMap<String, Object>();
		String pri = led_pri;
		String pub = led_pub;

		Map acc_gen = APIUtil.generateAcc();
		Object dest_add = acc_gen.get("address");
		Object private_key = acc_gen.get("private_key");
		Object public_key = acc_gen.get("public_key");
		JSONObject threshold = threshold(master_weight, med_threshold,
				low_threshold, high_threshold);
		List opers = TxUtil.operCreateAccount(type, dest_add, init_balance,
				account_metadata, threshold); // ledger发行未初始化资产
		String result = SignUtil.tx(opers, source_address, fee,
				sequence_number, metadata, pri, pub);
		int err_code = Result.getErrorCode(result);
		if (err_code == 0) {
			acc.put("address", dest_add);
			acc.put("private_key", private_key);
			acc.put("public_key", public_key);
			// System.out.println("==========生成的值");
			// System.out.println("address: : " + dest_add);
			// System.out.println("private_key: " + private_key);
			// System.out.println("public_key: " + public_key);
			return acc;
		} else {
			System.out.println("账户创建出错,error_code: " + err_code);
			return acc;
		}
		// return null;

	}

	/**
	 * 创建账户有权重，可设置一个联名账户
	 * 
	 * @param master_weight
	 * @param med_threshold
	 * @param low_threshold
	 * @param high_threshold
	 * @param address
	 * @param weight
	 * @return
	 */
	public static Map createAccount(Object master_weight, Object med_threshold,
			Object low_threshold, Object high_threshold, Object address,
			int weight) {
		int type = 0;
		String account_metadata = "abcd";
		String metadata = "abcd";
		Object source_address = led_acc;
		long sequence_number = Result.seq_num(source_address);
		Map<String, Object> acc = new LinkedHashMap<String, Object>();
		String pri = led_pri;
		String pub = led_pub;

		Map acc_gen = APIUtil.generateAcc();
		Object dest_add = acc_gen.get("address");
		Object private_key = acc_gen.get("private_key");
		Object public_key = acc_gen.get("public_key");
		JSONObject threshold = threshold(master_weight, med_threshold,
				low_threshold, high_threshold);
		List signers = TxUtil.signers(address, weight);
		List opers = TxUtil.operCreateAccount(type, dest_add, init_balance,
				account_metadata, threshold, signers); // ledger发行未初始化资产
		String result = SignUtil.tx(opers, source_address, fee,
				sequence_number, metadata, pri, pub);
		int err_code = Result.getErrorCode(result);
		if (err_code == 0) {
			acc.put("address", dest_add);
			acc.put("private_key", private_key);
			acc.put("public_key", public_key);
			// System.out.println("==========生成的值");
			// System.out.println("address: : " + dest_add);
			// System.out.println("private_key: " + private_key);
			// System.out.println("public_key: " + public_key);
			return acc;
		} else {
			System.out.println("账户创建出错,error_code: " + err_code);
			return acc;
		}
		// return null;

	}

	/**
	 * 创建账户带两个联合签名账户
	 * 
	 * @param master_weight
	 * @param med_threshold
	 * @param low_threshold
	 * @param high_threshold
	 * @param address1
	 * @param weight1
	 * @param address2
	 * @param weight2
	 * @return
	 */
	public static Map createAccount(Object master_weight, Object med_threshold,
			Object low_threshold, Object high_threshold, Object address1,
			int weight1, Object address2, int weight2) {
		int type = 0;
		String account_metadata = "abcd";
		String metadata = "abcd";
		Object source_address = led_acc;
		long sequence_number = Result.seq_num(source_address);
		Map<String, Object> acc = new LinkedHashMap<String, Object>();
		String pri = led_pri;
		String pub = led_pub;

		Map acc_gen = APIUtil.generateAcc();
		Object dest_add = acc_gen.get("address");
		Object private_key = acc_gen.get("private_key");
		Object public_key = acc_gen.get("public_key");
		JSONObject threshold = threshold(master_weight, med_threshold,
				low_threshold, high_threshold);
		List signers = TxUtil.signers(address1, weight1, address2, weight2);
		List opers = TxUtil.operCreateAccount(type, dest_add, init_balance,
				account_metadata, threshold, signers); // ledger发行未初始化资产
		String result = SignUtil.tx(opers, source_address, fee,
				sequence_number, metadata, pri, pub);
		int err_code = Result.getErrorCode(result);
		if (err_code == 0) {
			acc.put("address", dest_add);
			acc.put("private_key", private_key);
			acc.put("public_key", public_key);
			System.out.println("==========生成的值");
			System.out.println("address: : " + dest_add);
			System.out.println("private_key: " + private_key);
			System.out.println("public_key: " + public_key);
			return acc;
		} else {
			new CheckPoint().isFailed("账户创建出错,error_code: " + err_code);
		}
		return null;

	}

	/**
	 * 资产发行 operation
	 * 
	 * @param type
	 * @param asset_type
	 * @param asset_issuer
	 * @param asset_code
	 * @param asset_amount
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static List operations(Object type, Object asset_type,
			Object asset_issuer, Object asset_code, Object asset_amount) {
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
		operMap.put("type", type);
		operMap.put("asset_type", asset_type);
		operMap.put("asset_issuer", asset_issuer);
		operMap.put("asset_code", asset_code);
		operMap.put("asset_amount", asset_amount);
		JSONObject jsonObject = JSONObject.fromObject(operMap);
		List operlist = new ArrayList<>();
		operlist.add(jsonObject);
		// System.out.println("--------");
		// System.out.println(operlist);
		return operlist;
	}

	/**
	 * 发行唯一资产
	 * 
	 * @param type
	 *            默认是7
	 * @param asset_issuer
	 * @param asset_code
	 *            范围1-64
	 * @param asset_detailed
	 *            16进制小写，偶数
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static List operUniIssue(Object type, Object asset_issuer,
			Object asset_code, Object asset_detailed) {
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
		operMap.put("type", type);
		operMap.put("asset_issuer", asset_issuer);
		operMap.put("asset_code", asset_code);
		operMap.put("asset_detailed", asset_detailed);
		JSONObject jsonObject = JSONObject.fromObject(operMap);
		List operlist = new ArrayList<>();
		operlist.add(jsonObject);
		// operArray.add(operlist);
		return operlist;
	}
	
	public static List operUniIssue(Object type,Object asset_type, Object asset_issuer,
			Object asset_code, Object asset_detailed) {
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
		operMap.put("type", type);
		operMap.put("asset_issuer", asset_issuer);
		operMap.put("asset_type", asset_type);
		operMap.put("asset_code", asset_code);
		operMap.put("asset_detailed", asset_detailed);
		JSONObject jsonObject = JSONObject.fromObject(operMap);
		List operlist = new ArrayList<>();
		operlist.add(jsonObject);
		// operArray.add(operlist);
		return operlist;
	}

	/**
	 * 转移唯一资产
	 * 
	 * @param type
	 * @param dest_address
	 * @param asset_issuer
	 * @param asset_code
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static List operUniIssueTransfer(Object type, Object dest_address,
			Object asset_issuer, Object asset_code) {
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
		operMap.put("type", type);
		operMap.put("asset_issuer", asset_issuer);
		operMap.put("asset_code", asset_code);
		operMap.put("dest_address", dest_address);
		JSONObject jsonObject = JSONObject.fromObject(operMap);
		List operlist = new ArrayList<>();
		operlist.add(jsonObject);
		// operArray.add(operlist);
		return operlist;
	}
	
	public static List operUniIssueTransfer(Object type,Object asset_type, Object dest_address,
			Object asset_issuer, Object asset_code) {
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
		operMap.put("type", type);
		operMap.put("asset_issuer", asset_issuer);
		operMap.put("asset_type", asset_type);
		operMap.put("asset_code", asset_code);
		operMap.put("dest_address", dest_address);
		JSONObject jsonObject = JSONObject.fromObject(operMap);
		List operlist = new ArrayList<>();
		operlist.add(jsonObject);
		// operArray.add(operlist);
		return operlist;
	}

	/**
	 * 存证 operStorage 有record_address
	 * 
	 * @param type
	 *            默认是9
	 * @param record_id
	 *            1-64位
	 * @param record_ext
	 *            16进制小写
	 * @param record_address
	 *            可选，原声明的账号地址,若填写该字段，就是追加声明，若不填写，就是原声明，追加声明必须要有原声明存在
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static List operStorage(Object type, Object record_id,
			Object record_ext, Object record_address) {
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
		operMap.put("type", type);
		operMap.put("record_id", record_id);
		operMap.put("record_ext", record_ext);
		operMap.put("record_address", record_address);
		JSONObject jsonObject = JSONObject.fromObject(operMap);
		List operlist = new ArrayList<>();
		operlist.add(jsonObject);
		// operArray.add(operlist);
		return operlist;
	}

	/**
	 * 存证 operStorage 没有record_address
	 * 
	 * @param type
	 * @param record_id
	 * @param record_ext
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static List operStorage(Object type, Object record_id,
			Object record_ext) {
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
		operMap.put("type", type);
		operMap.put("record_id", record_id);
		operMap.put("record_ext", record_ext);
		JSONObject jsonObject = JSONObject.fromObject(operMap);
		List operlist = new ArrayList<>();
		operlist.add(jsonObject);
		// operArray.add(operlist);
		return operlist;
	}

	/**
	 * 发行资产operIssue 没有details
	 * 
	 * @param type
	 * @param asset_type
	 * @param asset_issuer
	 * @param asset_code
	 * @param asset_amount
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static List operIssue(Object type, Object asset_type,
			Object asset_issuer, Object asset_code, Object asset_amount) {
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
		operMap.put("type", type);
		operMap.put("asset_type", asset_type);
		operMap.put("asset_issuer", asset_issuer);
		operMap.put("asset_code", asset_code);
		operMap.put("asset_amount", asset_amount);
		JSONObject jsonObject = JSONObject.fromObject(operMap);
		List operlist = new ArrayList<>();
		operlist.add(jsonObject);
		return operlist;
	}
	
	public static void issue(Account srcAcc, String asset_code, Long asset_amount, String metadata){
		List opers = operIssue(2, 1, srcAcc.getAddress(), asset_code, asset_amount);
		String result = SignUtil.tx(opers, srcAcc.getAddress(), fee, metadata, srcAcc.getPri_key(), srcAcc.getPub_key());
//		System.out.println("发行资产后返回结果：");
//		System.out.println(result);
	}
	
	public static void transfer(Account srcAcc, List<Account> accounts, String asset_code, Long asset_amount){
		List opers = opertransfer(srcAcc, accounts, asset_amount, asset_code);
		String result = SignUtil.tx(opers, srcAcc.getAddress(), fee, "abcd", srcAcc.getPri_key(), srcAcc.getPub_key());
//		System.out.println("转账后返回结果：");
//		System.out.println(result);
	}
	
	public static void transfer(Account srcAcc, List<Transfer> tran_adds, String asset_code){
		List opers = opertransfer(srcAcc, tran_adds, asset_code);
		String result = SignUtil.tx(opers, srcAcc.getAddress(), fee, "abcd", srcAcc.getPri_key(), srcAcc.getPub_key());
//		System.out.println("转账后返回结果：");
//		System.out.println(result);
	}
	
	/**
	 * 获取资产排行，一个地址过滤
	 * @param asset_issuer
	 * @param asset_code
	 * @param fladd
	 * @param count
	 * @return
	 */
	public static String getAssetRank(Account asset_issuer, Object asset_code, Object fladd, Object count){
		APIUtil.wait(3);
		List<Object> filter_add = new ArrayList<>();
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
		if (asset_issuer!=null) {
			operMap.put("asset_issuer", asset_issuer.getAddress());
		}
		if (asset_code!=null) {
			operMap.put("asset_code", asset_code);
		}
		if (fladd !=null) {
				filter_add.add(fladd);
			operMap.put("filter_address", filter_add);
		}else if (fladd instanceof List) {
			operMap.put("filter_address", fladd);
		} 
		if (count !=null) {
			operMap.put("count", count);
		}
		JSONObject jsonObject = JSONObject.fromObject(operMap);
//		System.out.println("请求参数：");
//		System.out.println(jsonObject);
		String result = HttpPool.doPost("getAssetRank",jsonObject);
//		System.out.println("获取资产排行返回结果： ");
//		System.out.println(result);
		return result;
	}
	
	public static String getAssetRank(Account asset_issuer, Object asset_code, List<String> fladd, Object count){
		APIUtil.wait(3);
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
		if (asset_issuer!=null) {
			operMap.put("asset_issuer", asset_issuer.getAddress());
		}
		if (asset_code!=null) {
			operMap.put("asset_code", asset_code);
		}
		if (fladd!=null) {
			operMap.put("filter_address", fladd);
		}
		if (count !=null) {
			operMap.put("count", count);
		}
		JSONObject jsonObject = JSONObject.fromObject(operMap);
//		System.out.println("请求参数：");
//		System.out.println(jsonObject);
		String result = HttpPool.doPost("getAssetRank",jsonObject);
		APIUtil.wait(3);
//		System.out.println("获取资产排行返回结果： ");
//		System.out.println(result);
		return result;
	}
	
	public static List<Account> issue_transfer3(Account srcAcc, String asset_code, Long asset_amount, String metadata){
		TxUtil.issue(srcAcc, asset_code, asset_amount, metadata);
		Account a1 = TxUtil.createNewAccount();
		Account a2 = TxUtil.createNewAccount();
		Account a3 = TxUtil.createNewAccount();
		List<Transfer> transfers = new ArrayList<>();
		Transfer transfer = new Transfer();
		transfer.setAddress(a1);
		transfer.setAmount(10L);
		Transfer transfer1 = new Transfer();
		transfer1.setAddress(a2);
		transfer1.setAmount(30L);
		Transfer transfer2 = new Transfer();
		transfer2.setAddress(a3);
		transfer2.setAmount(40L);
		transfers.add(transfer);
		transfers.add(transfer1);
		transfers.add(transfer2);
		TxUtil.transfer(srcAcc, transfers, asset_code);
		List<Account> accounts = new ArrayList<>();
		accounts.add(0,a1);
		accounts.add(1,a2);
		accounts.add(2,a3);
		return accounts;
	}
	
	public static List<Account> issue_transfer(Account srcAcc, Account a1, Long amount1, Account a2, Long amount2, Account a3, Long amount3, String asset_code, Long asset_amount, String metadata){
		TxUtil.issue(srcAcc, asset_code, asset_amount, metadata);
		List<Transfer> transfers = new ArrayList<>();
		Transfer transfer = new Transfer();
		transfer.setAddress(a1);
		transfer.setAmount(amount1);
		Transfer transfer1 = new Transfer();
		transfer1.setAddress(a2);
		transfer1.setAmount(amount2);
		Transfer transfer2 = new Transfer();
		transfer2.setAddress(a3);
		transfer2.setAmount(amount3);
		transfers.add(transfer);
		transfers.add(transfer1);
		transfers.add(transfer2);
		APIUtil.wait(3);
		TxUtil.transfer(srcAcc, transfers, asset_code);
		List<Account> accounts = new ArrayList<>();
		accounts.add(0,a1);
		accounts.add(1,a2);
		accounts.add(2,a3);
		return accounts;
	}
	/**
	 * 校验asset_issuer参数的类型
	 * @param asset_issuer
	 * @param asset_code
	 * @param fladd
	 * @param count
	 * @return
	 */
	public static String getAssetRank(Object asset_issuer, Object asset_code, Object fladd, Object count){
		APIUtil.wait(3);
		List<Object> filter_add = new ArrayList<>();
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
		operMap.put("asset_issuer", asset_issuer);
		operMap.put("asset_code", asset_code);
		if (fladd !=null) {
			filter_add.add(fladd);
			operMap.put("filter_address", filter_add);
		}
		if (count !=null) {
			operMap.put("count", count);
		}
		JSONObject jsonObject = JSONObject.fromObject(operMap);
//		System.out.println("请求参数：");
//		System.out.println(jsonObject);
		String result = HttpPool.doPost("getAssetRank",jsonObject);
//		System.out.println("获取资产排行返回结果： ");
//		System.out.println(result);
		return result;
	}
	/**
	 * 发行并且转账给两个目标地址
	 * @param srcAcc
	 * @param asset_code
	 * @param asset_amount
	 * @param metadata
	 */
	public static List<String> issue_transfer(Account srcAcc, String asset_code, Long asset_amount, String metadata){
		issue(srcAcc, asset_code, asset_amount, metadata);
		Account dest_addr1 = TxUtil.createNewAccount();
		Account dest_addr2 = TxUtil.createNewAccount();
		List<Account> accounts = new ArrayList<>();
		accounts.add(dest_addr1);
		accounts.add(dest_addr2);
		transfer(srcAcc, accounts, asset_code, asset_amount);
		List<String> addrs = new ArrayList<>();
		addrs.add(0,dest_addr1.getAddress());
		addrs.add(1,dest_addr2.getAddress());
		return addrs;
	}
	
	public static List operIssue(Object type, Object source_add,Object asset_type,
			Object asset_issuer, Object asset_code, Object asset_amount) {
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
		operMap.put("type", type);
		operMap.put("source_address", source_add);
		operMap.put("asset_type", asset_type);
		operMap.put("asset_issuer", asset_issuer);
		operMap.put("asset_code", asset_code);
		operMap.put("asset_amount", asset_amount);
		JSONObject jsonObject = JSONObject.fromObject(operMap);
		List operlist = new ArrayList<>();
		operlist.add(jsonObject);
		return operlist;
	}
	
	public static List operIssue(Object type, Object asset_type,
			Object asset_issuer, Object asset_code, Object asset_amount,Object metadata,Object source_address) {
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
		operMap.put("type", type);
		operMap.put("asset_type", asset_type);
		operMap.put("asset_issuer", asset_issuer);
		operMap.put("asset_code", asset_code);
		operMap.put("metadata", metadata);
		operMap.put("source_address", source_address);
		operMap.put("asset_amount", asset_amount);
		JSONObject jsonObject = JSONObject.fromObject(operMap);
		List operlist = new ArrayList<>();
		operlist.add(jsonObject);
		return operlist;
	}

	/**
	 * 发行资产operIssue 有details
	 * 
	 * @param type
	 * @param asset_type
	 * @param asset_issuer
	 * @param asset_code
	 * @param asset_amount
	 * @param details
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static List operIssue(Object type, Object asset_type,
			Object asset_issuer, Object asset_code, Object asset_amount,
			List details) {
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
		operMap.put("type", type);
		operMap.put("asset_type", asset_type);
		operMap.put("asset_issuer", asset_issuer);
		operMap.put("asset_code", asset_code);
		operMap.put("asset_amount", asset_amount);
		operMap.put("details", details);
		JSONObject jsonObject = JSONObject.fromObject(operMap);
		List operlist = new ArrayList<>();
		operlist.add(jsonObject);
		return operlist;
	}

	/**
	 * 转账、初始化转账 operations 没有details
	 * 
	 * @param type
	 * @param dest_address
	 * @param asset_type
	 * @param asset_issuer
	 * @param asset_code
	 * @param asset_amount
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static List operations(Object type, Object dest_address,
			Object asset_type, Object asset_issuer, Object asset_code,
			Object asset_amount) {
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
		operMap.put("type", type);
		operMap.put("dest_address", dest_address);
		operMap.put("asset_type", asset_type);
		operMap.put("asset_issuer", asset_issuer);
		operMap.put("asset_code", asset_code);
		operMap.put("asset_amount", asset_amount);
		JSONObject jsonObject = JSONObject.fromObject(operMap);
		List operlist = new ArrayList<>();
		operlist.add(jsonObject);
		return operlist;
	}

	/**
	 * 转账-多个operation
	 * 
	 * @param dest_address
	 * @param asset_amount
	 * @return
	 */
	public static JSONObject opertran(Object dest_address, int asset_amount) {
		int type = 1;
		int asset_type = 0;
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
		operMap.put("type", type);
		operMap.put("dest_address", dest_address);
		operMap.put("asset_type", asset_type);
		operMap.put("asset_amount", asset_amount);
		JSONObject jsonObject = JSONObject.fromObject(operMap);
		return jsonObject;
	}

	/**
	 * 初始化转账-多个operation
	 * 
	 * @param dest_address
	 * @param asset_issuer
	 * @param asset_code
	 * @param asset_amount
	 * @return
	 */
	public static JSONObject operinittran(Object dest_address,
			Object asset_issuer, Object asset_code, int asset_amount) {
		int type = 1;
		int asset_type = 0;
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
		operMap.put("type", type);
		operMap.put("dest_address", dest_address);
		operMap.put("asset_type", asset_type);
		operMap.put("asset_issuer", asset_issuer);
		operMap.put("asset_code", asset_code);
		operMap.put("asset_amount", asset_amount);
		JSONObject jsonObject = JSONObject.fromObject(operMap);
		return jsonObject;
	}

	/**
	 * 转账 opertransfer 无details
	 * 
	 * @param type
	 * @param asset_type
	 * @param dest_address
	 * @param asset_amount
	 * @param asset_issuer
	 * @param asset_code
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static List opertransfer(Object type, Object asset_type,
			Object dest_address, Object asset_amount, Object asset_issuer,
			Object asset_code) {
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
		operMap.put("type", type);
		operMap.put("dest_address", dest_address);
		operMap.put("asset_type", asset_type);
		operMap.put("asset_amount", asset_amount);
		operMap.put("asset_issuer", asset_issuer);
		operMap.put("asset_code", asset_code);
		JSONObject jsonObject = JSONObject.fromObject(operMap);
		List operlist = new ArrayList<>();
		operlist.add(jsonObject);
		return operlist;
	}
	
	/**
	 * 转账操作有多个目标地址
	 * @param srcAcc
	 * @param dest_address
	 * @param asset_amount
	 * @param asset_code
	 * @return
	 */
	public static List opertransfer(Account srcAcc, List<Account> dest_address, Long asset_amount,
			Object asset_code) {
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
		List operlist = new ArrayList<>();
		for (Account account : dest_address) {
			operMap.put("type", 1); // 操作类型1为转账操作
			operMap.put("dest_address", account.getAddress());
			operMap.put("asset_type", 1); // 1为用户发行的资产
			operMap.put("asset_amount", APIUtil.nextLong(new java.util.Random(), asset_amount/2));
			operMap.put("asset_issuer", srcAcc.getAddress());
			operMap.put("asset_code", asset_code);
			JSONObject jsonObject = JSONObject.fromObject(operMap);
			operlist.add(jsonObject);
		}
		return operlist;
	}
	
	/**
	 * 转账给多个地址
	 * @param srcAcc
	 * @param tran_adds
	 * @param asset_code
	 * @return
	 */
	public static List opertransfer(Account srcAcc, List<Transfer> tran_adds, Object asset_code) {
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
		List operlist = new ArrayList<>();
		for (Transfer tran_add : tran_adds) {
			operMap.put("type", 1); // 操作类型1为转账操作
			operMap.put("dest_address", tran_add.getAddress());
			operMap.put("asset_type", 1); // 1为用户发行的资产
			operMap.put("asset_amount", tran_add.getAmount());
			operMap.put("asset_issuer", srcAcc.getAddress());
			operMap.put("asset_code", asset_code);
			JSONObject jsonObject = JSONObject.fromObject(operMap);
			operlist.add(jsonObject);
		}
		return operlist;
	}
	
	public static List opertransfer1(Object type, Object asset_type,
			Object dest_address, Object asset_amount, Object asset_issuer,
			Object asset_code,Object source_address) {
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
		operMap.put("type", type);
		operMap.put("dest_address", dest_address);
		operMap.put("asset_type", asset_type);
		operMap.put("asset_amount", asset_amount);
		operMap.put("asset_issuer", asset_issuer);
		operMap.put("asset_code", asset_code);
		operMap.put("source_address", source_address);
		JSONObject jsonObject = JSONObject.fromObject(operMap);
		List operlist = new ArrayList<>();
		operlist.add(jsonObject);
		return operlist;
	}
	
	
	
	public static List opertransfer(Object type, Object source_add,Object asset_type,
			Object dest_address, Object asset_amount, Object asset_issuer,
			Object asset_code) {
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
		operMap.put("type", type);
		operMap.put("source_address", source_add);
		operMap.put("dest_address", dest_address);
		operMap.put("asset_type", asset_type);
		operMap.put("asset_amount", asset_amount);
		operMap.put("asset_issuer", asset_issuer);
		operMap.put("asset_code", asset_code);
		JSONObject jsonObject = JSONObject.fromObject(operMap);
		List operlist = new ArrayList<>();
		operlist.add(jsonObject);
		return operlist;
	}

	/**
	 * 转账 opertransfer 转布币
	 * 
	 * @param type
	 * @param asset_type
	 * @param dest_address
	 * @param asset_amount
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static List opertransfer(Object type, Object asset_type,
			Account dest_address, Object asset_amount) {
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
		operMap.put("type", type);
		operMap.put("dest_address", dest_address.getAddress());
		operMap.put("asset_type", asset_type);
		operMap.put("asset_amount", asset_amount);
		JSONObject jsonObject = JSONObject.fromObject(operMap);
		List operlist = new ArrayList<>();
		operlist.add(jsonObject);
		return operlist;
	}

	/**
	 * 转账-opertransfer 有details
	 * 
	 * @param type
	 * @param asset_type
	 * @param dest_address
	 * @param asset_amount
	 * @param asset_issuer
	 * @param asset_code
	 * @param details
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static List opertransfer(Object type, Object asset_type,
			Object dest_address, Object asset_amount, Object asset_issuer,
			Object asset_code, List details) {
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
		operMap.put("type", type);
		operMap.put("dest_address", dest_address);
		operMap.put("asset_type", asset_type);
		operMap.put("asset_amount", asset_amount);
		operMap.put("asset_issuer", asset_issuer);
		operMap.put("asset_code", asset_code);
		operMap.put("details", details);
		JSONObject jsonObject = JSONObject.fromObject(operMap);
		List operlist = new ArrayList<>();
		operlist.add(jsonObject);
		return operlist;
	}

	/**
	 * 初始化转账 operInitTransfer 无details
	 * 
	 * @param type
	 * @param asset_type
	 * @param dest_address
	 * @param asset_amount
	 * @param asset_issuer
	 * @param asset_code
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static List operInitTransfer(Object type, Object asset_type,
			Object dest_address, Object asset_amount, Object asset_issuer,
			Object asset_code) {
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
		operMap.put("type", type);
		operMap.put("dest_address", dest_address);
		operMap.put("asset_type", asset_type);
		operMap.put("asset_amount", asset_amount);
		operMap.put("asset_issuer", asset_issuer);
		operMap.put("asset_code", asset_code);
		JSONObject jsonObject = JSONObject.fromObject(operMap);
		List operlist = new ArrayList<>();
		operlist.add(jsonObject);
		return operlist;
	}
	/**
	 * for init_transfer with no asset_issuer check
	 * @param type
	 * @param asset_type
	 * @param dest_address
	 * @param asset_amount
	 * @param asset_issuer
	 * @param asset_code
	 * @return
	 */
	public static List operInitTranNoissuer(Object type, Object asset_type,
			Object dest_address, Object asset_amount, Object asset_issuer,
			Object asset_code) {
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
		operMap.put("type", type);
		operMap.put("dest_address", dest_address);
		operMap.put("asset_type", asset_type);
		operMap.put("asset_amount", asset_amount);
		operMap.put("asset_code", asset_code);
		JSONObject jsonObject = JSONObject.fromObject(operMap);
		List operlist = new ArrayList<>();
		operlist.add(jsonObject);
		return operlist;
	}

	/**
	 * 初始化转账 operInitTransfer 有details
	 * 
	 * @param type
	 * @param asset_type
	 * @param dest_address
	 * @param asset_amount
	 * @param asset_issuer
	 * @param asset_code
	 * @param details
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static List operInitTransfer(Object type, Object asset_type,
			Object dest_address, Object asset_amount, Object asset_issuer,
			Object asset_code, List details) {
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
		operMap.put("type", type);
		operMap.put("dest_address", dest_address);
		operMap.put("asset_type", asset_type);
		operMap.put("asset_amount", asset_amount);
		operMap.put("asset_issuer", asset_issuer);
		operMap.put("asset_code", asset_code);
		operMap.put("details", details);
		JSONObject jsonObject = JSONObject.fromObject(operMap);
		List operlist = new ArrayList<>();
		operlist.add(jsonObject);
		return operlist;
	}

	/**
	 * 设置属性 operations
	 * 
	 * @param type
	 * @param threshold
	 * @param metadata_version
	 * @param metadata
	 * @param asset_code
	 * @param signers
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes", "null" })
	public JSONArray operations(Object type, List threshold,
			Object metadata_version, Object metadata, JSONArray signers) {
		JSONArray operArray = null;
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
		operMap.put("type", type);
		operMap.put("threshold", threshold);
		operMap.put("metadata_version", metadata_version);
		operMap.put("metadata", metadata);
		operMap.put("signers", signers);
		List operlist = new ArrayList<>();
		operlist.add(operMap);
		operArray.add(operlist);
		return operArray;
	}

	/**
	 * 设置属性 operSetOption
	 * 
	 * @param type
	 * @param threshold
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static List operSetOption(Object type, JSONObject threshold) {
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
		operMap.put("type", type);
		operMap.put("threshold", threshold);
		JSONObject jsonObject = JSONObject.fromObject(operMap);
		List operlist = new ArrayList<>();
		operlist.add(jsonObject);
		return operlist;
	}

	/**
	 * 设置属性operSetOption
	 * 
	 * @param type
	 * @param threshold
	 * @param signers
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static List operSetOption(Object type, JSONObject threshold,
			List signers) {
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
		operMap.put("type", type);
		operMap.put("threshold", threshold);
		operMap.put("signers", signers);
		JSONObject jsonObject = JSONObject.fromObject(operMap);
		List operlist = new ArrayList<>();
		operlist.add(jsonObject);
		return operlist;
	}

	/**
	 * 设置属性，只有signers
	 * 
	 * @param type
	 * @param signers
	 * @return
	 */
	public static List operSetOption(Object type, List signers) {
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
		operMap.put("type", type);
		operMap.put("signers", signers);
		JSONObject jsonObject = JSONObject.fromObject(operMap);
		List operlist = new ArrayList<>();
		operlist.add(jsonObject);
		return operlist;
	}

	/**
	 * 设置属性，多个operation
	 * 
	 * @param type
	 * @param threshold
	 * @return
	 */
	public static JSONObject operSetOptions(Object type, JSONObject threshold) {
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
		operMap.put("type", type);
		operMap.put("threshold", threshold);
		JSONObject jsonObject = JSONObject.fromObject(operMap);
		return jsonObject;
	}

	/**
	 * 供应链
	 * 
	 * @param type
	 * @param inputs
	 * @param outputs
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static List<Object> operSupplyChain(Object type, List inputs, List outputs) {
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
		operMap.put("type", type);
		operMap.put("inputs", inputs);
		operMap.put("outputs", outputs);
		List operlist = new ArrayList<>();
		operlist.add(operMap);
		return operlist;
	}
	//复杂的操作模型1，多个output
	public static List<Output> createOutput(Integer count){
		List<Account> accounts = new ArrayList<>();
		List<Output> outputs = new ArrayList<>();
		for (int i = 0; i < count; i++) {
			Account account = TxUtil.createNewAccount();
			accounts.add(account);
			Output output = new Output();
			output.setAddress(account.getAddress());
			output.setMetadata("1234");
			outputs.add(i, output);
		}
		return outputs;
	}
	
	/**
	 * 创建一级供应链
	 */
	public static InputInfo input(){
		InputInfo inputInfo = new InputInfo();
		int type = 6;
		Account firstlevel = TxUtil.createNewAccount();
		Account secondlevel = TxUtil.createNewAccount();
		List<Object> inputs = new ArrayList<>();
		List<Object> outputs = TxUtil.outputs(secondlevel.getAddress(), metadata);
		List<Object> opers = TxUtil.operSupplyChain(type,inputs,outputs);
		String response = SignUtil.tx(opers, firstlevel.getAddress(), fee, metadata, firstlevel.getPri_key(), firstlevel.getPub_key());
		String hash = Result.getHash(response);
//		System.out.println("一级：" + response);
		String result = Result.getTranHisByHash(hash);
		int err_code = Result.getErrorCode(result);
		if (err_code==0) {
			inputInfo.setHash(hash);
			inputInfo.setSecondLevel(secondlevel);
			inputInfo.setFirstLevel(firstlevel);
			return inputInfo;
		}else {
			for (int i = 0; i < timeout; i++) {
				APIUtil.wait(1);
				err_code = Result.getErrorCode(Result.getTranHisByHash(hash));
				if (err_code==0) {
					inputInfo.setHash(hash);
					inputInfo.setSecondLevel(secondlevel);
					inputInfo.setFirstLevel(firstlevel);
					return inputInfo;
				}
			}
			Log.error("一级供应链创建失败");
			return null;
		}
	}
	/**
	 * 创建二级供应链
	 * @param srcAcc
	 * @return
	 */
	public static InputInfo input2(){
		InputInfo inputInfo = input();
		String hash = inputInfo.getHash();
		Account secondLevel = inputInfo.getSecondLevel(); 	//	获取一级供应链的目标地址，二级
		Account firstLevel = inputInfo.getFirstLevel();		//获取一级供应链的原地址
		int type = 6;
		Account threeLevel = TxUtil.createNewAccount();		//二级供应链的目标地址
		List<Object> inputs = new ArrayList<>();
		Input input = new Input();
		input.setHash(hash);
		input.setIndex(0);
		inputs.add(input);
		List<Object> outputs = TxUtil.outputs(threeLevel.getAddress(), metadata);
		List<Object> opers = TxUtil.operSupplyChain(type,inputs,outputs);
		String response = SignUtil.tx(opers, secondLevel.getAddress(), fee, metadata, secondLevel.getPri_key(), secondLevel.getPub_key());
		String hash1 = Result.getHash(response);  //获取二级供应链操作返回的交易hash
		String result = Result.getTranHisByHash(hash1);
//		System.out.println("二级供应链提交返回： " + response);
		int err_code = Result.getErrorCode(result);
		if (err_code==0) {
			inputInfo.setHash(hash1);
			inputInfo.setFirstLevel(firstLevel);
			inputInfo.setSecondLevel(secondLevel);
			inputInfo.setThreeLevel(threeLevel);  //	保存三级目标地址
			return inputInfo;
		}else {
			for (int i = 0; i < timeout; i++) {
				APIUtil.wait(1);
				err_code = Result.getErrorCode(Result.getTranHisByHash(hash));
				if (err_code==0) {
					inputInfo.setHash(hash1);
					inputInfo.setFirstLevel(firstLevel);
					inputInfo.setSecondLevel(secondLevel);
					inputInfo.setThreeLevel(threeLevel); 
					return inputInfo;
				}
			}
			Log.error("二级供应链创建失败");
			return null;
		}
	}
	/**
	 * 创建三级供应链
	 * @return
	 */
	public static InputInfo input3(){
		InputInfo inputInfo = input2();
		String hash = inputInfo.getHash();
		Account firstLevel = inputInfo.getSrcAcc();
		Account secondLevel = inputInfo.getSecondLevel();
		Account threeLevel = inputInfo.getThreeLevel();
		int type = 6;
		Account fourLevel = TxUtil.createNewAccount();
		List<Object> inputs = new ArrayList<>();
		Input input = new Input();
		input.setHash(hash);
		input.setIndex(0);
		inputs.add(input);
		List<Object> outputs = TxUtil.outputs(fourLevel.getAddress(), metadata);
		List<Object> opers = TxUtil.operSupplyChain(type,inputs,outputs);
		String response = SignUtil.tx(opers, threeLevel.getAddress(), fee, metadata, threeLevel.getPri_key(), threeLevel.getPub_key());
		String hash1 = Result.getHash(response);  //获取三级供应链操作返回的交易hash
		String result = Result.getTranHisByHash(hash1);
//		System.out.println("三级供应链提交返回： " + response);
		int err_code = Result.getErrorCode(result);
		if (err_code==0) {
			inputInfo.setHash(hash1);
			inputInfo.setOutputAccount(fourLevel);
			inputInfo.setFirstLevel(firstLevel);
			inputInfo.setSecondLevel(secondLevel);
			inputInfo.setThreeLevel(threeLevel);
			return inputInfo;
		}else {
			for (int i = 0; i < timeout; i++) {
				APIUtil.wait(1);
				err_code = Result.getErrorCode(Result.getTranHisByHash(hash));
				if (err_code==0) {
					inputInfo.setHash(hash1);
					inputInfo.setOutputAccount(fourLevel);
					inputInfo.setFirstLevel(firstLevel);
					inputInfo.setSecondLevel(secondLevel);
					inputInfo.setThreeLevel(threeLevel);
					return inputInfo;
				}
			}
			Log.error("三级供应链创建失败");
			return null;
		}
	}
	
	/**
	 * 发行唯一资产
	 * @param account
	 * @param asset_code
	 * @param asset_detailed
	 * @return
	 */
	public static String issueUnique(Account account, String asset_code, String asset_detailed){
		List opers_ = TxUtil.operUniIssue(7, account.getAddress(), asset_code,
				asset_detailed);
		String result_ = SignUtil.tx(opers_, account.getAddress(), fee,
				metadata, account.getPri_key(), account.getPub_key());
		int error_code = Result.getErrorCode(result_);
		if (error_code==0) {
			return asset_code;
		}else{
			APIUtil.wait(1);
			System.out.println("失败");
			return asset_code;
		}
	}
	public static List operSupplyChain(Object type, List inputs, List outputs,Object source_address) {
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
//		JSONObject input_ = tx(inputs);
//		JSONObject output_ = tx(outputs);
		operMap.put("type", type);
		operMap.put("inputs", inputs);
		operMap.put("outputs", outputs);
		operMap.put("source_address", source_address);
		List operlist = new ArrayList<>();
		operlist.add(operMap);
		return operlist;
	}
	
	public static List operSupplyChain(Object type, JSONObject inputs, JSONObject outputs) {
		Map<String, Object> operMap = new LinkedHashMap<String, Object>();
		operMap.put("type", type);
		operMap.put("inputs", inputs);
		operMap.put("outputs", outputs);
		List operlist = new ArrayList<>();
		operlist.add(operMap);
		return operlist;
	}

	// details 发行资产、转账和初始化转账
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static List details(Object amount, Object start, Object length,
			Object ext) {
		Map<String, Object> detMap = new LinkedHashMap<String, Object>();
		detMap.put("amount", amount);
		detMap.put("start", start);
		detMap.put("length", length);
		detMap.put("ext", ext);
		JSONObject jsonObject = JSONObject.fromObject(detMap);
		List delist = new ArrayList<>();
		delist.add(jsonObject);
		return delist;
	}

	/**
	 * outputs
	 * 
	 * @param address
	 *            ,hash
	 * @param weight
	 *            ,metadata,index
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static List<Object> outputs(Object address, Object metadata) {
		JSONObject output = outputsjsonJsonObject(address, metadata);
		List<Object> list = new ArrayList<>();
		list.add(output);
		return list;
	}

	/**
	 * 供应链 outputs 返回JSONObject
	 * 
	 * @param address
	 * @param metadata
	 * @return
	 */
	public static JSONObject outputsjsonJsonObject(Object address,
			Object metadata) {
		Map<String, Object> map = new LinkedHashMap<String, Object>();
		map.put("address", address);
		map.put("metadata", metadata);
		JSONObject jsonObject = JSONObject.fromObject(map);
		return jsonObject;
	}

	/**
	 * 供应链输出 outputs
	 * 
	 * @param address1
	 * @param metadata1
	 * @param address2
	 * @param metadata2
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static List outputs(Object address1, Object metadata1,
			Object address2, Object metadata2) {
		JSONObject output1 = outputsjsonJsonObject(address1, metadata1);
		JSONObject output2 = outputsjsonJsonObject(address2, metadata2);
		List list = new ArrayList<>();
		list.add(output1);
		list.add(output2);
		return list;
	}

	/**
	 * inputs
	 * 
	 * @param address
	 * @param weight
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static List inputs(Object hash, Object index) {
		Map<String, Object> map = new LinkedHashMap<String, Object>();
		map.put("hash", hash);
		map.put("index", index);
		JSONObject jsonObject = JSONObject.fromObject(map);
		List list = new ArrayList<>();
		list.add(jsonObject);
		return list;
	}

	/**
	 * inputs with metadata
	 * 
	 * @param hash
	 * @param index
	 * @param metadata
	 * @return
	 */
	public static List inputs(Object hash, Object index, Object metadata) {
		Map<String, Object> map = new LinkedHashMap<String, Object>();
		map.put("hash", hash);
		map.put("index", index);
		map.put("metadata", metadata);
		JSONObject jsonObject = JSONObject.fromObject(map);
		List list = new ArrayList<>();
		list.add(jsonObject);
		return list;
	}

	/**
	 * 签名 signers 返回一个jsonobject
	 * 
	 * @param address
	 * @param weight
	 * @return
	 */
	public static JSONObject signer(Object address, Object weight) {
		Map<String, Object> map = new LinkedHashMap<String, Object>();
		map.put("address", address);
		map.put("weight", weight);
		JSONObject jsonObject = JSONObject.fromObject(map);
		return jsonObject;
	}

	/**
	 * 两个用户联合签名
	 * 
	 * @param address1
	 * @param weight1
	 * @param address2
	 * @param weight2
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static List signers(Object address1, Object weight1,
			Object address2, Object weight2) {
		JSONObject signer1 = signer(address1, weight1);
		JSONObject signer2 = signer(address2, weight2);
		List list = new ArrayList<>();
		list.add(signer1);
		list.add(signer2);
		return list;
	}

	/**
	 * 只有一个用户联合签名
	 * 
	 * @param address
	 * @param weight
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static List signers(Object address, Object weight) {
		JSONObject signer = signer(address, weight);
		List list = new ArrayList<>();
		list.add(signer);
		return list;
	}

	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static List operations(Object type, List inputs, List outputs) {
		Map<String, Object> map = new LinkedHashMap<String, Object>();
		map.put("type", type);
		map.put("inputs", inputs);
		map.put("outputs", outputs);
		JSONObject jsonObject = JSONObject.fromObject(map);
		List list = new ArrayList<>();
		list.add(jsonObject);
		return list;

	}

	/**
	 * threshold 创建账户、设置属性
	 * 
	 * @param master_weight
	 * @param med_threhold
	 * @param low_threshold
	 * @param hight_threshold
	 * @return
	 */
	public static JSONObject threshold(Object master_weight,
			Object med_threshold, Object low_threshold, Object high_threshold) {
		Map<String, Object> thrMap = new LinkedHashMap<String, Object>();
		thrMap.put("master_weight", master_weight);
		thrMap.put("med_threshold", med_threshold);
		thrMap.put("low_threshold", low_threshold);
		thrMap.put("high_threshold", high_threshold);
		JSONObject jsonObject = JSONObject.fromObject(thrMap);
		return jsonObject;
	}
	
	public static JSONObject timeRange(Object min_time,Object max_time){
		Map<String, Object> map = new LinkedHashMap<String, Object>();
		map.put("min_time", min_time);
		map.put("max_time", max_time);
		JSONObject jsonObject = JSONObject.fromObject(map);
		return jsonObject;
	}

	/**
	 * 权重设置，设置metadata_version和metadata
	 * 
	 * @param master_weight
	 * @param med_threshold
	 * @param low_threshold
	 * @param high_threshold
	 * @param metadata_version
	 * @param metadata
	 * @return
	 */
	public static JSONObject threshold(Object master_weight,
			Object med_threshold, Object low_threshold, Object high_threshold,
			Object metadata_version, Object metadata) {
		Map<String, Object> thrMap = new LinkedHashMap<String, Object>();
		thrMap.put("master_weight", master_weight);
		thrMap.put("med_threshold", med_threshold);
		thrMap.put("low_threshold", low_threshold);
		thrMap.put("high_threshold", high_threshold);
		thrMap.put("metadata_version", metadata_version);
		thrMap.put("metadata", metadata);

		JSONObject jsonObject = JSONObject.fromObject(thrMap);
		return jsonObject;
	}

	/**
	 * 权重设置，只设置metadata_version
	 * 
	 * @param master_weight
	 * @param med_threshold
	 * @param low_threshold
	 * @param high_threshold
	 * @param metadata_version
	 * @return
	 */
	public static JSONObject setMv(Object master_weight, Object med_threshold,
			Object low_threshold, Object high_threshold, Object metadata_version) {
		Map<String, Object> thrMap = new LinkedHashMap<String, Object>();
		thrMap.put("master_weight", master_weight);
		thrMap.put("med_threshold", med_threshold);
		thrMap.put("low_threshold", low_threshold);
		thrMap.put("high_threshold", high_threshold);
		thrMap.put("metadata_version", metadata_version);
		JSONObject jsonObject = JSONObject.fromObject(thrMap);
		return jsonObject;
	}

	/**
	 * 权重设置，只设置metadata
	 * 
	 * @param master_weight
	 * @param med_threshold
	 * @param low_threshold
	 * @param high_threshold
	 * @param metadata
	 * @return
	 */
	public static JSONObject setMd(Object master_weight, Object med_threshold,
			Object low_threshold, Object high_threshold, Object metadata) {
		Map<String, Object> thrMap = new LinkedHashMap<String, Object>();
		thrMap.put("master_weight", master_weight);
		thrMap.put("med_threshold", med_threshold);
		thrMap.put("low_threshold", low_threshold);
		thrMap.put("high_threshold", high_threshold);
		thrMap.put("metadata", metadata);

		JSONObject jsonObject = JSONObject.fromObject(thrMap);
		return jsonObject;
	}

}
