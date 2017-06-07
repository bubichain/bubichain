package cases;

import org.testng.annotations.Test;

import base.TestBase;
import utils.HttpPool;
import utils.Result;
@Test
public class GetModulesStatusTest extends TestBase {

	public void getModulesStatus(){
		String result = HttpPool.doGet("getModulesStatus");
		String ledger_manager_sync_completed = Result.getModulesTh(result,
				"ledger_manager", "sync_completed");
		check.notEquals(ledger_manager_sync_completed, null, "sync_completed Ϊnull");
		check.notEquals(ledger_manager_sync_completed, "0", "sync_completed Ϊ0");
		
		String ledger_manager_tx_count = Result.getModulesTh(result,
				"ledger_manager", "tx_count");
		
		check.notEquals(ledger_manager_tx_count, null, "tx_count Ϊnull");
		check.assertNotEquals(ledger_manager_tx_count, "0", "tx_count Ϊ0");
	}
}
