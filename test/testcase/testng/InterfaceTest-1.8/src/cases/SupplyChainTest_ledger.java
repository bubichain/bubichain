package cases;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import org.testng.annotations.Test;

import utils.APIUtil;
import utils.Result;
import utils.SignUtil;
import utils.TxUtil;
import base.Log;
import base.TestBase;

@Test
public class SupplyChainTest_ledger extends TestBase {
	String geturl = get_Url2;
	@SuppressWarnings("rawtypes")
	Map acc = TxUtil.createAccount();
	Object source_address = acc.get("address");
	String pri = acc.get("private_key").toString();
	Object pub = acc.get("public_key");
	long sequence_number = Result.seq_num(source_address);
	String metadata = "1234";
	int type = 6;
//@Test
	public void supplyChainCheck(){
		//最简单的，供应链input为空的情况
		Map acc = TxUtil.createAccount();
		Object address = acc.get("address");
		List inputs = new ArrayList<>();
		List outputs = TxUtil.outputs(address, metadata);
		sequence_number = Result.seq_num(source_address);
		List opers = TxUtil.operSupplyChain(type,inputs,outputs);
		String response = SignUtil.tx(opers, source_address, fee, sequence_number, metadata, pri, pub);
		int error_code = Result.getErrorCode(geturl,response);
		check.assertEquals(error_code, 0, "供应链创建失败");
		
	}

//	@Test
	@SuppressWarnings("rawtypes")
	public void inputs_0Check(){
		//创建第一条供应链，inputs为空
		//获取output address的私钥和id
		Map acc = TxUtil.createAccount();
		Object o_address = acc.get("address");
		String o_pri = acc.get("private_key").toString();
		Object o_pub = acc.get("public_key");
		List inputs = new ArrayList<>();
		List outputs = TxUtil.outputs(o_address, metadata);
		List opers = TxUtil.operSupplyChain(type,inputs,outputs);   //input为空，生成第一条链
		sequence_number = Result.seq_num(led_acc);
		String response = SignUtil.tx(opers, led_acc, fee, sequence_number, metadata, led_pri, led_pub);
		int error_code = Result.getErrorCode(geturl,response);
		if(error_code==0){			//第一条链创建成功后在创建第二条链
			Object o_add2 = TxUtil.createAccount().get("address");
			Object hash = Result.getHash(response);
			List inputs1 = TxUtil.inputs(hash, 0);
			List outputs1 = TxUtil.outputs(o_add2, metadata);
			List opers1 = TxUtil.operSupplyChain(type,inputs1,outputs1);
			sequence_number = Result.seq_num(o_address);
			String response1 = SignUtil.tx(opers1, o_address, fee, sequence_number, metadata, o_pri, o_pub);
			int error_code1 = Result.getErrorCode(geturl,response1);
			check.assertEquals(error_code1, 0, "input不为空，供应链创建失败");
		}else{
			Log.info("第一条供应链创建失败，无法创建第二条");
		}
	}
	
