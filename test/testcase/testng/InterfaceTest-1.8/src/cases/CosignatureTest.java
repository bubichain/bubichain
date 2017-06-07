package cases;

import java.util.List;
import java.util.Map;

import org.testng.annotations.Test;

import base.TestBase;
import model.Account;
import net.sf.json.JSONObject;
import utils.Result;
import utils.SignUtil;
import utils.TxUtil;

//联合签名
//@Test
public class CosignatureTest extends TestBase {

	/*
	 * 1.创建一个账户acc1 
	 * 2.创建两个联合签名账户acc2，acc3
	 * 3.设置acc的账户属性，修改threshold=10，和signers的权重6
	 * 4.创建一个交易（任意一个交易，最好所有交易都做一遍），交易使用联合签名，验证交易是否通过
	 */
	// 联合签名发行资产（联合签名权重大于门限值）
	public void cosign01() {
		Map acc1 = TxUtil.createAccount(); // 创建第一个账户
		Object source_address = acc1.get("address");
		String pri = acc1.get("private_key").toString();
		Object pub = acc1.get("public_key");
		long sequence_number = Result.seq_num(source_address);

		Map acc2 = TxUtil.createAccount(); // 创建第二个账户
		Object address2 = acc2.get("address");
		Object pri2 = acc2.get("private_key");
		Object pub2 = acc2.get("public_key");
		int weight2 = 6;

		Map acc3 = TxUtil.createAccount(); // 创建第三个账户
		Object address3 = acc3.get("address");
		Object pri3 = acc3.get("private_key").toString();
		Object pub3 = acc3.get("public_key");
		int weight3 = 6;

		Object master_weight = 10;
		Object low_threshold = 10;
		Object med_threshold = 10;
		Object high_threshold = 10;

		String metadata = "1234";
		int type = 4;

		JSONObject threshold = TxUtil.threshold(master_weight, med_threshold,
				low_threshold, high_threshold);
		List signers = TxUtil.signers(address3, weight3, address2, weight2);
		List operations = TxUtil.operSetOption(type, threshold, signers);
		String response = SignUtil.tx(operations, source_address, fee,
				sequence_number, metadata, pri, pub);
		int error_code = Result.getErrorCode(response);
		check.equals(error_code, 0, "设置属性失败");

		if(error_code==0){
			int typeaction = 2;
			int asset_type = 1;
			Object asset_issuer = source_address;
			String asset_code = "abc" ;
			int asset_amount = 100;
			long sequence_number1 = Result.seq_num(source_address);
			// 开始联合签名
			List opers = TxUtil.operIssue(typeaction, asset_type, asset_issuer, asset_code, asset_amount);	
			JSONObject tran = TxUtil.tran_json(source_address, fee,
					sequence_number1, metadata, opers);
			String tran_blob = TxUtil.getBlob(tran);
			JSONObject s2 = TxUtil.signature(tran, pri2, pub2);
			JSONObject s3 = TxUtil.signature(tran, pri3, pub3);
			List signatures = TxUtil.signatures(s2, s3);
			List items = TxUtil.itemlist(signatures, tran_blob);
			String result = TxUtil.tx_cosign(items);

			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 0, "联合账户权重和大于源账户门限值，资产发行失败");
		}
		
	}
	
	/**
	 * 创建一个联名账户A，master_w = 10,Low = 1,Med = 10, Hig = 255
	 * signerB weight=255
	 * 验证A发起交易，只有B签名，交易成功
	 */
//	@Test
	public void consign_test_with_HighWeightSigner(){
		Map acc1 = TxUtil.createAccount(); // 创建第一个账户
		Object source_address = acc1.get("address");
		String pri = acc1.get("private_key").toString();
		Object pub = acc1.get("public_key");
		long sequence_number = Result.seq_num(source_address);

		Map acc2 = TxUtil.createAccount(); // 创建第二个账户
		Object address2 = acc2.get("address");
		Object pri2 = acc2.get("private_key");
		Object pub2 = acc2.get("public_key");
		int weight2 = 255;
		
		Object master_weight = 10;
		Object low_threshold = 1;
		Object med_threshold = 10;
		Object high_threshold = 255;

		String metadata = "1234";
		int type = 4;

		JSONObject threshold = TxUtil.threshold(master_weight, med_threshold,
				low_threshold, high_threshold);
		List signers = TxUtil.signers(address2, weight2);
		List operations = TxUtil.operSetOption(type, threshold, signers);
		String response = SignUtil.tx(operations, source_address, fee,
				sequence_number, metadata, pri, pub);
		int error_code = Result.getErrorCode(response);
		check.equals(error_code, 0, "设置属性失败");
		if(error_code==0){
			int typeaction = 2;
			int asset_type = 1;
			Object asset_issuer = source_address;
			String asset_code = "abc" ;
			int asset_amount = 100;
			long sequence_number1 = Result.seq_num(source_address);
			List opers = TxUtil.operIssue(typeaction, asset_type, asset_issuer, asset_code, asset_amount);	
			JSONObject tran = TxUtil.tran_json(source_address, fee,
					sequence_number1, metadata, opers);
			String tran_blob = TxUtil.getBlob(tran);
			JSONObject s1 = TxUtil.signature(tran, pri, pub);
//			JSONObject s2 = TxUtil.signature(tran, pri2, pub2);
			List signatures = TxUtil.signatures(s1);
			List items = TxUtil.itemlist(signatures, tran_blob);
			String result = TxUtil.tx_cosign(items);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 0, "源账户门限权重为10，signers权重分别为6，源账户单独对发行进行签名，交易失败");
		}
		
	}
	
	/*
	 * 1.创建一个账户acc1 
	 * 2.创建两个联合签名账户acc2，acc3
	 * 3.设置acc1的账户属性，修改threshold=10，和signers的权重6
	 * 4.使用acc1创建一个交易（只有acc1签名），验证交易成功
	 */
