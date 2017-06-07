package test;

import org.testng.annotations.Test;

import com.google.protobuf.ByteString;

import base.TestBase;
import cn.bubi.blockchain.adapter.Message;
import model.Account;
import model.Operation;
import newop.Transaction;
import utils.APIUtil;

@Test
public class PeerMessageTest extends TestBase{
	
	Operation opt = new Operation();
	
	private int bubiport = 19343;
	String dest_peer_address = APIUtil.getDesAddress("127.0.0.1", bubiport);
	Transaction tran = new Transaction(bbc_peer);
	
//	@Test
	public void peerMessage(){
		System.out.println(dest_peer_address);
		String temp ="peer message test";
		Message.ChainPeerMessage.Builder chain_peer_message = Message.ChainPeerMessage.newBuilder();
		chain_peer_message.setSrcPeerAddr("");
		chain_peer_message.addDesPeerAddrs(dest_peer_address);
		chain_peer_message.setData(ByteString.copyFromUtf8(temp));
		
		bbc_peer.Send(Message.ChainMessageType.CHAIN_PEER_MESSAGE_VALUE, chain_peer_message.build().toByteArray());
		APIUtil.wait(3);
		check.equals(Transaction.txInfo.getPeerMessage(), temp, "peer接收信息错误");
	}
}
