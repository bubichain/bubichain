package utils;

import java.util.Map;

public class GenerateAccount {

	public static void main(String args[]) {
		for (int i = 1; i <= 5; i++) {
			Map acc1 = APIUtil.generateAcc();
			Object address = acc1.get("address");
			Object private_key = acc1.get("private_key");
			Object public_key = acc1.get("public_key");
			System.out.println("address" + i + " " + address);
			System.out.println("private_key" + i + " " + private_key);
			System.out.println("public_key" + i + " " + public_key + "\n");
		}
	}

}
