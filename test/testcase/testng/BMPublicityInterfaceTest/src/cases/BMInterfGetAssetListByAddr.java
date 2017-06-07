package cases;

import java.util.LinkedHashMap;

import org.testng.annotations.Test;

import base.TestBase;
import net.sf.json.JSONArray;
import net.sf.json.JSONObject;
import utils.APIUtil;
import utils.HttpUtil;

public class BMInterfGetAssetListByAddr extends TestBase {
	
	//definition of conditions
	final static int noAddress			= 1<<1;
	final static int invalidAddress		= 1<<2;
	final static int validAddress		= 1<<3;
	
	final static int noPageSize			= 1<<4;
	final static int invalidPageSize	= 1<<5;
	final static int validPageSize		= 1<<6;
	
	final static int noPageStart		= 1<<7;
	final static int invalidPageStart	= 1<<8;
	final static int validPageStart		= 1<<9;
	
	//definition of env
	final static int totalNewUser = 1<<0;
	final static int issueAsset = 1<<1;
	
	final static int issueAssetFailed = 1<<2;
	final static int issueAssetThenSend =1<<3;
	final static int issueAssetThenSendall = 1<<4;
	final static int issueAssetThenSendFail =1<<13;
	final static int issueAssetThenGrant =1<<5;
	final static int issueAssetThenGrantAll = 1<<6;
	final static int issueAssetThenGrantFail = 1<<7;
	final static int getAssetViaSend = 1<<8;
	final static int getAssetViaGrant = 1<<9;
	final static int getAssetViaSendfail = 1<<10;
	final static int getAssetViaGrantfail = 1<<11;
	final static int issueAssetThenAdd2Issue = 1<<12;
	
	//has assets of issued by itself, from other send, from other grant, once owned assets.
	final static int hasMandAssets = 1<<13;
	
	
	
	static int current_condition;
	static int current_env;
	
	
	//the address under test
	static StringBuffer address = new StringBuffer();
	
	
	//assets to be expected in list, or not
	LinkedHashMap<String, Boolean> assets = new LinkedHashMap<String,Boolean>();
	
	private void TestGetAssetListByAddr(Object param)
	{
		envSetup();
		String api_path = "user/v1/getAssetListByAddr" ;
		LinkedHashMap<String, String> parameter = new LinkedHashMap<String,String>();
		
		if(!APIUtil.getbool(current_condition&noAddress))
		{			
			if(APIUtil.getbool(current_condition&(invalidAddress|validAddress)))
				parameter.put("address",(String)param);
			else
				parameter.put("address",address.toString());
		}
		
		if(!APIUtil.getbool(current_condition&noPageSize))
		{
			if(APIUtil.getbool(current_condition&(invalidPageSize|validPageSize)))
				parameter.put("page.size",(String)param);
			else
				parameter.put("page.size","2");
		}
		
		if(!APIUtil.getbool(current_condition&noPageStart))
		{
			if(APIUtil.getbool(current_condition&(invalidPageStart|validPageStart)))
				parameter.put("page.start",(String)param);
			else
				parameter.put("page.start","2");
		}
		
		System.out.println("=========Get Asset List By Addr========");
		String result = HttpUtil.dogetApi(interface_url, api_path, parameter);
		JSONObject jsonObject =JSONObject.fromObject(result);  
		//Verification(jsonObject);
		
		address.delete(0, address.length());
	}
	

	public void a_normalTest()
	{
		current_condition = 0;
		for (int i = 0; i<14;i++)
		{
			current_env = 1<<i;
			TestGetAssetListByAddr(null);
		}
		
		
		
	}

	public void b_addressTest()
	{
		current_condition = noAddress;
		TestGetAssetListByAddr(null);
		
		current_condition = invalidAddress;
		String conditions[]={
				"",
				"wrong_address",
				"****&&%",
				APIUtil.getRandomString(40)				
		};
		for (int i =0 ;i<4;++i)
		{
			TestGetAssetListByAddr(conditions[i]);
		}		
	}
	@Test
	public void c_pageSizeTest()
	{
		current_condition = noPageSize;
		TestGetAssetListByAddr(null);
		
		current_condition = invalidPageSize;
		String conditions[]={
				"-1",
				"0",
				"****&&%",
				"3.1415",
				" "				
		};
		for (int i =0 ;i<5;++i)
		{
			TestGetAssetListByAddr(conditions[i]);
		}		
	}

	public void d_pageStartTest()
	{
		current_condition = noPageStart;
		TestGetAssetListByAddr(null);
		
		current_condition = invalidPageStart;
		String conditions[]={
				"-1",
				"0",
				"****&&%",
				"3.1415",
				" "									
		};
		for (int i =0 ;i<5;++i)
		{
			TestGetAssetListByAddr(conditions[i]);
		}		
	}
	
