package cases;

import java.util.Calendar;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Random;
import org.testng.annotations.Test;

import base.TestBase;
import net.sf.json.JSONArray;
import net.sf.json.JSONObject;
import utils.APIUtil;
import utils.HttpUtil;

public class BumengTestSetUp extends TestBase {
	
	//beta env
	//String server_url = "https://beta_api.bumeng.cn/api";
	//String qianxiang_id = "62fb7101ca1e869e36f4a5144a2aa332";
	//String qianxiang_key = "e81e9c85a3ca26b40f0344b77a0453e2";
	
	//String server_url = "https://api.bubi.cn";
	//String qianxiang_id = "befa6fbf5548c9309abd3d32a93b3deb";
	//String qianxiang_key = "4603d752d86b7f7368634b079a9fe654";
	
	//String app_id = "99de72729f1564389bfb89d6b7e4976b";
	//String app_key = "b0384b2fc1011a688c79ce1dbecb89b0";
	
	
	String app_id = "736da23c77aa9c08c2b29fadfeac922d";
	String app_key ="972e2510655261dd39bd6d29dc5ce682";
	
	static Random rand = new Random();
	static int tradeNoBase = rand.nextInt(255);
	static String trade_no = new String();
	static int index = 0;
	
	public static StringBuffer user_name1 = new StringBuffer("");  
	public static StringBuffer address1 = new StringBuffer("");  
	public static StringBuffer assetcode = new StringBuffer("");
	
	public static StringBuffer user_name2 = new StringBuffer("");  
	public static StringBuffer address2 = new StringBuffer("");  	
	
	public static StringBuffer user_name3 = new StringBuffer("");  
	public static StringBuffer address3 = new StringBuffer("");  	
	
	static String asset_unit = "元";
	static String asset_name = "某种资产";
	

	private void MultipleSend(String token,Map<String, Object> thrMap,JSONArray detail,int count)
	{
			
			String api_path = "/asset/v1/send" ;
			LinkedHashMap<String, String> parameter = new LinkedHashMap<String,String>();
			parameter.put("access_token",token);
			for (int i = 0;i<count; i++)
			{
			Map<String, Object> thrMapforsign = new LinkedHashMap<String, Object>();
			
			
			thrMapforsign.put("current_string", thrMap.get("current_string"));			
			thrMapforsign.put("asset_code",thrMap.get("asset_code") );	
			thrMapforsign.put("from_bubi_address", thrMap.get("from_bubi_address"));	
			thrMapforsign.put("to_bubi_address", thrMap.get("to_bubi_address"));	
			
			
			
			if(detail!=null)
			{
				thrMapforsign.put("details", detail);		
				
			}
			
			
			trade_no = Long.toString((tradeNoBase++)+(System.currentTimeMillis()<<8));
			thrMapforsign.put("trade_no", trade_no);	
			thrMapforsign.put("asset_amount", Integer.toString(count+i));	
			String sign1 = APIUtil.sign(thrMapforsign, zhongtuobang_key);
			thrMapforsign.put("sign", sign1);					
			thrMapforsign.put("password", thrMap.get("password"));	
			JSONObject jsonBody = JSONObject.fromObject(thrMapforsign);
			System.out.println("=========Send asset========");				
			String result = HttpUtil.dopostApi(api_url, api_path, parameter, jsonBody);
			
		}	
		
	}	
	
	

	public void SingleAssetSendManytimes(String assetcode)
	{
		String token  = getToken(api_url,zhongtuobang_id,zhongtuobang_key);
		Map<String, Object> thrMap = new LinkedHashMap<String, Object>();
		thrMap.put("current_string","MqUq6TASQju57VcH");
	    thrMap.put("asset_code",assetcode);
	    thrMap.put("from_bubi_address","bubiV8iEzmbVLwAk46eNWErR3X2MfNauVFqeWDEw");
	    thrMap.put("to_bubi_address","bubiV8hzekwc7SERS7y3JHxtEPzoNsAp8PeuH57g");
	    thrMap.put("password","MuhMUOYbjz");	    
	    MultipleSend(token,thrMap,null,2);
		
	}
		
