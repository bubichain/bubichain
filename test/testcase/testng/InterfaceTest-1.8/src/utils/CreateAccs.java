package utils;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.PrintWriter;
import java.util.Map;

import base.TestBase;

public class CreateAccs extends TestBase{

	@SuppressWarnings("rawtypes")
	public static void main(String[] args) {

		for(int i = 1;i<=10;i++){
			System.out.println("begin: "+DateHandler.getSimpleDay());
			Map acc1 = TxUtil.createAccount();
			Object address = acc1.get("address");
			Object private_key = acc1.get("private_key");
			Object public_key = acc1.get("public_key");
			System.out.println("address" + i + " " + address);
			System.out.println("private_key" + i + " " + private_key);
			System.out.println("public_key" + i + " " + public_key );
			System.out.println("end: "+DateHandler.getSimpleDay() + "\n");
		}
		
		
	}
	
	public void out(){
		try {
		FileOutputStream fos = new FileOutputStream(new File("output.txt"));
		PrintWriter pw = new PrintWriter(fos);
		String str = "";//str可以输出你想输出的数据类容
		pw.write(str);
		} catch (FileNotFoundException e) {
		e.printStackTrace();
		}
		}

}
