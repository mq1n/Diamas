#pragma once

namespace net_engine
{
	class NetServiceBase : asio::noncopyable
	{
		public:
			size_t Run();
			size_t Poll();
			void Stop();
			bool HasStopped();

			asio::io_context& operator()();

		private:
			asio::io_context m_service;
	};
}
