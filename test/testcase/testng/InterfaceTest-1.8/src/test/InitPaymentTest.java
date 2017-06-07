package test;

import java.util.ArrayList;
import java.util.List;

import org.testng.annotations.Test;

import base.TestBase;
import cn.bubi.blockchain.adapter.Message.Operation.Type;
import model.Account;
import model.Operation;
import model.asset.Detail;
import newop.Transaction;
@Test
public class InitPaymentTest extends TestBase{
	Transaction tran = new Transaction();
	
	/**
	 * 验证初始化转账功能正确（资产没有details）
	 */
//	@Test
	public void testInitPayment(){
		Operation opt = new Operation();
		Account a = tran.createAccountOne(genesis);
		Account b = tran.createAccountOne(genesis);
		opt.setDest_address(b.getAddress());
		opt.setAsset_issuer(a.getAddress());
		opt.setAsset_code("123");
		opt.setAsset_amount(100L);
		tran.initpayment(a, b, opt);
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 0,"init payment check failed");
	}
	
//	@Test
	public void initPayment_details(){
		Operation opt = new Operation();
		Account a1 = tran.createAccountOne(genesis);
		Account a2 = tran.createAccountOne(genesis);
		opt.setDest_address(a2.getAddress());
		opt.setAsset_issuer(a1.getAddress());
		opt.setAsset_code("123");
		opt.setAsset_amount(100L);
		List<Detail> details = new ArrayList<>();
		Detail detail = new Detail();
		detail.setAmount(100L);
		detail.setStart(0L);
		detail.setLength(31536000L);
		detail.setExt("");
		details.add(detail);
		opt.setDetails(details);
		tran.initpayment(a1, a2, opt);
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 0,"init payment detail check failed");
	}
	@Test
	public void initPayment_detail_amount(){
		Operation opt = new Operation();
		Account a1 = tran.createAccountOne(genesis);
		Account a2 = tran.createAccountOne(genesis);
		opt.setDest_address(a2.getAddress());
		opt.setAsset_issuer(a1.getAddress());
		opt.setAsset_code("123");
		opt.setAsset_amount(0L);
		List<Detail> details1 = new ArrayList<>();
		Detail detail1 = new Detail();
		detail1.setAmount(100L);
		detail1.setStart(0L);
		detail1.setLength(31536000L);
		detail1.setExt("");
		details1.add(detail1);
		opt.setDetails(details1);
		tran.initpayment(a1, a2, opt);
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 2,"init payment detail amount check failed");
	}
//	@Test
	public void initPayment_detail_start(){
		Operation opt = new Operation();
		Account a1 = tran.createAccountOne(genesis);
		Account a2 = tran.createAccountOne(genesis);
		opt.setDest_address(a2.getAddress());
		opt.setAsset_issuer(a1.getAddress());
		opt.setAsset_code("123");
		opt.setAsset_amount(100L);
		List<Detail> details1 = new ArrayList<>();
		Detail detail1 = new Detail();
		detail1.setAmount(100L);
		detail1.setStart(-1L);
		detail1.setLength(31536000L);
		detail1.setExt("");
		details1.add(detail1);
		opt.setDetails(details1);
		tran.initpayment(a1, a2, opt);
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 2,"init payment detail start check failed");
	}
//	@Test
	public void initPayment_detail_length(){
		Operation opt = new Operation();
		Account a1 = tran.createAccountOne(genesis);
		Account a2 = tran.createAccountOne(genesis);
		opt.setDest_address(a2.getAddress());
		opt.setAsset_issuer(a1.getAddress());
		opt.setAsset_code("123");
		opt.setAsset_amount(100L);
		List<Detail> details1 = new ArrayList<>();
		Detail detail1 = new Detail();
		detail1.setAmount(100L);
		detail1.setStart(0L);
		detail1.setLength(-2L);
		detail1.setExt("");
		details1.add(detail1);
		opt.setDetails(details1);
		tran.initpayment(a1, a2, opt);
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 2,"init payment detail length check failed");
	}
//	@Test
	public void initPayment_detail_amount_not_equal(){
		Operation opt = new Operation();
		Account a1 = tran.createAccountOne(genesis);
		Account a2 = tran.createAccountOne(genesis);
		opt.setDest_address(a2.getAddress());
		opt.setAsset_issuer(a1.getAddress());
		opt.setAsset_code("123");
		opt.setAsset_amount(100L);
		List<Detail> details1 = new ArrayList<>();
		Detail detail1 = new Detail();
		detail1.setAmount(10L);
		detail1.setStart(0L);
		detail1.setLength(-1L);
		detail1.setExt("");
		details1.add(detail1);
		opt.setDetails(details1);
		tran.initpayment(a1, a2, opt);
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 2,"init payment detail amount not equal check failed");
	}
//	@Test
	public void initpayment_asset_amount(){
		Operation opt = new Operation();
		Account a1 = tran.createAccountOne(genesis);
		Account a2 = tran.createAccountOne(genesis);
		opt.setDest_address(a2.getAddress());
		opt.setAsset_issuer(a1.getAddress());
		opt.setAsset_code("123");
		opt.setAsset_amount(0L);
		tran.initpayment(a1, a2, opt);
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 2,"init payment asset amount check failed");
	}
//	@Test
	public void initPayment_asset_type(){
		Operation opt = new Operation();
		Account a1 = tran.createAccountOne(genesis);
		Account a2 = tran.createAccountOne(genesis);
		opt.setDest_address(a2.getAddress());
		opt.setAsset_issuer(a1.getAddress());
		opt.setAsset_code("123");
		opt.setAsset_amount(100L);
		opt.setAsset_type(1);
		tran.initpayment(a1, a2, opt);
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 2,"init payment asset type check failed");
	}
//	@Test
	public void initPayment_destaddress(){
		Operation opt = new Operation();
		Account a1 = tran.createAccountOne(genesis);
		Account a2 = tran.createAccountOne(genesis);		opt.setDest_address("123");
		opt.setAsset_issuer(a1.getAddress());
		opt.setAsset_code("123");
		opt.setAsset_amount(100L);
		tran.initpayment(a1, a2, opt);
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 2,"init payment dest_address check failed");
	}
//	@Test
	public void initPayment_asset_issuer(){
		Operation opt = new Operation();
		Account a1 = tran.createAccountOne(genesis);
		Account a2 = tran.createAccountOne(genesis);
		opt.setDest_address(a2.getAddress());
		opt.setAsset_issuer("123");
		opt.setAsset_code("123");
		opt.setAsset_amount(100L);
		tran.initpayment(a1, a2, opt);
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 2,"init payment asset issuer check failed");
	}
//	@Test
	public void initpayment_operation_not_exist(){
		Account a = tran.createAccountOne(genesis);
		tran.noOperation(a, Type.INIT_PAYMENT);	//create account
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 2,"init payment no operation check failed");
	}
}