//	@Test
	public void cosign015() {
		Map acc1 = TxUtil.createAccount(); // 创建第一个账户
		Object source_address = acc1.get("address");
		String pri = acc1.get("private_key").toString();
		Object pub = acc1.get("public_key");
		long sequence_number = Result.seq_num(source_address);

		Map acc2 = TxUtil.createAccount(); // 创建第二个账户
		Object address2 = acc2.get("address");
		Object pri2 = acc2.get("private_key");
		Object pub2 = acc2.get("public_key");
		int weight2 = 6;

		Map acc3 = TxUtil.createAccount(); // 创建第三个账户
		Object address3 = acc3.get("address");
		Object pri3 = acc3.get("private_key").toString();
		Object pub3 = acc3.get("public_key");
		int weight3 = 6;

		Object master_weight = 10;
		Object low_threshold = 10;
		Object med_threshold = 10;
		Object high_threshold = 10;

		String metadata = "1234";
		int type = 4;

		JSONObject threshold = TxUtil.threshold(master_weight, med_threshold,
				low_threshold, high_threshold);
		List operations = TxUtil.operSetOption(type, threshold);
		String response = SignUtil.tx(operations, source_address, fee,
				sequence_number, metadata, pri, pub);
		int error_code = Result.getErrorCode(response);
		check.equals(error_code, 0, "设置属性失败");

		if(error_code==0){
			int typeaction = 2;
			int asset_type = 1;
			Object asset_issuer = source_address;
			String asset_code = "abc" ;
			int asset_amount = 100;
			long sequence_number1 = Result.seq_num(source_address);
			List opers = TxUtil.operIssue(typeaction, asset_type, asset_issuer, asset_code, asset_amount);	
			JSONObject tran = TxUtil.tran_json(source_address, fee,
					sequence_number1, metadata, opers);
			String tran_blob = TxUtil.getBlob(tran);
			//只有acc1进行签名
			JSONObject s1 = TxUtil.signature(tran, pri, pub);
//			JSONObject s2 = TxUtil.signature(tran, pri2, pub2);
//			JSONObject s3 = TxUtil.signature(tran, pri3, pub3);
			List signatures = TxUtil.signatures(s1);
			List items = TxUtil.itemlist(signatures, tran_blob);
			String result = TxUtil.tx_cosign(items);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 0, "源账户门限权重为10，signers权重分别为6，源账户单独对发行进行签名，交易失败");
		}
		
	}

	/*
	 * 1.创建一个账户acc1 2.创建两个联合签名账户acc2，acc3 3.设置acc的账户属性，修改threshold
	 * 10，和signers的权重5
	 */
	// 联合签名发行资产（联合签名权重等于门限值）
	public void cosign02() {
		Map acc1 = TxUtil.createAccount(); // 创建第一个账户
		Object source_address = acc1.get("address");
		String pri = acc1.get("private_key").toString();
		Object pub = acc1.get("public_key");
		long sequence_number = Result.seq_num(source_address);

		Map acc2 = TxUtil.createAccount(); // 创建第二个账户
		Object address2 = acc2.get("address");
		Object pri2 = acc2.get("private_key");
		Object pub2 = acc2.get("public_key");
		int weight2 = 5;

		Map acc3 = TxUtil.createAccount(); // 创建第三个账户
		Object address3 = acc3.get("address");
		Object pri3 = acc3.get("private_key").toString();
		Object pub3 = acc3.get("public_key");
		int weight3 = 5;

		Object master_weight = 10;
		Object low_threshold = 10;
		Object med_threshold = 10;
		Object high_threshold = 10;

		String metadata = "1234";
		int type = 4;

		JSONObject threshold = TxUtil.threshold(master_weight, med_threshold,
				low_threshold, high_threshold);
		List signers = TxUtil.signers(address3, weight3, address2, weight2);
		List operations = TxUtil.operSetOption(type, threshold, signers);
		String response = SignUtil.tx(operations, source_address, fee,
				sequence_number, metadata, pri, pub);
		int error_code = Result.getErrorCode(response);
		check.equals(error_code, 0, "设置属性失败");

		if (error_code == 0) {
			int typeaction = 2;
			int asset_type = 1;
			Object asset_issuer = source_address;
			String asset_code = "abc";
			int asset_amount = 100;
			sequence_number = Result.seq_num(source_address);
			// 开始联合签名
			List opers = TxUtil.operIssue(typeaction, asset_type, asset_issuer,
					asset_code, asset_amount);
			JSONObject tran = TxUtil.tran_json(source_address, fee,
					sequence_number, metadata, opers);
			String tran_blob = TxUtil.getBlob(tran);
			JSONObject s2 = TxUtil.signature(tran, pri2, pub2);
			JSONObject s3 = TxUtil.signature(tran, pri3, pub3);
			List signatures = TxUtil.signatures(s2, s3);
			List items = TxUtil.itemlist(signatures, tran_blob);
			String result = TxUtil.tx_cosign(items);

			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 0, "联合账户权重和大于源账户门限值，资产发行失败");
		}
	}

	/*
	 * 1.创建两个账户A,B 
	 * 2.设置A账户权重是10，B账户门限是9（A\B没有关系） 
	 * 3.B账户发起交易，使用A账户签名，交易失败
	 */
