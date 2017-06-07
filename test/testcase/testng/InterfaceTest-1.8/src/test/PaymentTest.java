package test;

import java.util.ArrayList;
import java.util.List;

import org.testng.annotations.Test;

import base.TestBase;
import cn.bubi.blockchain.adapter.Message;
import cn.bubi.blockchain.adapter.Message.Operation.Type;
import model.Account;
import model.Operation;
import model.TxInfo;
import model.asset.Detail;
import newop.Transaction;
import utils.APIUtil;

@Test
public class PaymentTest extends TestBase{
	Transaction tran = new Transaction();
//	@Test
	public void payment_NATIVE(){
		Account a1 = tran.createAccountOne(genesis);
		Account a2 = tran.createAccountOne(genesis);
		Operation op = new Operation();
		op.setAsset_amount(100L);
		op.setAPtype(Message.AssetProperty.Type.NATIVE);
		tran.payment(a1, a2, op);
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 0,"NATIVE transfer check failed");
	}
//	@Test
	public void payment_UNIQUE(){
		Account a1 = tran.createAccountOne(genesis);
		Account a2 = tran.createAccountOne(genesis);
		Operation op = new Operation();
		op.setAsset_amount(100L);
		op.setAPtype(Message.AssetProperty.Type.UNIQUE);
		tran.payment(a1, a2, op);
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 2,"UNIQUE transfer check failed");
	}
//	@Test
	public void payment_IOU(){
		Account a1 = tran.createAccountOne(genesis);
		Account a2 = tran.createAccountOne(genesis);
		Operation op = new Operation();
		op.setAsset_amount(100L);
		op.setAsset_code("abcd");
		op.setAPtype(Message.AssetProperty.Type.IOU);
		tran.paymentIOU(a1, a2, op);
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 0,"IOU transfer check failed");
	}
//	@Test
	public void payment_asset_issuer_invalid(){
		Account a1 = tran.createAccountOne(genesis);
		Account a2 = tran.createAccountOne(genesis);
		Operation op = new Operation();
		op.setAsset_amount(100L);
		op.setAsset_code("abcd");
		op.setAsset_issuer("123");
		op.setAsset_type(1);
		tran.paymentIOU(a1, a2, op);
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 0,"payment asset issuer invalid check failed");
	}
//	@Test
	public void payment_operation_not_exist(){
		Account a1 = tran.createAccountOne(genesis);
		tran.noOperation(a1, Type.PAYMENT);	//create account
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 2,"payment no operation check failed");
	}
	/**
	 * 验证目标地址值无效
	 */
//	@Test
	public void payment_destaddress_invalid(){
		Account a1 = tran.createAccountOne(genesis);
		Account a2 = new Account();
		a2.setAddress("abcd");
		Operation op = new Operation();
		op.setAsset_amount(100L);
		op.setAsset_code("abcd");
		op.setAPtype(Message.AssetProperty.Type.IOU);
		tran.paymentIOU(a1, a2, op);
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 2,"transfer dest_address invalid check failed");
	}
	/**
	 * 验证目标地址不存在
	 */
//	@Test
	public void payment_destaddress_not_exist(){
		Account a1 = tran.createAccountOne(genesis);
		Account a2 = Account.generateAccount();
		Operation op = new Operation();
		op.setAsset_amount(100L);
		op.setAsset_code("abcd");
		op.setAPtype(Message.AssetProperty.Type.IOU);
		tran.paymentIOU(a1, a2, op);
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 103,"transfer dest_address invalid check failed");
	}
//	@Test
	public void payment_amount(){
		Account a1 = tran.createAccountOne(genesis);
		Account a2 = tran.createAccountOne(genesis);
		Operation op = new Operation();
		op.setAsset_amount(-1L);
		op.setAsset_code("abcd");
		op.setAPtype(Message.AssetProperty.Type.IOU);
		tran.paymentIOU(a1, a2, op);
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 2,"transfer asset amount greater than 0 check failed");
	}
//	@Test
	public void payment_asset_type(){
		Account a1 = tran.createAccountOne(genesis);
		Account a2 = tran.createAccountOne(genesis);
		Operation op = new Operation();
		op.setAsset_amount(100L);
		op.setAsset_code("abcd");
		op.setAPtype(Message.AssetProperty.Type.UNIQUE);
		tran.paymentIOU(a1, a2, op);
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 2,"transfer UNIQUE asset type check failed");
	}
//	@Test
	public void payment_asset_code(){
		Account a1 = tran.createAccountOne(genesis);
		Account a2 = tran.createAccountOne(genesis);
		Operation op = new Operation();
		op.setAsset_amount(100L);
		op.setAsset_code("1234567890abcdefghijklmnopqrstuvwxyz1234567890abcdefghijklmnopqrstu");
		op.setAPtype(Message.AssetProperty.Type.IOU);
		tran.paymentIOU(a1, a2, op);
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 2,"transfer asset code size greater than 64 check failed");
	}
//	@Test
	public void payment_asset_code_size(){
		Account a1 = tran.createAccountOne(genesis);
		Account a2 = tran.createAccountOne(genesis);
		Operation op = new Operation();
		op.setAsset_amount(100L);
		op.setAsset_code("1234567890abcdefghijklmnopqrstuvwxyz1234567890abcdefghijklmnopqrstu");
		op.setAPtype(Message.AssetProperty.Type.IOU);
		tran.paymentIOU(a1, a2, op);
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 2,"transfer asset code size greater than 64 check failed");
	}
