package utils;

import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import net.sf.json.JSONArray;
import net.sf.json.JSONObject;

public class JsonStruct {

	public static void createAccount(){
		
	}
	/**
	 * 拼创建账户的JsonString
	 * @param private_keys 创建账户私钥
	 * @param source_address  源账户
	 * @param dest_address  目标账户
	 * @return
	 */
	public static JSONObject itemsAll(String private_keys,String source_address, String dest_address,int init_banlance){
		
		Map<String, Object> itemMap = new LinkedHashMap<>();
		itemMap.put("items", items1(private_keys,source_address,dest_address,init_banlance));
		JSONObject jsonObject = JSONObject.fromObject(itemMap);
//		System.out.println(jsonObject);
//		return jsonObject;
//		System.out.println(jsonObject);
		return jsonObject;
	}
	
	@SuppressWarnings("rawtypes")
	public static JSONObject itemsAll(String private_keys,String source_address, List items){
		
		Map<String, Object> itemMap = new LinkedHashMap<>();
		itemMap.put("items", items);
		JSONObject jsonObject = JSONObject.fromObject(itemMap);
//		System.out.println(jsonObject);
//		return jsonObject;
//		System.out.println(jsonObject);
		return jsonObject;
	}
//	public static JSONObject items(){
//		String private_keys = "privbUVfgWsMNvFro7WDCci9NUhiR6eaLcZ3JDGBfRf8BgDULK9sx3p2";
//		Map<String, Object> items = new LinkedHashMap<String,Object>();
//		items.put("private_keys", private_keys(private_keys));
//		items.put("transaction_json", transaction_json());
//		JSONObject jsonObject = JSONObject.fromObject(items);
////		System.out.println(jsonObject);
//		return jsonObject;
//	}
	
	/**
	 * 创建账户-items-需要设置init_balance
	 * @param private_keys
	 * @param source_address
	 * @param dest_address
	 * @return
	 */
	public static List<JSONObject> items1(String private_keys,String source_address, String dest_address,Object init_balance){
//		if(private_keys){
//			
//		}
		Map<String, Object> items = new LinkedHashMap<String,Object>();
		items.put("private_keys", private_keys(private_keys));
		items.put("transaction_json", transaction_json(source_address,dest_address,init_balance));
		
		JSONObject jsonObject = JSONObject.fromObject(items);
		List<JSONObject> itemsList = new ArrayList<JSONObject>();
		itemsList.add(jsonObject);
		return itemsList;
	}
	/**
	 * 创建账户-items-默认init_balance
	 * @param private_keys
	 * @param source_address
	 * @param dest_address
	 * @return
	 */
	public static List<JSONObject> items1(String private_keys,String source_address, Object dest_address){
		
		Map<String, Object> items = new LinkedHashMap<String,Object>();
		items.put("private_keys", private_keys(private_keys));
		items.put("transaction_json", transaction_json(source_address,dest_address));
		
		JSONObject jsonObject = JSONObject.fromObject(items);
		List<JSONObject> itemsList = new ArrayList<JSONObject>();
		itemsList.add(jsonObject);
		return itemsList;
	}
	
	/**
	 * 资产发行-items-json
	 * @param private_keys
	 * @param source_address
	 * @param asset_issuer
	 * @param asset_code
	 * @param asset_amount
	 * @return
	 */
	public static List<JSONObject> items1(String private_keys,String source_address, String asset_issuer,String asset_code,int asset_amount){
		
		Map<String, Object> items = new LinkedHashMap<String,Object>();
		items.put("private_keys", private_keys(private_keys));
		items.put("transaction_json", transaction_json(source_address,asset_issuer,asset_code,asset_amount));
		
		JSONObject jsonObject = JSONObject.fromObject(items);
		List<JSONObject> itemsList = new ArrayList<JSONObject>();
		itemsList.add(jsonObject);
		return itemsList;
	}
	/**
	 * 转账-items
	 * @param private_keys
	 * @param source_address
	 * @param asset_issuer
	 * @param asset_code
	 * @param asset_amount
	 * @return
	 */
	public static List<JSONObject> items1(String private_keys,String source_address, String dest_address,String asset_issuer,String asset_code,int asset_amount,int asset_type){
		
		Map<String, Object> items = new LinkedHashMap<String,Object>();
		items.put("private_keys", private_keys(private_keys));
		items.put("transaction_json", transaction_json(source_address,dest_address,asset_issuer,asset_code,asset_amount,asset_type));
		
		JSONObject jsonObject = JSONObject.fromObject(items);
		List<JSONObject> itemsList = new ArrayList<JSONObject>();
		itemsList.add(jsonObject);
		return itemsList;
	}
	
