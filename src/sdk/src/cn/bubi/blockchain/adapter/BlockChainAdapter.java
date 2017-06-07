package cn.bubi.blockchain.adapter;

import java.net.URI;
import java.nio.ByteBuffer;
import java.util.Timer;
import java.util.TimerTask;
import java.util.Vector;
import java.util.concurrent.LinkedBlockingQueue;

import org.java_websocket.WebSocket;
import org.java_websocket.WebSocketImpl;
import org.java_websocket.client.WebSocketClient;
import org.java_websocket.drafts.Draft;
import org.java_websocket.drafts.Draft_17;
import org.java_websocket.framing.Framedata;
import org.java_websocket.framing.FramedataImpl1;
import org.java_websocket.handshake.ServerHandshake;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.google.protobuf.ByteString;

import cn.bubi.blockchain.adapter.Message.WsMessage;


public class BlockChainAdapter {
	private Logger logger_ = LoggerFactory.getLogger(BlockChainAdapter.class);
	private BlockChainAdapterProc chain_handler_[] = new BlockChainAdapterProc[256];	
	private long heartbeat_time_ = 0;
	private final long connection_timeout_ = 60 * 1000;
	private final long check_interval = 15 * 1000;
	
	private class BlockChainManager extends Thread {
		private BlockChain block_chain_;
		private LinkedBlockingQueue<WsMessage> send_queue_;
		private boolean is_exit = true;
		private boolean is_connected_ = false;
		private int index_ = -1;
		private Draft draft_;
		private URI uri_;
		public BlockChainManager(int index, String uri_address) {
			index_ = index;
			draft_ = new Draft_17();
			uri_ = URI.create(uri_address);
			send_queue_ = new LinkedBlockingQueue<WsMessage>();
			start();
		}
		
		// start thread
		public void run() {
			if (!is_exit) {
				logger_.error("already running");
				return;
			}
			is_exit = false;
			while (!is_exit) {
				try {
					block_chain_ = new BlockChain(draft_, uri_);
					Thread block_chain_thread = new Thread(block_chain_);
					block_chain_thread.setName("worker thread");
					block_chain_thread.start();
					block_chain_thread.join();
				} catch (Exception e) {
					logger_.error("connect failed, " + e.getMessage());
				}
				
				try {
					Thread.sleep(5000);
				} catch (Exception e) {
					logger_.error("sleep 5000 failed, " + e.getMessage());
				}
			}
		}
		
		public boolean IsConnected() {
			return is_connected_;
		}
		
		public void Stop() {
			try {
				is_exit = true;
				join();
				if (is_connected_) {
					block_chain_.close();
				}
			} catch (Exception e) {
				logger_.error("join failed, " + e.getMessage());
			}
			
		}
		
		private class BlockChain extends WebSocketClient {
			
			private SendMessageThread send_message_thread_= new SendMessageThread("Send Thread");
			private Timer timer_ = new Timer();
			public BlockChain(Draft d, URI uri) {
				super(uri, d);
				WebSocketImpl.DEBUG = false;
				OnTimer();
			}
			
			@Override
			public void onError( Exception ex ) {
				logger_.info( "Error: " + ex.getMessage());
			}

			@Override
			public void onOpen( ServerHandshake handshake ) {
				is_connected_ = true;
				heartbeat_time_ = System.currentTimeMillis();
				logger_.info("open successful");
			}

			@Override
			public void onClose( int code, String reason, boolean remote ) {
				is_connected_ = false;
				timer_.cancel();
				send_message_thread_.Stop();
				send_message_thread_.Join();
				logger_.error( "Closed: " + index_ + ", code:" + code + ", reason:" + reason );
			}
			
			@Override
			public void onMessage(String message) {
				try {
					WsMessage wsMessage = WsMessage.parseFrom(ByteString.copyFrom(message.getBytes()));
					int type = (int)wsMessage.getType();
					if (chain_handler_[type] != null) {
						byte[] msg = wsMessage.getData().toByteArray();
						chain_handler_[type].ChainMethod(msg, msg.length);
					}
				} catch (Exception e) {
					logger_.error("the message of receive is not WsMessage format");
				}
			}
			
			@Override
			public void onMessage(ByteBuffer message) {
				try {
					WsMessage wsMessage = WsMessage.parseFrom(ByteString.copyFrom(message));
					int type = (int)wsMessage.getType();
					if (chain_handler_[type] != null) {
						byte[] msg = wsMessage.getData().toByteArray();
						chain_handler_[type].ChainMethod(msg, msg.length);
					}
				} catch (Exception e) {
					logger_.error("the message of receive is not WsMessage format");
				}
			}
			
			@Override
			public void onWebsocketPong(WebSocket conn, Framedata f) {
				super.onWebsocketPong(conn, f);
				heartbeat_time_ = System.currentTimeMillis();
			}
			
			private void OnTimer() {
				timer_.schedule(new TimerTask() {

					@Override
					public void run() {
						if (is_connected_) {
							// send ping
							FramedataImpl1 ping = new FramedataImpl1(Framedata.Opcode.PING);
							ping.setFin(true);
							getConnection().sendFrame(ping);
							
							// check timeout
							long current_time = System.currentTimeMillis();
							if (current_time - heartbeat_time_ > connection_timeout_) {
								close();
							}
						}
					}
				}, 2000, check_interval);
			}
			
			private class SendMessageThread implements Runnable {
				private boolean send_enabled_ = true;
				private Thread send_message_thread_;
				
