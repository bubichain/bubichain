package test;

import java.math.BigInteger;
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
public class IssueTest extends TestBase{
	Transaction tran = new Transaction();
//	@Test
	public void testIssue(){
		Account a = tran.createAccountOne(genesis);
		Operation oper = new Operation();
		oper.setAsset_code("123456");
		oper.setAsset_amount(100L);
		tran.issue(a, oper);   //issue
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 0 , "资产发行失败");
	}
	
//	@Test
	public void issue_asset_issuer(){
		Operation oper = new Operation();
		Account a = tran.createAccountOne(genesis);
		oper.setAsset_code("123456");
		oper.setAsset_amount(100L);
		oper.setAsset_issuer("123");
		tran.issue(a, oper);   //issue
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 0 , "issue asset issuer check failed");
	}
//	@Test
	public void test_issue_details(){
		Operation oper = new Operation();
		Account a = tran.createAccountOne(genesis);
		oper.setAsset_code("123456");
		oper.setAsset_amount(100L);
		List<Detail> details = new ArrayList<>();
		Detail detail = new Detail();
		detail.setAmount(100L);
		detail.setExt("1234");
		details.add(detail);
		oper.setDetails(details);
		tran.issue(a, oper);   //issue
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 0 , "资产发行失败");
	}
//	@Test
	public void issue_operation_not_exist(){
		Account a = tran.createAccountOne(genesis);
		tran.noOperation(a, Type.ISSUE_ASSET);	//create account
		int e = Transaction.getErrorCode();
		check.assertEquals(e, 2,"issue no operation check failed");
	}
//	@Test
	public void issue_operation_metadata_Max(){
		Operation oper = new Operation();
		Account a = tran.createAccountOne(genesis);
		oper.setAsset_code("123456");
		oper.setAsset_amount(100L);
		BigInteger bigInteger = new BigInteger("1025");
		BigInteger a1 = bigInteger.pow(1024).pow(1024);
		oper.setMetadata(a1.toString());
		tran.issue(a, oper);   //issue
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 2 , "issue asset operation metadata max check failed");
	}
//	@Test
	public void issue_asset_amount_low(){
		Operation oper = new Operation();
		Account a = tran.createAccountOne(genesis);
		oper.setAsset_code("123456");
		oper.setAsset_amount(-100L);
		tran.issue(a, oper);   //issue
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 2 , "issue asset amount low check failed");
	}
//	@Test
	public void issue_asset_type_not_equal_IOU(){
		Operation oper = new Operation();
		Account a = tran.createAccountOne(genesis);
		oper.setAsset_code("123456");
		oper.setAsset_type(cn.bubi.blockchain.adapter.Message.AssetProperty.Type.NATIVE_VALUE);
		oper.setAsset_amount(100L);
		tran.issue(a, oper);   //issue
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 2 , "issue asset type not equal IOU check failed");
	}
//	@Test
	public void issue_asset_code_greater_than_64(){
		Operation oper = new Operation();
		Account a = tran.createAccountOne(genesis);
		oper.setAsset_code("1234567890abcdefghijklmnopqrstuvwxyz1234567890abcdefghijklmnopqrstu");
		oper.setAsset_amount(100L);
		tran.issue(a, oper);   //issue
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 2 , "issue asset code size greater than 64 check failed");
	}
//	@Test
	public void issue_publickey(){
		Operation oper = new Operation();
		Account a = tran.createAccountOne(genesis);
		oper.setAsset_code("1234");
		oper.setAsset_amount(100L);
		tran.invalidPublickey(a, oper, Type.ISSUE_ASSET, "123");
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 90 , "issue public key check failed");
	}
//	@Test
	public void issue_invalidpublickey(){
		Operation oper = new Operation();
		Account a = tran.createAccountOne(genesis);
		a.seqence = Account.getSeq(a);
		oper.setAsset_code("1234");
		oper.setAsset_amount(100L);
		tran.invalidPublickey(a, oper, Type.ISSUE_ASSET, "Fmv6zb1pcd5sUaKNs6Vve7tKt2hiXGwgszTGCaxs6kB9");
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 93 , "issue public key check failed");
	}
//	@Test
	public void issue_details_amount(){
		Operation oper = new Operation();
		Account a = tran.createAccountOne(genesis);
		List<Detail> details = new ArrayList<>();
		Detail detail = new Detail();
		detail.setAmount(-100L);
		detail.setStart(0L);
		detail.setLength(31536000L);
		detail.setExt("");
		details.add(detail);
		oper.setAsset_code("1234");
		oper.setAsset_amount(100L);
		oper.setDetails(details);
		tran.issue(a, oper);   //issue
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 2 , "issue asset details amount greater than 0 check failed");
	}
//	@Test
	public void issue_details_start(){
		Operation oper = new Operation();
		Account a = tran.createAccountOne(genesis);
		List<Detail> details = new ArrayList<>();
		Detail detail = new Detail();
		detail.setAmount(100L);
		detail.setStart(-10L);
		detail.setLength(31536000L);
		detail.setExt("");
		details.add(detail);
		oper.setAsset_code("1234");
		oper.setAsset_amount(100L);
		oper.setDetails(details);
		tran.issue(a, oper);   //issue
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 2 , "issue asset details start greater than 0 check failed");
	}
//	@Test
	public void issue_details_length(){
		Operation oper = new Operation();
		Account a = tran.createAccountOne(genesis);
		List<Detail> details = new ArrayList<>();
		Detail detail = new Detail();
		detail.setAmount(100L);
		detail.setStart(0L);
		detail.setLength(-2L);
		detail.setExt("");
		details.add(detail);
		oper.setAsset_code("1234");
		oper.setAsset_amount(100L);
		oper.setDetails(details);
		tran.issue(a, oper);   //issue
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 2 , "issue asset details length lower than -1 check failed");
	}
//	@Test
	public void issue_details_amount_not_equal_amount(){
		Operation oper = new Operation();
		Account a = tran.createAccountOne(genesis);
		List<Detail> details = new ArrayList<>();
		Detail detail = new Detail();
		detail.setAmount(101L);
		detail.setStart(0L);
		detail.setLength(-1L);
		detail.setExt("");
		details.add(detail);
		oper.setAsset_code("1234");
		oper.setAsset_amount(100L);
		oper.setDetails(details);
		tran.issue(a, oper);   //issue
		int error_code = Transaction.getErrorCode();
		check.assertEquals(error_code, 2 , "issue asset details length lower than -1 check failed");
	}
}
