package cases;


import org.testng.annotations.Test;

import base.TestBase;
import newop.Transaction;
import utils.TxUtil;

//@Test
public class ABtest extends TestBase {
	public static void main(String[] args) {
		while (true) {
			TxUtil.createAccount1();
		}
//	public void tesaa(){
//		Transaction tran = new Transaction();
//		while (true) {
//			tran.createAccountOne(genesis);
//		}
	
//	public static void main(String[] args) {
//		BufferedReader bf;
////		File f = new File("E:/workspace_eclipse/TestJM/destadds20170302.csv");
//		File f = new File("D:/accountdb/1m_1.db");
//		String lineTxt;	
//		try {
//			bf = new BufferedReader(new FileReader(f));
//			while ((lineTxt = bf.readLine()) != null) {
//				Account acc = new Account();
//				int b1 = lineTxt.indexOf(' ');
//				int b2 = lineTxt.indexOf(' ', b1 + 1);
//				acc.setPri_key(lineTxt.substring(0, b1)); 
//				acc.setPub_key(lineTxt.substring(b1 + 1, b2));
//				acc.setAddress(lineTxt.substring(b2 + 1));
//				System.out.println("aa: " + lineTxt.substring(0, b1));
//			}
//			bf.close();
//		} catch (IOException e1) {
//			e1.printStackTrace();
//		}
	}
	
//	public  void testss() {
//		
//		tran.createAccountOne(genesis);
////		while (true) {
////			tran.createAccountOne(genesis);
////		}
//	}

}
