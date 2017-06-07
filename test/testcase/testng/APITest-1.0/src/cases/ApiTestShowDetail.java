package cases;

import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Random;

import org.testng.annotations.Test;

import base.TestBase;
import net.sf.json.JSONArray;
import net.sf.json.JSONObject;
import utils.APIUtil;
import utils.HttpUtil;



public class ApiTestShowDetail extends ApiGeneralCases {

	//definition of conditions
	
	
	final static int invalidAssetCode	= 1<<31;
	final static int noAssetCode	    = 1<<30;
	final static int validAssetCode		= 1<<29;
			
	private int current_condition = 0;
	
	static Random rand = new Random();
	static int tradeNoBase = rand.nextInt(10000);
	static int index = 0;
	
	LinkedHashMap<Integer, String> asset_unit = new LinkedHashMap<Integer,String> ();
	LinkedHashMap<Integer, String> asset_type = new LinkedHashMap<Integer,String> ();
	
	
	
	static String asset_name = "某种资产";

	private void mapInit()
	{
		String asset_unit_str[] = {
				"个",
				"枚",
				"份",
				"分",
				"积分",
				"颗",
				"棵",
				"粒",
				"元",
				"角",
				"分",
				"吨",
				"千克",
				"克",
				"毫克",
				"其他"
		};
		
		for(int i = 0;i<asset_unit_str.length;i++)
		asset_unit.put(i+1,asset_unit_str[i]);
		
		
		String asset_type_str[] = {
				"商业积分",
				"保险卡单",
				"网络互助",
				"慈善公益",
				"P2P理财",
				"数字黄金",
				"游戏道具",
				"其他"

		};
		
		for(int i = 0;i<asset_type_str.length;i++)
		asset_type.put((10100+i*100),asset_type_str[i]);	

	}
	
	private void TestShowDetail(int index,Object param, JSONArray detail)
	{
		//init the map
		mapInit();
		access_token = getToken();
		StringBuffer user_name1 = new StringBuffer("");  
		StringBuffer address1 = new StringBuffer("");  
		//StringBuffer user_name2 = new StringBuffer("");  
		//StringBuffer address2 = new StringBuffer("");  
		createUser(access_token, user_name1,address1);
		//createUser(access_token, user_name2,address2);
		StringBuffer assetcode1 = new StringBuffer("");
		//StringBuffer assetcode2 = new StringBuffer("");
		
		//issue an asset
		
		String name = user_name1.toString();
		String address = address1.toString();
		String asset_unit_str = asset_unit.get(index);
		String metadata = metadataFormat(null, "", "", "");
		issueAsset(access_token,name,address,assetcode1,asset_name,asset_unit_str,metadata);
					
		String api_path = "/asset/v1/showDetail" ;
		parameter.clear();
		
		if(!APIUtil.getbool(current_condition&noAssetCode))
		{
			parameter.put("asset_code",assetcode1.toString());
			if(APIUtil.getbool(current_condition&invalidAssetCode))
				parameter.put("asset_code",(String)param);
			else
				parameter.put("asset_code",assetcode1.toString());
		}
		
		if(!APIUtil.getbool(current_condition&noAccessToken))
		{
			parameter.put("access_token",(String)param);
			if(APIUtil.getbool(current_condition&invalidAccessToken))
				parameter.put("access_token",(String)param);
			else
				parameter.put("access_token",access_token);
		}
		
		System.out.println("=========Get detail========");
		String result = HttpUtil.dogetApi(api_url, api_path, parameter);
		JSONObject jsonObject =JSONObject.fromObject(result);  
		//Verification(jsonObject);
		
	}
	/*private void Verification(JSONObject jsonObject)
	{
		switch (current_condition)
		{
			case validAccessToken	:
			case validUser:	
			case userHasManyAsset:
			case 0:
				check.equals(jsonObject.getString("err_code"), "0", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "操作成功", "msg is not as expected");
				break;
			
			case invalidAccessToken		:
			case noAccessToken		:
				check.equals(jsonObject.getString("err_code"), "20015", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "授权码不存在或已过期", "msg is not as expected");
				break;
			case noUser:
			case invalidUser:
				check.equals(jsonObject.getString("err_code"), "20031", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "账户不存在或账户未激活", "msg is not as expected");
				break;			
		}		
	}*/	

	public void a_normaltest()
	{
		current_condition = 0;
		TestShowDetail(1,null,null);
	}

	public void b_accessTokenTest()
	{
		current_condition = noAccessToken;
		TestShowDetail(3,null,null);
		
		current_condition = invalidAccessToken;
		String conditions[]={
				"",
				"wrong_condition"
				
		};
		for (int i =0 ;i<2;++i)
		{
			TestShowDetail(3,conditions[i],null);
		}
		
		current_condition = validAccessToken;
		TestShowDetail(3,null,null);
	}
	@Test
	public void c_AssetCodeTest()
	{
		current_condition = noAssetCode;
		TestShowDetail(4,null,null);
		
		current_condition = invalidAssetCode;
		String conditions[]={
				"",
				"wronguser",
				APIUtil.getRandomString(100)
				
		};
		for (int i =0 ;i<3;++i)
		{
			TestShowDetail(4,conditions[i],null);
		}
		
		current_condition = validAssetCode;
		
		TestShowDetail(4,null,null);
		
		
	}

		 
}