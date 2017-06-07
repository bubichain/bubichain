package utils;

import java.util.List;
import net.sf.json.JSONArray;
import net.sf.json.JSONObject;

public class Tx {

	public static void main(String[] args) {
		int flag = 0;
		while (true) {
			Object source_address = "bubiV8iEEcBFxsiwNQ91qmaojoeiAD4JMQBH1gvC";
			String pri = "privbtaP5b9c795G1SEanZ8BxuB4B5gccVVgDXT7zFE3ptFm7aMGmg9j";
			String pub = "1F6ZLYr1YTHFeT3gii4sHCN1sMZs8DcvMkNXdnhpsJ4";
			String baseurl2 = "http://bubichain-qa1.chinacloudapp.cn:19333/";
			String baseurl1 = "http://bubichain-qa.chinacloudapp.cn:19333/";
			JSONArray key = new JSONArray();
			key.add(pri);
			Object dest_add = APIUtil.generateAddress();
			List opers = TxUtil.operCreateAccount(0, dest_add, 30000000,"abcd");
			long sequence_number = Result.seq_num(source_address);
			JSONObject tran = TxUtil.tran_json(source_address, 1000, sequence_number, "abcd", opers);
			List item =TxUtil.items(key, tran);
			JSONObject items = TxUtil.items(item);
			String result = HttpUtil.dopost(items);
			System.out.println(result);
			APIUtil.wait(5);
			flag++;
			System.out.println("创建账户交易，发送请求数： "+flag);
		}
	}
}