	public void operationsCheck(){
		//创建第一条供应链，inputs为空
		//获取output address的私钥和id
		Map acc = TxUtil.createAccount();
		Object o_address = acc.get("address");
		String o_pri = acc.get("private_key").toString();
		Object o_pub = acc.get("public_key");
		List inputs = new ArrayList<>();
		List outputs = TxUtil.outputs(o_address, metadata);
		List opers = new ArrayList<>();
		opers.add(TxUtil.operSupplyChain(type,inputs,outputs)); 
		opers.add(TxUtil.operSupplyChain(type,inputs,outputs));
		sequence_number = Result.seq_num(led_acc);
		String response = SignUtil.tx(opers, led_acc, fee, sequence_number, metadata, led_pri, led_pub);
		int error_code = Result.getErrorCode(geturl,response);
		if(error_code==0){			//第一条链创建成功后在创建第二条链
			Object o_add2 = TxUtil.createAccount().get("address");
			Object hash = Result.getHash(response);
			List inputs1 = TxUtil.inputs(hash, 0);
			List outputs1 = TxUtil.outputs(o_add2, metadata);
			List opers1 = TxUtil.operSupplyChain(type,inputs1,outputs1);
			sequence_number = Result.seq_num(o_address);
			String response1 = SignUtil.tx(opers1, o_address, fee, sequence_number, metadata, o_pri, o_pub);
			int error_code1 = Result.getErrorCode(geturl,response1);
			check.assertEquals(error_code1, 0, "input不为空，供应链创建失败");
		}else{
			Log.info("第一条供应链创建失败，无法创建第二条");
		}
	}
//	@Test
	public void inputs_1Check(){
		//创建第一条供应链，inputs为空
		//获取两个output address的私钥和id
		Map acc1 = TxUtil.createAccount();
		Object o_add1 = acc1.get("address");
		Map acc2 = TxUtil.createAccount();
		Object o_add2 = acc2.get("address");
		String o_pri2 = acc2.get("private_key").toString();
		Object o_pub2 = acc2.get("public_key");
		List inputs = new ArrayList<>();
		List outputs = TxUtil.outputs(o_add1, metadata,o_add2,metadata);
		List opers = TxUtil.operSupplyChain(type,inputs,outputs);   //input为空，生成第一条链
		sequence_number = Result.seq_num(led_acc);
		String response = SignUtil.tx(opers, led_acc, fee, sequence_number, metadata, led_pri, led_pub);
		int error_code = Result.getErrorCode(geturl,response);
		if(error_code==0){			//第一条链创建成功后在创建第二条链
			Object o_add3 = TxUtil.createAccount().get("address");
			Object hash = Result.getHash(response);
			List inputs1 = TxUtil.inputs(hash, 1);
			List outputs1 = TxUtil.outputs(o_add3, metadata);
			List opers1 = TxUtil.operSupplyChain(type,inputs1,outputs1);
			sequence_number = Result.seq_num(o_add2);
			String response1 = SignUtil.tx(opers1, o_add2, fee, sequence_number, metadata, o_pri2, o_pub2);
			int error_code1 = Result.getErrorCode(geturl,response1);
			check.assertEquals(error_code1, 0, "input不为空，供应链创建失败");
		}else{
			Log.info("第一条供应链创建失败，无法创建第二条");
		}
	}
	