//	@Test
	public void consign03() {
		int type = 4;
		String metadata = "1234";
		Account a1 = TxUtil.createNewAccount();
//		Map acc1 = TxUtil.createAccount(); // 创建第一个账户
//		Object address1 = acc1.get("address");
//		String pri1 = acc1.get("private_key").toString();
//		Object pub1 = acc1.get("public_key");

		JSONObject threshold = TxUtil.threshold(10, 10, 10, 10); // 设置账户权重是10
		List operations = TxUtil.operSetOption(type, threshold);
		String response = SignUtil.tx(operations, a1.getAddress(), fee,
				metadata, a1.getPri_key(), a1.getPub_key());
		int error_code = Result.getErrorCode(response);
		check.equals(error_code, 0, "账户1设置属性失败");

		Account a2 = TxUtil.createNewAccount();
//		Map acc2 = TxUtil.createAccount(); // 创建第二个账户
//		Object address2 = acc2.get("address");
//		String pri2 = acc2.get("private_key").toString();
//		Object pub2 = acc2.get("public_key");

		JSONObject threshold2 = TxUtil.threshold(9, 9, 9, 9); // 设置账户门限是9
		List operations2 = TxUtil.operSetOption(type, threshold2);
		String response2 = SignUtil.tx(operations2, a2.getAddress(), fee,
				metadata, a2.getPri_key(), a2.getPub_key());
		int error_code2 = Result.getErrorCode(response2);
		check.equals(error_code2, 0, "账户2设置属性失败");

		if (error_code == 0 && error_code2 == 0) {
			int typeaction = 2;
			int asset_type = 1;
			Object asset_issuer = a2.getAddress();
			String asset_code = "abc";
			int asset_amount = 100;
			// 开始联合签名
			List opers = TxUtil.operIssue(typeaction, asset_type, asset_issuer, // B账户发起交易，使用A账户签名
					asset_code, asset_amount);
			JSONObject tran = TxUtil.tran_json(a2.getAddress(), fee, 
					metadata, opers);
			String tran_blob = TxUtil.getBlob(tran);
			JSONObject s2 = TxUtil.signature(tran, a1.getPri_key(), a1.getPub_key());
			List signatures = TxUtil.signatures(s2);
			List items = TxUtil.itemlist(signatures, tran_blob);
			String result = TxUtil.tx_cosign(items);
			int err_code = Result.getErrorCode(result);
			check.assertNotEquals(err_code, 0, "A账户签名B的交易，验证失败");
		}

	}

	/*
	 * 1.创建两个账户A,B 
	 * 2.设置A账户权重是10，B账户门限是11（A是B的signer） 
	 * 3.B账户发起交易，使用A账户签名，交易失败
	 */
	public void consign04() {
		int type = 4;
		String metadata = "1234";
		Map acc1 = TxUtil.createAccount(); // 创建第一个账户
		Object address1 = acc1.get("address");
		String pri1 = acc1.get("private_key").toString();
		Object pub1 = acc1.get("public_key");
		long sequence_number1 = Result.seq_num(address1);

		JSONObject threshold = TxUtil.threshold(10, 10, 10, 10); // 设置A账户权重是10
		List operations = TxUtil.operSetOption(type, threshold);
		String response = SignUtil.tx(operations, address1, fee,
				sequence_number1, metadata, pri1, pub1);
		int error_code = Result.getErrorCode(response);
		check.equals(error_code, 0, "账户1设置属性失败");

		Map acc2 = TxUtil.createAccount(); // 创建第二个账户
		Object address2 = acc2.get("address");
		String pri2 = acc2.get("private_key").toString();
		Object pub2 = acc2.get("public_key");
		long sequence_number2 = Result.seq_num(address2);

		JSONObject threshold2 = TxUtil.threshold(11, 11, 11, 11); // 设置B账户门限是11
		List signers = TxUtil.signers(address1, 10);
		List operations2 = TxUtil.operSetOption(type, threshold2, signers);
		String response2 = SignUtil.tx(operations2, address2, fee,
				sequence_number2, metadata, pri2, pub2);
		int error_code2 = Result.getErrorCode(response2);
		check.equals(error_code2, 0, "账户2设置属性失败");

		if (error_code == 0 && error_code2 == 0) { // 属性设置成功后B账户开始发行资产交易
			int typeaction = 2;
			int asset_type = 1;
			Object asset_issuer = address2;
			String asset_code = "abc";
			int asset_amount = 100;
			sequence_number2 = Result.seq_num(address2);
			// 开始联合签名
			List opers = TxUtil.operIssue(typeaction, asset_type, asset_issuer, // B账户发起交易，使用A账户签名
					asset_code, asset_amount);
			JSONObject tran = TxUtil.tran_json(address2, fee, sequence_number2,
					metadata, opers);
			String tran_blob = TxUtil.getBlob(tran);
			JSONObject s2 = TxUtil.signature(tran, pri1, pub1);
			List signatures = TxUtil.signatures(s2);
			List items = TxUtil.itemlist(signatures, tran_blob);
			String result = TxUtil.tx_cosign(items);
			int err_code = Result.getErrorCode(result);
			check.assertNotEquals(err_code, 0, "A账户签名B的交易，验证失败");
		}
	}
	// 1.创建两个账户A\B
	// 2.设置A的权重为10（作为B的signer），B的high_threshold是10
	// 3.B做设置属性操作，用A签名，交易成功
