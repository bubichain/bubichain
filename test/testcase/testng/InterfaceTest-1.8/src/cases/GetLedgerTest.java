package cases;

import java.util.Map;

import org.testng.annotations.Test;

import utils.APIUtil;
import utils.HttpUtil;
import utils.Result;
import utils.TxUtil;
import base.TestBase;

@Test
public class GetLedgerTest extends TestBase{

	String transaction = "getLedger";
	//查询基本手续费和 系统默认的是否一致
	public void base_feeCheck() {
		long base_fee = TestBase.fee;
		String result = HttpUtil.doget(transaction);
		int base_fee1 = Result.getbase_fee(result);

		check.assertEquals(base_fee1, base_fee, "基本手续费和系统默认的不一致");
	}
	//查询账户基本余额和 系统默认的是否一致 ()
	public void base_reserveCheck() {
		String result = HttpUtil.doget(transaction);
		int base_reserve1 = Result.getbase_reserve(result);

		check.assertNotEquals(base_reserve1, 0, "区块的base_reserve错误");
	}
	
	/*
	 * 1.先查询当前seq
	 * 2.发起任意一笔交易
	 * 3.交易成功后再次查询seq
	 * 4.判断后面的seq要大于之前的
	 */
//	@Test
	public void ledger_seqCheck() {
		String result = HttpUtil.doget(transaction);
		int ledger_seq = Result.getledger_seqDefault(result);
		// System.out.println("交易前ledger_seq:" + ledger_seq);
		Map acc = TxUtil.createAccount();
		String result1 = HttpUtil.doget(transaction);
		int ledger_seq1 = Result.getledger_seqDefault(result1);
		// System.out.println("交易后ledger_seq:" + ledger_seq1);
		check.largerThan(ledger_seq1, ledger_seq, "ledger_seq没有增长");
	}
}
