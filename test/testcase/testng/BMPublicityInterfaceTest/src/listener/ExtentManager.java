package listener;

import java.io.File;

import com.relevantcodes.extentreports.ExtentReports;
import com.relevantcodes.extentreports.NetworkMode;

public class ExtentManager {
	private static ExtentReports extent;

	public synchronized static ExtentReports getReporter(String filePath) {
		if (extent == null) {
			extent = new ExtentReports(filePath, true); // NetworkMode.OFFLINEhtml报告是使用离线的CSS和JS,
			// extent.assignProject("aaa"); // ONLINE使用报告本地目录的中的CSS与JS
			extent.loadConfig(new File("./extent-config.xml"));
		}
		return extent;
	}
}