//	@Test
	public void consign05() {
		int type = 4;
		String metadata = "1234";
		Map acc1 = TxUtil.createAccount(); // 创建第一个账户A
		Object address1 = acc1.get("address");
		String pri1 = acc1.get("private_key").toString();
		Object pub1 = acc1.get("public_key");
		long sequence_number1 = Result.seq_num(address1);

		Map acc2 = TxUtil.createAccount(10, 10, 10, 10, address1, 10); // 创建第二个账户B,并且设置了A作为B的signer
		Object address2 = acc2.get("address");
		String pri2 = acc2.get("private_key").toString();
		Object pub2 = acc2.get("public_key");
		long sequence_number2 = Result.seq_num(address2);
		
		JSONObject threshold = TxUtil.threshold(10, 10, 10, 10);
		List operations = TxUtil.operSetOption(type, threshold);
		String response = SignUtil.tx(operations, address2, fee,
				sequence_number2, metadata, pri1, pub1);
		int error_code = Result.getErrorCode(response);
		check.assertEquals(error_code, 0, "A的权重等于B的high_threshold，用B进行设置属性失败");

	}
	// 1.创建两个账户A\B
	// 2.设置A的权重为10（作为B的signer），B的high_threshold是11
	// 3.B做设置属性操作，用A签名，交易失败
//	@Test
	public void consign06() {
		int type = 4;
		String metadata = "1234";
		Map acc1 = TxUtil.createAccount(); // 创建第一个账户A
		Object address1 = acc1.get("address");
		String pri1 = acc1.get("private_key").toString();
		Object pub1 = acc1.get("public_key");
		long sequence_number1 = Result.seq_num(address1);

		Map acc2 = TxUtil.createAccount(10, 10, 10, 11, address1, 10); // 创建第二个账户B,并且设置了A作为B的signer
		Object address2 = acc2.get("address");
		String pri2 = acc2.get("private_key").toString();
		Object pub2 = acc2.get("public_key");
		long sequence_number2 = Result.seq_num(address2);

		JSONObject threshold = TxUtil.threshold(10, 10, 10, 10);
		List operations = TxUtil.operSetOption(type, threshold);
		String response = SignUtil.tx(operations, address2, fee,
				sequence_number2, metadata, pri1, pub1);
		int error_code = Result.getErrorCode(response);
		check.assertNotEquals(error_code, 0,
				"A的权重小于B的high_threshold，用B进行设置属性失败");
	}
	// 1.创建两个账户A\B
	// 2.设置A的权重为10（作为B的signer），B的high_threshold是9
	// 3.B做设置属性操作，用A签名，交易成功
