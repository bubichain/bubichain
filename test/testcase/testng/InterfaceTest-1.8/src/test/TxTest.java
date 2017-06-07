 package test;

import java.math.BigInteger;
import java.util.Random;

import org.testng.annotations.Test;

import base.TestBase;
import model.Account;
import model.Operation;
import model.TimeRange;
import newop.Transaction;
import utils.APIUtil;

@Test
public class TxTest extends TestBase{
	Transaction tran = new Transaction();

//	@Test
	public void tran_sourceaddress(){
		Account a = tran.createAccountOne(genesis);
		Operation opt = new Operation();
		opt.setTranSource_add("123");
		tran.createAccount1(a, opt);
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 2,"transaction source address check failed");
	}
//	@Test
	public void tran_fee(){
		Account a = tran.createAccountOne(genesis);
		Operation opt = new Operation();
		opt.setFee(1);
		tran.createAccount1(a, opt);
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 111,"transaction fee check failed");
	}
//	public void builder_default(){
//		srcAcc.acc(led_acc, led_pri, led_pub, url_getAccInfo); //get genesis account
//		tran.e
//	}
//	@Test
	public void tran_mintime_lessthan_0(){
		Account a = tran.createAccountOne(genesis);
		Operation opt = new Operation();
		TimeRange timeRange = new TimeRange();
		timeRange.setMinTime(-1L);
		timeRange.setMaxTime(1L);
		opt.setTimeRange(timeRange);
		tran.createAccount1(a, opt);
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 2,"min time check failed");
	}
//	@Test
	public void tran_maxtime_lessthan_0(){
		Account a = tran.createAccountOne(genesis);
		Operation opt = new Operation();
		TimeRange timeRange = new TimeRange();
		timeRange.setMinTime(1L);
		timeRange.setMaxTime(-1L);
		opt.setTimeRange(timeRange);
		tran.createAccount1(a, opt);
		int e = Transaction.txInfo.getError_code();
		check.assertEquals(e, 2,"max time check failed");
	}
//	@Test
	public void tran_timerange(){
		Account a1 = tran.createAccountOne(genesis);
		Operation opt = new Operation();
		TimeRange timeRange = new TimeRange();
		int a = 99 +new Random().nextInt(999);
		Long max = System.currentTimeMillis()*1000+a;
		System.out.println("最小时间戳："+max);
		timeRange.setMinTime(max-1000000000);
		timeRange.setMaxTime(max+1000000000);
		opt.setTimeRange(timeRange);
		tran.createAccount1(a1, opt);
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 0,"create account in time range check failed");
	}
//	@Test
	public void tran_timerange_early(){
		Account a1 = tran.createAccountOne(genesis);
		Operation opt = new Operation();
		TimeRange timeRange = new TimeRange();
		int a = 99 +new Random().nextInt(999);
		Long max = System.currentTimeMillis()*1000+a;
		System.out.println("最小时间戳："+max);
		timeRange.setMinTime(max+8000000000L);
		timeRange.setMaxTime(max+9000000000L);
		opt.setTimeRange(timeRange);
		tran.createAccount1(a1, opt);
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 112,"create account early than normal check failed");
	}
//	@Test
	public void tran_timerange_late(){
		Account a1 = tran.createAccountOne(genesis);
		Operation opt = new Operation();
		TimeRange timeRange = new TimeRange();
		int a = 99 +new Random().nextInt(999);
		Long max = System.currentTimeMillis()*1000+a;
		System.out.println("时间戳： " + max);
		timeRange.setMinTime(max-9000000000L);
		timeRange.setMaxTime(max-8000000000L);
		opt.setTimeRange(timeRange);
		tran.createAccount1(a1, opt);
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 113,"create account late than normal check failed");
	}
//	@Test
	public void tran_metadata_Max(){
		Account a1 = tran.createAccountOne(genesis);
		Operation opt = new Operation();
		BigInteger bigInteger = new BigInteger("1024");
		BigInteger a = bigInteger.pow(1024).pow(1024);
		opt.setTransaction_metadata(a.toString());
		tran.createAccount1(a1, opt);
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 2,"创建账户交易失败");
	}
	
}
