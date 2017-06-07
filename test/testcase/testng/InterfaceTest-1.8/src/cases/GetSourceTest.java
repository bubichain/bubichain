package cases;

import utils.Result;
import utils.TxUtil;

import org.testng.annotations.Test;

import base.TestBase;
import model.Account;
import model.InputInfo;

//供应链溯源查询
@Test
public class GetSourceTest extends TestBase{

	Account srcAcc = TxUtil.createNewAccount();
	/*
	 * 1.创建一笔供应链操作，取得交易成功的hash
	 * 2.通过该hash进行供应链溯源查询
	 * 3.供应链操作中的source_address是否和返回中的address一致
	 * 4.供应链操作中的input-对应的address是否和返回中的from一致
	 */
//	@Test
	public void getSource01Check() {
		InputInfo info = TxUtil.input();
		String hash = info.getHash();
		String result = Result.getSources(hash); // 得到供应链溯源的结果
		String address = Result.getAddress(result);
		check.assertEquals(address, info.getFirstLevel().getAddress(), "一级供应链溯源返回源地址错误");
	}
	//验证二级供应链溯源返回上级地址正确（一个input，一个output）
//	@Test
	public void getSource02Check(){
		InputInfo info = TxUtil.input2();
		String result = Result.getSources(info.getHash(),"2");
		System.out.println(result);
		String address = Result.getAddInFrom(result);
		check.assertEquals(address, info.getFirstLevel().getAddress(), "一级供应链溯源返回源地址错误");
	}
	
	//验证二级供应链溯源，depth为1返回上级的上级地址正确
//	@Test
	public void getSource03Check(){
		InputInfo info = TxUtil.input2();
		String result = Result.getSources(info.getHash(),"1");
		String address = Result.getAddInFrom(result);
		String firstLevel = info.getFirstLevel().getAddress();
		check.assertEquals(address, firstLevel, "一级供应链地址错误");
	}
	
	/*
	 * 创建三级供应链，然后depth随机取值，查address是否和最开始的一致
	 */
//	@Test
	public void getSources04Check(){
		InputInfo info = TxUtil.input3();
		String result = Result.getSources(info.getHash(),"1");
		String address = Result.getAddInFrom(result);
		String secondLevel = info.getSecondLevel().getAddress();
		check.assertEquals(address, secondLevel,"供应链二级地址错误");
	}
}