	/*
	 "current_string":"MqUq6TASQju57VcH",
    "trade_no":"38057506609377cm11111k",
    "asset_code":"2UX4xvQ4aXQdfKMP2bGnfKDAakjcRrvizTxKJrZqsdKmvWsw8CLUyA7J65bgLtUsCCpgdZiDtmBFZ5UboaQYQGYYnSWuUhpDRnanV5v14m8YGmgVHRLNNQAbNtifg83KFm7ihnDRFtejuF8Z",
    "asset_amount":"123",
    "from_bubi_address":"bubiV8iEzmbVLwAk46eNWErR3X2MfNauVFqeWDEw",
    "to_bubi_address":"bubiV8hzekwc7SERS7y3JHxtEPzoNsAp8PeuH57g",
    "sign":"bce07af8b8678b46a7170b69655b926e",
    "password":"MuhMUOYbjz",
    "metadata":"444"
	 * */

	public void MultiAssetSendManytimes()
	{
		String assetlist[]={
					"2UX4xvQ4aXQdfKMP2bGnfKDAakjcRrvizTxKJrZqsdKmvWsw8CLUyA7J65bgLtUsCCpgdZiDtmBFZ5UboaQYQGYYnSWuUhpDRnanV5v14m8YGmgVHRLNNQAbNtifg83KFm7ihnDRFtejuF8Z",
					"2UX4xvQ4aXQdfKMP2bGnfKDAakjcRrvizTxKJrZqsdKmvWsw8CLUyA7J64yYef8y2krW3QGA2MraLhwewwkFqvGw6o3RQXMNo5DQ1ai6CHC2i7m3TPa7DZvUZiRoJNrKNMZXn4bYBYbHFBFL",
					"2UX4xvQ4aXQdfKMP2bGnfKDAakjcRrvizTxKJrZqsdKmvWsw8CLUyA7J58gYu2QCZrfnEXzVCM2joiGtdTYCSWpyJi7y6T9T8aqFCbaxwfMji6HT1EX5Q6NvLQ6zd9jM1PGG1xJVcVWWBMao",
					"2UX4xvQ4aXQdfKMP2bGnfKDAakjcRrvizTxKJrZqsdKmvWsw8CLUyA7J5Cb6kkDhmWBgdvBLY4t1NRKw81VnVmxp3D9tcYbKSFvAFMB8Mv6ALruUtabmtmjPRBRf2rTzhLdsJE3mkpeZ1PyC"
		};
		 for(String tmp:assetlist){
			 SingleAssetSendManytimes(tmp);
	    }
	}
	
	//issue multiple asset, and for a single asset send it to a different user

	public void MultiAssetToMultiUser()
	{
		int count = 20;
		
		String token = getToken(api_url,madianzhang_id,madianzhang_secret);
		Calendar now = Calendar.getInstance();  
		String header  = Integer.toString(now.get(Calendar.YEAR))+Integer.toString(now.get(Calendar.MONTH) + 1)+Integer.toString(now.get(Calendar.DAY_OF_MONTH))+"多资产发给多用户";
		issueAndSendMultiAsset(api_url,token, "v3nC4uELR0","bubiV8hwAKE6uyiswZWcLfnynbpcRbc3gvyU79pp",header,"f",count);
		
	}
	//this test create an issuer to issue an asset,then 
	@Test
	public void batchOpt() 
	{
		String issue_amount = "1670005";
		String batch_num = "12";
		String batch_single_amount = "10";
		
		String token  = getToken(api_url,madianzhang_id,madianzhang_secret);
		//String token  = getToken(api_url,yangguang_id,yangguang_key);
		//String token  = getToken(api_url,jinxiang_id,jinxiang_secret);
		//String token  = getToken(server_url,qianxiang_id,qianxiang_key);
		//String token  = getToken(server_url,app_id,app_key);
		Calendar now = Calendar.getInstance();  
		String header  = Integer.toString(now.get(Calendar.YEAR))+Integer.toString(now.get(Calendar.MONTH) + 1)+Integer.toString(now.get(Calendar.DAY_OF_MONTH))+"批量";
		
		//this part is used to create a new usr to blend send
		StringBuffer name = new StringBuffer();
		StringBuffer address = new StringBuffer();
		createUser(token, name, address);
		batchSendAsset(token, address.toString(),name.toString(),header,madianzhang_secret,issue_amount,batch_single_amount,batch_num);
		//yangguang address
		//batchSendAsset(token, "bubiV8ib39JDpLi81WB2TcHuxi5zz7BenwhHkviM","z8Fgwbqukte1HEu3TZTMXMgDmwy3ixTbOu3CjJndTammF",header,zhongtuobang_key,issue_amount,batch_single_amount,batch_num);
		//zhongtuobang address
		//batchSendAsset(token, "bubiV8i4VZSwFYWA5jYS1BmSZwGnJsjcQV4E5r9B","2MMd2OgqwH",header,madianzhang_secret,issue_amount,batch_single_amount,batch_num);
				
	}

