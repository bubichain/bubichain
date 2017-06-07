package base;

import java.lang.reflect.Method;

import org.testng.annotations.AfterSuite;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.BeforeMethod;
import org.testng.annotations.BeforeSuite;
import listener.ExtentManager;
import model.Account;
import utils.DateHandler;

import com.relevantcodes.extentreports.ExtentReports;
import com.relevantcodes.extentreports.ExtentTest;
import cn.bubi.blockchain.adapter.BlockChainAdapter;
import utils.JsonReader;

public class TestBase {
	
	static JsonReader jr = new JsonReader();

	static String testjson = jr.ReadFile("./test.json");

	public static String led_acc = jr.parseString(testjson, "source_address");
	public static String led_pri = jr.parseString(testjson, "private_key");
	public static String led_pub = jr.parseString(testjson, "public_key");
	public static String baseUrl = jr.parseString(testjson, "url2.0");
	public static String get_Url = jr.parseString(testjson, "url2.0");
	public static String get_Url2 = jr.parseString(testjson, "url19343");//url_get
	public static String url = jr.parseString(testjson, "url");
	public static String bubiport = jr.parseString(testjson, "bubiport");
	public static String url_getAccInfo = "http://"+url+":"+bubiport+"/";
	public static int thread_count = 1;
	public static int timeout = 7;
	public static String pull_ip = jr.parseString(testjson, "pull_ip");
	public static String push_ip = jr.parseString(testjson, "push_ip");
	public static String pull_port = jr.parseString(testjson, "pull_port");
	public static String push_port = jr.parseString(testjson, "push_port");
	public static int fee =Integer.parseInt(jr.parseString(testjson,"base_fee"));
	public static long base_reserve = Long.parseLong(jr.parseString(testjson,
			"base_reserve"));

	public static long init_balance = Long.parseLong(jr.parseString(testjson,
			"init_balance"));
	public static int close_time = 4;
	public static int resend_count = 0;
	private String push_peer = "192.168.10.202:4053";
	private String pull_peer = "192.168.10.203:4054";
	public static BlockChainAdapter bbc = new BlockChainAdapter(push_ip+":"+push_port, pull_ip+":"+pull_port, thread_count);
	public BlockChainAdapter bbc_peer = new BlockChainAdapter(push_peer, pull_peer, thread_count);
	public static Account genesis = new Account(led_acc, led_pri, led_pub);
	public CheckPoint check = new CheckPoint();
	private static ExtentReports extentReports;
	protected ExtentTest test;
	private static String reportPath = "report/" + DateHandler.getTimeStamp()
			+ ".html";
	@BeforeMethod
	public void testBegin(Method method) {
		Log.info("测试方法： " + method.getName() + "开始执行~~~~~");
	}
	@BeforeSuite
	public void start(){
		bbc.Start();
	}
	@BeforeClass
	public void classBegin() {
		
		System.out.println("baseUrl: " + baseUrl);
		System.out.println("getUrl: " + get_Url);
		Log.info(this.getClass().getCanonicalName() + "开始执行~~~~~");
	}
	
	private static String reportLocation = "report/ExtentReport.html";

	@BeforeSuite
	public void beforeSuite() {
		extentReports = ExtentManager.getReporter(reportLocation);
	}
	@AfterSuite
	protected void afterSuite() {
		// extentReports.close();
	}

	public static ExtentReports getExtent() {
		return extentReports;
	}

	public static String getReportPath() {
		return reportPath;
	}
}
