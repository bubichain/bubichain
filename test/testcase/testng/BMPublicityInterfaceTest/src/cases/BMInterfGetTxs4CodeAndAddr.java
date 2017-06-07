package cases;

import java.util.LinkedHashMap;

import org.testng.annotations.Test;

import base.TestBase;
import net.sf.json.JSONObject;
import utils.APIUtil;
import utils.HttpUtil;

public class BMInterfGetTxs4CodeAndAddr extends TestBase {
	
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
	
	final static int noAssetCode		= 1<<10;
	final static int invalidAssetCode	= 1<<11;
	final static int validAssetCode		= 1<<12;
	
	
	
	static int current_condition;
	static int current_condition_sub;
	
	private void TestGetTxs4CodeAndAddr(Object param)
	{
		String api_path = "transaction/v1/getTxs4CodeAndAddr" ;
		LinkedHashMap<String, String> parameter = new LinkedHashMap<String,String>();
		
		if(!APIUtil.getbool(current_condition&noAddress))
		{			
			if(APIUtil.getbool(current_condition&(invalidAddress|validAddress)))
				parameter.put("address",(String)param);
			else
				parameter.put("address","bubiV8iHLkDsV14YPQpuVzmNWKNYpBT18ULyeJfm");
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
		
		if(!APIUtil.getbool(current_condition&noAssetCode))
		{
			if(APIUtil.getbool(current_condition&(invalidAssetCode|validAssetCode)))
				parameter.put("assetCode",(String)param);
			else
				parameter.put("assetCode","28a62be0c5cfd93570ee3e967aae143103fd96f7e54db52d127f0dd5bec7ca97");
		}
		
		System.out.println("=========Get Txs 4 Code And Addr========");
		String result = HttpUtil.dogetApi(interface_url, api_path, parameter);
		JSONObject jsonObject =JSONObject.fromObject(result);  
		Verification(jsonObject);
	}
	
	public void Verification(JSONObject jsonObject)
	{
		switch(current_condition)
		{
			case noAddress			:    break;
			case invalidAddress	:	     break;
			case validAddress		:    break;
			                             
			case noPageSize		:	     break;
			case invalidPageSize	:    break;
			case validPageSize		:    break;
			                             
			case noPageStart		:    break;
			case invalidPageStart	:    break;
			case validPageStart	:	     break;
			                            
			case noAssetCode		:    break;
			case invalidAssetCode	:    break;
			case validAssetCode	:	     break;
		}

		
	}

	public void a_normalTest()
	{
		current_condition = 0;
		TestGetTxs4CodeAndAddr(null);
		
	}
	
	public void b_addressTest()
	{
		current_condition = noAddress;
		TestGetTxs4CodeAndAddr(null);
		
		current_condition = invalidAddress;
		String conditions[]={
				"",
				"wrong_address",
				"****&&%",
				APIUtil.getRandomString(40)				
		};
		for (int i =0 ;i<4;++i)
		{
			TestGetTxs4CodeAndAddr(conditions[i]);
		}		
	}

	public void c_pageSizeTest()
	{
		current_condition = noPageSize;
		TestGetTxs4CodeAndAddr(null);
		
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
			TestGetTxs4CodeAndAddr(conditions[i]);
		}		
	}
	@Test
	public void d_pageStartTest()
	{
		current_condition = noPageStart;
		TestGetTxs4CodeAndAddr(null);
		
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
			TestGetTxs4CodeAndAddr(conditions[i]);
		}		
	}
	
	public void e_assetCodeTest()
	{
		current_condition = noAssetCode;
		TestGetTxs4CodeAndAddr(null);
		current_condition = invalidAssetCode;
		String conditions[]={
				"",
				"wrong_address",
				"****&&%",
				APIUtil.getRandomString(40),
				"asset code exist but not his"
		};
		for (int i =0 ;i<5;++i)
		{
			TestGetTxs4CodeAndAddr(conditions[i]);
		}		
	}
}
	
	