	public void batchissue()
	{
		for(int i = 0;i<9;i++)
		{
			batchOpt();
		}
		
		
	}
	
	//this test create two users,one user issues an asset and send to another user.

	public void normalsend()
	{
		
		//int send_amount = 1;
		String id = yangguang_id;
		String secret = yangguang_key;
		
		
		String name = companyname.get(id);
		//String token  = getToken(api_url,zhongtuobang_id,zhongtuobang_key);
		String token  = getToken(api_url,id,secret);
		//String token  = getToken(server_url,app_id,app_key);
		String issue_amount = "8";
		String send_amount = "8";
		String grant_amount = "1";
		StringBuffer user_name1 = new StringBuffer();
		StringBuffer user_name2 = new StringBuffer();
		StringBuffer address1 = new StringBuffer();
		StringBuffer address2 = new StringBuffer();
		StringBuffer assetcode = new StringBuffer();
		StringBuffer assetcode1 = new StringBuffer();
		createUser(token, user_name1,address1);		
		
		//header  = name+Integer.toString(now.get(Calendar.YEAR))+Integer.toString(now.get(Calendar.MONTH) + 1)+Integer.toString(now.get(Calendar.DAY_OF_MONTH))+"阳光在其他平台发行";
		//issueAsset(token, "KYniI6is0P","bubiV8hxAXUi2P3omJpdvJHuvyig7xh1upqjS9o9",assetcode1,header,asset_unit,issue_amount);
		
		for(int i = 0;i<1;i++)
		{
			createUser(token, user_name2,address2);
			Calendar now = Calendar.getInstance();  
			String header  = name+Integer.toString(now.get(Calendar.YEAR))+Integer.toString(now.get(Calendar.MONTH) + 1)+Integer.toString(now.get(Calendar.DAY_OF_MONTH))+"测试";
			issueAsset(token, user_name2.toString(),address2.toString(),assetcode,header,asset_unit,issue_amount);
			
		String result = sendAsset(false,token, address2.toString(),user_name2.toString(),address1.toString(), assetcode.toString(),send_amount,secret);
		
		
	/*	result = grantAsset(token, address1.toString(),user_name1.toString(),address2.toString(), assetcode.toString(),grant_amount,secret);
		user_name2.delete(0,user_name2.length());
		address2.delete(0,address2.length());
		result = sendAsset(false,token, address1.toString(),user_name1.toString(),address2.toString(), assetcode.toString(),send_amount,yangguang_key);
		*/
	
		}
		
		
		//user1 send asset to user2, then user2 give all back.
		/*createUser(token, user_name2,address2);
		String result = sendAsset(false,token, address1.toString(),user_name1.toString(),address2.toString(), assetcode.toString(),send_amount,zhongtuobang_key);
		*/
		
		/*String result = grantAsset(token, "bubiV8hxAXUi2P3omJpdvJHuvyig7xh1upqjS9o9","KYniI6is0P","bubiV8i2tzu5WZWaMT833Nv9vA4UcwCBWNGxE8Nb", "2UX4xvQ4aWztRo24PGpPe7wnuFg9ZVfYEje1fZ8xsN81yeGUoRtYrTnYuMZHrhPkPvhLWCbDyWv7c3QPin6EScZgSzMzqveQNzLbjHPEevLpoLcyRXZ9bZoHi7wwtajwW2uhSL2wu2CyKEMQ"
,grant_amount,secret);
		user_name2.delete(0,user_name2.length());
		address2.delete(0,address2.length());
		
		*/
		//JSONObject ret = new JSONObject();
		//ret.put("issuer", user_name1.toString());

		
	}
	
