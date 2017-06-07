package utils;

import java.io.FileWriter;
import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Random;

import net.sf.json.JSONObject;

import base.ErrorHandler;
import base.TestBase;

public class Create100 extends TestBase{
	static int times  = 10;
	public static void main(String[] args) {
		new Create100().create();
	}
	
	public void create(){
		FileWriter fw = null;   
        int count=10;//写文件行数   
        try {   

            fw = new FileWriter("./abc1.csv");   
            for (int i = 0; i < count; i++) {   
            	List acc = create_oper();
        		for (int j = 0; j < times; j++) {
        			Map acca = (Map)acc.get(j);
        			Map accb = (Map)acc.get(new Random().nextInt(times-1));
        			 fw.write(acca.get("address")+","+acca.get("private_key")+","+acca.get("public_key")+","+accb.get("address")+"\n"); 
        		}
        		System.out.println("第" + i + "次创建");
            }   
                fw.close();   
        } catch (Exception e) {   
            e.printStackTrace();   
        }   
        finally {   
            try {   
                fw.close();   
            } catch (Exception e) {   
                e.printStackTrace();   
            }   
        }   

    }   

	
	public static List create_oper(){
		String account_metadata = "abcd";
		String metadata = "abcd";
		Object source_address = led_acc;
		Map<String, Object> acc = new LinkedHashMap<String, Object>();
		String pri = led_pri;
		String pub = led_pub;
		List opers = new ArrayList();
		List operAcc = new ArrayList();
		Object dest_add =null;
		Object private_key =null;
		Object public_key =null;
		Map acc_gen =null;
		
		for(int i=0;i<times;i++){
			acc_gen = APIUtil.generateAcc();
			dest_add = acc_gen.get("address");
			private_key = acc_gen.get("private_key");
			public_key = acc_gen.get("public_key");
			operAcc.add(acc_gen);
			JSONObject oper = TxUtil.operCreateAccountjson(0, dest_add, 90000000,account_metadata);
			opers.add(oper);
		}
		long sequence_number = Result.seq_num(source_address);
		String result = TxUtil.tx_result(opers, source_address, fee*times,sequence_number, metadata, pri, pub);
		int err_code = Result.getErrorCode(result);
		if (err_code == 0) {
			return operAcc;
		} else {
			 ErrorHandler.continueRunning("账户创建异常,error_code: " + err_code);
		}
		return null;
	}
	
}
