package utils;

public class GetModulesStatus {

	public static void main(String[] args) {

		String tran = "getModulesStatus";
		String url201 = "http://192.168.10.201:19333/";
		String url202 = "http://192.168.10.202:19333/";
		String url203 = "http://192.168.10.203:19333/";
		String url204 = "http://192.168.10.204:19333/";
//		String result133 = HttpUtil.doget(url133, tran);
//		String result136 = HttpUtil.doget(url136, tran);
		String result201 = HttpUtil.doget(url201, tran);
		String result202 = HttpUtil.doget(url202, tran);
		// System.out.println(result);
//		String ledger_manager_sync_completed133 = Result.getModulesTh(result133,
//				"ledger manager", "ledger_manager.sync_completed");
//		String ledger_manager_tx_count133 = Result.getModulesTh(result133,
//				"ledger manager", "ledger_manager.tx_count");
//		String ledger_manager_sync_completed136 = Result.getModulesTh(result136,
//				"ledger manager", "ledger_manager.sync_completed");
//		String ledger_manager_tx_count136 = Result.getModulesTh(result136,
//				"ledger manager", "ledger_manager.tx_count");
		String ledger_manager_sync_completed137 = Result.getModulesTh(result201,
				"ledger manager",
				"ledger_manager.sync_completed");
		String ledger_manager_tx_count137 = Result.getModulesTh(result202,
				"ledger manager",
				"ledger_manager.tx_count");
//		System.out.println("133 tx_count: " + ledger_manager_tx_count133);
//		System.out.println("133 sync_completed: "
//				+ ledger_manager_sync_completed133);
//
//		System.out.println("136 tx_count: " + ledger_manager_tx_count136);
//		System.out.println("136 sync_completed: "
//				+ ledger_manager_sync_completed136);

		System.out.println("137 tx_count: " + ledger_manager_tx_count137);
		System.out.println("137 sync_completed: "
				+ ledger_manager_sync_completed137);
		
		System.out.println("1.9 tx_count: " + ledger_manager_tx_count137);
		System.out.println("1.9 sync_completed: "
				+ ledger_manager_sync_completed137);
	}

}