	//A send to B, use C's token
	

	public void threePeople()
	{
		String id = yangguang_id;
		String secret = yangguang_key;
		
		String token  = getToken(api_url,id,secret);
		
		String issue_amount = "8";
		String send_amount = "3";
		String grant_amount = "1";
		StringBuffer user_name1 = new StringBuffer();
		StringBuffer user_name2 = new StringBuffer();
		StringBuffer address1 = new StringBuffer();
		StringBuffer address2 = new StringBuffer();
		StringBuffer assetcode = new StringBuffer();
		StringBuffer assetcode1 = new StringBuffer();
		createUser(token, user_name1,address1);	
		
		id = zhongtuobang_id;
		secret = zhongtuobang_key;
		token  = getToken(api_url,id,secret);
		createUser(token, user_name2,address2);	
		
		id = zhongtuobang_id;
		secret = zhongtuobang_key;
		token  = getToken(api_url,id,secret);
		
		
		String name = companyname.get(id);
		Calendar now = Calendar.getInstance();  
		String header  = name+Integer.toString(now.get(Calendar.YEAR))+Integer.toString(now.get(Calendar.MONTH) + 1)+Integer.toString(now.get(Calendar.DAY_OF_MONTH))+"测试";
		
		issueAsset(token, user_name2.toString(),address2.toString(),assetcode,header,asset_unit,issue_amount);
		
		id = yangguang_id;
		secret = yangguang_key;
		
		token  = getToken(api_url,id,secret);
		
		String result = sendAsset(false,token, address2.toString(),user_name2.toString(),address1.toString(), assetcode.toString(),send_amount,secret);
		
	}
	

	public void create()
	{
		String id = yangguang_id;
		String secret = yangguang_key;		
		String token  = getToken(api_url,id,secret);			
		StringBuffer user_name1 = new StringBuffer();		
		StringBuffer address1 = new StringBuffer();		
		createUser(token, user_name1,address1);	
		
		
	}

	public void createandissue()
	{
		String id = yangguang_id;
		String secret = yangguang_key;		
		String token  = getToken(api_url,id,secret);			
		StringBuffer user_name1 = new StringBuffer();		
		StringBuffer address1 = new StringBuffer();		
		createUser(token, user_name1,address1);	
		
		String name = companyname.get(id);
		Calendar now = Calendar.getInstance();  
		String header  = name+Integer.toString(now.get(Calendar.YEAR))+Integer.toString(now.get(Calendar.MONTH) + 1)+Integer.toString(now.get(Calendar.DAY_OF_MONTH))+"测试";
		issueAsset(token, user_name1.toString(),address1.toString(),assetcode,header,asset_unit,"6666");
		
				
	}

