package cases;

import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Random;
//import java.util.UUID;

import base.TestBase;
import net.sf.json.JSONArray;
import net.sf.json.JSONObject;
import utils.APIUtil;


//this case define general cases which could be reused for more than twice..
public class ApiGeneralCases extends TestBase {
	
	//definition of conditions
	final static int noTradeNo			= 1<<1;
	final static int invalidTradeNo		= 1<<2;
	final static int validTradeNo		= 1<<3;
		
	final static int noPassword			= 1<<4;
	final static int invalidPassword	= 1<<5;
	final static int validPassword		= 1<<6;
	
	final static int invalidAccessToken	= 1<<7;
	final static int noAccessToken	    = 1<<8;
	final static int validAccessToken	= 1<<9;
	
	final static int noMetaData			= 1<<10;
	final static int invalidMetaData	= 1<<11;
	final static int validMetaData		= 1<<12;
	
	final static int noAssetCount		= 1<<13;
	final static int invalidAssetCount	= 1<<14;
	final static int validAssetCount	= 1<<15;
	
	final static int validDetail		= 1<<16;
	final static int invalidDetail		= 1<<17;
	
	final static int noSign				=1<<19;
	final static int invalidSign		=1<<18;
	final static int validSign			=1<<0;
	
	//definition of sub conditions
	final static int overlimit		=	1<<0;
	final static int specialchar 	=	1<<1;
	final static int floatnumber 	=	1<<2;
	final static int isnull			=	1<<3;
	final static int isminus		=	1<<4;
	final static int illegal		=	1<<5;
	final static int notexist		=	1<<6;
	final static int iszero			=	1<<7;
	final static int isChinese  	=	1<<8;
	final static int issame			=	1<<9;
	
	//condition recorder
	static int current_condition = 0;
	static int current_condition_sub = 0;	
	
	//limits
	final static int password_limit = 45;
	final static int tradeno_limit = 55;
	final static int assetNameLimit = 20;
	final static int assetUnitLimit = 6;
			
	static Random rand = new Random();
	static int tradeNoBase = rand.nextInt(10000);
	static String trade_no = new String();
	static String asset_amount = new String();
	static String asset_unit = new String();
	static String asset_name = new String();
	StringBuffer user_name = new StringBuffer("");  
	StringBuffer address = new StringBuffer("");  
	//private static String s = APIUtil.getRandomString(10);
	static int index = 0;
	
	LinkedHashMap<String, Object> parameter = new LinkedHashMap<String,Object>();
	
	LinkedHashMap<String, Object> thrMap = new LinkedHashMap<String, Object>();
	
	
	//this interface is not implemented in this class
	public void TestProcess(Object obj_param, JSONArray json_param) {
		
		
	}
	//this interface is not implemented in this class
	public void Confirmation(boolean issuccess, String ...param){
		
	}
	public void verification(JSONObject jsonObject)
	{
		switch (current_condition)
		{
		case invalidAccessToken		:
		case noAccessToken		:
			check.equals(jsonObject.getString("err_code"), "20015", "errorcode is not as expected");
			check.equals(jsonObject.getString("msg"), "授权码不存在或已过期", "msg is not as expected");
			Confirmation(false);
			break;
		
		case noTradeNo            :
			check.equals(jsonObject.getString("err_code"), "20017", "errorcode is not as expected");
			check.equals(jsonObject.getString("msg"), "账号凭据号为空", "msg is not as expected");
			Confirmation(false);
			break;
		
		case invalidTradeNo       :
			if(current_condition_sub == isnull)
			{
				check.equals(jsonObject.getString("err_code"), "10001", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "凭据号不能为空", "msg is not as expected");
				Confirmation(false);
			
			}
			if(current_condition_sub == illegal)
			{
				check.equals(jsonObject.getString("err_code"), "20020", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "请求号trade_no非法，格式错误", "msg is not as expected");
			
			}	
			if(current_condition_sub == overlimit)
			{
				check.equals(jsonObject.getString("err_code"), "20021", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "凭据号长度不能大于55", "msg is not as expected");
				Confirmation(false);
			}	
			
			break;
	
		default:
			break;
		}
	}
	
