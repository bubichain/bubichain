package base;

import base.Log;

public class ErrorHandler {

	/**
	 * 
	 * 在系统日志和测试报告中记录自定义信息message
	 * 
	 * @param message
	 *            要输出的自定义信息
	 * 
	 * @param isReport
	 *            是否将自定义信息输出到测试报告中
	 */

	public static void continueRunning(String message) {

		Log.error(message);

	}

	/**
	 * 
	 * 在程序中捕获到异常后，记录message和异常的堆栈信息到日志。并在报告中输出自定义信息message
	 * 
	 * @param cause
	 *            捕获到的原始异常
	 * 
	 * @param message
	 *            要输出的自定义信息
	 * 
	 * @param isReport
	 *            是否将自定义信息输出到测试报告中
	 */
	public static void continueRunning(Throwable cause, String message) {
		Log.error(message, cause);
	}

	/**
	 * 
	 * 抛出JuiceException，并在系统日志和测试报告中记录自定义信息message
	 * 
	 * @param message
	 *            要输出的自定义信息
	 * 
	 * @param isReport
	 *            是否将自定义信息输出到测试报告中
	 */

	public static void stopRunning(String message) {

		Log.error(message);

		throw new BubiException(message);

	}

	/**
	 * 
	 * 在程序中捕获到异常后，记录message和异常的堆栈信息到日志。抛出JuiceException，并在报告中输出自定义信息message
	 * 
	 * @param cause
	 *            捕获到的原始异常
	 * 
	 * @param message
	 *            要输出的自定义信息
	 * 
	 * @param isReport
	 *            是否将自定义信息输出到测试报告中
	 */

	public static void stopRunning(Throwable cause, String message) {

		Log.error(message, cause);

		throw new BubiException(message);

	}

}