	/**
	 * 初始化转账-items1
	 * @param private_keys
	 * @param source_address
	 * @param dest_address
	 * @param asset_issuer
	 * @param asset_code
	 * @param asset_amount
	 * @return
	 */
	public static List<JSONObject> items1(String private_keys,String source_address, String dest_address,String asset_issuer,String asset_code,int asset_amount){
		Map<String, Object> items = new LinkedHashMap<String,Object>();
		items.put("private_keys", private_keys(private_keys));
		items.put("transaction_json", transaction_json(source_address,dest_address,asset_issuer,asset_code,asset_amount));
		
		JSONObject jsonObject = JSONObject.fromObject(items);
		List<JSONObject> itemsList = new ArrayList<JSONObject>();
		itemsList.add(jsonObject);
		return itemsList;
	}
	
	/**
	 * 设置属性items1
	 * @param private_keys
	 * @param source_address
	 * @param master_weight
	 * @param low_threshold
	 * @param med_threshold
	 * @param high_threshold
	 * @return
	 */
	public static List<JSONObject> items1(String private_keys,String source_address, int master_weight,int low_threshold,int med_threshold,int high_threshold){
		Map<String, Object> items = new LinkedHashMap<String,Object>();
		items.put("private_keys", private_keys(private_keys));
		items.put("transaction_json", transaction_json(source_address,master_weight, low_threshold, med_threshold, high_threshold));
		JSONObject jsonObject = JSONObject.fromObject(items);
		List<JSONObject> itemsList = new ArrayList<JSONObject>();
		itemsList.add(jsonObject);
		return itemsList;
	}
	/**
	 * 供应链-items1，有inputs
	 * @param private_keys
	 * @param source_address
	 * @param inputs
	 * @param outputs
	 * @return
	 */
	public static List<JSONObject> items1(String private_keys,String source_address,JSONArray inputs,JSONArray outputs ){
		Map<String, Object> items = new LinkedHashMap<String,Object>();
		items.put("private_keys", private_keys(private_keys));
		items.put("transaction_json", transaction_json(source_address,inputs, outputs));
		JSONObject jsonObject = JSONObject.fromObject(items);
		List<JSONObject> itemsList = new ArrayList<JSONObject>();
		itemsList.add(jsonObject);
		return itemsList;
	}
	/**
	 * 供应链-items1-没有inputs
	 * @param private_keys
	 * @param source_address
	 * @param outputs
	 * @return
	 */
	public static List<JSONObject> items1(String private_keys,String source_address,JSONArray outputs ){
		Map<String, Object> items = new LinkedHashMap<String,Object>();
		items.put("private_keys", private_keys(private_keys));
		items.put("transaction_json", transaction_json(source_address, outputs));
		JSONObject jsonObject = JSONObject.fromObject(items);
		List<JSONObject> itemsList = new ArrayList<JSONObject>();
		itemsList.add(jsonObject);
		return itemsList;
	}
	public static JSONArray private_keys(String key1){
		List<String> private_keyList = new ArrayList<String>();
		private_keyList.add(key1);
		JSONArray keysArray = JSONArray.fromObject(private_keyList);
		return keysArray;
	}
	/**
	 * 创建账户-transaction
	 * @param source_address
	 * @param dest_address
	 * @return
	 */
	public static JSONObject transaction_json(String source_address , Object dest_address,Object init_balance){
		
		int fee = 1000;
		Map<String, Object> transaction = new LinkedHashMap<String, Object>();
		transaction.put("source_address", source_address);
		transaction.put("fee", fee);
		transaction.put("operations", operations(dest_address,init_balance));
		JSONObject jsonObject = JSONObject.fromObject(transaction);
		return jsonObject;
//		System.out.println(jsonObject);
	}
	/**
	 * 创建账户-transaction-使用默认的init_balance
	 * @param source_address
	 * @param dest_address
	 * @return
	 */
	public static JSONObject transaction_json(String source_address , Object dest_address){
		
		int fee = 1000;
		Map<String, Object> transaction = new LinkedHashMap<String, Object>();
		transaction.put("source_address", source_address);
		transaction.put("fee", fee);
		transaction.put("operations", operations(dest_address,null));
		JSONObject jsonObject = JSONObject.fromObject(transaction);
		return jsonObject;
//		System.out.println(jsonObject);
	}
	/**
	 * 发行资产-transaction
	 * @param source_address  源账户
	 * @param asset_issuer  发行者
	 * @param asset_code  发行代码
	 * @param asset_amount  发行资产数量
	 * @return
	 */
	public static JSONObject transaction_json(String source_address , String asset_issuer,String asset_code,int asset_amount){
		
		int fee = 1000;
		Map<String, Object> transaction = new LinkedHashMap<String, Object>();
		transaction.put("source_address", source_address);
		transaction.put("fee", fee);
		transaction.put("operations", operations(asset_issuer,asset_code,asset_amount));
		JSONObject jsonObject = JSONObject.fromObject(transaction);
		return jsonObject;
//		System.out.println(jsonObject);
	}
	/**
	 * 转账交易-transaction-json
	 * @param source_address
	 * @param dest_address
	 * @param asset_issuer
	 * @param asset_code
	 * @param asset_amount
	 * @param asset_type
	 * @return
	 */
	public static JSONObject transaction_json(String source_address , String dest_address,String asset_issuer,String asset_code,int asset_amount,int asset_type){
		
		int fee = 1000;
		Map<String, Object> transaction = new LinkedHashMap<String, Object>();
		transaction.put("source_address", source_address);
		transaction.put("fee", fee);
		transaction.put("operations", operations(dest_address,asset_issuer,asset_code,asset_amount,asset_type));
		JSONObject jsonObject = JSONObject.fromObject(transaction);
		return jsonObject;
//		System.out.println(jsonObject);
	}
	/**
	 * 初始化转账-transaction
	 * @param source_address
	 * @param dest_address
	 * @param asset_issuer
	 * @param asset_code
	 * @param asset_amount
	 * @return
	 */
	public static JSONObject transaction_json(String source_address , String dest_address,String asset_issuer,String asset_code,int asset_amount){
		int fee = 1000;
		Map<String, Object> transaction = new LinkedHashMap<String, Object>();
		transaction.put("source_address", source_address);
		transaction.put("fee", fee);
		transaction.put("operations", operations(dest_address,asset_issuer,asset_code,asset_amount));
		JSONObject jsonObject = JSONObject.fromObject(transaction);
		return jsonObject;
//		System.out.println(jsonObject);
	}
	
