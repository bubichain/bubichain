#ifndef WEBSOCKET_SERVER_H_
#define WEBSOCKET_SERVER_H_

#include <proto/message.pb.h>
#include <common/network.h>

namespace bubi {
	class WebSocketServer :public utils::Singleton<WebSocketServer>,
		public StatusModule,
		public Network,
		public utils::Runnable {
		friend class utils::Singleton<bubi::WebSocketServer>;
	public:
		WebSocketServer();
		~WebSocketServer();

		//重写send实现方法，加入队列,允许多线程访问
		//virtual bool Send(const ZMQTaskType type, const std::string& buf);

		bool Initialize(WsServerConfigure & ws_server_configure);
		bool Exit();

		//Handlers
		bool OnChainHello(protocol::WsMessage &message, Connection *conn);
		bool OnChainPeerMessage(protocol::WsMessage &message, Connection *conn);
		bool OnSubmitTransaction(protocol::WsMessage &message, Connection *conn);

		void BroadcastMsg(int64_t type, const std::string &data);

		virtual void GetModuleStatus(Json::Value &data);
	protected:
		virtual void Run(utils::Thread *thread) override;

	private:
		//virtual void Recv(const ZMQTaskType type, std::string& buf);
		//因为mqserver中send涉及到多线程访问，因此加入消息队列来处理这一问题
		//utils::ReadWriteLock send_list_mutex_;
		//std::list<std::pair<ZMQTaskType, std::string>> send_list_;

		bool init_;
		utils::Thread *thread_ptr_;

		static bool CheckCreateAccountOpe(const protocol::Operation &ope, Result &result);

		static bool CheckInitPayment(const protocol::Operation &ope, Result &result);

		static bool CheckPayment(const protocol::Operation &ope, Result &result);

		static bool CheckIssueAsset(const protocol::Operation &ope, Result &result);

		static bool CheckIssueUniqueAsset(const protocol::Operation &ope, Result &result);

		static bool CheckPaymentUniqueAsset(const protocol::Operation &ope, Result &result);

		static bool CheckSetOptions(const protocol::Operation &ope, Result &result);

		static bool CheckProduction(const protocol::Operation &ope, Result &result);

		static bool CheckRecord(const protocol::Operation &ope, Result &result);
	};
}

#endif