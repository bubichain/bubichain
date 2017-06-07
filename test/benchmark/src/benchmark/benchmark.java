package benchmark;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.PrivateKey;
import java.security.Signature;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Properties;

import org.apache.commons.codec.binary.Hex;

//explicit import
import cn.bubi.common.http.HttpKit;
import cn.bubi.common.util.Tools;
import net.i2p.crypto.eddsa.EdDSAEngine;
import net.i2p.crypto.eddsa.EdDSAPrivateKey;
import net.i2p.crypto.eddsa.spec.EdDSANamedCurveTable;
import net.i2p.crypto.eddsa.spec.EdDSAParameterSpec;
import net.i2p.crypto.eddsa.spec.EdDSAPrivateKeySpec;
import cn.bubi.blockchain.adapter.BlockChainAdapter;
import cn.bubi.blockchain.adapter.BlockChainAdapterProc;
import cn.bubi.blockchain.adapter.Message;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import com.google.protobuf.*;

class Account {
	public String address;
	public String pub_key;
	public String priv_Key;
	public long seqence;
	public Account next; //help create another when child is too young to born
}

class Signatures {
	private String sign_data;
	private String public_key;

	public String getSign_data() {
		return sign_data;
	}

	public void setSign_data(String sign_data) {
		this.sign_data = sign_data;
	}

	public String getPublic_key() {
		return public_key;
	}

	public void setPublic_key(String public_key) {
		this.public_key = public_key;
	}
}

public class benchmark  {

	public Account genesis; 

	private HashMap<String, Account> traceAccountMap;
	
	private int maxSpeed;
	private int successTx;
	private int failureTx;
	private long blockHigh;
	private File fp;
	private InputStreamReader fs;
	private BufferedReader bufferedReader;
	private String accountFile;
	
	private BlockChainAdapter bbc;
	
	private String httpAddress;
	private String websocketAddress;
	private String pushAddress;
	private String pullAddress;

	public benchmark() {
	}

	public void OnChainTxStatus(byte[] msg, int length) {
		try {
				Message.ChainTxStatus txs = Message.ChainTxStatus.parseFrom(msg);
	
				switch (txs.getStatus().getNumber()) {
				case Message.ChainTxStatus.TxStatus.COMPLETE_VALUE:

				//System.out.printf("<success> tx_hash(%s)", txs.getTxHash());
				blockHigh = txs.getLedgerSeq();
				Account man = traceAccountMap.get(txs.getTxHash());

				if (man != null)
				{
					//System.out.printf(" new account(%s, %d)\n", man.address, txs.getNewAccountSeq());
					traceAccountMap.remove(txs.getTxHash());
					man.seqence = txs.getNewAccountSeq() + 1;
					createAccount(man, GetNextAccount());
					
					if(traceAccountMap.size() < maxSpeed * 3)
					{
						man.seqence++;
						createAccount(man, GetNextAccount());
					}
					
					successTx++;
				}
				break;
			case Message.ChainTxStatus.TxStatus.FAILURE_VALUE:
				System.out.printf("<failure> tx_hash(%s), src account(%s), error_code(%s)\n", txs.getTxHash(), 
						txs.getSourceAddress(), txs.getErrorCode().toString());
				failureTx++;
				break;
			default:
				break;
			}

		} catch (InvalidProtocolBufferException e) {
			e.printStackTrace();
		}
	}
	
	public Account GetNextAccount()
	{
		Account acc = null;
		String lineTxt = null;
		try {
			if ((lineTxt = bufferedReader.readLine()) != null) {
				acc = new Account();
				int b1 = lineTxt.indexOf(' ');
				int b2 = lineTxt.indexOf(' ', b1 + 1);
				acc.priv_Key = lineTxt.substring(0, b1);
				acc.pub_key = lineTxt.substring(b1 + 1, b2);
				acc.address = lineTxt.substring(b2 + 1);
			}
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			System.exit(0);
		}
		return acc;
	}