	//some general cases that can be used directly
	public void a_normaltest()
	{
		current_condition = 0;
		TestProcess(null,null);
	}

	public void b_accessTokenTest()
	{
		current_condition = noAccessToken;
		TestProcess(null,null);
		
		current_condition = invalidAccessToken;
		String conditions[]={
				"",
				"wrong_condition"
				
		};
		for (int i =0 ;i<2;++i)
		{
			TestProcess(conditions[i],null);
		}
		
		current_condition = validAccessToken;
		TestProcess(null,null);
	}

	public void c_passwordTest()
	{
		current_condition = noPassword;
		TestProcess(null,null);
		
		current_condition = invalidPassword;
		LinkedHashMap<String,Integer> conditions = new LinkedHashMap<String,Integer> ();
		conditions.put("",isnull);
		conditions.put(APIUtil.getRandomString(password_limit+1),overlimit);					
		for (Map.Entry<String, Integer> entry : conditions.entrySet()) {  
			 
			current_condition_sub = entry.getValue();
			TestProcess(entry.getKey(),null);
		}  
		current_condition = validPassword;
		String conditions1[]={
				APIUtil.getRandomString(password_limit),
				APIUtil.getRandomString(password_limit-1),			
		};
		for (int i =0 ;i<2;++i)
		{
			TestProcess(conditions1[i],null);
		}		
	}
	
	public void d_tradeNoTest()
	{
		current_condition = noTradeNo;
		TestProcess(null,null);
		
		current_condition = invalidTradeNo;
		LinkedHashMap<String,Integer> conditions = new LinkedHashMap<String,Integer> ();
		conditions.put("",isnull);
		conditions.put("$#@$#@",illegal);
		conditions.put(" "+APIUtil.getRandomString(4),illegal);
		conditions.put(APIUtil.getRandomString(tradeno_limit+1),overlimit);
		conditions.put(APIUtil.getRandomString(tradeno_limit+2),overlimit);		
		
		for (Map.Entry<String, Integer> entry : conditions.entrySet()) {  
			 
			current_condition_sub = entry.getValue();
			TestProcess(entry.getKey(),null);
		}  
		
		current_condition = validTradeNo;
		String conditions1[]={
				APIUtil.getRandomString(tradeno_limit),
				APIUtil.getRandomString(tradeno_limit-1),
				
		};
		for (int i =0 ;i<2;++i)
		{
			TestProcess(conditions1[i],null);
		}	
		
	}

	public void e_metaDataTest()
	{
		current_condition = noMetaData;
		TestProcess(null,null);
		current_condition = invalidMetaData;
		TestProcess(APIUtil.getRandomString(65535),null);
		current_condition = validMetaData;
		TestProcess(null,null);		
	}
		
