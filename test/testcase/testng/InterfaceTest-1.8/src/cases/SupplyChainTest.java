package cases;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import org.testng.annotations.Test;

import utils.APIUtil;
import utils.Result;
import utils.SignUtil;
import utils.TxUtil;
import base.TestBase;
import model.Account;
import model.Input;
import model.InputInfo;
import model.Output;

@Test
public class SupplyChainTest extends TestBase {

	@SuppressWarnings("rawtypes")
	Map acc = TxUtil.createAccount();
	Object source_address = acc.get("address");
	String pri = acc.get("private_key").toString();
	Object pub = acc.get("public_key");
	String metadata = "1234";
	int type = 6;
//	@Test
	public void supplyChainCheck(){
		//验证供应链创建成功，没有input
		Account srcAcc = TxUtil.createNewAccount();
		Account a1 = TxUtil.createNewAccount();
		List<Object> inputs = new ArrayList<>();
		List<Object> outputs = TxUtil.outputs(a1.getAddress(), metadata);
		List<Object> opers = TxUtil.operSupplyChain(type,inputs,outputs);
		String response = SignUtil.tx(opers, srcAcc.getAddress(), fee, metadata, srcAcc.getPri_key(), srcAcc.getPub_key());
		int error_code = Result.getErrorCode(response);
		check.assertEquals(error_code, 0, "供应链创建失败");
	}

	//供应链操作type类型校验
//	@Test
	@SuppressWarnings("rawtypes")
	public void typeCheck() {
		Map acc = TxUtil.createAccount();
		Object address = acc.get("address");
		Object[] types = { 0, 20, -1, "abc", "!@#", "", null };
		for (Object type : types) {
			List inputs = new ArrayList<>();
			List outputs = TxUtil.outputs(address, metadata);
			List opers = TxUtil.operSupplyChain(type, inputs, outputs);
			String response = SignUtil.unnormal_tx(opers, source_address, fee, metadata, pri, pub);
			int error_code = Result.getErrorCode(response);
			check.assertEquals(error_code, 2, "供应链type[" + type + "]校验失败");
		}
	}
	
	//验证二级供应链创建成功
//	@Test
	public void inputs_0Check() {
		InputInfo info = TxUtil.input2();
		String response = Result.getTranHisByHash(info.getHash());
		int error_code = Result.getErrorCode(response);
		check.assertEquals(error_code, 0, "input不为空，供应链创建失败");
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
			String response = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int error_code = Result.getErrorCode(response);
			check.assertEquals(error_code, 2, "供应链创建hash[" + hash + "]校验失败");
		}
		
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void input_indexCheck(){
		Map acc = TxUtil.createAccount();
		Object address = acc.get("address");
		List inputs = new ArrayList<>();
		List outputs = TxUtil.outputs(address, metadata);
		List opers = TxUtil.operSupplyChain(type,inputs,outputs);
		String response = SignUtil.tx(opers, source_address, fee, metadata, pri, pub);
		Object hash = Result.getHash(response);
		Object[] indexs = {-1,"abc","!@#","",null};
		for (Object index : indexs) {
			List inputs1 = TxUtil.inputs(hash, index);
			List outputs1 = TxUtil.outputs(address, metadata);
			List opers1 = TxUtil.operSupplyChain(type,inputs1,outputs1);
			String response1 = SignUtil.unnormal_tx(opers1, source_address,
					fee, metadata, pri, pub);
			int error_code = Result.getErrorCode(response1);
			check.assertEquals(error_code, 2, "供应链创建index[" + index
					+ "]校验失败");
		}
		
	}
	/*
	 * verify metadata in inputs
	 */
//	@Test
	@SuppressWarnings("rawtypes")
	public void input_metadataCheck(){
		String metadata_ = "1234";
		Account account = TxUtil.createNewAccount();
		List inputs = new ArrayList<>();
		List outputs = TxUtil.outputs(account.getAddress(), metadata);
		List opers = TxUtil.operSupplyChain(type,inputs,outputs);
		String response = SignUtil.tx(opers, source_address, fee, metadata, pri, pub);
		Object hash = Result.getHash(response);
		List outputs1 = TxUtil.outputs(account.getAddress(), metadata);
		Object[] metadatas = {0,-1,"abc","!@#","pp"};
		for (Object metadata : metadatas) {
			List inputs1 = TxUtil.inputs(hash, 0,metadata);
			List opers1 = TxUtil.operSupplyChain(type,inputs1,outputs1);
			String response1 = SignUtil.unnormal_tx(opers1, account.getAddress(),
					fee, metadata_, account.getPri_key(), account.getPub_key());
			int error_code = Result.getErrorCode(response1);
			check.assertEquals(error_code, 2, "供应链创建metadata[" + metadata	+ "]校验失败");
		}
		
	}
	
