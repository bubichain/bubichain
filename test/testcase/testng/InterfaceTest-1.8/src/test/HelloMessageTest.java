package test;

import org.testng.annotations.Test;

import base.TestBase;
import cn.bubi.blockchain.adapter.*;
import newop.Transaction;
import utils.APIUtil;


public class HelloMessageTest extends TestBase{

	@Test
	public void testHelloMessage(){
		Message.ChainHello.Builder helloMessage = Message.ChainHello.newBuilder();
		helloMessage.setTimestamp(System.currentTimeMillis());
		bbc.Send(Message.ChainMessageType.CHAIN_HELLO_VALUE, helloMessage.build().toByteArray());
		APIUtil.wait(3);
	}
}