	public void Initialize() throws Exception  {
		genesis = new Account();
		successTx = 0;
		failureTx = 0;
		
		Properties propertie = new Properties();
        try {
        	FileInputStream inputFile = new FileInputStream("test.properties");
            propertie.load(inputFile);
            inputFile.close();
        } catch (FileNotFoundException ex) {
            System.out.println("test.properties not found");
            ex.printStackTrace();
        } catch (IOException ex) {
            System.out.println("test.properties format error");
            ex.printStackTrace();
        } 
        
		//load env 
        httpAddress = propertie.getProperty("HTTP_ADDRESS", "NOT_SET");
        websocketAddress = propertie.getProperty("WEBSOCKET_ADDRESS", "NOT_SET");
		//pushAddress = propertie.getProperty("PUSH_ADDRESS", "NOT_SET");
		//pullAddress = propertie.getProperty("PULL_ADDRESS", "NOT_SET");
		genesis.address = propertie.getProperty("GENESIS_ADDRESS", "NOT_SET");
		genesis.pub_key = propertie.getProperty("GENESIS_PUBLIC_KEY", "NOT_SET");
		genesis.priv_Key = propertie.getProperty("GENESIS_PRIVATE_KEY", "NOT_SET");
		accountFile = propertie.getProperty("TEST_DATA_FILE", "NOT_SET");
		maxSpeed = Integer.parseInt(propertie.getProperty("MAX_SPEED", "3334"));
		
		//output env
		System.out.printf("\nTEST Environment:\n", pushAddress);
		System.out.printf("*************************************************************************************\n");
		System.out.printf("PUSH_ADDRESS=%s\n", pushAddress);
		System.out.printf("PULL_ADDRESS=%s\n", pullAddress);
		System.out.printf("GENESIS_ADDRESS=%s\n", genesis.address);
		System.out.printf("GENESIS_PUBLIC_KEY=%s\n", genesis.pub_key);
		System.out.printf("GENESIS_PRIVATE_KEY=%s\n", genesis.priv_Key);
		System.out.printf("TEST_DATA_FILE=%s\n", accountFile);
		System.out.printf("MAX_SPEED=%s\n", maxSpeed);
		System.out.printf("*************************************************************************************\n\n");
		
		bbc = new BlockChainAdapter(websocketAddress);
		
		bbc.AddChainMethod(Message.ChainMessageType.CHAIN_TX_STATUS_VALUE, new BlockChainAdapterProc() {
			public void ChainMethod(byte[] msg, int length) {
				OnChainTxStatus(msg, length);
			}
		});

		traceAccountMap = new HashMap<String, Account>();

		System.out.printf("loading data file %s...\n", accountFile);
		fp = new File(accountFile);
		fs = new InputStreamReader(new FileInputStream(fp));
		bufferedReader = new BufferedReader(fs);
		
		//bbc.Start();
		
		System.out.println("block chain service started!\n");
		System.out.printf("*************************************************************************************\n\n");
	}