//	@Test
	public void consign07() {
		int type = 4;
		String metadata = "1234";
		Map acc1 = TxUtil.createAccount(); // 创建第一个账户A
		Object address1 = acc1.get("address");
		String pri1 = acc1.get("private_key").toString();
		Object pub1 = acc1.get("public_key");
		long sequence_number1 = Result.seq_num(address1);

		Map acc2 = TxUtil.createAccount(10, 10, 10, 9, address1, 10); // 创建第二个账户B,并且设置了A作为B的signer
		Object address2 = acc2.get("address");
		String pri2 = acc2.get("private_key").toString();
		Object pub2 = acc2.get("public_key");
		long sequence_number2 = Result.seq_num(address2);

		JSONObject threshold = TxUtil.threshold(10, 10, 10, 10);
		List operations = TxUtil.operSetOption(type, threshold);
		String response = SignUtil.tx(operations, address2, fee,
				sequence_number2, metadata, pri1, pub1);
		int error_code = Result.getErrorCode(response);
		check.assertEquals(error_code, 0, "A的权重大于B的high_threshold，用B进行设置属性失败");
	}
	// 1.创建一个账户A，med_threshold门限是10
	// 2.创建两个账户B,C，权重和小于10
	// 3.发行资产（其他任意操作都可以），用B、C签名，交易失败
	public void consign08() {

		Map acc1 = TxUtil.createAccount(); // 创建第一个账户
		Object source_address = acc1.get("address");
		String pri = acc1.get("private_key").toString();
		Object pub = acc1.get("public_key");
		long sequence_number = Result.seq_num(source_address);

		Map acc2 = TxUtil.createAccount(10, 10, 10, 10); // 创建第二个账户
		Object address2 = acc2.get("address");
		Object pri2 = acc2.get("private_key");
		Object pub2 = acc2.get("public_key");
		int weight2 = 4;

		Map acc3 = TxUtil.createAccount(10, 10, 10, 10); // 创建第三个账户
		Object address3 = acc3.get("address");
		Object pri3 = acc3.get("private_key").toString();
		Object pub3 = acc3.get("public_key");
		int weight3 = 5;

		String metadata = "1234";
		int type = 4;

		JSONObject threshold = TxUtil.threshold(10, 10, 10, 10);
		List signers = TxUtil.signers(address3, weight3, address2, weight2);
		List operations = TxUtil.operSetOption(type, threshold, signers);
		String response = SignUtil.tx(operations, source_address, fee,
				sequence_number, metadata, pri, pub);
		int error_code = Result.getErrorCode(response);
		check.assertEquals(error_code, 0, "设置属性失败");

		if (error_code == 0) {
			int typeaction = 2;
			int asset_type = 1;
			Object asset_issuer = source_address;
			String asset_code = "abc";
			int asset_amount = 100;
			sequence_number = Result.seq_num(source_address);
			// 开始联合签名
			List opers = TxUtil.operIssue(typeaction, asset_type, asset_issuer,
					asset_code, asset_amount);
			JSONObject tran = TxUtil.tran_json(source_address, fee,
					sequence_number, metadata, opers);
			String tran_blob = TxUtil.getBlob(tran);
			JSONObject s2 = TxUtil.signature(tran, pri2, pub2);
			JSONObject s3 = TxUtil.signature(tran, pri3, pub3);
			List signatures = TxUtil.signatures(s2, s3);
			List items = TxUtil.itemlist(signatures, tran_blob);
			String result = TxUtil.tx_cosign(items);

			int err_code = Result.getErrorCode(result);
			check.assertNotEquals(err_code, 0, "联合账户权重和小于源账户门限值，校验失败");
		}

	}
	// 1.创建一个账户A，threshold=20,master_weight=10
	@SuppressWarnings("rawtypes")
	// 2.signerB的weigh=10
	// 3.A发起交易，联合签名交易成功