				SendMessageThread(String thread_name) {
					send_enabled_ = true;
					send_message_thread_ = new Thread(this);
					send_message_thread_.setName(thread_name);
					send_message_thread_.start();
				}
				public void run() {
					while (send_enabled_) {
						WsMessage send_message = null;
						try {
							if (!is_connected_) {
								Thread.sleep(3000);
								continue;
							}
							
							send_message = send_queue_.take();
							if (send_message == null) {
								continue;
							}
							
							send(send_message.toByteArray());
						}
						catch(Exception e) {
							logger_.error("send failed, " + e.getMessage());
							send_queue_.add(send_message);
							try {
								Thread.sleep(3000);
							} catch (Exception ex) {
								logger_.error("sleep failed, " + ex.getMessage());
							}
						}
					}
				}
				
				void Stop() {
					send_enabled_ = false;
					try {
						WsMessage.Builder send_message = WsMessage.newBuilder();
						send_message.setData(ByteString.copyFrom("".getBytes()));
						send_message.setSequence(0);
						send_message.setRequest(true);
						send_message.setType(-1);
						send_queue_.put(send_message.build());
					} catch (InterruptedException e) {
						logger_.error("MessageThread Stop Error, " + e.getMessage());
					}
				}
				
				void Join() {
					try {
						send_message_thread_.join();
					} catch (InterruptedException e) {
						logger_.error("SendMessageThread join error, " + e.getMessage());
					}
				}
				
			}
		}
	}
	
	private String adapter_version_ = "2.0.0.0.2221";
	private Vector<BlockChainManager> blockchain_managers_ = new Vector<BlockChainManager>();
	private long seq = 0;
	
	
	public BlockChainAdapter(String uri_address) {
		String[] uri_addresses = uri_address.split(";");
		for (int i = 0; i < uri_addresses.length; i++) {
			BlockChainManager blockchain_manager = new BlockChainManager(i, uri_addresses[i]);
			blockchain_managers_.add(blockchain_manager);
		}
	}
	
	// stop thread
	public void Stop() {
		for(int i = 0; i < blockchain_managers_.size(); i++) {
			blockchain_managers_.get(i).Stop();
		}
	}
	
	// start status
	public boolean IsConnected() {
		boolean is_connect = false;
		for(int i = 0; i < blockchain_managers_.size(); i++) {
			if (blockchain_managers_.get(i).IsConnected()) {
				is_connect = true;
				break;
			}
		}
		return is_connect;
	}
	
	// add callback function
	// add method to BlockChainAdapter
	public void AddChainMethod(int type, BlockChainAdapterProc chainMethodProc) {
		chain_handler_[type] = chainMethodProc;
	}
	
	// send message
	public boolean Send(int type, byte[] msg) {
		boolean bret = false;
		do {
			try {
				if (!IsConnected()) {
					logger_.error("disconnected");
					bret = false;
					break;
				}
				WsMessage.Builder ws_message =  WsMessage.newBuilder();
				ws_message.setData(ByteString.copyFrom(msg));
				ws_message.setSequence(seq++);
				ws_message.setRequest(true);
				ws_message.setType(type);
				for(int i = 0; i < blockchain_managers_.size(); i++) {
					BlockChainManager blockchain_manager = blockchain_managers_.get(i);
					if (blockchain_manager.IsConnected()) {
						if (!blockchain_manager.send_queue_.add(ws_message.build())) {
							logger_.error("add send queue failed");
						}
					}
				}
				
				bret =  true;
			} catch(Exception e) {
				logger_.error("add message failed, " + e.getMessage());
			}
		} while (false);
		
		
		return bret;
	}
	
	// get adapter version
	// get message for BlockChainAdapter jar info
	public String GetVersion() {
		String adapter_version = "version: " + adapter_version_;
		return adapter_version;
	}
	
//	 public static void main(String[] argv) { 
//		 BlockChainAdapter chain_message_ = new BlockChainAdapter("ws://192.168.1.195:7053"/*"http://127.0.0.1:19999"*/);
//		 chain_message_.AddChainMethod(Message.ChainMessageType.CHAIN_HELLO.getNumber(), new BlockChainAdapterProc() {
//			public void ChainMethod(long seq, byte[] msg, int length) {
//				try {
//					Message.ChainHello hello = Message.ChainHello.parseFrom(msg);
//					System.out.println("==============================");
//					System.out.println(msg);
//				} catch (InvalidProtocolBufferException e) {
//					// TODO Auto-generated catch block
//					e.printStackTrace();
//				}
//			} }); 
//		 chain_message_.AddChainMethod(3, new BlockChainAdapterProc() {
//			public void ChainMethod(long seq, byte[] msg, int length) {
//				OnChainTxStatus(msg, length);
//			} }); 
//		 
//		 try {
//			Thread.sleep(2000);
//		} catch (InterruptedException e) {
//			// TODO Auto-generated catch block
//			e.printStackTrace();
//		}
//		
//		while (true) {
//			Message.ChainHello.Builder chain_hello = Message.ChainHello.newBuilder();
//			chain_hello.setTimestamp(System.currentTimeMillis());
//			if (!chain_message_.Send(Message.ChainMessageType.CHAIN_HELLO.getNumber(), chain_hello.build().toByteArray())) {
//				System.out.println("send hello failed");
//			}
//			try {
//				Thread.sleep(3000);
//			} catch (InterruptedException e) {
//				// TODO Auto-generated catch block
//				e.printStackTrace();
//			}
//		}
//		
//		 
//		 //chain_message_.Send(ChainMessageType.CHAIN_SUBMITTRANSACTION_VALUE, "hello".getBytes());
//	 }
//	 
//	 private static void OnChainTxStatus(byte[] msg, int length) {
//		 System.out.println(msg); 
//	 }
}
