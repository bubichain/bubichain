package base;

import org.apache.log4j.Level;
import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.apache.log4j.xml.DOMConfigurator;

public class Log {

static final Logger logger = LogManager.getLogger(Log.class.getName());

	static{

		DOMConfigurator.configure("log4j.xml");

	}

	/**
	 * Debug级别LOG
	 * @param msg 用户赋值，期望打印的内容
	 */

	public synchronized static void debug(String msg){

		logger.log(Log.class.getName(), Level. DEBUG, msg, null);

	}

	/**

	 * Info级别LOG

	 * @param msg 用户赋值，期望打印的内容

	 */

	public synchronized static void info(String msg){
//		 logger.log(Log.class.getName(), Level.
//		 INFO, msg, null);
//		 logger.log(Log.class.getCanonicalName(),
//		 Level.INFO,
//		 msg, null);
		logger.log(logger.getClass().getName(),Level.INFO, msg,null);

	}



	

	/**

	 * Warn级别的LOG

	 * @param msg 用户赋值，期望打印的内容

	 */

	public synchronized static void warn(String msg){

		logger.log(Log.class.getName(), Level. WARN, msg, null);

	}

	

	/**

	 * Error级别的LOG

	 * @param msg 用户赋值，期望打印的内容

	 */

	public synchronized static void error(String msg){

		logger.log(Log.class.getName(), Level. ERROR, msg, null);

	}

	

	public synchronized static void error(String msg, Throwable cause){

		logger.log(Log.class.getName(), Level. ERROR, msg, cause);

	}

	


}
