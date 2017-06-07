package test;

import java.util.List;

import org.testng.annotations.Test;

import base.TestBase;
import model.Account;
import newop.Transaction;

//@Test
public class ConsignTest extends TestBase{

	Transaction tran = new Transaction();
	/**
	 * 联合签名发行资产
	 * 创建一个账户a1，设置联合签名的账号a2,a3
	 * master_weight=10,a.weight=6,b.weight=6
	 * a2,a3联合签名进行发行资产
	 */
//	@Test
	public void consign_issue_enoughThreshold(){
		List<Account>  accounts = tran.createAccountwithSigners(6, 6);
		tran.issue(accounts.get(0), accounts.get(1), accounts.get(2));
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 0, "consign issue failed with enough threshold");
	}
	/**
	 * 创建一个账户，设置联合签名的账号a,b
	 * master_weight=10,a.weight=5,b.weight=5
	 * a,b联合签名进行创建账号
	 */
//	@Test
	public void consign_issue_equalThreshold(){
		Transaction tran = new Transaction();
		List<Account>  accounts = tran.createAccountwithSigners(5, 5);
		tran.issue(accounts.get(0), accounts.get(1), accounts.get(2));
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 0, "consign issue failed with same threshold");
	}
	
	/**
	 * 验证一个联合签名账户A，signerB
 	 * A的master_weight=10,low=1,med=10,high=255
	 * B的weight=255
	 * 验证只有B进行签名，交易成功
	 * 
	 */
//	@Test
	public void consign_issue_withOneSigner(){
		Transaction tran = new Transaction();
//		Integer high, Integer low, Integer med, Integer master_weight, Integer weight
		List<Account>  accounts = tran.createAccountwithSigners(255, 1, 10, 10, 255);
		tran.issue(accounts.get(0), accounts.get(1));
		int error_code =Transaction.getErrorCode();
		check.assertEquals(error_code, 0, "consign issue failed with enough weight");
	}
	/**
	 * 创建一个账户，设置联合签名的账号a,b
	 * master_weight=10,a.weight=4,b.weight=4
	 * a,b联合签名进行创建账号,交易创建失败
	 */
	@Test
	public void consign_issue_lessThreshold(){
		Transaction tran = new Transaction();
		List<Account>  accounts = tran.createAccountwithSigners(4, 4);
		tran.issue(accounts.get(0), accounts.get(1), accounts.get(2));
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 93, "consign issue failed with less threshold");
	}
	/**
	 * 联合签名创建账户
	 * 创建一个账户a1，设置联合签名的账号a2,a3
	 * master_weight=10,a.weight=6,b.weight=6
	 * a2,a3联合签名进行创建账号，交易创建成功
	 */
//	 @Test  
	public void consign_create_enoughThreshold() {
		List<Account>  accounts = tran.createAccountwithSigners(6, 6);
		tran.createAccount(accounts.get(0), accounts.get(1), accounts.get(2));
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 0, "consign issue failed with enough threshold");
	}
	/**
	 * 创建一个账户a1，设置联合签名的账号a2,a3
	 * master_weight=10,a.weight=6,b.weight=6
	 * 验证a2,a3联合签名进行发行唯一资产，交易成功
	 */
//	 @Test
	public void consign_issueUnique_enoughThreshold() {
		List<Account>  accounts = tran.createAccountwithSigners(6, 6);
		tran.issueUnique(accounts.get(0), accounts.get(1), accounts.get(2));
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 0, "consign issue failed with enough threshold");
	}
	/**
	 * 创建一个账户a1，设置联合签名的账号a2,a3
	 * master_weight=10,a.weight=5,b.weight=5
	 * a2,a3联合签名进行初始化转账，交易成功
	 */
//	 @Test
	public void consign_initPayment() {
		List<Account>  accounts = tran.createAccountwithSigners(5, 5);
		tran.initPayment(accounts.get(0), accounts.get(1), accounts.get(2));
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 0, "consign issue failed with enough threshold");
	}
	/**
	 * 创建一个账户a1，设置联合签名的账号a2,a3
	 * master_weight=10,a.weight=5,b.weight=5
	 * a2,a3联合签名进行转移唯一资产
	 */
//	 @Test
	public void consign_uniquePayment() {
		List<Account>  accounts = tran.createAccountwithSigners(5, 5);
		tran.uniquePayment(accounts.get(0), accounts.get(1), accounts.get(2));
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 0, "consign issue failed with enough threshold");
	}
	
	/**
	 * 创建一个账户a1，设置联合签名的账号a2,a3
	 * master_weight=10,a.weight=5,b.weight=5
	 * a2,a3联合签名进行存证
	 */
//	 @Test
	public void consign_evidence() {
		List<Account>  accounts = tran.createAccountwithSigners(5, 5);
		tran.evidence(accounts.get(0), accounts.get(1), accounts.get(2));
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 0, "consign issue failed with enough threshold");
	}
	
	/**
	 * 创建一个账户a1，设置联合签名的账号a2,a3
	 * master_weight=10,a.weight=5,b.weight=5
	 * a2,a3联合签名进行设置属性操作
	 */
//	 @Test
	public void consign_setOption() {
		List<Account>  accounts = tran.createAccountwithSigners(5, 5);
		tran.setOption(accounts.get(0), accounts.get(1), accounts.get(2));
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 0, "consign issue failed with enough threshold");
	}
	
}
