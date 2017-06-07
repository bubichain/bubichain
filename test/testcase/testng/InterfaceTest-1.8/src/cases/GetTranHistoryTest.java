package cases;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import net.sf.json.JSONObject;

import org.testng.annotations.Test;

import utils.APIUtil;
import utils.HttpUtil;
import utils.Result;
import utils.SignUtil;
import utils.TxUtil;
import base.Log;
import base.TestBase;

@Test
public class GetTranHistoryTest extends TestBase{

	String transaction = "getTransactionHistory";

	// @Test
	//通过hash查询交易记录，验证error_code=0
	public void hashCheckAfterIssue(){
		int type = 2;
		int asset_type = 1;
		Map acc = TxUtil.createAccount();
		Object address = acc.get("address");
		String pri = acc.get("private_key").toString();
		Object pub = acc.get("public_key");
		// System.out.println("pri=" + pri);
		// System.out.println("pub=" + pub);
		Object asset_issuer = address;
		String asset_code = "abc" ;
		int asset_amount = 100;
		Object source_address = address;
		long sequence_number = Result.seq_num(address);
		String metadata = "abcd";
		//先做一笔发行资产，通过hash去查询交易记录
		List opers = TxUtil.operIssue(type, asset_type, asset_issuer, asset_code, asset_amount);		//ledger发行未初始化资产
		String result = SignUtil.tx(opers, source_address, fee, sequence_number, metadata, pri, pub);
//		System.out.println("rrrrrrrr"+result);
		String hash = Result.getHash(result);
		int err_code = Result.getErrorCode(result);
		check.assertEquals(err_code, 0, "资产发行失败");
		
		//验证交易记录中的operation
		
	}
//@Test
	//验证获取交易记录时，start字段非法值
	public void startinValidCheck(){
		String url = "getTransactionHistory";
		String key = "start";
		String value = "abc";
		String result = HttpUtil.doget(url, key, value);
		int err_code = Result.getErrorCode(result);
		check.assertEquals(err_code, 2,"start值非法校验失败");
	}
//@Test
	public void startValidCheck(){
		String url = "getTransactionHistory";
		String key = "start";
		String value = "1";
		String result = HttpUtil.doget(url, key, value);
		int err_code = Result.getErrorCode(result);
		check.assertEquals(err_code, 0,"start为1时校验失败");
	}
//	@Test
	public void limitinValidCheck(){
		String url = "getTransactionHistory";
		String key = "limit";
		String value = "abc";
		String result = HttpUtil.doget(url, key, value);
		int err_code = Result.getErrorCode(result);
		check.assertEquals(err_code, 2,"limit值非法时校验失败");
	}
//	@Test
	public void limitValidCheck(){
		String url = "getTransactionHistory";
		String key = "limit";
		int value = 3;
		String result = HttpUtil.doget(url, key, value);
		int err_code = Result.getErrorCode(result);
		check.equals(err_code, 0,"limit值为3时校验失败");
		
		int total_count = Result.getTotalCount(result);
		if (total_count>value) {
			int tran_size = Result.getTranSize(result);
			check.assertEquals(tran_size, value,"设定的limit为[" + value+"]时，查询结果有误");
		}else{
			value = 1;
			String result_ = HttpUtil.doget(url, key, value);
			int err_code_ = Result.getErrorCode(result_);
			check.assertEquals(err_code_, 0,"设定的limit为[" + value+"]时，查询结果有误");
		}
		
	}
//	@Test
	public void ledger_seqValidCheck(){
		//先做资产发行，再获取交易记录中的ledger_seq,然后使用此ledger_seq进行查询
		int type = 2;
		int asset_type = 1;
		long sequence_number = Result.seq_num(led_acc);
		List opers = TxUtil.operIssue(type, asset_type, led_acc, "abc", 10);		//ledger发行未初始化资产
		String result = SignUtil.tx(opers, led_acc, fee, sequence_number, "1234", led_pri, led_pub);
		int err_code = Result.getErrorCode(result);
		if (err_code==0) {
			String hash = Result.getHash(result);
			String response = HttpUtil.doget("getTransactionHistory", "hash", hash);
			String ledger_seq = Result.getLed_seqInTranHistory(response);
			
			String re = HttpUtil.doget("getTransactionHistory", "ledger_seq", ledger_seq);
			int err_code_ = Result.getErrorCode(re);
			check.assertEquals(err_code_, 0, "通过ledger_seq查询交易记录失败");
		}else {
			Log.info("资产发行失败，无法获取ledger_seq，不能进行ledger_seq查询交易记录操作");
		}
	}
//	@Test
//	public void ledger_seqinValidCheck(){
//		String url = "getTransactionHistory";
//		String key = "ledger_seq";
//		String value = "a*";
//		String result = HttpUtil.doget(url, key, value);
//		int err_code = Result.getErrorCode(result);
//		check.assertEquals(err_code, 2,"ledger_seq值为["+value+"]时校验失败");
//	}
//	@Test
	public void begin_timeValidCheck(){
		String url = "getTransactionHistory";
		String key = "begin_time";
//		long value = System.currentTimeMillis()*1000;
//		System.out.println(value);
		String value = "1481519617434068";
		String result = HttpUtil.doget(url, key, value);
		int err_code = Result.getErrorCode(result);
		check.assertEquals(err_code, 0,"begin_time值为["+value+"]时校验失败");
	}
	
//	@Test
	public void end_timeValidCheck(){
		String url = "getTransactionHistory";
		String key = "end_time";
		long value = System.currentTimeMillis()*1000-10000000;
		String result = HttpUtil.doget(url, key, value);
		int err_code = Result.getErrorCode(result);
		check.assertEquals(err_code, 0,"end_time值为["+value+"]时校验失败");
	}
//	@Test
	public void addressValidCheck(){
		String url = "getTransactionHistory";
		String key = "address";
		String value = led_acc;
		String result = HttpUtil.doget(url, key, led_acc);
		int err_code = Result.getErrorCode(result);
		check.assertEquals(err_code, 0,"address值为["+value+"]时校验失败");
	}