	public void Exit() {
		try {
			fs.close();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}


	public static void main(String[] args) throws IOException, InterruptedException {
		// TODO Auto-generated method stub

		benchmark test = new benchmark();
		try {
			test.Initialize();
			
			test.createTraceAccount();
			
			// 被测对象调用
		} catch (Throwable e) {
			e.printStackTrace();
		}

		test.Exit();
	}
	
	

	public void createTraceAccount() throws Exception {

		genesis.seqence = Util.getAccountSeq(httpAddress, genesis.address);
				
		genesis.seqence++;

		Account destAcc = GetNextAccount();
		assert (destAcc != null);
		createAccount(genesis, destAcc);
		

		long lastBlockHigh = 0;
		long lastTime = System.currentTimeMillis();
		int lastTx = 0;
		while(true)
		{
			if (blockHigh > lastBlockHigh)
			{
				long currentTime = System.currentTimeMillis();
				System.out.printf("\nblock %-8d success %-8d failure %-8d speed  %d/s", blockHigh, successTx, failureTx, (successTx - lastTx) * 1000 / (currentTime - lastTime));
				lastTime = currentTime;
				lastTx = successTx;
				lastBlockHigh = blockHigh;
			}
			Thread.sleep(1);
		}
	}
	
	public void createAccount(Account srcAcc, Account destAcc) {

		Message.OperationCreateAccount.Builder opcb = Message.OperationCreateAccount.newBuilder();

		// get account from redis server

		opcb.setDestAddress(destAcc.address); // read from file to accelerate
		opcb.setInitBalance(0);

		Message.Operation.Builder opb = Message.Operation.newBuilder();
		opb.setType(Message.Operation.Type.CREATE_ACCOUNT);
		opb.setCreateAccount(opcb);

		Message.Transaction.Builder txb = Message.Transaction.newBuilder();
		txb.setSourceAddress(srcAcc.address);
		txb.setSequenceNumber(srcAcc.seqence);
		txb.setFee(0);
		txb.addOperations(opb);

		Message.TransactionEnv.Builder txeb = Message.TransactionEnv.newBuilder();
		txeb.setTransaction(txb);
		
		Message.Signature.Builder s = Message.Signature.newBuilder();
		byte[] buffer = txb.build().toByteArray();
		try {
			s.setPublicKey(srcAcc.pub_key);
			s.setSignData(ByteString.copyFrom(Util.sign1(buffer, srcAcc.priv_Key)));
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		txeb.addSignatures(s);
		
		traceAccountMap.put(Util.getHash(buffer), destAcc);
		
		// use zero mq
		while(!bbc.Send(Message.ChainMessageType.CHAIN_SUBMITTRANSACTION_VALUE, txeb.build().toByteArray()))
		{
			try {
				Thread.sleep(1000);
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				System.out.println("failed to send transaction to chain");
			}
		}
	}
	
}

class Util {


	public static void execute(String httpAddress, List<Signatures> signatures, String transactionBlob, String srcAccAddr)
			throws Exception {

		final String submitJson = "{\"items\":[{\"signatures\":" + JSON.toJSONString(signatures)
				+ ",\"transaction_blob\":\"" + transactionBlob + "\"}]}";

		String resultData = HttpKit.post(httpAddress + "/submitTransaction", submitJson, false);
		JSONObject resultDataJson = JSON.parseObject(resultData);
		JSONArray resultsJson = resultDataJson.getJSONArray("results");
		JSONObject resultJson = (JSONObject) resultsJson.get(0);
		if (!resultJson.containsKey("error_code") || !resultJson.containsKey("hash")) {
			System.out.println("BUBI_BC_RESP_FAIL_ERR");
		}
		Integer errorCode = resultJson.getInteger("error_code");

		if (!(errorCode == 0 || errorCode == 102)) {
			String msg = "Blockchain error：" + errorCode + "]";
			System.out.printf("%s, %s\n", msg, resultJson.getString("error_desc"));
		}
	}

	/**
	 * 用�?钥签�??blob内存
	 * 
	 * @param blobStr
	 *            blob内容串
	 * @param srcPrivateKey
	 *            �?钥串
	 * @return
	 * @throws Exception
	 */
	public static String sign(byte blobArr[], String srcPrivateKey) throws Exception {
		String privateSrcKey = srcPrivateKey;
		byte privSrcArr[] = cn.bubi.common.util.Base58.decode(privateSrcKey);
		byte[] privArr = new byte[32];
		System.arraycopy(privSrcArr, 4, privArr, 0, privArr.length);
		Signature sgr = new EdDSAEngine(MessageDigest.getInstance("SHA-512"));
		EdDSAParameterSpec spec = EdDSANamedCurveTable.getByName("ed25519-sha-512");
		EdDSAPrivateKeySpec privKey = new EdDSAPrivateKeySpec(privArr, spec);
		PrivateKey sKey = new EdDSAPrivateKey(privKey);
		sgr.initSign(sKey);
		sgr.update(blobArr);
		return Tools.bytesToHex(sgr.sign());
	}
	
	public static byte[] sign1(byte blobArr[], String srcPrivateKey) throws Exception {
		String privateSrcKey = srcPrivateKey;
		byte privSrcArr[] = cn.bubi.common.util.Base58.decode(privateSrcKey);
		byte[] privArr = new byte[32];
		System.arraycopy(privSrcArr, 4, privArr, 0, privArr.length);
		Signature sgr = new EdDSAEngine(MessageDigest.getInstance("SHA-512"));
		EdDSAParameterSpec spec = EdDSANamedCurveTable.getByName("ed25519-sha-512");
		EdDSAPrivateKeySpec privKey = new EdDSAPrivateKeySpec(privArr, spec);
		PrivateKey sKey = new EdDSAPrivateKey(privKey);
		sgr.initSign(sKey);
		sgr.update(blobArr);
		return sgr.sign();
	}
	
	public static String getHash(byte[] buffer) {
		MessageDigest md=null;
		String hash ="";
		try {
			md = MessageDigest.getInstance("SHA-256");
		} catch (NoSuchAlgorithmException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		md.update(buffer);
		hash=Hex.encodeHexString( md.digest());
		
		return hash;
	}

	public static long getAccountSeq(String httpAddress, String address) throws Exception {
		LinkedHashMap<String, String> params = new LinkedHashMap<String, String>();
		params.put("address", address);
		String resultData = HttpKit.get(httpAddress + "/getAccount", params);
		if (resultData == null) {
			throw new Exception("区�?�链�?应出现异常");
		}
		JSONObject resultJson = JSON.parseObject(resultData);
		if (!resultJson.containsKey("error_code") || !resultJson.containsKey("result")) {
			throw new Exception("区�?�链�?应数�?�格�?�?对");
		}
		long seq = resultJson.getJSONObject("result").getLongValue("tx_seq");
		return seq;
	}
}