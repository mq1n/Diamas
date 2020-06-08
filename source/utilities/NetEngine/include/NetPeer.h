#pragma once
#include "NetPacket.h"

namespace net_engine
{
	enum EPeerErrorTypes : uint8_t
	{
		NET_PEER_ERR_NULL,
		NET_PEER_ERR_WRITE_SIZE_MISMATCH,
		NET_PEER_ERR_WRITE_SIZE_NULL,
		NET_PEER_ERR_WRITE_ERROR,
		NET_PEER_ERR_READ_SIZE_NULL,
		NET_PEER_ERR_READ_ERROR,
		NET_PEER_ERR_CONN_FAIL
	};

	enum ELogFlags : uint32_t
	{
		LL_ONREAD = (1 << 0),
		LL_ONWRITE_PRE = (1 << 1),
		LL_ONWRITE_POST = (1 << 2),
		LL_ONREGISTERPACKET = (1 << 3),
	};

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

			std::size_t Send(const void* data, std::size_t length, bool flush = false);
			// std::size_t Send(const SNetPacket& p, bool flush = false);
			void Flush();
			// void AsyncSend(const void* data, std::size_t length);

			void BeginRead();

			void Disconnect(const asio::error_code& er);
			void DelayedDisconnect(int timeOut);

			inline bool IsAssignedFlag(uint32_t flag);
			inline void SetLogLevel(uint32_t flag);
			inline void UnsetLogLevel(uint32_t flag);
			inline void ToggleLogLevel(uint32_t flag);

			virtual void OnConnect() = 0;
			virtual void OnDisconnect(const asio::error_code& er) = 0;
			virtual std::size_t OnRead(const void* data, std::size_t length) = 0;
			virtual std::size_t OnWritePre(const void* data, std::size_t length) = 0;
			virtual void OnWritePost(bool bCompleted) = 0;
			virtual void OnError(uint32_t ulErrorType, const asio::error_code& er) = 0;

		private:
			asio::io_context& m_service;
			asio::ip::tcp::socket m_socket;

			std::vector<uint8_t> m_input;
			std::size_t m_inputOffset;
			std::vector<uint8_t> m_output;
			std::size_t m_outputOffset;

			// async
			// asio::streambuf m_readstream;

			std::string m_ip;
			uint16_t m_port;

			asio::high_resolution_timer disconnect_timer;

			uint32_t m_logFlag;

		private:
			static void HandleRead(std::weak_ptr <NetPeerBase> self, const asio::error_code& er, std::size_t succesed_size);
			static void HandleWrite(std::weak_ptr <NetPeerBase> self, const asio::error_code& er, std::size_t succesed_size, std::size_t length);
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