	public void createIssueSendAll()
	{
		String id = zhongtuobang_id;
		String secret = zhongtuobang_key;		
		String token  = getToken(api_url,id,secret);			
		StringBuffer user_name1 = new StringBuffer();		
		StringBuffer address1 = new StringBuffer();		
		StringBuffer user_name2 = new StringBuffer();		
		StringBuffer address2 = new StringBuffer();		
		createUser(token, user_name1,address1);	
		createUser(token, user_name2,address2);	
		
		String issueamount = "100";
		String sendamount = "100";
		
		String name = companyname.get(id);
		Calendar now = Calendar.getInstance();  
		String header  = name+Integer.toString(now.get(Calendar.YEAR))+Integer.toString(now.get(Calendar.MONTH) + 1)+Integer.toString(now.get(Calendar.DAY_OF_MONTH))+"测试";
		issueAsset(token, user_name1.toString(),address1.toString(),assetcode,header,asset_unit,issueamount);
		sendAsset(false,token, address1.toString(),user_name1.toString(),address2.toString(), assetcode.toString(),sendamount,secret);		
				
	}

	public void createIssueGrantAll()
	{
		String id = zhongtuobang_id;
		String secret = zhongtuobang_key;		
		String token  = getToken(api_url,id,secret);			
		StringBuffer user_name1 = new StringBuffer();		
		StringBuffer address1 = new StringBuffer();		
		StringBuffer user_name2 = new StringBuffer();		
		StringBuffer address2 = new StringBuffer();		
		createUser(token, user_name1,address1);	
		createUser(token, user_name2,address2);	
		
		String issueamount = "100";
		String sendamount = "100";
		
		String name = companyname.get(id);
		Calendar now = Calendar.getInstance();  
		String header  = name+Integer.toString(now.get(Calendar.YEAR))+Integer.toString(now.get(Calendar.MONTH) + 1)+Integer.toString(now.get(Calendar.DAY_OF_MONTH))+"测试";
		issueAsset(token, user_name1.toString(),address1.toString(),assetcode,header,asset_unit,issueamount);
		grantAsset(token, address1.toString(),user_name1.toString(),address2.toString(), assetcode.toString(),sendamount,secret);		
		
	}

	public void receiveandsendall()
	{
		String id = zhongtuobang_id;
		String secret = zhongtuobang_key;		
		String token  = getToken(api_url,id,secret);			
		StringBuffer user_name1 = new StringBuffer();		
		StringBuffer address1 = new StringBuffer();		
		StringBuffer user_name2 = new StringBuffer();		
		StringBuffer address2 = new StringBuffer();		
		createUser(token, user_name1,address1);	
		createUser(token, user_name2,address2);	
		
		String issueamount = "1000";
		String sendamount = "100";
		
		String name = companyname.get(id);
		Calendar now = Calendar.getInstance();  
		String header  = name+Integer.toString(now.get(Calendar.YEAR))+Integer.toString(now.get(Calendar.MONTH) + 1)+Integer.toString(now.get(Calendar.DAY_OF_MONTH))+"测试";
		issueAsset(token, user_name1.toString(),address1.toString(),assetcode,header,asset_unit,issueamount);
		sendAsset(false,token, address1.toString(),user_name1.toString(),address2.toString(), assetcode.toString(),sendamount,secret);		
		sendAsset(false,token, address2.toString(),user_name2.toString(),address1.toString(), assetcode.toString(),sendamount,secret);		
		
	}
	

	public void receiveandgrantall()
	{
		String id = zhongtuobang_id;
		String secret = zhongtuobang_key;		
		String token  = getToken(api_url,id,secret);			
		StringBuffer user_name1 = new StringBuffer();		
		StringBuffer address1 = new StringBuffer();		
		StringBuffer user_name2 = new StringBuffer();		
		StringBuffer address2 = new StringBuffer();		
		createUser(token, user_name1,address1);	
		createUser(token, user_name2,address2);	
		
		String issueamount = "1000";
		String sendamount = "100";
		
		String name = companyname.get(id);
		Calendar now = Calendar.getInstance();  
		String header  = name+Integer.toString(now.get(Calendar.YEAR))+Integer.toString(now.get(Calendar.MONTH) + 1)+Integer.toString(now.get(Calendar.DAY_OF_MONTH))+"测试";
		issueAsset(token, user_name1.toString(),address1.toString(),assetcode,header,asset_unit,issueamount);
		sendAsset(false,token, address1.toString(),user_name1.toString(),address2.toString(), assetcode.toString(),sendamount,secret);		
		grantAsset(token, address2.toString(),user_name2.toString(),address1.toString(), assetcode.toString(),sendamount,secret);		
		
	}

