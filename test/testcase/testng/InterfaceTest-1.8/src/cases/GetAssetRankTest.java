package cases;

import java.util.List;

import org.testng.annotations.Test;

import base.TestBase;
import model.Account;
import model.Address;
import utils.Result;
import utils.TxUtil;

@Test
public class GetAssetRankTest extends TestBase{
	String asset_code = "123code";
	Long asset_amount = 1000L;
	String metadata = "aabc";
	Account srcAcc = TxUtil.createNewAccount();

	/*
	 * 验证资产排行功能正确（没有地址过滤）
	 * 返回结果都有该asset_code对应的资产
	 */
//	@Test
	public void getAssetRank_normalCheck(){
		Account srcAcc = TxUtil.createNewAccount();
		TxUtil.issue_transfer(srcAcc, asset_code, asset_amount, metadata);
		String result = TxUtil.getAssetRank(srcAcc, asset_code, null, null);
		System.out.println(result);
		List<Address> asd = Result.getAssetRankAdds(result);
		int err_code = Result.getErrorCode(result);
		check.assertEquals(err_code, 0, "获取资产排行校验失败");
		int num = Result.getNumberByAddress(asd.get(0));
		check.assertEquals(num, 1,"资产排行返回地址校验失败");
	}
	
	/*
	 * 验证资产发行后未进行转移，做资产排行查询，返回结果正确（不返回发行者信息）
	 */
//	@Test
	public void getAssetRank_normal01Check(){
		TxUtil.issue(srcAcc, asset_code, asset_amount, metadata);
		String result = TxUtil.getAssetRank(srcAcc, asset_code, null, null);
		int err_code = Result.getErrorCode(result);
		String re = Result.getResultTh(result);
		check.assertEquals(err_code, 0, "获取资产排行errorcode校验失败");
		check.assertEquals(re, "null","获取资产排行result校验失败");
	}
	/*
	 * 验证验证资产排行功能正确（有地址过滤）
	 */
//	@Test		//	过滤地址，结果返回筛掉的地址
	public void getAssetRank_normal02Check(){
		Account srcAcc = TxUtil.createNewAccount();
		List<String> addrs = TxUtil.issue_transfer(srcAcc, asset_code, asset_amount, metadata);
		String result = TxUtil.getAssetRank(srcAcc, asset_code, addrs.get(0), null);
		List<Address> asd = Result.getAssetRankAdds(result);
		int err_code = Result.getErrorCode(result);
		check.assertEquals(err_code, 0, "获取资产排行校验失败");
		check.assertEquals(asd.get(0).getAddress(), addrs.get(1),"资产排行返回地址校验失败");
	}
	
	/*
	 * 验证asset_issuer参数错误，校验成功
	 */
//	@Test
	public void asset_issuerCheck(){
		TxUtil.issue_transfer(srcAcc, asset_code, asset_amount, metadata);
		Object[] assetIssuers = {"abcd", "1234", " ", "null", null, "!@#"};
		for (Object assetIssuer : assetIssuers) {
			String result = TxUtil.getAssetRank(assetIssuer, asset_code, null, null);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 2, "获取资产排行asset_issuer校验失败");
		}
	}
	/*
	 * 验证asset_issuer字段缺失，报错正确
	 */
//	@Test
	public void asset_issuerNoCheck(){
		String result = TxUtil.getAssetRank(null, asset_code, null, null);
		int err_code = Result.getErrorCode(result);
		check.assertEquals(err_code, 2, "获取资产排行asset_issuer字段缺失校验失败");
	}
	
	/*
	 * 验证asset_issuer账户不存在，返回结果为null，error_code 0
	 */
//	@Test
	public void asset_issuerNotExistCheck(){
		String result = TxUtil.getAssetRank(Account.generateAccount().getAddress(), asset_code, null, null);
		int err_code = Result.getErrorCode(result);
		String re = Result.getResultTh(result);
		check.assertEquals(err_code, 0, "获取资产排行errorcode校验失败");
		check.assertEquals(re, "null","获取资产排行result校验失败");
	}
	//验证asset_issuer对应的asset_code不存在，返回错误信息正确
	public void asset_codeNotExistCheck(){
		String result = TxUtil.getAssetRank(srcAcc, "bc", null, null);
		int err_code = Result.getErrorCode(result);
		String re = Result.getResultTh(result);
		check.assertEquals(err_code, 0, "获取资产排行errorcode校验失败");
		check.assertEquals(re, "null","获取资产排行result校验失败");
	}
	//验证asset_code值非法，返回错误信息正确