//	 @Test
	public void consign09() {
		int type = 4;
		String metadata = "1234";
		// Object master_weight, Object med_threshold,
		// Object low_threshold, Object high_threshold
		Map acc1 = TxUtil.createAccount(); // 创建第二个账户B
		Object source_address = acc1.get("address");
		String pri = acc1.get("private_key").toString();
		Object pub = acc1.get("public_key");
		long sequence_number = Result.seq_num(source_address);

		Map acc2 = TxUtil.createAccount(10, 20, 20, 20); // 创建第一个账户A
		Object address2 = acc2.get("address");
		Object pri2 = acc2.get("private_key");
		Object pub2 = acc2.get("public_key");
		int weight2 = 10;
		long sequence_number2 = Result.seq_num(source_address);

		JSONObject threshold = TxUtil.threshold(10, 10, 10, 10);
		List signers = TxUtil.signers(address2, weight2);
		List operations = TxUtil.operSetOption(type, threshold, signers);
		String response = SignUtil.tx(operations, source_address, fee,
				sequence_number, metadata, pri, pub);
		int error_code = Result.getErrorCode(response);
		check.equals(error_code, 0, "设置属性失败");
		if (error_code == 0) {
			int typeaction = 2;
			int asset_type = 1;
			Object asset_issuer = source_address;
			String asset_code = "abc";
			int asset_amount = 100;
			sequence_number2 = Result.seq_num(source_address);
			// 开始联合签名
			List opers = TxUtil.operIssue(typeaction, asset_type, asset_issuer, // B账户发起交易，使用A账户签名
					asset_code, asset_amount);
			JSONObject tran = TxUtil.tran_json(source_address, fee,
					sequence_number2, metadata, opers);
			String tran_blob = TxUtil.getBlob(tran);
			JSONObject s1 = TxUtil.signature(tran, pri, pub);
			JSONObject s2 = TxUtil.signature(tran, pri2, pub2);
			List signatures = TxUtil.signatures(s2, s1);
			List items = TxUtil.itemlist(signatures, tran_blob);
			String result = TxUtil.tx_cosign(items);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 0, "AB联合签名，AB的权重和等于A的门限的交易，验证失败");
		}
	}
	// 1.创建一个账户A，threshold=20,master_weight=9
	// 2.signerB的weigh=10
	// 3.A发起交易，联合签名交易成功
//	 @Test
	public void consign10() {
		int type = 4;
		String metadata = "1234";
		Map acc1 = TxUtil.createAccount(); // 创建第一个账户A
		Object source_address = acc1.get("address");
		System.out.println("source_address: " + source_address);
		String pri = acc1.get("private_key").toString();
		Object pub = acc1.get("public_key");
		long sequence_number = Result.seq_num(source_address);

		Map acc2 = TxUtil.createAccount(); // 创建第二个账户B
		Object address2 = acc2.get("address");
		System.out.println("address2: " + address2);
		Object pri2 = acc2.get("private_key");
		Object pub2 = acc2.get("public_key");
		int weight2 = 10;
		long sequence_number2 = Result.seq_num(source_address);

		JSONObject threshold = TxUtil.threshold(9, 20, 20, 20); // 设置第一个账户的门限和权重
		List signers = TxUtil.signers(address2, weight2);
		List operations = TxUtil.operSetOption(type, threshold, signers);
		String response = SignUtil.tx(operations, source_address, fee,
				sequence_number, metadata, pri, pub);
		int error_code = Result.getErrorCode(response);
		check.equals(error_code, 0, "设置属性失败");
		if (error_code == 0) {
			int typeaction = 2;
			int asset_type = 1;
			Object asset_issuer = source_address;
			String asset_code = "abc";
			int asset_amount = 100;
			sequence_number2 = Result.seq_num(source_address);
			// 开始联合签名
			List opers = TxUtil.operIssue(typeaction, asset_type, asset_issuer, // B账户发起交易，使用A账户签名
					asset_code, asset_amount);
			JSONObject tran = TxUtil.tran_json(source_address, fee,
					sequence_number2, metadata, opers);
			String tran_blob = TxUtil.getBlob(tran);
			JSONObject s1 = TxUtil.signature(tran, pri, pub);
			JSONObject s2 = TxUtil.signature(tran, pri2, pub2);
			List signatures = TxUtil.signatures(s2, s1);
			List items = TxUtil.itemlist(signatures, tran_blob);
			String result = TxUtil.tx_cosign(items);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 0, "AB联合签名，AB的权重和小于A的门限的交易，验证失败");
		}
	}
	// 1.创建一个账户A，threshold=20,master_weight=10
	// 2.signerB的weigh=11
	// 3.A发起交易，联合签名交易成功