	public void issue2andsend1all()
	{
		String id = zhongtuobang_id;
		String secret = zhongtuobang_key;		
		String token  = getToken(api_url,id,secret);			
		StringBuffer user_name1 = new StringBuffer();		
		StringBuffer address1 = new StringBuffer();		
		StringBuffer user_name2 = new StringBuffer();		
		StringBuffer address2 = new StringBuffer();		
		createUser(token, user_name1,address1);	
		createUser(token, user_name2,address2);	
		
		String issueamount = "1000";
		String sendamount = "1000";
		
		String name = companyname.get(id);
		Calendar now = Calendar.getInstance();  
		String header  = name+Integer.toString(now.get(Calendar.YEAR))+Integer.toString(now.get(Calendar.MONTH) + 1)+Integer.toString(now.get(Calendar.DAY_OF_MONTH))+"测试";
		issueAsset(token, user_name1.toString(),address1.toString(),assetcode,header,asset_unit,issueamount);
		
		assetcode.delete(0, assetcode.length());
		now = Calendar.getInstance();  
		header  = name+Integer.toString(now.get(Calendar.YEAR))+Integer.toString(now.get(Calendar.MONTH) + 1)+Integer.toString(now.get(Calendar.DAY_OF_MONTH))+"测试";
		issueAsset(token, user_name1.toString(),address1.toString(),assetcode,header,asset_unit,issueamount);
		
		sendAsset(false,token, address1.toString(),user_name1.toString(),address2.toString(), assetcode.toString(),sendamount,secret);		
		
	}
	

	public void addingAsset()
	{
		String id = zhongtuobang_id;
		String secret = zhongtuobang_key;	
		int issuenum = 387;
		String token  = getToken(api_url,id,secret);			
		StringBuffer user_name1 = new StringBuffer();		
		StringBuffer address1 = new StringBuffer();		
		
		createUser(token, user_name1,address1);	
		createUser(token, user_name2,address2);	
		
		String issueamount = "1000";
				
		for(int i = 0;i<issuenum;i++)
		{
			assetcode.delete(0, assetcode.length());
			StringBuffer user_name2 = new StringBuffer();		
			StringBuffer address2 = new StringBuffer();		
			String name = companyname.get(id);
			Calendar now = Calendar.getInstance();  
			String header  = name+Integer.toString(now.get(Calendar.YEAR))+Integer.toString(now.get(Calendar.MONTH) + 1)+Integer.toString(now.get(Calendar.DAY_OF_MONTH))+"测试"+Integer.toString(i);
			issueAsset(token, user_name1.toString(),address1.toString(),assetcode,header,asset_unit,issueamount);
			
			createUser(token, user_name2,address2);	
			sendAsset(false,token, address1.toString(),user_name1.toString(),address2.toString(), assetcode.toString(),Integer.toString(i+1),secret);		
				
		}
		
		
	}
	

	public void declineAsset()
	{
		String id = zhongtuobang_id;
		String secret = zhongtuobang_key;	
		int issuenum = 10;
		String token  = getToken(api_url,id,secret);			
		StringBuffer user_name1 = new StringBuffer();		
		StringBuffer address1 = new StringBuffer();		
		
		createUser(token, user_name1,address1);	
		
		StringBuffer user_name2 = new StringBuffer();		
		StringBuffer address2 = new StringBuffer();		
		
		String issueamount = "1000";
				
		StringBuffer asset_code = new StringBuffer();
		String name = companyname.get(id);
		Calendar now = Calendar.getInstance();  
		String header  = name+Integer.toString(now.get(Calendar.YEAR))+Integer.toString(now.get(Calendar.MONTH) + 1)+Integer.toString(now.get(Calendar.DAY_OF_MONTH))+"测试";
		issueAsset(token, user_name1.toString(),address1.toString(),asset_code,header,asset_unit,issueamount);
		
		
		for(int i = 0;i<issuenum;i++)
		{
			assetcode.delete(0, assetcode.length());
			name = companyname.get(id);
			now = Calendar.getInstance();  
			header  = name+Integer.toString(now.get(Calendar.YEAR))+Integer.toString(now.get(Calendar.MONTH) + 1)+Integer.toString(now.get(Calendar.DAY_OF_MONTH))+"测试"+Integer.toString(i);
			issueAsset(token, user_name1.toString(),address1.toString(),assetcode,header,asset_unit,issueamount);
			
				
		}
		
		createUser(token, user_name2,address2);	
		sendAsset(false,token, address1.toString(),user_name1.toString(),address2.toString(), asset_code.toString(),issueamount,secret);		
		
		
	}