	// @Test
	//创建账户成功后，查询交易中operation里的所有字段
	public void createAccount(){
		Object type = 0;
		int init_balance = 200000;
		String account_metadata = "abcd";
		String metadata = "abcd";
		
		Map acc = TxUtil.createAccount(); // 创建第一个账户
		Object address = acc.get("address");
		String pri = acc.get("private_key").toString();
		Object pub = acc.get("public_key");
		Object source_address = address;
		Object dest_add = APIUtil.generateAddress(); // 生成一个目标账户
		//通过账户地址获取balance
		String balance1 = Result.getBalanceInAcc(source_address);
		long sequence_number = Result.seq_num(address);

		List opers = TxUtil.operCreateAccount(type, dest_add, init_balance, account_metadata);	//ledger发行未初始化资产
		String result = SignUtil.tx(opers, source_address, fee, sequence_number, metadata, pri, pub);
		int err_code = Result.getErrorCode(result);
		check.equals(err_code, 0, "创建账户失败");
		
		//交易成功后获取hash
		String hash = Result.getHash(result);
		// System.out.println("hash="+hash);
		String re = Result.getResult(transaction, "hash", hash);
		int err = Result.getErrorCode(re);
		check.equals(err,0,"交易记录返回error_code不等于0");
		
		String dest_addr = Result.getOperthInTranHistory(re, "dest_address");
		String init_br = Result.getOperthInTranHistory(re, "init_balance");
		String feer = Result.getTranthInTranHistory(re, "fee");
		check.equals(feer, String.valueOf(fee), "交易成功后手续费不一致");
		check.equals(err, 0, "交易成功后，通过hash查询交易记录失败");
		check.equals(dest_addr, dest_add.toString(), "交易成功后，查询的目标账户地址不一致");
		check.equals(init_br, String.valueOf(init_balance), "交易成功后，查询的init_balance不一致");
		
		//获取交易记录中的ledger_seq,再获取账户的previous_ledger_seq,验证两个值是否一致
//		String led_seqr = Result.getTranthInTranHistory(re, "ledger_seq");
//		String led_seqa = Result.getPre_ledger_seqFromAddress(source_address);
//		check.equals(led_seqr, led_seqa, "交易成功后，交易记录中的ledger_seq与源账户的previous_ledger_seq不一致");
		
		//获取交易记录中operations里的type,查看值是否和提交的type一致
		String typer = Result.getOperthInTranHistory(re, "type");
		check.assertEquals(typer, type.toString(), "交易成功后，交易记录中的type与创建时不一致");
		
		// 创始账号余额太大，不能取成int或long
		// int ba = Integer.valueOf(balance1) - fee;
		// String balancer = Result.getAccBalance(source_address);
		// check.equals(balancer, String.valueOf(ba), "交易成功后，源账户余额不正确");
		
	}