//	@Test
	public void asset_codeCheck(){
		TxUtil.issue_transfer(srcAcc, asset_code, asset_amount, metadata);
		Object[] assetIssuers = {"abcd", "1234", " ", "!@#","-1"};
		for (Object assetIssuer : assetIssuers) {
			String result = TxUtil.getAssetRank(srcAcc, assetIssuer, null, null);
			int err_code = Result.getErrorCode(result);
			String re = Result.getResultTh(result);
			check.assertEquals(err_code, 0, "获取资产排行asset_code校验失败");
			check.assertEquals(re, "null","获取资产排行result校验失败");
		}
	}
	//验证asset_code值为null，返回错误信息正确
//	@Test
	public void asset_codeNullCheck(){
		TxUtil.issue_transfer(srcAcc, asset_code, asset_amount, metadata);
		Object[] assetIssuers = {"null", null};
		for (Object assetIssuer : assetIssuers) {
			String result = TxUtil.getAssetRank(srcAcc, assetIssuer, null, null);
			int err_code = Result.getErrorCode(result);
			String re = Result.getResultTh(result);
			check.assertEquals(err_code, 2, "获取资产排行asset_code校验失败");
			check.assertEquals(re, "null","获取资产排行result校验失败");
		}
	}
	//验证asset_code字段缺少，报错信息正确
//	@Test
	public void asset_codeNoCheck(){
		String result = TxUtil.getAssetRank(srcAcc, null, null, null);
		int err_code = Result.getErrorCode(result);
		check.assertEquals(err_code, 2, "获取资产排行asset_code字段缺失校验失败");
	}
	//验证过滤地址不存在，返回结果正确
//	@Test		//	参数错误仍返回所有未过滤结果
	public void filteraddrNotExist(){
		TxUtil.issue_transfer(srcAcc, asset_code, asset_amount, metadata);
		Object[] filterAdds = {"abcd", "1234", " ", "!@#","-1"};
		for (Object filteraddr : filterAdds) {
			String result = TxUtil.getAssetRank(srcAcc, asset_code, filteraddr, null);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 2, "获取资产排行asset_code字段缺失校验失败");
		}
	}
	//验证过滤地址值非法，返回信息正确
	
	//验证过滤地址为至少两个个时，返回结果正确
//	@Test
	public void filter_addr_twoCheck(){
		List<String> addrs = TxUtil.issue_transfer(srcAcc, asset_code, asset_amount, metadata);
		String result = TxUtil.getAssetRank(srcAcc, asset_code, addrs, null);
		int err_code = Result.getErrorCode(result);
		check.assertEquals(err_code, 0, "获取资产排行asset_code字段缺失校验失败");
		String re = Result.getResultTh(result);
		check.assertEquals(re, "null","获取资产排行result校验失败");
	}
	//验证过滤地址格式不正确，返回信息正确
	//验证count字段显示数量正确
//	@Test
	public void countCheck(){
		TxUtil.issue_transfer(srcAcc, asset_code, asset_amount, metadata);
		String result = TxUtil.getAssetRank(srcAcc, asset_code, null, 1);
		int count = Result.getAssetRankResultSize(result);
		check.assertEquals(count, 1, "返回结果个数校验失败");
	}
	//验证count值非法，返回错误信息正确
//	@Test
	public void countilligalCheck(){
		Object[] counts = {"abcd", "1234", " ", "!@#","-1"};
		for (Object count : counts) {
			String result = TxUtil.getAssetRank(srcAcc, asset_code, null, count);
			int err_code = Result.getErrorCode(result);
			check.assertEquals(err_code, 2, "获取资产排行count字段校验失败");
		}
	}
	//验证count大于拥有者个数，结果返回正确
//	@Test
	public void countmaxCheck(){
		Account srcAcc = TxUtil.createNewAccount();
		TxUtil.issue_transfer(srcAcc, asset_code, asset_amount, metadata);
		String result = TxUtil.getAssetRank(srcAcc, asset_code, null, 3);
		int count = Result.getAssetRankResultSize(result);
		check.assertEquals(count, 2, "返回结果个数校验失败");
	}
	//验证返回amount和number正确
//	@Test
	public void amountCheck(){
		Account srcAcc = TxUtil.createNewAccount();
		Account a1 = TxUtil.createNewAccount();
		Account a2 = TxUtil.createNewAccount();
		Account a3 = TxUtil.createNewAccount();
		List<Account> accounts = TxUtil.issue_transfer(srcAcc, a1, 10L, a2, 20L, a3, 40L, asset_code, asset_amount, metadata);
		String result = TxUtil.getAssetRank(srcAcc, asset_code, null, null);
		List<Address> adds = Result.getAssetRankAdds(result);
		String add1 = adds.get(0).getAddress();
		int num = adds.get(0).getNumber();
		check.assertEquals(add1, accounts.get(2).getAddress(),"排行第一的地址校验错误");
		check.assertEquals(num, 1,"拥有资产最多的排行校验错误");
		//验证资产对多的地址，排名第一
	}
	
}