	/**
	 * 供应链-transaction
	 * @param source_address
	 * @param inputs
	 * @param outputs
	 * @return
	 */
	public static JSONObject transaction_json(String source_address ,JSONArray inputs,JSONArray outputs){
		int fee = 1000;
		Map<String, Object> transaction = new LinkedHashMap<String, Object>();
		transaction.put("source_address", source_address);
		transaction.put("fee", fee);
		transaction.put("operations", operations(inputs,outputs));
		JSONObject jsonObject = JSONObject.fromObject(transaction);
		return jsonObject;
	}
	/**
	 * 供应链-transaction-没有inputs
	 * @param source_address
	 * @param outputs
	 * @return
	 */
	public static JSONObject transaction_json(String source_address ,JSONArray outputs){
		int fee = 1000;
		Map<String, Object> transaction = new LinkedHashMap<String, Object>();
		transaction.put("source_address", source_address);
		transaction.put("fee", fee);
		transaction.put("operations", operations(outputs));
		JSONObject jsonObject = JSONObject.fromObject(transaction);
		return jsonObject;
	}
	/**
	 * 创建账户-transaction
	 * @param source_address
	 * @return
	 */
	public static JSONObject transaction_json(String source_address){
		int fee = 1000;
		Map<String, Object> transaction = new LinkedHashMap<String, Object>();
		transaction.put("source_address", source_address);
		transaction.put("fee", fee);
		transaction.put("operations", operations(source_address));
		JSONObject jsonObject = JSONObject.fromObject(transaction);
		return jsonObject;
	}
	
