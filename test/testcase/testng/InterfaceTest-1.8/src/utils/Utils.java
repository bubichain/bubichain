package utils;

import java.io.FileWriter;
import java.util.Collection;

public class Utils {

	public void write(String docname, String content, int count) {
		FileWriter fw = null;
		try {
			fw = new FileWriter(docname);
			for (int i = 0; i < count; i++) {
				fw.write(content + "\n");
				System.out.println("第" + i + "次写入完毕");
			}
			fw.close();
		} catch (Exception e) {
			e.printStackTrace();
		} finally {
			try {
				fw.close();
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}
	
	public static boolean isNullByList(Collection<?> coll) {
		if (coll == null || coll.isEmpty() || coll.size() == 0) {
			return true;
		}
		return false;
	}
}