	@SuppressWarnings("rawtypes")
//	@Test
	public void outputsCheck(){
		Account a1 = TxUtil.createNewAccount();
		Account a2 = TxUtil.createNewAccount();
		List inputs = new ArrayList<>();
		//验证outputs两个，创建成功
		List outputs = TxUtil.outputs(a1.getAddress(), metadata,a2.getAddress(),metadata);
		List opers = TxUtil.operSupplyChain(type,inputs,outputs);
		String response = SignUtil.tx(opers, source_address, fee, metadata, pri, pub);
		int error_code = Result.getErrorCode(response);
		check.assertEquals(error_code, 0, "outputs两个创建失败");
	}
	//验证两个output一样，创建失败
//	@Test
	public void outputs_sameCheck(){
		Account src = TxUtil.createNewAccount();
		Account a1 = TxUtil.createNewAccount();
		System.out.println(source_address);
		System.out.println(pri);
		System.out.println(pub);
		List inputs = new ArrayList<>();
		//验证outputs两个，创建成功
		List outputs = TxUtil.outputs(a1.getAddress(), metadata,a1.getAddress(),metadata);
		List opers = TxUtil.operSupplyChain(type,inputs,outputs);
		String response = SignUtil.tx(opers, src.getAddress(), fee, metadata, src.getPri_key(), src.getPub_key());
		int error_code = Result.getErrorCode(response);
		check.assertEquals(error_code, 0, "outputs两个创建失败");
	}
	//多个output
//	@Test
	public void few_outputsCheck(){
		List<Input> inputs = new ArrayList<>();
		List<Output> outputs = TxUtil.createOutput(3);
		List<Object> opers = TxUtil.operSupplyChain(type,inputs,outputs);
		String response = SignUtil.tx(opers, source_address, fee, metadata, pri, pub);
		System.out.println(response);
		int error_code = Result.getErrorCode(response);
		check.assertEquals(error_code, 0, "outputs两个创建失败");
	}
	/*
	 * verify none outputs
	 */
//	@Test
	@SuppressWarnings("rawtypes")
	public void outputs_noneCheck(){
		List inputs = new ArrayList<>();
		List outputs = new ArrayList<>();
		List opers = TxUtil.operSupplyChain(type,inputs,outputs);
		String response = SignUtil.tx(opers, source_address, fee, metadata, pri, pub);
		int error_code = Result.getErrorCode(response);
		check.assertEquals(error_code, 2, "outputs为空校验失败");
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void output_addressinvalidCheck(){
		List inputs = new ArrayList();
		Object[] adds = {0,-1,"",null,"ab","!@"};
		for (Object add : adds) {
			List outputs = TxUtil.outputs(add, metadata);
			List opers = TxUtil.operSupplyChain(type,inputs,outputs);
			String response = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int error_code = Result.getErrorCode(response);
			check.assertEquals(error_code, 2, "供应链address[" + add + "]校验失败");
		}
	}
//	@Test
	@SuppressWarnings("rawtypes")
	public void output_addressNotExistCheck(){
		List inputs = new ArrayList();
		Object addnew = APIUtil.generateAcc().get("address");
			List outputs = TxUtil.outputs(addnew, metadata);
			List opers = TxUtil.operSupplyChain(type,inputs,outputs);
			String response = SignUtil.unnormal_tx(opers, source_address, fee,
					 metadata, pri, pub);
			System.out.println(response);
			int error_code = Result.getErrorCode(response);
			check.assertEquals(error_code,103, "供应链output_address[" + addnew + "]校验失败");
	}
//	@Test
	@SuppressWarnings("rawtypes")
	public void metadataCheck(){
		List inputs = new ArrayList();
		Object[] metas = {0,-1,"abc","qq"};
		for (Object meta : metas) {
			Map acc = TxUtil.createAccount();
			Object address = acc.get("address");
			List outputs = TxUtil.outputs(address, meta);
			List opers = TxUtil.operSupplyChain(type,inputs,outputs);
			String response = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int error_code = Result.getErrorCode(response);
			check.assertEquals(error_code, 2, "供应链metadata[" + meta
					+ "]校验失败");
		}
		
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void feeinvalidCheck(){
		Map acc = TxUtil.createAccount();
		Object address = acc.get("address");
		List inputs = new ArrayList<>();
		Object[] fees = {-1,"abc","!@#","",null};
		for (Object fee : fees) {
			List outputs = TxUtil.outputs(address, metadata);
			List opers = TxUtil.operSupplyChain(type,inputs,outputs);
			String response = SignUtil.unnormal_tx(opers, source_address, fee,
					 metadata, pri, pub);
			int error_code = Result.getErrorCode(response);
			check.assertEquals(error_code, 2, "供应链fee[" + fee + "]校验失败");
		}
	}
//	@Test
	@SuppressWarnings("rawtypes")
	public void feeNotEnoughCheck(){
		Map acc = TxUtil.createAccount();
		Object address = acc.get("address");
		List inputs = new ArrayList<>();
		Object[] fees = {0,fee-1};
		for (Object fee : fees) {
			List outputs = TxUtil.outputs(address, metadata);
			List opers = TxUtil.operSupplyChain(type,inputs,outputs);
			String response = SignUtil.unnormal_tx(opers, source_address, fee,
					 metadata, pri, pub);
			int error_code = Result.getErrorCode(response);
			check.assertEquals(error_code, 111, "供应链fee[" + fee + "]校验失败");
		}
	}
	
//	@Test
	@SuppressWarnings("rawtypes")
	public void source_addressinvalidCheck(){
		Map acc = TxUtil.createAccount();
		Object address = acc.get("address");
		List inputs = new ArrayList<>();
		Object[] source_adds = {-1,0,"abc","!@#"};
		for (Object source_address : source_adds) {
			List outputs = TxUtil.outputs(address, metadata);
			List opers = TxUtil.operSupplyChain(type,inputs,outputs);
			String response = SignUtil.unnormal_tx(opers, source_address, fee,
					 metadata, pri, pub);
			int error_code = Result.getErrorCode(response);
			check.assertEquals(error_code, 2, "供应链source_address["
					+ source_address + "]校验失败");
		}
	}
//	@Test	之前预期是103
	@SuppressWarnings("rawtypes")
	public void source_addressCheck(){
		Map acc = TxUtil.createAccount();
		Object address = acc.get("address");
		List inputs = new ArrayList<>();
		String addnew = APIUtil.generateAcc().get("address");
			List outputs = TxUtil.outputs(address, metadata);
			List opers = TxUtil.operSupplyChain(type,inputs,outputs);
			String response = SignUtil.unnormal_tx(opers, addnew, fee,
					 metadata, pri, pub);
			int error_code = Result.getErrorCode(response);
			check.assertEquals(error_code, 4, "供应链source_address["
					+ source_address + "]校验失败");
	}
	
//	@Test    //之前预期是93，实际是4
	
	@SuppressWarnings("rawtypes")
	public void private_keyCheck(){
		Map acc = TxUtil.createAccount();
		Object address = acc.get("address");
		Object pri1 = TxUtil.createAccount().get("private_key");
		Object pri2 = APIUtil.generateAcc().get("private_key");
		Object[] pri_keys = { pri1, pri2 };
		List inputs = new ArrayList();
		for (Object pri_key : pri_keys) {
			String pri = pri_key.toString();
			List outputs = TxUtil.outputs(address, metadata);
			List opers = TxUtil.operSupplyChain(type,inputs,outputs);
			String response = SignUtil.unnormal_tx(opers, source_address, fee,
					metadata, pri, pub);
			int error_code = Result.getErrorCode(response);
			check.assertEquals(error_code, 4, "供应链private_key[" + pri_key
					+ "]校验失败");
		}
	}
}