	/**
	 * 设置属性 transaction
	 * @param source_address
	 * @param master_weight
	 * @param low_threshold
	 * @param med_threshold
	 * @param high_threshold
	 * @return
	 */
	public static JSONObject transaction_json(String source_address , int master_weight,int low_threshold,int med_threshold,int high_threshold ){
		
		int fee = 1000;
		Map<String, Object> transaction = new LinkedHashMap<String, Object>();
		transaction.put("source_address", source_address);
		transaction.put("fee", fee);
		transaction.put("operations", operations(master_weight, low_threshold, med_threshold, high_threshold));
		JSONObject jsonObject = JSONObject.fromObject(transaction);
		return jsonObject;
//		System.out.println(jsonObject);
	}
	
	/**
	 * 发行资产-operation-json
	 * @param asset_issuer
	 * @param asset_code
	 * @param asset_amount
	 * @return
	 */
	@SuppressWarnings({ "unused", "unchecked", "rawtypes" })
	public static JSONArray operations(String asset_issuer,String asset_code,int asset_amount){
		int type = 2;
		int asset_type = 1;
		Map<String, Object> operation = new LinkedHashMap<String, Object>();
		operation.put("asset_issuer", asset_issuer);
		operation.put("asset_code", asset_code);
		operation.put("asset_amount", asset_amount);
		operation.put("type", type);
		List operList = new ArrayList<>();
		operList.add(operation);
		JSONArray operListjson = JSONArray.fromObject(operList);
		return operListjson;
	}
	/**
	 * 转账交易-operation-json
	 * @param dest_address
	 * @param asset_issuer
	 * @param asset_code
	 * @param asset_amount
	 * @param asset_type
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static JSONArray operations(String dest_address,String asset_issuer,String asset_code,int asset_amount,int asset_type){
		int type = 1;
		JSONArray operListjson = null ;
		if(asset_type==0){  //0代表布币
			Map<String, Object> operation = new LinkedHashMap<String, Object>();
			operation.put("asset_amount", asset_amount);
			operation.put("type", type);
			operation.put("dest_address", dest_address);
			operation.put("asset_type", asset_type);
			List operList = new ArrayList<>();
			operList.add(operation);
			operListjson = JSONArray.fromObject(operList);
			
		}else if (asset_type==1) {  //1代表发行资产
			Map<String, Object> operation = new LinkedHashMap<String, Object>();
			operation.put("asset_issuer", asset_issuer);
			operation.put("asset_code", asset_code);
			operation.put("asset_amount", asset_amount);
			operation.put("type", type);
			operation.put("dest_address", dest_address);
			List operList = new ArrayList<>();
			operList.add(operation);
			operListjson = JSONArray.fromObject(operList);
		}else {
			System.out.println("asset_type填写错误");
			System.exit(0);
		}
		return operListjson;
	}
	
	/**
	 * 初始化转账-operation
	 * @param dest_address
	 * @param asset_issuer
	 * @param asset_code
	 * @param asset_amount
	 * @return
	 */
	@SuppressWarnings({ "rawtypes", "unchecked" })
	public static JSONArray operations(String dest_address,String asset_issuer,String asset_code,int asset_amount){
		int asset_type = 1;
		int type = 5;
		JSONArray operListjson = null ;
		Map<String, Object> operation = new LinkedHashMap<String, Object>();
		operation.put("asset_issuer", asset_issuer);
		operation.put("asset_code", asset_code);
		operation.put("asset_amount", asset_amount);
		operation.put("type", type);
		operation.put("dest_address", dest_address);
		operation.put("asset_type", asset_type);
		List operList = new ArrayList<>();
		operList.add(operation);
		operListjson = JSONArray.fromObject(operList);
		return operListjson;
	}