	private void envSetup()
	{
		StringBuffer name = new StringBuffer();
		StringBuffer assetcode = new StringBuffer();
		StringBuffer decodedAsset = new StringBuffer();
		StringBuffer issuer = new StringBuffer();
		StringBuffer address2 = new StringBuffer();
		StringBuffer name2 = new StringBuffer();
		String token = getToken(zhongtuobang_id, zhongtuobang_key);		
		createUser(token,name,address);
		createUser(token,name2,address2);
		
		switch(current_env)
		{
			case totalNewUser:
				break;
			case issueAsset:
				//issue amount is 100				
				issueAsset(token, name.toString(), address.toString(), assetcode, "test", "one","100");
				assetDecoder(assetcode.toString(),decodedAsset,issuer);
				assets.put(decodedAsset.toString(), true);				
			break;
		
		case issueAssetFailed:
			//issue amount is 0, issue failed.			
				issueAsset(token, name.toString(), address.toString(), assetcode, "test", "one","0");
				//assetDecoder(assetcode.toString(),decodedAsset,issuer);
				//assets.put(decodedAsset.toString(), true);				
			break;
		case issueAssetThenSend:
				issueAsset(token, name.toString(), address.toString(), assetcode, "test", "one","100");				
				sendAsset(false, token, address.toString(), name.toString(), address2.toString(), assetcode.toString(), "99", zhongtuobang_key);
				assetDecoder(assetcode.toString(),decodedAsset,issuer);
				assets.put(decodedAsset.toString(), true);				
			break;
		case issueAssetThenSendall:
			issueAsset(token, name.toString(), address.toString(), assetcode, "test", "one","100");
			sendAsset(false, token, address.toString(), name.toString(), address2.toString(), assetcode.toString(), "100", zhongtuobang_key);
			assetDecoder(assetcode.toString(),decodedAsset,issuer);
			assets.put(decodedAsset.toString(), false);		
			break;
		case issueAssetThenSendFail:
			issueAsset(token, name.toString(), address.toString(), assetcode, "test", "one","100");
			sendAsset(false, token, address.toString(), name.toString(), address2.toString(), assetcode.toString(), "10000", zhongtuobang_key);
			assetDecoder(assetcode.toString(),decodedAsset,issuer);
			assets.put(decodedAsset.toString(), true);		
			break;
		case issueAssetThenGrant:
			issueAsset(token, name.toString(), address.toString(), assetcode, "test", "one","100");
			grantAsset(token, address.toString(), name.toString(), address2.toString(), assetcode.toString(), "99", zhongtuobang_key);
			assetDecoder(assetcode.toString(),decodedAsset,issuer);
			assets.put(decodedAsset.toString(),true );		
			break;
		case issueAssetThenGrantAll:
			issueAsset(token, name.toString(), address.toString(), assetcode, "test", "one","100");
			grantAsset(token, address.toString(), name.toString(), address2.toString(), assetcode.toString(), "100", zhongtuobang_key);
			assetDecoder(assetcode.toString(),decodedAsset,issuer);
			assets.put(decodedAsset.toString(),true );		
			break;
		case issueAssetThenGrantFail :
			issueAsset(token, name.toString(), address.toString(), assetcode, "test", "one","100");
			grantAsset(token, address.toString(), name.toString(), address2.toString(), assetcode.toString(), "100000", zhongtuobang_key);
			assetDecoder(assetcode.toString(),decodedAsset,issuer);
			assets.put(decodedAsset.toString(),true );					
			break;
		case getAssetViaSend :
			issueAsset(token, name2.toString(), address2.toString(), assetcode, "test", "one","100");
			sendAsset(false, token, address2.toString(), name2.toString(), address.toString(), assetcode.toString(), "99", zhongtuobang_key);
			assetDecoder(assetcode.toString(),decodedAsset,issuer);
			assets.put(decodedAsset.toString(), true);	
			break;
		case getAssetViaGrant :
			issueAsset(token, name2.toString(), address2.toString(), assetcode, "test", "one","100");
			grantAsset(token, address2.toString(), name2.toString(), address.toString(), assetcode.toString(), "99", zhongtuobang_key);
			assetDecoder(assetcode.toString(),decodedAsset,issuer);
			assets.put(decodedAsset.toString(), true);	
			break;
		case getAssetViaSendfail :
			issueAsset(token, name2.toString(), address2.toString(), assetcode, "test", "one","100");
			sendAsset(false, token, address2.toString(), name2.toString(), address.toString(), assetcode.toString(), "10000", zhongtuobang_key);
			assetDecoder(assetcode.toString(),decodedAsset,issuer);
			assets.put(decodedAsset.toString(), true);	
			break;
		case getAssetViaGrantfail :
			issueAsset(token, name2.toString(), address2.toString(), assetcode, "test", "one","100");
			grantAsset(token, address2.toString(), name2.toString(), address.toString(), assetcode.toString(), "1000", zhongtuobang_key);
			assetDecoder(assetcode.toString(),decodedAsset,issuer);
			assets.put(decodedAsset.toString(), true);	
			break;
		case issueAssetThenAdd2Issue:
			//issue amount is 100				
			issueAsset(token, name.toString(), address.toString(), assetcode, "test", "one","100");
			add2Issue(token, name.toString(), assetcode.toString(), "1000");
			assetDecoder(assetcode.toString(),decodedAsset,issuer);
			assets.put(decodedAsset.toString(), true);		
			break;
		default:
			break;
		}
	}
	
}
	
	