	 @Test
	//转账成功后，查询交易记录中operation里的所有字段
	public void transfer(){
		Map acc = TxUtil.createAccount();
		Object address = acc.get("address");
		String d_balan = Result.getBalanceInAcc(address);
		Map acc1 = TxUtil.createAccount();
		Object source_address = acc1.get("address");
		String pri = acc1.get("private_key").toString();
		Object pub = acc1.get("public_key");
		String balance1 = Result.getBalanceInAcc(source_address);
		Object type = 1;
		int asset_type = 0;
		Object dest_address = address;
		int asset_amount = 10;
		Object asset_issuer = source_address;
		Object asset_code = "abcd";
		long sequence_number = Result.seq_num(source_address);
		String metadata = "1234";
		List opers = TxUtil.opertransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code);
		String result = SignUtil.tx(opers, source_address, fee, sequence_number, metadata, pri, pub);
		int err_code = Result.getErrorCode(result);
		check.equals(err_code,0,"转账失败");
		
		//交易成功后获取hash
		String hash = Result.getHash(result);
		// System.out.println("hash="+hash);
		String re = Result.getResult(transaction, "hash", hash);
		int err = Result.getErrorCode(re);
		check.equals(err, 0, "交易成功后，通过hash查询交易记录失败");
		String feer = Result.getTranthInTranHistory(re, "fee");
		check.equals(feer, String.valueOf(fee), "交易成功后手续费不一致");
		String dest_addr = Result.getOperthInTranHistory(re, "dest_address");
		check.equals(dest_addr, dest_address.toString(), "交易成功后，查询的目标账户地址不一致");
		
		//获取交易记录中的ledger_seq,再获取账户的previous_ledger_seq,验证两个值是否一致
		String led_seqr = Result.getTranthInTranHistory(re, "ledger_seq");
		String led_seqa = Result.getPre_ledger_seqFromAddress(source_address);
		check.equals(led_seqr, led_seqa, "交易成功后，交易记录中的ledger_seq与源账户的previous_ledger_seq不一致");
		
		//获取交易记录中operations里的type,查看值是否和提交的type一致
		String typer = Result.getOperthInTranHistory(re, "type");
		check.equals(typer, type.toString(), "交易成功后，交易记录中的type与创建时不一致");
		
		long ba = Integer.valueOf(balance1) - asset_amount - fee;
		String balancer = Result.getBalanceInAcc(source_address);
		check.equals(balancer, String.valueOf(ba), "交易成功后，源账户余额不正确");
		
		//获取交易记录中的asset_mount
		String asset_r = Result.getOperthInTranHistory(re, "asset_amount");
		check.equals(Integer.valueOf(asset_r), asset_amount, "交易成功后，asset_mount不一致");
		
