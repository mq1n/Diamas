#pragma once
#include <queue>

namespace net_engine
{
	enum EPeerErrorTypes : uint8_t
	{
		PeerErrorNone,
		PeerErrorConnectionFail,
		PeerErrorReadFail,
		PeerErrorWriteFail,
		PeerErrorUnknownHeader,
		/*
		NET_PEER_ERR_WRITE_SIZE_MISMATCH,
		NET_PEER_ERR_WRITE_SIZE_NULL,
		NET_PEER_ERR_WRITE_ERROR,
		NET_PEER_ERR_READ_SIZE_NULL,
		NET_PEER_ERR_CONN_FAIL
		*/
	};

	enum ELogFlags : uint32_t
	{
		LL_ONREAD = (1 << 0),
		LL_ONWRITE_PRE = (1 << 1),
		LL_ONWRITE_POST = (1 << 2),
		LL_ONREGISTERPACKET = (1 << 3),
	};

	class Packet;

	class NetPeerBase : asio::noncopyable, public std::enable_shared_from_this <NetPeerBase>
	{
		public:
			NetPeerBase(asio::io_context& service);
			virtual ~NetPeerBase() {};

			asio::ip::tcp::socket& GetSocket();
			const asio::ip::tcp::socket& GetSocket() const;
			const std::string& GetIP() const;
			uint16_t GetPort() const;
			bool IsConnected() const;

			void Send(std::shared_ptr <Packet> packet);

			void BeginRead();

			void Disconnect(const asio::error_code& er);
			void DelayedDisconnect(int timeOut);
			void PostShutdown();

			inline bool IsAssignedFlag(uint32_t flag);
			inline void SetLogLevel(uint32_t flag);
			inline void UnsetLogLevel(uint32_t flag);
			inline void ToggleLogLevel(uint32_t flag);

			virtual void OnConnect() = 0;
			virtual void OnDisconnect(const asio::error_code& er) = 0;
			virtual void OnRead(std::shared_ptr <Packet> packet) = 0;
			virtual std::size_t OnWritePre(const void* data, std::size_t length) = 0;
			virtual void OnWritePost(bool bCompleted) = 0;
			virtual void OnError(uint32_t ulErrorType, const asio::error_code& er) = 0;

		private:
			asio::io_context& m_service;
			asio::strand<asio::io_context::executor_type> m_strand;
			asio::ip::tcp::socket m_socket;

			std::mutex m_sendMutex;

			asio::streambuf m_buffer;
			asio::streambuf m_writeBuffer;
			std::queue<std::shared_ptr<Packet>> m_sendQueue;

			std::string m_ip;
			uint16_t m_port;

			asio::high_resolution_timer disconnect_timer;
			std::atomic <bool> m_isShutingDown;

			uint32_t m_logFlag;

		private:
			static void HandleReadHeader(std::weak_ptr <NetPeerBase> self, const asio::error_code& er, std::size_t succesed_size);
			static void HandleReadSize(std::weak_ptr <NetPeerBase> self, std::shared_ptr<Packet> packet, const asio::error_code& er, std::size_t succesed_size);
			static void HandleReadData(std::weak_ptr <NetPeerBase> self, std::shared_ptr<Packet> packet, const asio::error_code& er, std::size_t succesed_size);

			static void HandleWrite(std::weak_ptr <NetPeerBase> self, const asio::error_code& er, std::size_t succesed_size);
			static void HandleSend(std::weak_ptr <NetPeerBase> self);

			static void HandleDelayedDisconnect(std::weak_ptr <NetPeerBase> self, const asio::error_code& er);
	};

	inline bool NetPeerBase::IsAssignedFlag(uint32_t flag)
	{
		return m_logFlag & flag;
	}

	inline void NetPeerBase::SetLogLevel(uint32_t flag)
	{
		m_logFlag |= flag;
	}

	inline void NetPeerBase::UnsetLogLevel(uint32_t flag)
	{
		m_logFlag &= ~flag;
	}

	inline void NetPeerBase::ToggleLogLevel(uint32_t flag)
	{
		m_logFlag ^= flag;
	}
}
