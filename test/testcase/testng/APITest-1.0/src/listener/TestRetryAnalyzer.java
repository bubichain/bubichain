package listener;

import org.testng.IRetryAnalyzer;
import org.testng.ITestResult;

public class TestRetryAnalyzer implements IRetryAnalyzer {
	private int retryCount = 1;

	private int maxRetryTimes = 2;

	@Override
	public boolean retry(ITestResult result) {
		if (retryCount <= maxRetryTimes) {

			result.setAttribute("RETRY", retryCount);

			System.out.println("用例：" + result.getName() + " 正在进行第" + retryCount
					+ "次失败重跑");

			retryCount++;

			return true;

		}

		return false;
	}

}
