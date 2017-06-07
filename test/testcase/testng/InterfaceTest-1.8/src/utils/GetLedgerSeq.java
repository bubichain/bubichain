package utils;

public class GetLedgerSeq {

	public static void main(String[] args) {

		String url1 = "http://192.168.10.201:19333/"; 
		String url2 = "http://192.168.10.202:19333/"; 
		String url3 = "http://192.168.10.203:19333/"; 
		String url4 = "http://192.168.10.204:19333/"; 
//		String url5 = "http://123.56.68.100:29333/"; 
		
		while (true) {
			System.out.println("\n");
			APIUtil.wait(2);
			System.out.println(DateHandler.getSimpleDay());
			System.out.println("peer1_ledger_seq: "+Result.getledger_seqDefault(HttpUtil.dogetTh(url1,"getLedger")));
			System.out.println("peer2_ledger_seq: "+Result.getledger_seqDefault(HttpUtil.dogetTh(url2,"getLedger")));
			System.out.println("peer3_ledger_seq: "+Result.getledger_seqDefault(HttpUtil.dogetTh(url3,"getLedger")));
			System.out.println("peer4_ledger_seq: "+Result.getledger_seqDefault(HttpUtil.dogetTh(url4,"getLedger")));
//			System.out.println("trasactionSize: "+Result.getModulesTh(HttpUtil.dogetTh(url4, "getModulesStatus"), "glue manager", "glueManager.trasactionSize") + "\n");
			System.out.println("peer1_tx_count: " + Result.getTx_count(HttpUtil.dogetTh(url1,"getLedger")));
			System.out.println("peer2_tx_count: " + Result.getTx_count(HttpUtil.dogetTh(url2,"getLedger")));
			System.out.println("peer3_tx_count: " + Result.getTx_count(HttpUtil.dogetTh(url3,"getLedger")));
			System.out.println("peer4_tx_count: " + Result.getTx_count(HttpUtil.dogetTh(url4,"getLedger")));
			
//			System.out.println("peer5_ledger_seq: "+Result.getledger_seqDefault(HttpUtil.dogetTh(url5,"getLedger")));
//			System.out.println("glueManager.trasactionSize: "+Result.getModulesTh(HttpUtil.dogetTh(url5, "getModulesStatus"), "glue manager", "glueManager.trasactionSize"));
		}
	}

}