	//1~9 send or grant

	public void sendorgrant()
	{
		String id = yangguang_id;
		String secret = yangguang_key;	
		int sendnum = 1;
		String token  = getToken(api_url,id,secret);			
		StringBuffer user_name1 = new StringBuffer();		
		StringBuffer address1 = new StringBuffer();		
		
		createUser(token, user_name1,address1);	
		
		StringBuffer user_name2 = new StringBuffer();		
		StringBuffer address2 = new StringBuffer();		
		
			
		String issueamount = "1";
				
		StringBuffer asset_code = new StringBuffer();
		String name = companyname.get(id);
		Calendar now = Calendar.getInstance();  
		String header  = name+Integer.toString(now.get(Calendar.YEAR))+Integer.toString(now.get(Calendar.MONTH) + 1)+Integer.toString(now.get(Calendar.DAY_OF_MONTH))+"测试";
		issueAsset(token, user_name1.toString(),address1.toString(),asset_code,header,asset_unit,issueamount);
		
		
		for(int i = 0;i<sendnum;i++)
		{
			if(i%2==0)
			{
				user_name2.delete(0, user_name2.length());
				address2.delete(0, address2.length());
				createUser(token, user_name2,address2);	
				sendAsset(false,token, address1.toString(),user_name1.toString(),address2.toString(), asset_code.toString(),Integer.toString(i+2),secret);		
			}
			else
			{
				
				sendAsset(false,token, address2.toString(),user_name2.toString(),address1.toString(), asset_code.toString(),Integer.toString(i),secret);		
				
			}
		}
		
	
	}
	

	//send some than break, then send
	public void stopandsend()
	{
		String id = jinxiang_id;
		String secret = jinxiang_secret;	
		int sendnum = 8;
		String token  = getToken(api_url,id,secret);			
		StringBuffer user_name1 = new StringBuffer();		
		StringBuffer address1 = new StringBuffer();		
		
		createUser(token, user_name1,address1);	
		
		StringBuffer user_name2 = new StringBuffer();		
		StringBuffer address2 = new StringBuffer();		
		
			
		String issueamount = "10000000";
				
		StringBuffer asset_code = new StringBuffer();
		String name = companyname.get(id);
		Calendar now = Calendar.getInstance();  
		String header  = name+Integer.toString(now.get(Calendar.YEAR))+Integer.toString(now.get(Calendar.MONTH) + 1)+Integer.toString(now.get(Calendar.DAY_OF_MONTH))+"测试";
		issueAsset(token, user_name1.toString(),address1.toString(),asset_code,header,asset_unit,issueamount);
		
		
		for(int i = 0;i<sendnum;i++)
		{
			
				user_name2.delete(0, user_name2.length());
				address2.delete(0, address2.length());
				createUser(token, user_name2,address2);	
				sendAsset(false,token, address1.toString(),user_name1.toString(),address2.toString(), asset_code.toString(),Integer.toString(i+1),secret);		
			
		}
		
		sendAsset(false,token, address1.toString(),user_name1.toString(),address2.toString(), asset_code.toString(),Integer.toString(1000000),secret);		
		sendAsset(false,token, address1.toString(),user_name1.toString(),address2.toString(), asset_code.toString(),Integer.toString(10000000),secret);		
		
		
	
	}
	

	
	public void issueandaddissue()
	{
		String id = jinxiang_id;
		String secret = jinxiang_secret;	
				
		String token  = getToken(api_url,id,secret);			
		StringBuffer user_name1 = new StringBuffer();		
		StringBuffer address1 = new StringBuffer();		
		
		createUser(token, user_name1,address1);	
		
		
		String issueamount = "10000000";
		String addtoissueamount = "33000000";
				
		StringBuffer asset_code = new StringBuffer();
		String name = companyname.get(id);
		Calendar now = Calendar.getInstance();  
		String header  = name+Integer.toString(now.get(Calendar.YEAR))+Integer.toString(now.get(Calendar.MONTH) + 1)+Integer.toString(now.get(Calendar.DAY_OF_MONTH))+"测试";
		issueAsset(token, user_name1.toString(),address1.toString(),asset_code,header,asset_unit,issueamount);
		
		addToIssue(token, user_name1.toString(),asset_code.toString(),addtoissueamount);		
	
	}