	/**
	 *创建账户-operation-json
	 * 默认type=0  创建账户操作
	 * 初始化账户余额为200000
	 * 初始化account_metadata为空
	 * @param dest_address  目标账户
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static JSONArray operations(String dest_address){
		int type =0;
		int init_balance =200000;
		String account_metadata = null;
		Map<String, Object> operation = new LinkedHashMap<String, Object>();
		operation.put("type", type);
		operation.put("dest_address", dest_address);
		operation.put("init_balance", init_balance);
		operation.put("account_metadata", account_metadata);
		List operList = new ArrayList<>();
		operList.add(operation);
		JSONArray operListjson = JSONArray.fromObject(operList);
		return operListjson;
	}
	
	
	/**
	 * 供应链-operation
	 * @param inputs
	 * @param outputs
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static JSONArray operations(JSONArray inputs,JSONArray outputs){
		int type =6;
		Map<String, Object> operationMap = new LinkedHashMap<String, Object>();
		operationMap.put("inputs", inputs);
		operationMap.put("outputs", outputs);
		operationMap.put("type", type);
		List operList = new ArrayList<>();
		operList.add(operationMap);
		JSONArray operListjson = JSONArray.fromObject(operList);
		return operListjson;
	}
	
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static JSONArray operations(JSONArray outputs){
		int type =6;
		Map<String, Object> operationMap = new LinkedHashMap<String, Object>();
		operationMap.put("inputs", null);
		operationMap.put("outputs", outputs);
		operationMap.put("type", type);
		List operList = new ArrayList<>();
		operList.add(operationMap);
		JSONArray operListjson = JSONArray.fromObject(operList);
		return operListjson;
	}
	
	/**
	 * 供应链-outputs，没有input
	 * @param out_add
	 * @param metadata
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static JSONArray outputs(String out_add,String metadata){
		Map<String, Object> outputMap = new LinkedHashMap<String, Object>();
		outputMap.put("address", out_add);
		outputMap.put("metadata", metadata);
		List outList = new ArrayList<>();
		outList.add(outputMap);
		JSONArray operListjson = JSONArray.fromObject(outList);
		return operListjson;
	}
	/**
	 * 供应链-input
	 * @param hash
	 * @param index
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static JSONArray inputs(String hash,int index){
		Map<String, Object> inputMap = new LinkedHashMap<String, Object>();
		inputMap.put("hash", hash);
		inputMap.put("index", index);
		List inList = new ArrayList<>();
		inList.add(inputMap);
		JSONArray operListjson = JSONArray.fromObject(inList);
		return operListjson;
	}
	
	
	/**
	 * 创建账户-operation-json
	 * @param dest_address   目标账户
	 * @param init_balance   初始化账户余额
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static JSONArray operations(Object dest_address, Object init_balance){
		int type =0;
		String account_metadata = null;
		Map<String, Object> operation = new LinkedHashMap<String, Object>();
		operation.put("type", type);
		operation.put("dest_address", dest_address);
		operation.put("init_balance", init_balance);
		operation.put("account_metadata", account_metadata);
		List operList = new ArrayList<>();
		operList.add(operation);
		JSONArray operListjson = JSONArray.fromObject(operList);
		return operListjson;
	}
	/**
	 * 创建账户-operation-json
	 * @param dest_address  目标账户
	 * @param init_balance   初始化账户余额
	 * @param account_metadata  账户说明   //可选，该数据写进帐号的metadata属性中，必须是16进制小写格式
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static JSONArray operations(String dest_address, int init_balance, String account_metadata){
		int type =0;
		Map<String, Object> operation = new LinkedHashMap<String, Object>();
		operation.put("type", type);
		operation.put("dest_address", dest_address);
		operation.put("init_balance", init_balance);
		operation.put("account_metadata", account_metadata);
		List operList = new ArrayList<>();
		operList.add(operation);
		JSONArray operListjson = JSONArray.fromObject(operList);
		return operListjson;
	}
	
	/**
	 * 创建账户-operation-json
	 * @param dest_address  目标账户
	 * @param master_weight   权重
	 * @param low_threshold    门限最小值（暂时不用）
	 * @param med_threshold   门限（门限值的修改，暂时可以让三个字段保持一致）
	 * @param high_threshold   门限最大值（暂时不用）
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static JSONArray operations(String dest_address,int master_weight,int low_threshold,int med_threshold,int high_threshold){
		int type =0;
		int init_balance =200000;
		String account_metadata = null;
		Map<String, Object> operation = new LinkedHashMap<String, Object>();
		operation.put("type", type);
		operation.put("dest_address", dest_address);
		operation.put("init_balance", init_balance);
		operation.put("account_metadata", account_metadata);
		operation.put("threshold", threshold(master_weight, low_threshold, med_threshold, high_threshold));
		List operList = new ArrayList<>();
		operList.add(operation);
		JSONArray operListjson = JSONArray.fromObject(operList);
		return operListjson;
	}
	
	/**
	 * 创建账户-门限值设置-json
	 * @param master_weight  权重
	 * @param low_threshold 	门限最小值（暂时不用）
	 * @param med_threshold   门限（门限值的修改，暂时可以让三个字段保持一致）
	 * @param high_threshold	门限最大值（暂时不用）
	 * @return
	 */
	@SuppressWarnings({ "rawtypes", "unchecked" })
	public static List threshold(int master_weight,int low_threshold,int med_threshold,int high_threshold){
		Map<String, Integer> thresholdMap = new LinkedHashMap<String ,Integer>();
		thresholdMap.put("master_weight", master_weight);
		thresholdMap.put("low_threshold", low_threshold);
		thresholdMap.put("med_threshold", med_threshold);
		thresholdMap.put("high_threshold", high_threshold);
		List  threList = new ArrayList();
		threList.add(thresholdMap);
		return threList;
	}
	/**
	 * 设置属性-operation
	 * @param master_weight
	 * @param low_threshold
	 * @param med_threshold
	 * @param high_threshold
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static JSONArray operations(int master_weight,int low_threshold,int med_threshold,int high_threshold){
		int type = 4;
		Map<String, Object> operation = new LinkedHashMap<String, Object>();
		operation.put("type", type);
		operation.put("threshold", threshold(master_weight, low_threshold, med_threshold, high_threshold));
		List operList = new ArrayList<>();
		operList.add(operation);
		JSONArray operListjson = JSONArray.fromObject(operList);
		return operListjson;
	}
	/**
	 * 创建账户-operation-json，设置地址和联合签名
	 * @param dest_address  目标地址
	 * @param signer		设置联合签名
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static JSONArray operations(String dest_address,JSONArray signer){
		int type =0;
		int init_balance =200000;
		String account_metadata = null;
		Map<String, Object> operation = new LinkedHashMap<String, Object>();
		operation.put("type", type);
		operation.put("dest_address", dest_address);
		operation.put("init_balance", init_balance);
		operation.put("account_metadata", account_metadata);
		operation.put("signers", signer);
		List operList = new ArrayList<>();
		operList.add(operation);
		JSONArray operListjson = JSONArray.fromObject(operList);
		return operListjson;
	}
	
	/**
	 * 创建账户-operation-json 需设置两个signer
	 * @param dest_address
	 * @param address1
	 * @param weigth1
	 * @param address2
	 * @param weigth2
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static JSONArray operations(String dest_address,String address1,int weigth1,String address2,int weigth2){
		int type =0;
		int init_balance =200000;
		String account_metadata = null;
		Map<String, Object> operation = new LinkedHashMap<String, Object>();
		operation.put("type", type);
		operation.put("dest_address", dest_address);
		operation.put("init_balance", init_balance);
		operation.put("account_metadata", account_metadata);
		operation.put("signers", signers(address1, weigth1, address2, weigth2));
		List operList = new ArrayList<>();
		operList.add(operation);
		JSONArray operListjson = JSONArray.fromObject(operList);
		return operListjson;
	}
	/**
	 * 设置联合签名，一组账户
	 * @param address
	 * @param weigth
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static JSONArray signers(String address, int weigth){
		Map<String, Object> signerMap = new LinkedHashMap<String ,Object>();
		signerMap.put("address", address);
		signerMap.put("weigth", weigth);
		List signerList = new ArrayList<>();
		signerList.add(signerMap);
		JSONArray signerArray = JSONArray.fromObject(signerList);
		return signerArray;
	}
	/**
	 * 设置联合签名，两个用户签名
	 * @param address1
	 * @param weigth1
	 * @param address2
	 * @param weigth2
	 * @return
	 */
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public static JSONArray signers(String address1, int weigth1,String address2, int weigth2){
		Map<String, Object> signerMap1 = new LinkedHashMap<String ,Object>();
		signerMap1.put("address", address1);
		signerMap1.put("weigth", weigth1);
		Map<String, Object> signerMap2 = new LinkedHashMap<String ,Object>();
		signerMap2.put("address", address2);
		signerMap2.put("weigth", weigth2);
		List signerList = new ArrayList<>();
		signerList.add(signerMap1);
		signerList.add(signerMap2);
		JSONArray signerArray = JSONArray.fromObject(signerList);
		return signerArray;
	}
	public static void issued(){
		
	}

}

	class signer{
		@SuppressWarnings({ "unchecked", "rawtypes" })
		public signer(String address, int weigth){
			Map<String, Object> signerMap = new LinkedHashMap<String ,Object>();
			signerMap.put("address", address);
			signerMap.put("weigth", weigth);
			List signerList = new ArrayList<>();
			signerList.add(signerMap);
		}
	}
