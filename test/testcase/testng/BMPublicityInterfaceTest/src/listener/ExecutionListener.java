package listener;

import java.io.File;
import java.util.Iterator;

import org.testng.ITestContext;
import org.testng.ITestListener;
import org.testng.ITestNGMethod;
import org.testng.ITestResult;
import org.testng.Reporter;
import org.testng.annotations.AfterMethod;

import com.relevantcodes.extentreports.ExtentReports;
import com.relevantcodes.extentreports.ExtentTest;
import com.relevantcodes.extentreports.LogStatus;
import com.relevantcodes.extentreports.ReporterType;

import base.Log;
import base.TestBase;

public class ExecutionListener implements ITestListener {
	protected ExtentReports extent;
	protected static ExtentTest test;

	// String filePath = "report/ExtentReport.html";
	@Override
	public void onFinish(ITestContext context) {

		Iterator<ITestResult> listOfFailedTests = context.getFailedTests()
				.getAllResults().iterator();

		while (listOfFailedTests.hasNext()) {
			ITestResult failedTest = listOfFailedTests.next();
			ITestNGMethod method = failedTest.getMethod();

			if (context.getFailedTests().getResults(method).size() > 1) {
				listOfFailedTests.remove();
			}
		}
		 for (ITestNGMethod method : context.getAllTestMethods()) {
//			 System.out.println(method.getId().getClass().getName());
//			 System.out.println(method.getMethodName());
		 }
	}

	@Override
	public void onStart(ITestContext context) {

		extent = TestBase.getExtent();
		
		// test = extent.
		// for (ITestNGMethod method : context.getAllTestMethods()) {
		// // test = extent.startTest(method.getMethodName());
		// // extent.startTest(method.getMethodName(), method.getMethodName()
		// // + "¿ªÊ¼Ö´ÐÐ!!");
		// // extent.loadConfig(new File("extent-config.xml"));
		// method.setRetryAnalyzer(new TestRetryAnalyzer());
		// Log.info(method.getMethodName() + " -> set retry");
		// }
	}

	@Override
	public void onTestStart(ITestResult arg0) {
		
		// System.out.println("Test start" + arg0.getTestName()
		// + arg0.getClass().getName());
		test = extent.startTest(arg0.getName());
		// extent.flush();
	}

	@Override
	public void onTestFailedButWithinSuccessPercentage(ITestResult arg0) {
		// TODO Auto-generated method stub

	}


	@Override
	public void onTestFailure(ITestResult result) {
		test.log(LogStatus.FAIL, result.getTestClass().getRealClass().getName()
				+ " " + result.getThrowable().toString());
		Log.info(result.getMethod().getMethodName() + " Test failed");
		extent.endTest(test);
		extent.flush();
	}

	@Override
	public void onTestSkipped(ITestResult result) {
		test.log(LogStatus.SKIP, result.getTestClass().getRealClass().getName());
		Log.info(result.getMethod().getMethodName() + "Test skipped");
		extent.endTest(test);
		extent.flush();
	}

	@Override
	public void onTestSuccess(ITestResult result) {
		test.log(LogStatus.PASS, result.getTestClass().getRealClass().getName());
		extent.endTest(test);
		extent.flush();
	}

}