	//a issue an asset under b's token
	public void issueandaddissuedifferent()
	{
		String id = jinxiang_id;
		String secret = jinxiang_secret;	
				
		String token  = getToken(api_url,id,secret);			
		StringBuffer user_name1 = new StringBuffer();		
		StringBuffer address1 = new StringBuffer();		
		
		createUser(token, user_name1,address1);	
		
		id = zhongtuobang_id;
		secret = zhongtuobang_key;	
				
		token  = getToken(api_url,id,secret);	
		
		String issueamount = "10000000";
		String addtoissueamount = "33000000";
				
		StringBuffer asset_code = new StringBuffer();
		String name = companyname.get(id);
		Calendar now = Calendar.getInstance();  
		String header  = name+Integer.toString(now.get(Calendar.YEAR))+Integer.toString(now.get(Calendar.MONTH) + 1)+Integer.toString(now.get(Calendar.DAY_OF_MONTH))+"测试";
		issueAsset(token, user_name1.toString(),address1.toString(),asset_code,header,asset_unit,issueamount);
		
		id = madianzhang_id;
		secret = madianzhang_secret;	
				
		token  = getToken(api_url,id,secret);	
		
		addToIssue(token, user_name1.toString(),asset_code.toString(),addtoissueamount);		
	
	}
	

	//A平台账户使用B的token发行一个资产，再用c的token转给D平台账户。
	public void integration()
	{
		String id = jinxiang_id;
		String secret = jinxiang_secret;	
				
		String token  = getToken(api_url,id,secret);			
		StringBuffer user_name1 = new StringBuffer();		
		StringBuffer address1 = new StringBuffer();		
		
		createUser(token, user_name1,address1);	
		
		id = zhongtuobang_id;
		secret = zhongtuobang_key;	
				
		token  = getToken(api_url,id,secret);	
		
		StringBuffer user_name2 = new StringBuffer();		
		StringBuffer address2 = new StringBuffer();				
		createUser(token, user_name2,address2);	
		
		String issueamount = "10000000";
		String sendamount = "888";
				
		
		id = madianzhang_id;
		secret = madianzhang_secret;	
				
		token  = getToken(api_url,id,secret);	
		StringBuffer asset_code = new StringBuffer();
		String name = companyname.get(id);
		Calendar now = Calendar.getInstance();  
		String header  = name+Integer.toString(now.get(Calendar.YEAR))+Integer.toString(now.get(Calendar.MONTH) + 1)+Integer.toString(now.get(Calendar.DAY_OF_MONTH))+"测试";
		issueAsset(token, user_name1.toString(),address1.toString(),asset_code,header,asset_unit,issueamount);
		
		id = renren_id;
		secret = renren_secret;	
				
		token  = getToken(api_url,id,secret);	
		
		sendAsset(false,token, address1.toString(),user_name1.toString(),address2.toString(), asset_code.toString(),sendamount,secret);		
			
	}
	
}