		//获取目标账户的余额，验证是否和amount一致
		String d_balanr = Result.getBalanceInAcc(dest_address);
		int d_b = Integer.valueOf(d_balan)+asset_amount;
		check.assertEquals(d_balanr, String.valueOf(d_b), "交易成功后，目标账户余额错误");

	}

	// @Test
	//初始化转账成功后，查询交易记录中operation里的所有字段
	public void initTransfer(){
		Map acc = TxUtil.createAccount(); // 创建一个目标账户
		Object dest_address = acc.get("address");
		Map acc1 = TxUtil.createAccount(); // 创建一个源账户
		Object source_address = acc1.get("address");
		String pri = acc1.get("private_key").toString();
		Object pub = acc1.get("public_key");

		int type_ = 2;
		int asset_type_ = 1;
		Object type = 5;
		Object asset_type = 1;
		Object asset_code = "abc";
		Object asset_issuer = source_address;
		int asset_amount = 10;
		String metadata = "abcd";
		long sequence_number = Result.seq_num(source_address);
		List opers_ = TxUtil.operIssue(type_, asset_type_, source_address,
				asset_code, asset_amount);
		String result_ = SignUtil.tx(opers_, source_address, fee,
				sequence_number, metadata, pri, pub);
		int err_code1 = Result.getErrorCode(result_);
		check.equals(err_code1, 0, "资产发行失败");

		String balance1 = Result.getBalanceInAcc(source_address);
		sequence_number = Result.seq_num(source_address);
		List opers = TxUtil.operInitTransfer(type, asset_type, dest_address, asset_amount, asset_issuer, asset_code);
		String result = SignUtil.tx(opers, source_address, fee, sequence_number, metadata, pri, pub);
		int err_code = Result.getErrorCode(result);
		check.equals(err_code, 0,"初始化转账失败");
		
		String hash = Result.getHash(result);
		// System.out.println("hash="+hash);
		String re = Result.getResult(transaction, "hash", hash);
		 int err = Result.getErrorCode(re);
		 check.equals(err, 0, "交易成功后，通过hash查询交易记录失败");
		 String feer = Result.getTranthInTranHistory(re, "fee");
		 check.equals(feer, String.valueOf(fee), "交易成功后手续费不一致");
		 String dest_addr = Result.getOperthInTranHistory(re, "dest_address");
		check.equals(dest_addr, dest_address.toString(), "交易成功后，查询的目标账户地址不一致");
		
		 //获取交易记录中的ledger_seq,再获取账户的previous_ledger_seq,验证两个值是否一致
		 String led_seqr = Result.getTranthInTranHistory(re, "ledger_seq");
		 String led_seqa =
		 Result.getPre_ledger_seqFromAddress(source_address);
		 check.equals(led_seqr, led_seqa,
		 "交易成功后，交易记录中的ledger_seq与源账户的previous_ledger_seq不一致");
		
		 //获取交易记录中operations里的type,查看值是否和提交的type一致
		 String typer = Result.getOperthInTranHistory(re, "type");
		 check.equals(typer, type.toString(), "交易成功后，交易记录中的type与创建时不一致");
		
		long ba = Integer.valueOf(balance1) - fee;
		String balancer = Result.getBalanceInAcc(source_address);
		check.equals(balancer, String.valueOf(ba), "交易成功后，源账户余额不正确");
		
		//获取交易记录中的asset_mount
		String asset_r = Result.getOperthInTranHistory(re, "asset_amount");
		check.equals(Integer.valueOf(asset_r), asset_amount, "交易成功后，asset_mount不一致");
		
		//获取交易记录中的asset_code
		String asset_cr = Result.getOperthInTranHistory(re, "asset_code");
		check.assertEquals(asset_cr, asset_code.toString(),
				"交易成功后，asset_code不一致");
	}
	
	// @Test
	//发行资产成功后，查询交易记录中operation里的所有字段
	public void issue(){
		Object type = 2;
		int asset_type = 1;
		Map acc = TxUtil.createAccount();
		Object address = acc.get("address");
		String pri = acc.get("private_key").toString();
		Object pub = acc.get("public_key");
		
		Object asset_issuer = address;
		String asset_code = "abc" ;
		int asset_amount = 100;
		Object source_address = address;
		String balance1 = Result.getBalanceInAcc(source_address);
		long sequence_number = Result.seq_num(address);
		String metadata = "abcd";
		List opers = TxUtil.operIssue(type, asset_type, asset_issuer, asset_code, asset_amount);		//ledger发行未初始化资产
		String result = SignUtil.tx(opers, source_address, fee, sequence_number, metadata, pri, pub);
		int err_code = Result.getErrorCode(result);
		check.equals(err_code, 0, "资产发行失败");
		
		String hash = Result.getHash(result);
		// System.out.println("hash="+hash);
		String re = Result.getResult(transaction, "hash", hash);
		int err = Result.getErrorCode(re);
		check.equals(err, 0, "交易成功后，通过hash查询交易记录失败");
		String feer = Result.getTranthInTranHistory(re, "fee");
		check.equals(feer, String.valueOf(fee), "交易成功后手续费不一致");
		
		//获取交易记录中的ledger_seq,再获取账户的previous_ledger_seq,验证两个值是否一致
		String led_seqr = Result.getTranthInTranHistory(re, "ledger_seq");
		String led_seqa = Result.getPre_ledger_seqFromAddress(source_address);
		check.equals(led_seqr, led_seqa, "交易成功后，交易记录中的ledger_seq与源账户的previous_ledger_seq不一致");
		
		//获取交易记录中operations里的type,查看值是否和提交的type一致
		String typer = Result.getOperthInTranHistory(re, "type");
		check.equals(typer, type.toString(), "交易成功后，交易记录中的type与创建时不一致");
		
		long ba = Integer.valueOf(balance1) - fee;
		String balancer = Result.getBalanceInAcc(source_address);
		check.equals(balancer, String.valueOf(ba), "交易成功后，源账户余额不正确");
		
		//获取交易记录中的asset_mount
		String asset_r = Result.getOperthInTranHistory(re, "asset_amount");
		check.equals(Integer.valueOf(asset_r), asset_amount, "交易成功后，asset_mount不一致");
		
		//获取交易记录中的asset_code
		String asset_cr = Result.getOperthInTranHistory(re, "asset_code");
		check.equals(asset_cr, asset_code, "交易成功后，asset_code不一致");
		
		String asset_issuerr = Result.getOperthInTranHistory(re, "asset_issuer");
		check.assertEquals(asset_issuerr, asset_issuer.toString(),
				"资产发行的asset_issuer不一致");
		
		//如果发行的有details，就校验details里的值
	}

	// @Test
	//发行唯一资产成功后，查询交易记录中operation里的所有字段