//	 @Test
	public void consign11() {
		int type = 4;
		String metadata = "1234";
		Map acc1 = TxUtil.createAccount(); // 创建第一个账户
		Object source_address = acc1.get("address");
		String pri = acc1.get("private_key").toString();
		Object pub = acc1.get("public_key");
		long sequence_number = Result.seq_num(source_address);

		Map acc2 = TxUtil.createAccount(20, 20, 20, 20); // 创建第二个账户
		Object address2 = acc2.get("address");
		Object pri2 = acc2.get("private_key");
		Object pub2 = acc2.get("public_key");
		int weight2 = 11;
		long sequence_number2 = Result.seq_num(source_address);

		JSONObject threshold = TxUtil.threshold(10, 20, 20, 20);
		List signers = TxUtil.signers(address2, weight2);
		List operations = TxUtil.operSetOption(type, threshold, signers);
		String response = SignUtil.tx(operations, source_address, fee,
				sequence_number, metadata, pri, pub);
		int error_code = Result.getErrorCode(response);
		check.equals(error_code, 0, "设置属性失败");
		if (error_code == 0) {
			int typeaction = 2;
			int asset_type = 1;
			Object asset_issuer = source_address;
			String asset_code = "abc";
			int asset_amount = 100;
			sequence_number2 = Result.seq_num(source_address);
			// 开始联合签名
			List opers = TxUtil.operIssue(typeaction, asset_type, asset_issuer, // B账户发起交易，使用A账户签名
					asset_code, asset_amount);
			JSONObject tran = TxUtil.tran_json(source_address, fee,
					sequence_number2, metadata, opers);
			String tran_blob = TxUtil.getBlob(tran);
			JSONObject s1 = TxUtil.signature(tran, pri, pub);
			JSONObject s2 = TxUtil.signature(tran, pri2, pub2);
			List signatures = TxUtil.signatures(s2, s1);
			List items = TxUtil.itemlist(signatures, tran_blob);
			String result = TxUtil.tx_cosign(items);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 0, "AB联合签名，AB的权重和大于A的门限的交易，验证失败");
			check.result("Cosignature校验成功");
		}
	}

	/**
	 * A设置门限是20了signer B（w=9),C(w=10),权重和小于A的门限 B,C进行联合签名，签名失败
	 */
	// @Test
	public void consign12() {
		int type = 4;
		String metadata = "1234";
		Map acc1 = TxUtil.createAccount(); // 创建第一个账户A
		Object source_address = acc1.get("address");
		// System.out.println("source_address: " + source_address);
		String pri = acc1.get("private_key").toString();
		Object pub = acc1.get("public_key");
		// long sequence_number = Result.seq_num(source_address);

		Map acc2 = TxUtil.createAccount(); // 创建第二个账户B
		Object address2 = acc2.get("address");
		// System.out.println("address2: " + address2);
		Object pri2 = acc2.get("private_key");
		Object pub2 = acc2.get("public_key");
		int weight2 = 9;

		Map acc3 = TxUtil.createAccount(); // 创建第三个账户C
		Object address3 = acc3.get("address");
		// System.out.println("address3: " + address2);
		Object pri3 = acc3.get("private_key");
		Object pub3 = acc3.get("public_key");
		int weight3 = 10;

		long sequence_number = Result.seq_num(source_address);

		JSONObject threshold = TxUtil.threshold(9, 20, 20, 20); // 设置第一个账户的门限和权重
		List signers = TxUtil.signers(address2, weight2, address3, weight3);
		List operations = TxUtil.operSetOption(type, threshold, signers);
		String response = SignUtil.tx(operations, source_address, fee,
				sequence_number, metadata, pri, pub);
		int error_code = Result.getErrorCode(response);
		check.equals(error_code, 0, "设置属性失败");
		if (error_code == 0) {
			int typeaction = 2;
			int asset_type = 1;
			Object asset_issuer = source_address;
			String asset_code = "abc";
			int asset_amount = 100;
			sequence_number = Result.seq_num(source_address);
			// 开始联合签名
			List opers = TxUtil.operIssue(typeaction, asset_type, asset_issuer, // B账户发起交易，使用A账户签名
					asset_code, asset_amount);
			JSONObject tran = TxUtil.tran_json(source_address, fee,
					sequence_number, metadata, opers);
			String tran_blob = TxUtil.getBlob(tran);
			JSONObject s1 = TxUtil.signature(tran, pri3, pub3);
			JSONObject s2 = TxUtil.signature(tran, pri2, pub2);
			List signatures = TxUtil.signatures(s2, s1);
			List items = TxUtil.itemlist(signatures, tran_blob);
			String result = TxUtil.tx_cosign(items);
			int err_code = Result.getErrorCode(result);
			check.assertNotEquals(err_code, 0,
					"A发起交易由B,C联合签名，BC的权重和小于A的门限的交易，验证失败");
		}
	}

	/**
	 * A设置门限是20了signer B（w=10),C(w=10),权重和等于A的门限 B,C进行联合签名，签名成功
	 */