	public void inputs_2Check(){
		//创建第一条供应链，inputs为空
		//获取两个output address的私钥和id
		Map acc1 = TxUtil.createAccount();
		Object o_add1 = acc1.get("address");
		Map acc2 = TxUtil.createAccount();
		Object o_add2 = acc2.get("address");
		String o_pri2 = acc2.get("private_key").toString();
		Object o_pub2 = acc2.get("public_key");
		List inputs = new ArrayList<>();
		List outputs = TxUtil.outputs(o_add1, metadata,o_add2,metadata);
		List opers = TxUtil.operSupplyChain(type,inputs,outputs);   //input为空，生成第一条链
		sequence_number = Result.seq_num(led_acc);
		String response = SignUtil.tx(opers, led_acc, fee, sequence_number, metadata, led_pri, led_pub);
		int error_code = Result.getErrorCode(geturl,response);
		if(error_code==0){			//第一条链创建成功后在创建第二条链
			Object o_add3 = TxUtil.createAccount().get("address");
			Object hash = Result.getHash(response);
			List inputs1 = TxUtil.inputs(hash,3);
			List outputs1 = TxUtil.outputs(o_add3, metadata);
			List opers1 = TxUtil.operSupplyChain(type,inputs1,outputs1);
			sequence_number = Result.seq_num(o_add2);
			String response1 = SignUtil.tx(opers1, o_add2, fee, sequence_number, metadata, o_pri2, o_pub2);
			int error_code1 = Result.getErrorCode(geturl,response1);
			check.assertEquals(error_code1, 4, "input不为空，供应链创建失败");
		}else{
			Log.info("第一条供应链创建失败，无法创建第二条");
		}
	}
//	@Test
	@SuppressWarnings("rawtypes")
	public void input_hashCheck(){
		
		Map acc = TxUtil.createAccount();
		Object address = acc.get("address");
		Object[] hashs = { 0, -1, "abc", "!@#", "", null,"111111111" };
		for (Object hash : hashs) {
			List inputs = TxUtil.inputs(hash, 0);
			List outputs = TxUtil.outputs(address, metadata);
			List opers = TxUtil.operSupplyChain(type, inputs, outputs);
			sequence_number = Result.seq_num(source_address);
			String response = SignUtil.unnormal_tx(opers, source_address, fee,
					sequence_number, metadata, pri, pub);
			int error_code = Result.getErrorCode(geturl,response);
			check.assertEquals(error_code, 2, "供应链创建hash[" + hash + "]校验失败");
		}
		
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void input_indexCheck(){
		sequence_number = Result.seq_num(source_address);
		Map acc = TxUtil.createAccount();
		Object address = acc.get("address");
		List inputs = new ArrayList<>();
		List outputs = TxUtil.outputs(address, metadata);
		List opers = TxUtil.operSupplyChain(type,inputs,outputs);
		String response = SignUtil.tx(opers, source_address, fee, sequence_number, metadata, pri, pub);
		Object hash = Result.getHash(response);
		Object[] indexs = {0,-1,"abc","!@#","",null};
		for (Object index : indexs) {
			List inputs1 = TxUtil.inputs(hash, index);
			List outputs1 = TxUtil.outputs(address, metadata);
			List opers1 = TxUtil.operSupplyChain(type,inputs1,outputs1);
			String response1 = SignUtil.unnormal_tx(opers1, source_address,
					fee, sequence_number, metadata, pri, pub);
			int error_code = Result.getErrorCode(geturl,response1);
			check.assertEquals(error_code, 4, "供应链创建index[" + index
					+ "]校验失败");
		}
		
	}
	/**
	 * verify metadata in inputs
	 */
	@SuppressWarnings("rawtypes")
//	@Test
	public void input_metadataCheck(){
		
		String metadata_ = "1234";
		Map acc = TxUtil.createAccount();
		Object address = acc.get("address");
		String pri1 = acc.get("private_key").toString();
		Object pub1 = acc.get("public_key");
		List inputs = new ArrayList<>();
		List outputs = TxUtil.outputs(address, metadata);
		List opers = TxUtil.operSupplyChain(type,inputs,outputs);
		sequence_number = Result.seq_num(source_address);
		String response = SignUtil.tx(opers, source_address, fee, sequence_number, metadata, pri, pub);
		Object hash = Result.getHash(response);
		List outputs1 = TxUtil.outputs(address, metadata);
		Object[] metadatas = {0,-1,"abc","!@#","pp"};
//		Object[] metadatas = {"pp"};
		for (Object metadata : metadatas) {
			List inputs1 = TxUtil.inputs(hash, 0,metadata);
			List opers1 = TxUtil.operSupplyChain(type,inputs1,outputs1);
			sequence_number = Result.seq_num(address);
			String response1 = SignUtil.unnormal_tx(opers1, address,
					fee, sequence_number, metadata_, pri1, pub1);
//			System.out.println(response1);
			int error_code = Result.getErrorCode(geturl,response1);
			check.assertEquals(error_code, 2, "供应链创建metadata[" + metadata	+ "]校验失败");
		}
		
	}
	
	public void outputsCheck(){
		sequence_number = Result.seq_num(source_address);
		Map acc1 = TxUtil.createAccount();
		Object address1 = acc1.get("address");
		Map acc2 = TxUtil.createAccount();
		Object address2 = acc2.get("address");
		List inputs = new ArrayList<>();
		//outputs两个，创建成功
		List outputs = TxUtil.outputs(address1, metadata,address2,metadata);
		List opers = TxUtil.operSupplyChain(type,inputs,outputs);
		String response = SignUtil.tx(opers, source_address, fee, sequence_number, metadata, pri, pub);
		int error_code = Result.getErrorCode(geturl,response);
		check.assertEquals(error_code, 0, "outputs两个创建失败");
	}
	/*
	 * verify none outputs
	 */
//	@Test
	public void outputs_noneCheck(){
		sequence_number = Result.seq_num(source_address);
		List inputs = new ArrayList<>();
		//outputs两个，创建成功
		List outputs = new ArrayList<>();
		List opers = TxUtil.operSupplyChain(type,inputs,outputs);
		String response = SignUtil.tx(opers, source_address, fee, sequence_number, metadata, pri, pub);
		int error_code = Result.getErrorCode(geturl,response);
		check.assertEquals(error_code, 2, "outputs两个创建失败");
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void addressCheck(){
		sequence_number = Result.seq_num(source_address);
		List inputs = new ArrayList();
		Object addnew = APIUtil.generateAcc().get("address");
		Object[] adds = {0,-1,"",null,"ab","!@",addnew};
		for (Object add : adds) {
			List outputs = TxUtil.outputs(add, metadata);
			List opers = TxUtil.operSupplyChain(type,inputs,outputs);
			String response = SignUtil.unnormal_tx(opers, source_address, fee,
					sequence_number, metadata, pri, pub);
			int error_code = Result.getErrorCode(geturl,response);
			check.assertEquals(error_code, 4, "供应链address[" + add + "]校验失败");
		}
		
		
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void metadataCheck(){
		sequence_number = Result.seq_num(source_address);
		List inputs = new ArrayList();
		Object[] metas = {0,-1,"abc","qq"};
		for (Object meta : metas) {
			Map acc = TxUtil.createAccount();
			Object address = acc.get("address");
			List outputs = TxUtil.outputs(address, meta);
			List opers = TxUtil.operSupplyChain(type,inputs,outputs);
			String response = SignUtil.unnormal_tx(opers, source_address, fee,
					sequence_number, metadata, pri, pub);
			int error_code = Result.getErrorCode(geturl,response);
			check.assertEquals(error_code, 4, "供应链metadata[" + meta
					+ "]校验失败");
		}
		
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void feeCheck(){
		sequence_number = Result.seq_num(source_address);
		Map acc = TxUtil.createAccount();
		Object address = acc.get("address");
		List inputs = new ArrayList<>();
		Object[] fees = {-1,0,999,"abc","!@#","",null};
		for (Object fee : fees) {
			List outputs = TxUtil.outputs(address, metadata);
			List opers = TxUtil.operSupplyChain(type,inputs,outputs);
			String response = SignUtil.unnormal_tx(opers, source_address, fee,
					sequence_number, metadata, pri, pub);
			int error_code = Result.getErrorCode(geturl,response);
			check.assertEquals(error_code, 4, "供应链fee[" + fee + "]校验失败");
		}
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void source_addressCheck(){
		sequence_number = Result.seq_num(source_address);
		Map acc = TxUtil.createAccount();
		Object address = acc.get("address");
		List inputs = new ArrayList<>();
		String addnew = APIUtil.generateAcc().get("address");
		Object[] source_adds = {-1,0,"abc","!@#","",null,addnew};
		for (Object source_address : source_adds) {
			List outputs = TxUtil.outputs(address, metadata);
			List opers = TxUtil.operSupplyChain(type,inputs,outputs);
			String response = SignUtil.unnormal_tx(opers, source_address, fee,
					sequence_number, metadata, pri, pub);
			int error_code = Result.getErrorCode(geturl,response);
			check.assertEquals(error_code, 4, "供应链source_address["
					+ source_address + "]校验失败");
		}
		
	}
//	@Test
	public void oper_sourceAddValidCheck(){
		//check ERRCODE_ASSET_INVALID
		sequence_number = Result.seq_num(source_address);
		List inputs = new ArrayList<>();
		Map acc = TxUtil.createAccount();
		Object address = acc.get("address");
		List outputs = TxUtil.outputs(address, metadata);
		List opers = TxUtil.operSupplyChain(type,inputs,outputs,source_address);
		String result = SignUtil.tx(opers, source_address, fee, sequence_number, metadata, pri, pub);
		int err_code = Result.getErrorCode(geturl,result);
		check.assertEquals(err_code, 0, "供应链operation中sourceaddress为正确值时校验失败");
	}
	@Test
	public void oper_sourceAddinValidCheck(){
		//check ERRCODE_ASSET_INVALID
		sequence_number = Result.seq_num(source_address);
		List inputs = new ArrayList<>();
		Map acc = TxUtil.createAccount();
		Object address = acc.get("address");
		List outputs = TxUtil.outputs(address, metadata);
		List opers = TxUtil.operSupplyChain(type,inputs,outputs,address);
		String result = SignUtil.tx(opers, source_address, fee, sequence_number, metadata, pri, pub);
		int err_code = Result.getErrorCode(geturl,result);
		check.assertEquals(err_code, 4, "供应链operation中sourceaddress为错误值时校验失败");
	}
	
//	@Test
	
	@SuppressWarnings("rawtypes")
	public void private_keyCheck(){
		
		Map acc = TxUtil.createAccount();
		Object address = acc.get("address");
		// Object key = acc.get("private_key");
		Object pri1 = TxUtil.createAccount().get("private_key");
		Object pri2 = APIUtil.generateAcc().get("private_key");

		Object[] pri_keys = { pri1, pri2 };
		// Object[] pri_keys = {-1,10,"abc","!@#","",null,key};
		List inputs = new ArrayList();
		for (Object pri_key : pri_keys) {
			sequence_number = Result.seq_num(source_address);
			String pri = pri_key.toString();
			List outputs = TxUtil.outputs(address, metadata);
			List opers = TxUtil.operSupplyChain(type,inputs,outputs);
			String response = SignUtil.unnormal_tx(opers, source_address, fee,
					sequence_number, metadata, pri, pub);
			int error_code = Result.getErrorCode(geturl,response);
			check.assertEquals(error_code, 4, "供应链private_key[" + pri_key
					+ "]校验失败");
		}
//		check.result("供应链交易校验成功");
	}
}
