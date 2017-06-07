package cases;

import java.util.LinkedHashMap;

import org.testng.annotations.Test;

import base.TestBase;
import net.sf.json.JSONObject;
import utils.APIUtil;
import utils.HttpUtil;

public class BMInterfGetTxInfoByHash extends TestBase {
	//definition of conditions
	final static int noHash			= 1<<1;
	final static int invalidHash	= 1<<2;
	final static int validHash		= 1<<3;
	
	final static int noPageSize			= 1<<4;
	final static int invalidPageSize	= 1<<5;
	final static int validPageSize		= 1<<6;
	
	final static int noPageStart		= 1<<7;
	final static int invalidPageStart	= 1<<8;
	final static int validPageStart		= 1<<9;
	
	
	
		
	static int current_condition;
	static int current_condition_sub;
	
	private void TestGetAssetListByAddr(Object param)
	{
		String api_path = "transaction/v1/getTxInfoByHash" ;
		LinkedHashMap<String, String> parameter = new LinkedHashMap<String,String>();
		
		if(!APIUtil.getbool(current_condition&noHash))
		{			
			if(APIUtil.getbool(current_condition&(invalidHash|validHash)))
				parameter.put("hash",(String)param);
			else
				parameter.put("hash","2f4f2d10bbe81a973bc656caf1760c55b7af29662f132c82498558b7bb70d946");
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
						
		System.out.println("=========Get Tx Info By Hash========");
		String result = HttpUtil.dogetApi(interface_url, api_path, parameter);
		JSONObject jsonObject =JSONObject.fromObject(result);  
		//Verification(jsonObject);
	}
	

	public void a_normalTest()
	{
		current_condition = 0;
		TestGetAssetListByAddr(null);
		
	}

	public void b_hashTest()
	{
		current_condition = noHash;
		TestGetAssetListByAddr(null);
		
		//invalid
		String hashofregister = null;
		String hashofissue = null;
		String hashofissueAsync = null;
		String hashofadd2issue = null;
		String hashofadd2issueAsync = null;
		String hashofresetpassword = null;
		
		//valid
		String hashofsend = null;
		String hashofsendAsync = null;
		String hashofgrant = null;
		String hashofgrantAsync = null;
		
		StringBuffer name = new StringBuffer();
		StringBuffer address = new StringBuffer();
		StringBuffer assetcode = new StringBuffer();
		StringBuffer decodedAsset = new StringBuffer();
		StringBuffer issuer = new StringBuffer();
		StringBuffer address2 = new StringBuffer();
		StringBuffer name2 = new StringBuffer();
		
	
		
		String token  = getToken(zhongtuobang_id, zhongtuobang_key);
		createUser(token,name,address);
		createUser(token,name2,address2);
		issueAsset(token, name.toString(), address.toString(), assetcode, "test", "one","100");
		add2Issue(token, name.toString(), assetcode.toString(), "1000");
		grantAsset(token, address.toString(), name.toString(), address2.toString(), assetcode.toString(), "100000", zhongtuobang_key);
		grantAsset(token, address.toString(), name.toString(), address2.toString(), assetcode.toString(), "99", zhongtuobang_key);
		sendAsset(true, token, address.toString(), name.toString(), address2.toString(), assetcode.toString(), "1100", zhongtuobang_key);
		assetDecoder(assetcode.toString(),decodedAsset,issuer);
		
		current_condition = invalidHash;
		String conditions[]={
				"",
				"wrong_hash",
				"****&&%",
				APIUtil.getRandomString(64)			
		};
		for (int i =0 ;i<4;++i)
		{
			TestGetAssetListByAddr(conditions[i]);
		}		
	}	

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
	@Test
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

}