	private JSONArray detailLayout(int prameter_index,String parameter)
	{
		JSONArray detail = new JSONArray();
		JSONObject detailmember = new JSONObject();
		
		switch (prameter_index)
		{
		case 1:
			if(!APIUtil.getbool(current_condition_sub&isnull))
			{
				detailmember.put("asset_amount", parameter);				
			}
			detailmember.put("start" ,"1451535556");
			detailmember.put("length" ,"31536000");
			detailmember.put("ext" ,"xxxx");
			
		break;
		case 2:
			detailmember.put("asset_amount", "19999");
			if(!APIUtil.getbool(current_condition_sub&isnull))
			{
				detailmember.put("start", parameter);				
			}
			
			detailmember.put("length" ,"31536000");
			detailmember.put("ext" ,"xxxx");
			break;
		case 3:
			detailmember.put("asset_amount", "19999");
			detailmember.put("start" ,"1451535556");
			if(!APIUtil.getbool(current_condition_sub&isnull))
			{
				detailmember.put("length", parameter);				
			}
			detailmember.put("ext" ,"xxxx");
			break;
		case 4:
			detailmember.put("asset_amount", "19999");
			detailmember.put("start" ,"1451535556");
			detailmember.put("length" ,"31536000");
			if(!APIUtil.getbool(current_condition_sub&isnull))
			{
				detailmember.put("ext", parameter);				
			}
			break;
		default:
			break;
		
		
		}
		detail.add(detailmember);
		return detail;
		
	}
	public void f_detailTest()
	{
		current_condition = invalidDetail|validAssetCount;
		String[][] param = new String[][]{
				{"9999999999999999999999999999999999999999999999999999999","9999999999999999999999999999999999999999999999999999999","9999999999999999999999999999999999999999999999999999999"},
				{"","",""},
				{"-1","-1","-8"},
				{"数量","开始","长度"},
				{"%￥#","￥%￥#","@%#￥"},
				{"0","0","0"},
				{"18888","",""},
				{"3.1415","3.1415","3.1415"},
				};
		
		for (int i =1;i<4;i++)
		{
			int sub_condition[] = {
					overlimit,
					isnull,
					isminus,
					isChinese,
					illegal,
					iszero,
					floatnumber
			};
			for(int j = 0;j< 7;j++)
			{
				current_condition_sub = sub_condition[j];	
				
				TestProcess("1999999",detailLayout(i,param[j][i-1]));				
			}
		}
			
	}
	public void g_signTest()
	{
		current_condition = noSign;
		TestProcess(null,null);
		
		current_condition = invalidSign;
		LinkedHashMap<String,Integer> conditions = new LinkedHashMap<String,Integer> ();
		conditions.put("",isnull);
		conditions.put("-1",isminus);
		conditions.put("0",illegal);
		conditions.put("#$%#",illegal);	
		conditions.put("数量",illegal);	
		conditions.put(APIUtil.getRandomString(100),illegal);	
		conditions.put(APIUtil.getRandomString(15),illegal);
			
		for (Map.Entry<String, Integer> entry : conditions.entrySet()) {  
			 
			current_condition_sub = entry.getValue();
			TestProcess(entry.getKey(),null);
		}  
		
		current_condition = validSign;
		TestProcess(null,null);
		
	}
	//some tests need to pass in the condition definition
	public void assetCodeTest(int no,int invalid,int valid)
	{
		current_condition = no;
		TestProcess(null,null);
		
		current_condition = invalid;
		LinkedHashMap<String,Integer> conditions = new LinkedHashMap<String,Integer> ();
		conditions.put("",isnull);
		conditions.put("$#@$#@",illegal);
		conditions.put(APIUtil.getRandomString(100),illegal);
		conditions.put(APIUtil.getRandomString(1000),illegal);		
		
		for (Map.Entry<String, Integer> entry : conditions.entrySet()) {  
			 
			current_condition_sub = entry.getValue();
			TestProcess(entry.getKey(),null);
		}  
		
		current_condition = valid;
		TestProcess(null,null);		
	}
	
	public void addressTest(int no, int invalid)
	{
		
		current_condition = no;
		TestProcess(null,null);
		
		current_condition = invalid;
		LinkedHashMap<String,Integer> conditions = new LinkedHashMap<String,Integer> ();
		conditions.put("",isnull);
		conditions.put("-1",isminus);
		conditions.put("0",illegal);
		conditions.put("#$%#",illegal);	
		conditions.put("数量",illegal);	
		conditions.put(APIUtil.getRandomString(100),illegal);	
		conditions.put("user2",illegal);
			
		for (Map.Entry<String, Integer> entry : conditions.entrySet()) {  
			 
			current_condition_sub = entry.getValue();
			TestProcess(entry.getKey(),null);
		}  	
		
	}
	
	//general setup:if is not no {if is invalid,use param. else use default}
	public void generalSetup(int compare,int no,int invalid,String key,Object param,Object default_value,LinkedHashMap<String, Object> target )
	{	
		if(!APIUtil.getbool(compare&no))
		{
			if(APIUtil.getbool(compare&invalid))
				target.put(key,param);
			else
				target.put(key,default_value);
		}		
	}
	//general setup:if is not no {if is invalid|valid,use param. else use default}
	public void generalSetup(int compare,int no,int invalid,int valid,String key,Object param,Object default_value,LinkedHashMap<String, Object> target)
	{
		if(!APIUtil.getbool(compare&no))
		{
			if(APIUtil.getbool(compare&(invalid|valid)))
				target.put(key,param);
			else
				target.put(key,default_value);
		}
			
		
	}
	
	
	
	
		
}