//	public void uniIssue(){
//		Object type = 7;
//		@SuppressWarnings("rawtypes")
//		Map acc = TxUtil.createAccount();
//		Object source_address = acc.get("address");
//		String pri = acc.get("private_key").toString();
//		Object pub = acc.get("public_key");
//		String balance1 = Result.getBalanceInAcc(source_address);
//		String asset_code = "abc" ;
//		Object asset_issuer = source_address;
//		String asset_detailed = "1234";
//		long sequence_number = Result.seq_num(source_address);
//		String metadata = "1234";
//		List opers = TxUtil.operUniIssue(type, asset_issuer, asset_code, asset_detailed);
//		String result = SignUtil.tx(opers, source_address, fee, sequence_number, metadata, pri, pub);
//		int err_code = Result.getErrorCode(result);
//		check.equals(err_code,0,"发行唯一资产失败");
//		
//		String hash = Result.getHash(result);
//		// System.out.println("hash="+hash);
//		String re = Result.getResult(transaction, "hash", hash);
//		int err = Result.getErrorCode(re);
//		check.equals(err, 0, "交易成功后，通过hash查询交易记录失败");
//		String feer = Result.getTranthInTranHistory(re, "fee");
//		check.equals(feer, String.valueOf(fee), "交易成功后手续费不一致");
//		
//		//获取交易记录中的ledger_seq,再获取账户的previous_ledger_seq,验证两个值是否一致
//		String led_seqr = Result.getTranthInTranHistory(re, "ledger_seq");
//		String led_seqa = Result.getPre_ledger_seqFromAddress(source_address);
//		check.equals(led_seqr, led_seqa, "交易成功后，交易记录中的ledger_seq与源账户的previous_ledger_seq不一致");
//		
//		//获取交易记录中operations里的type,查看值是否和提交的type一致
//		String typer = Result.getOperthInTranHistory(re, "type");
//		check.equals(typer, type.toString(), "交易成功后，交易记录中的type与创建时不一致");
//		
//		long ba = Integer.valueOf(balance1) - fee;
//		String balancer = Result.getBalanceInAcc(source_address);
//		check.equals(balancer, String.valueOf(ba), "交易成功后，源账户余额不正确");
//		
//		//获取交易记录中的asset_code
//		String asset_cr = Result.getOperthInTranHistory(re, "asset_code");
//		check.equals(asset_cr, asset_code, "交易成功后，asset_code不一致");
//		
//		//获取交易记录中的asset_issuer
//		String asset_issuerr = Result.getOperthInTranHistory(re, "asset_issuer");
//		check.equals(asset_issuerr, asset_issuer.toString(),
//				"资产发行的asset_issuer不一致");
//		
//		//获取交易记录中的asset_detailed
//		String asset_detailedr = Result.getOperthInTranHistory(re, "asset_detailed");
//		check.equals(asset_detailedr, asset_detailed, "资产发行的asset_detailed不一致");
//		
//		String seq_numr = Result.getTranthInTranHistory(re, "sequence_number");
//		String seq_numa = Result.getledger_seqFromAddress(source_address);
//		check.assertEquals(seq_numr, seq_numa, "sequence_number与源账户当前的不一致");
//	}
//		
//	 @Test
//	//转移唯一资产成功后，查询交易记录中operation里的所有字段
//	public void uniIssueTran(){
//		Object type = 8;
//		Map acc = TxUtil.createAccount(); // 先创建一个源账户
//		Object source_address = acc.get("address");
//		String pri = acc.get("private_key").toString();
//		Object pub = acc.get("public_key");
//
//		Object dest_address = TxUtil.createAccount().get("address"); // 再创建一个目标账户
//		String asset_code = "abcd";
//		Object asset_issuer = source_address;
//		int type_ = 7;
//		String asset_detailed = "1234";
//		String metadata = "1234";
//		long sequence_number_ = Result.seq_num(source_address);
//		List opers_ = TxUtil.operUniIssue(type_, asset_issuer, asset_code,
//				asset_detailed); // 先发行唯一资产
//		String result_ = SignUtil.tx(opers_, source_address, fee,
//				sequence_number_, metadata, pri, pub);
//		// 再做转移唯一资产
//		String balance1 = Result.getBalanceInAcc(source_address);
//		long sequence_number = sequence_number_ + 1;
//		// System.out.println(sequence_number);
//		List opers = TxUtil.operUniIssueTransfer(type, dest_address, asset_issuer, asset_code);
//		String result = SignUtil.tx(opers, source_address, fee, sequence_number, metadata, pri, pub);
//		int err_code = Result.getErrorCode(result);
//		check.equals(err_code,0,"转移唯一资产出错");
//		
//		String hash = Result.getHash(result);
//		 System.out.println("hash="+hash);
//		String re = Result.getResult(transaction, "hash", hash);
//		int err = Result.getErrorCode(re);
//		check.equals(err, 0, "交易成功后，通过hash查询交易记录失败");
//		String feer = Result.getTranthInTranHistory(re, "fee");
//		check.equals(feer, String.valueOf(fee), "交易成功后手续费不一致");
//		
//		//获取交易记录中的dest_address是否和请求里的一致
//		String dest_addr = Result.getOperthInTranHistory(re, "dest_address");
//		check.equals(dest_addr, dest_address.toString(), "交易成功后，查询的目标账户地址不一致");
//		
//		//获取交易记录中的ledger_seq,再获取账户的previous_ledger_seq,验证两个值是否一致
//		String led_seqr = Result.getTranthInTranHistory(re, "ledger_seq");
//		String led_seqa = Result.getPre_ledger_seqFromAddress(source_address);
//		check.equals(led_seqr, led_seqa, "交易成功后，交易记录中的ledger_seq与源账户的previous_ledger_seq不一致");
//		
//		//获取交易记录中operations里的type,查看值是否和提交的type一致
//		String typer = Result.getOperthInTranHistory(re, "type");
//		check.equals(typer, type.toString(), "交易成功后，交易记录中的type与创建时不一致");
//		
//		long ba = Integer.valueOf(balance1) - fee - fee;
//		String balancer = Result.getBalanceInAcc(source_address);
//		check.equals(balancer, String.valueOf(ba), "交易成功后，源账户余额不正确");
//		
//		//获取交易记录中的asset_code
//		String asset_cr = Result.getOperthInTranHistory(re, "asset_code");
//		check.equals(asset_cr, asset_code, "交易成功后，asset_code不一致");
//		
//		//获取交易记录中的asset_issuer
//		String asset_issuerr = Result.getOperthInTranHistory(re, "asset_issuer");
//		check.equals(asset_issuerr, asset_issuer.toString(),
//				"资产发行的asset_issuer不一致");
//		
//		//查询交易记录中的sequence_number
//		String seq_numr = Result.getTranthInTranHistory(re, "sequence_number");
//		String seq_numa = Result.getledger_seqFromAddress(source_address);
//		check.assertEquals(seq_numr, seq_numa, "sequence_number与源账户当前的不一致");
////		check.result("交易记录校验通过");
//		
//	}

	// @Test
	//存证创建成功后，查询交易记录中operation里的所有字段
	public void storage(){
		Object type = 9;
		Map acc = TxUtil.createAccount();
		Object source_address = acc.get("address");
		String pri = acc.get("private_key").toString();
		Object pub = acc.get("public_key");
		String balance1 = Result.getBalanceInAcc(source_address);
//		String s_address = ledger ;
//		String s_key = led_pri;
		String record_id = "123";
		String record_ext = "1234";
		long sequence_number = Result.seq_num(source_address);
		String metadata = "1234";
		List opers = TxUtil.operStorage(type, record_id, record_ext);
		String result = SignUtil.tx(opers, source_address, fee, sequence_number, metadata, pri, pub);
		int error_code = Result.getErrorCode(result);
		check.equals(error_code,0,"存证交易出错");
		
		String hash = Result.getHash(result);
		// System.out.println("hash="+hash);
		String re = Result.getResult(transaction, "hash", hash);
		int err = Result.getErrorCode(re);
		check.equals(err, 0, "交易成功后，通过hash查询交易记录失败");
		String feer = Result.getTranthInTranHistory(re, "fee");
		check.equals(feer, String.valueOf(fee), "交易成功后手续费不一致");
		
		//获取交易记录中的ledger_seq,再获取账户的previous_ledger_seq,验证两个值是否一致
		String led_seqr = Result.getTranthInTranHistory(re, "ledger_seq");
		String led_seqa = Result.getPre_ledger_seqFromAddress(source_address);
		check.equals(led_seqr, led_seqa, "交易成功后，交易记录中的ledger_seq与源账户的previous_ledger_seq不一致");
		
		//获取交易记录中operations里的type,查看值是否和提交的type一致
		String typer = Result.getOperthInTranHistory(re, "type");
		check.equals(typer, type.toString(), "交易成功后，交易记录中的type与创建时不一致");
		
		long ba = Integer.valueOf(balance1) - fee;
		String balancer = Result.getBalanceInAcc(source_address);
		check.equals(balancer, String.valueOf(ba), "交易成功后，源账户余额不正确");
		
		//获取operations里的record_id
		String record_idr = Result.getOperthInTranHistory(re, "record_id");
		check.equals(record_idr, record_id, "交易成功后，record_id不正确");
		
		//获取operations里的record_ext
		String record_extr = Result.getOperthInTranHistory(re, "record_ext");
		check.equals(record_extr, record_ext, "交易成功后，record_ext不正确");
		
		//查询交易记录中的sequence_number
		String seq_numr = Result.getTranthInTranHistory(re, "sequence_number");
		String seq_numa = Result.getledger_seqFromAddress(source_address);
		check.assertEquals(seq_numr, seq_numa, "sequence_number与源账户当前的不一致");
		
	}
		
	// @Test
	//设置账户属性成功后，查询交易记录中operation里的所有字段
	public void setOption(){
		Object master_weight = 2;
		Object low_threshold = 2;
		Object med_threshold = 2;
		Object high_threshold = 2;
		Object type = 4;
		Map acc = TxUtil.createAccount();
		Object source_address = acc.get("address");
		String pri = acc.get("private_key").toString();
		Object pub = acc.get("public_key");
		String balance1 = Result.getBalanceInAcc(source_address);
		Map acc1 = TxUtil.createAccount();
		Object s1_address = acc1.get("address");
		Map acc2 = TxUtil.createAccount();
		Object s2_address = acc2.get("address");
		long sequence_number = Result.seq_num(source_address);
		String metadata = "1234";
		String address1 = s1_address.toString();
		Object weight1 = 2;
		JSONObject threshold = TxUtil.threshold(master_weight, med_threshold, low_threshold, high_threshold);
		List signers = TxUtil.signers(address1, weight1);
		List operations = TxUtil.operSetOption(type, threshold,signers);
		String result = SignUtil.tx(operations, source_address, fee, sequence_number, metadata, pri, pub);
		int error_code = Result.getErrorCode(result);
		check.equals(error_code,0,"设置属性失败");
		
		String hash = Result.getHash(result);
		// System.out.println("hash="+hash);
		String re = Result.getResult(transaction, "hash", hash);
		int err = Result.getErrorCode(re);
		check.equals(err, 0, "交易成功后，通过hash查询交易记录失败");
		String feer = Result.getTranthInTranHistory(re, "fee");
		check.equals(feer, String.valueOf(fee), "交易成功后手续费不一致");
		
		//获取交易记录中的ledger_seq,再获取账户的previous_ledger_seq,验证两个值是否一致
		String led_seqr = Result.getTranthInTranHistory(re, "ledger_seq");
		String led_seqa = Result.getledger_seqFromAddress(source_address);
		check.equals(led_seqr, led_seqa, "交易成功后，交易记录中的ledger_seq与源账户的previous_ledger_seq不一致");
		
		//获取交易记录中operations里的type,查看值是否和提交的type一致
		String typer = Result.getOperthInTranHistory(re, "type");
		check.equals(typer, type.toString(), "交易成功后，交易记录中的type与创建时不一致");
		
		long ba = Integer.valueOf(balance1) - fee;
		String balancer = Result.getBalanceInAcc(source_address);
		check.equals(balancer, String.valueOf(ba), "交易成功后，源账户余额不正确");
		
		//查询交易记录中的sequence_number
		String seq_numr = Result.getTranthInTranHistory(re, "sequence_number");
		String seq_numa = Result.getledger_seqFromAddress(source_address);
		check.equals(seq_numr, seq_numa, "sequence_number与源账户当前的不一致");
		
		//验证signer_address是否一致
		String signer_adr = Result.getOperSignerthInTranHistory(re, "address"); 
		check.equals(signer_adr, address1, "交易成功后，signer_address查询不一致");
		//验证signer_weight是否一致
		String weightr = Result.getOperSignerthInTranHistory(re, "weight");
		check.equals(weightr, weight1.toString(), "交易成功后，weight查询不一致");
		//验证threshold里的master_weight是否一致
		String master_wr = Result.getOperThresthInTranHistory(re,
				"master_weight");
		check.equals(master_wr, master_weight.toString(),
				"交易成功后，master_weight查询不一致");
		//验证threshold里的med_threshold是否一致
		String med_threr = Result.getOperThresthInTranHistory(re,
				"med_threshold");
		check.assertEquals(med_threr, med_threshold.toString(),
				"交易成功后，med_threshold查询不一致");
	}

	// @Test
	//创建供应链成功后，查询交易记录中operation里的所有字段
	public void supplyChain(){
		Map acc = TxUtil.createAccount();
		Object source_address = acc.get("address");
		String pri = acc.get("private_key").toString();
		Object pub = acc.get("public_key");
		String balance1 = Result.getBalanceInAcc(source_address);
		long sequence_number = Result.seq_num(source_address);
		String metadata = "1234";
		Object type = 6;
		Map acc1 = TxUtil.createAccount();
		Object address = acc1.get("address");
		List inputs = new ArrayList<>();
		List outputs = TxUtil.outputs(address, metadata);
		// System.out.println("===input为空===");
		List opers = TxUtil.operSupplyChain(type, inputs, outputs);
		String response = SignUtil.tx(opers, source_address, fee,
				sequence_number, metadata, pri, pub);
		int error_code = Result.getErrorCode(response);
		check.equals(error_code, 0, "供应链创建失败");

		String hash = Result.getHash(response);
		// System.out.println("hash=" + hash);
		String re = Result.getResult(transaction, "hash", hash);
		int err = Result.getErrorCode(re);
		check.equals(err, 0, "交易成功后，通过hash查询交易记录失败");
		String feer = Result.getTranthInTranHistory(re, "fee");
		check.equals(feer, String.valueOf(fee), "交易成功后手续费不一致");

		// 获取交易记录中的ledger_seq,再获取账户的previous_ledger_seq,验证两个值是否一致
		String led_seqr = Result.getTranthInTranHistory(re, "ledger_seq");
		String led_seqa = Result.getPre_ledger_seqFromAddress(source_address);
		check.equals(led_seqr, led_seqa,
				"交易成功后，交易记录中的ledger_seq与源账户的previous_ledger_seq不一致");

		// 获取交易记录中operations里的type,查看值是否和提交的type一致
		String typer = Result.getOperthInTranHistory(re, "type");
		check.equals(typer, type.toString(), "交易成功后，交易记录中的type与创建时不一致");

		long ba = Integer.valueOf(balance1) - fee;
		String balancer = Result.getBalanceInAcc(source_address);
		check.equals(balancer, String.valueOf(ba), "交易成功后，源账户余额不正确");

		// 查询交易记录中的sequence_number
		String seq_numr = Result.getTranthInTranHistory(re, "sequence_number");
		String seq_numa = Result.getledger_seqFromAddress(source_address);
		check.equals(seq_numr, seq_numa, "sequence_number与源账户当前的不一致");

		// input为空时去null会出错
		// String inputr = Result.getOperthInTranHistory(re, "inputs");
		// check.equals(inputr, null, "交易成功后，交易记录中的inputs与创建时不一致");

		String out_addr = Result.getOperThInTranHistory(re, "outputs",
				"address");
		check.equals(out_addr, address.toString(),
				"交易成功后，交易记录中的outputs-address与创建时不一致");

		String metadatar = Result.getOperThInTranHistory(re, "outputs",
				"metadata");
		check.assertEquals(metadatar, metadata,
				"交易成功后，交易记录中的outputs-metadata与创建时不一致");

	}
	
}