//	@Test
	public void consign13() {
		int type = 4;
		String metadata = "1234";
		Map acc1 = TxUtil.createAccount(); // 创建第一个账户A
		Object source_address = acc1.get("address");
		// System.out.println("source_address: " + source_address);
		String pri = acc1.get("private_key").toString();
		Object pub = acc1.get("public_key");
		// long sequence_number = Result.seq_num(source_address);

		Map acc2 = TxUtil.createAccount(); // 创建第二个账户B
		Object address2 = acc2.get("address");
		// System.out.println("address2: " + address2);
		Object pri2 = acc2.get("private_key");
		Object pub2 = acc2.get("public_key");
		int weight2 = 10;

		Map acc3 = TxUtil.createAccount(); // 创建第三个账户C
		Object address3 = acc3.get("address");
		// System.out.println("address3: " + address2);
		Object pri3 = acc3.get("private_key");
		Object pub3 = acc3.get("public_key");
		int weight3 = 10;

		long sequence_number = Result.seq_num(source_address);

		JSONObject threshold = TxUtil.threshold(9, 20, 20, 20); // 设置第一个账户的门限和权重
		List signers = TxUtil.signers(address2, weight2, address3, weight3);
		List operations = TxUtil.operSetOption(type, threshold, signers);
		String response = SignUtil.tx(operations, source_address, fee,
				sequence_number, metadata, pri, pub);
		int error_code = Result.getErrorCode(response);
		check.equals(error_code, 0, "设置属性失败");
		if (error_code == 0) {
			int typeaction = 2;
			int asset_type = 1;
			Object asset_issuer = source_address;
			String asset_code = "abc";
			int asset_amount = 100;
			sequence_number = Result.seq_num(source_address);
			// 开始联合签名
			List opers = TxUtil.operIssue(typeaction, asset_type, asset_issuer, // B账户发起交易，使用A账户签名
					asset_code, asset_amount);
			JSONObject tran = TxUtil.tran_json(source_address, fee,
					sequence_number, metadata, opers);
			String tran_blob = TxUtil.getBlob(tran);
			JSONObject s1 = TxUtil.signature(tran, pri3, pub3);
			JSONObject s2 = TxUtil.signature(tran, pri2, pub2);
			List signatures = TxUtil.signatures(s2, s1);
			List items = TxUtil.itemlist(signatures, tran_blob);
			String result = TxUtil.tx_cosign(items);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 0,
					"A发起交易由B,C联合签名，BC的权重和等于A的门限的交易，验证失败");
		}
	}

	/**
	 * A设置门限是20了signer B（w=11),C(w=10),权重和大于A的门限 B,C进行联合签名，签名成功
	 */
//	@Test
	public void consign14() {
		int type = 4;
		String metadata = "1234";
		Map acc1 = TxUtil.createAccount(); // 创建第一个账户A
		Object source_address = acc1.get("address");
		// System.out.println("source_address: " + source_address);
		String pri = acc1.get("private_key").toString();
		Object pub = acc1.get("public_key");
		// long sequence_number = Result.seq_num(source_address);

		Map acc2 = TxUtil.createAccount(); // 创建第二个账户B
		Object address2 = acc2.get("address");
		// System.out.println("address2: " + address2);
		Object pri2 = acc2.get("private_key");
		Object pub2 = acc2.get("public_key");
		int weight2 = 10;

		Map acc3 = TxUtil.createAccount(); // 创建第三个账户C
		Object address3 = acc3.get("address");
		// System.out.println("address3: " + address2);
		Object pri3 = acc3.get("private_key");
		Object pub3 = acc3.get("public_key");
		int weight3 = 10;

		long sequence_number = Result.seq_num(source_address);

		JSONObject threshold = TxUtil.threshold(9, 20, 20, 20); // 设置第一个账户的门限和权重
		List signers = TxUtil.signers(address2, weight2, address3, weight3);
		List operations = TxUtil.operSetOption(type, threshold, signers);
		String response = SignUtil.tx(operations, source_address, fee,
				sequence_number, metadata, pri, pub);
		int error_code = Result.getErrorCode(response);
		check.equals(error_code, 0, "设置属性失败");
		if (error_code == 0) {
			int typeaction = 2;
			int asset_type = 1;
			Object asset_issuer = source_address;
			String asset_code = "abc";
			int asset_amount = 100;
			sequence_number = Result.seq_num(source_address);
			// 开始联合签名
			List opers = TxUtil.operIssue(typeaction, asset_type, asset_issuer, // B账户发起交易，使用A账户签名
					asset_code, asset_amount);
			JSONObject tran = TxUtil.tran_json(source_address, fee,
					sequence_number, metadata, opers);
			String tran_blob = TxUtil.getBlob(tran);
			JSONObject s1 = TxUtil.signature(tran, pri3, pub3);
			JSONObject s2 = TxUtil.signature(tran, pri2, pub2);
			List signatures = TxUtil.signatures(s2, s1);
			List items = TxUtil.itemlist(signatures, tran_blob);
			String result = TxUtil.tx_cosign(items);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 0,
					"A发起交易由B,C联合签名，BC的权重和等于A的门限的交易，验证失败");
		}
	}
}
