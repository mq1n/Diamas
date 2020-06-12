#pragma once

#ifndef VC_EXTRALEAN

class CNetworkAddress
{
	public:
		static bool GetHostName(char* szName, int32_t size);

	public:
		CNetworkAddress();
		~CNetworkAddress();

		void Clear();

		bool Set(const char* c_szAddr, int32_t port);

		void SetLocalIP();
		void SetIP(uint32_t ip);
		void SetIP(const char* c_szIP);
		bool SetDNS(const char* c_szDNS);

		void SetPort(int32_t port);
		
		int32_t GetPort();
		int32_t GetSize();

		void GetIP(char* szIP, int32_t len);

		uint32_t GetIP();
		
		operator const SOCKADDR_IN&() const;	

		const in_addr& GetAddr() const { return m_sockAddrIn.sin_addr; }

	private:
		bool IsIP(const char* c_szAddr);

	private:
		SOCKADDR_IN m_sockAddrIn;
};

#endif