//	@Test
	public void payment_publickey(){
		Account a1 = tran.createAccountOne(genesis);
		Account a2 = tran.createAccountOne(genesis);
		Operation op = new Operation();
		op.setAsset_amount(100L);
		op.setAsset_code("abcd");
		op.setAPtype(Message.AssetProperty.Type.IOU);
		op.setDest_address(a2.getAddress());
		op.setAsset_issuer(a1.getAddress());
		tran.invalidPublickey(a1, op, Type.PAYMENT, "123");
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 90,"transfer public key check failed");
	}
//	@Test
	public void payment_details_amount(){
		Account a1 = tran.createAccountOne(genesis);
		Account a2 = tran.createAccountOne(genesis);
		Operation op1 = new Operation();
		op1.setAsset_amount(100L);
		op1.setAsset_code("abcd");
		List<Detail> details = new ArrayList<>();
		Detail detail = new Detail();
		detail.setAmount(100L);
		detail.setStart(0L);
		detail.setLength(31536000L);
		detail.setExt("");
		details.add(detail);
		op1.setDetails(details);
		a1.seqence = Account.getSeq(a1);
		tran.issue(a1, op1);
		Operation op = new Operation();
		op.setAsset_amount(100L);
		op.setAsset_code("abcd");
		op.setAPtype(Message.AssetProperty.Type.IOU);
		op.setDest_address(a2.getAddress());
		op.setAsset_issuer(a1.getAddress());
		List<Detail> details1 = new ArrayList<>();
		Detail detail1 = new Detail();
		detail1.setAmount(-100L);
		detail1.setStart(0L);
		detail1.setLength(31536000L);
		detail1.setExt("");
		details1.add(detail1);
		op.setDetails(details1);
		tran.payment(a1, a2, op);
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 2,"transfer details amount check failed");
	}
//	@Test
	public void payment_details_start(){
		Account a1 = tran.createAccountOne(genesis);
		Account a2 = tran.createAccountOne(genesis);
		Operation op1 = new Operation();
		op1.setAsset_amount(100L);
		op1.setAsset_code("abcd");
		List<Detail> details = new ArrayList<>();
		Detail detail = new Detail();
		detail.setAmount(100L);
		detail.setStart(0L);
		detail.setLength(31536000L);
		detail.setExt("");
		details.add(detail);
		op1.setDetails(details);
		tran.issue(a1, op1);
		Operation op = new Operation();
		op.setAsset_amount(100L);
		op.setAsset_code("abcd");
		op.setAPtype(Message.AssetProperty.Type.IOU);
		op.setDest_address(a2.getAddress());
		op.setAsset_issuer(a1.getAddress());
		List<Detail> details1 = new ArrayList<>();
		Detail detail1 = new Detail();
		detail1.setAmount(100L);
		detail1.setStart(-1L);
		detail1.setLength(31536000L);
		detail1.setExt("");
		details1.add(detail1);
		op.setDetails(details1);
		tran.payment(a1, a2, op);
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 2,"transfer details start check failed");
	}
//	@Test
	public void payment_details_length(){
		Account a1 = tran.createAccountOne(genesis);
		Account a2 = tran.createAccountOne(genesis);
		Operation op1 = new Operation();
		op1.setAsset_amount(100L);
		op1.setAsset_code("abcd");
		List<Detail> details = new ArrayList<>();
		Detail detail = new Detail();
		detail.setAmount(100L);
		detail.setStart(0L);
		detail.setLength(31536000L);
		detail.setExt("");
		details.add(detail);
		op1.setDetails(details);
		tran.issue(a1, op1);
		Operation op = new Operation();
		op.setAsset_amount(100L);
		op.setAsset_code("abcd");
		op.setAPtype(Message.AssetProperty.Type.IOU);
		op.setDest_address(a2.getAddress());
		op.setAsset_issuer(a1.getAddress());
		List<Detail> details1 = new ArrayList<>();
		Detail detail1 = new Detail();
		detail1.setAmount(100L);
		detail1.setStart(0L);
		detail1.setLength(-2L);
		detail1.setExt("");
		details1.add(detail1);
		op.setDetails(details1);
		tran.payment(a1, a2, op);
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 2,"transfer details length check failed");
	}
//	@Test
	public void payment_details_amount_not_equal(){
		Account a1 = tran.createAccountOne(genesis);
		Account a2 = tran.createAccountOne(genesis);
		Operation op1 = new Operation();
		op1.setAsset_amount(100L);
		op1.setAsset_code("abcd");
		List<Detail> details = new ArrayList<>();
		Detail detail = new Detail();
		detail.setAmount(100L);
		detail.setStart(0L);
		detail.setLength(31536000L);
		detail.setExt("");
		details.add(detail);
		op1.setDetails(details);
		tran.issue(a1, op1);		//issue asset with details
		Operation op = new Operation();
		op.setAsset_amount(100L);
		op.setAsset_code("abcd");
		op.setAPtype(Message.AssetProperty.Type.IOU);
		op.setDest_address(a2.getAddress());
		op.setAsset_issuer(a1.getAddress());
		List<Detail> details1 = new ArrayList<>();
		Detail detail1 = new Detail();
		detail1.setAmount(10L);
		detail1.setStart(0L);
		detail1.setLength(-1L);
		detail1.setExt("");
		details1.add(detail1);
		op.setDetails(details1);
		tran.payment(a1, a2, op);
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 2,"transfer amount not equal check failed");
	}
}
