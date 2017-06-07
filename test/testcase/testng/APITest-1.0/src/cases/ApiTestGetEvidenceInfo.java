package cases;

import java.util.LinkedHashMap;
import java.util.Random;

import org.testng.annotations.Test;

import base.TestBase;
import net.sf.json.JSONArray;
import net.sf.json.JSONObject;
import utils.APIUtil;
import utils.HttpUtil;

@Test
public class ApiTestGetEvidenceInfo extends ApiGeneralCases {
	
	//definition of conditions

	final static int invalidhash	= 1<<3;
	final static int nohash	    = 1<<4;
	final static int validhash	= 1<<5;
	
		
	final static int hashofcreateevidence = 1<<7;
	final static int hashofmodifyevidence = 1<<8;
	
	
	//integration test
	final static int userhasnoevidence = 1<<10;
	final static int usercreateevidence = 1<<11;
	final static int usermodifyevidence = 1<<12;
	

	static String asset_unit = "元";
	static String asset_name = "某种资产";
	
	
	
	
	private void TestEvidenceInfo(Object param, JSONArray detail,int signernum)
	{
		access_token = getToken();
		StringBuffer user_name1 = new StringBuffer("");  
		StringBuffer address1 = new StringBuffer("");  
		StringBuffer user_name2 = new StringBuffer("");  
		StringBuffer address2 = new StringBuffer("");  
		createUser(access_token, user_name1,address1);
		createUser(access_token, user_name2,address2);
		StringBuffer evidence_id = new StringBuffer("");
		
		StringBuffer bchash1 = new StringBuffer("");
		StringBuffer bchash2 = new StringBuffer("");
		JSONArray signers = new JSONArray();
		
		JSONObject singlesigner = new JSONObject();
		
		if(signernum >0)
		{
			singlesigner.put("bubi_address", address1.toString());				
			singlesigner.put("password" ,user_name1.toString());					
			signers.add(singlesigner);
			if(signernum>1)
			{
				singlesigner.put("bubi_address", address2.toString());				
				singlesigner.put("password" ,user_name2.toString());					
				signers.add(singlesigner);
				//if(signernum>2)
				//{
				//	singlesigner.put("bubi_address", address3.toString());				
				//	singlesigner.put("password" ,user_name3.toString());					
				//	signers.add(singlesigner);						
				//}
			}
			
		}
		//create an evidence
		
		CreateEvidence(access_token,signers,"metadata",evidence_id,bchash1);
		CreateEvidence(access_token,signers,"metadata",evidence_id,bchash1);
		
		//modify evidence
		
		String evidence = evidence_id.toString();
		
		//ModifyEvidence(access_token, signers,"metadata", evidence,bchash2);
		
		
	/*	if(APIUtil.getbool(current_condition&userhasnoevidence))
		{
			
		}	
		if(APIUtil.getbool(current_condition&usercreateevidence))
		{
			
		}	
		if(APIUtil.getbool(current_condition&usermodifyevidence))
		{
			
		}
		
		
		String api_path = "/evidence/v1/history" ;
		LinkedHashMap<String, String> parameter = new LinkedHashMap<String,String>();
		
		if(!APIUtil.getbool(current_condition&nohash))
		{
			parameter.put("hash",(String)param);
			if(APIUtil.getbool(current_condition&invalidhash))
				parameter.put("hash",(String)param);
			else
				parameter.put("hash",address1.toString());
		}
		
		if(!APIUtil.getbool(current_condition&noAccessToken))
		{
			parameter.put("access_token",(String)param);
			if(APIUtil.getbool(current_condition&invalidAccessToken))
				parameter.put("access_token",(String)param);
			else
				parameter.put("access_token",access_token);
		}
		
		System.out.println("=========Get Evidence Info========");
		String result = HttpUtil.dogetApi(api_url, api_path, parameter);
		JSONObject jsonObject =JSONObject.fromObject(result);  
		Verification(jsonObject);*/
		String api_path = "/evidence/v1/info" ;
		parameter.clear();
		
		if(!APIUtil.getbool(current_condition&nohash))
		{
			
			if(APIUtil.getbool(current_condition&invalidhash))
				parameter.put("hash",(String)param);
			else
				parameter.put("hash",bchash1.toString());
		}
		
		if(!APIUtil.getbool(current_condition&noAccessToken))
		{
			if(APIUtil.getbool(current_condition&invalidAccessToken))
				parameter.put("access_token",(String)param);
			else
				parameter.put("access_token",access_token);
		}
		
		System.out.println("=========Get User Info========");
		String result = HttpUtil.dogetApi(api_url, api_path, parameter);
		JSONObject jsonObject =JSONObject.fromObject(result);  
		
	}
	
	private void Verification(JSONObject jsonObject)
	{
		switch (current_condition)
		{
			case validAccessToken	:
			case 0:
				check.equals(jsonObject.getString("err_code"), "0", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "操作成功", "msg is not as expected");
				break;
			
			case invalidAccessToken		:
			case noAccessToken		:
				check.equals(jsonObject.getString("err_code"), "20015", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "授权码不存在或已过期", "msg is not as expected");
				break;
			case invalidhash:
			case nohash:
				check.equals(jsonObject.getString("err_code"), "20031", "errorcode is not as expected");
				check.equals(jsonObject.getString("msg"), "账户不存在或账户未激活", "msg is not as expected");
				break;			
		}		
	}	

	public void a_normaltest()
	{
		current_condition = 0;
		TestEvidenceInfo(null,null,2);
	}

	public void b_accessTokenTest()
	{
		current_condition = noAccessToken;
		TestEvidenceInfo(null,null,2);
		
		current_condition = invalidAccessToken;
		String conditions[]={
				"",
				"wrong_condition"
				
		};
		for (int i =0 ;i<2;++i)
		{
			TestEvidenceInfo(conditions[i],null,2);
		}
		
		current_condition = validAccessToken;
		
		TestEvidenceInfo(null,null,2);
		
	}

	public void c_hash()
	{
		current_condition = nohash;
		TestEvidenceInfo(null,null,2);
		
		current_condition = invalidhash;
		String conditions[]={
				"",
				"wronguser",
				APIUtil.getRandomString(100)
				
		};
		for (int i =0 ;i<3;++i)
		{
			TestEvidenceInfo(conditions[i],null,2);
		}
		
		current_condition = validhash;
		current_condition_sub = hashofcreateevidence;
		
		
		TestEvidenceInfo(null,null,2);
		current_condition_sub = hashofmodifyevidence;
		TestEvidenceInfo(null,null,2);
		
	}

	
	

}