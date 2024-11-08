//#define __MAIN__
#ifdef __MAIN__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <algorithm>

typedef uint8_t uint8_t;
typedef uint32_t uint32_t;

#else

#include "stdafx.h"
#include "ip_ban.h"

#endif

class IP
{
	public:
		IP()
			: dwStart(0), dwEnd(0), dwMask(0)
		{}

		IP(const IP & r)
		{
			dwStart = r.dwStart;
			dwEnd = r.dwEnd;
			dwMask = r.dwMask;
		}

		IP(const char * c_pszStart, const char * c_pszEnd = nullptr)
		{
			uint8_t start[4];
			uint8_t end[4];
			uint8_t mask[4];

			Read(c_pszStart, start);

			if (c_pszEnd && *c_pszEnd)
				Read(c_pszEnd, end);
			else
				memcpy(end, start, sizeof(uint8_t) * 4);

			mask[0] = 255 - (start[0] ^ end[0]);
			mask[1] = 255 - (start[1] ^ end[1]);
			mask[2] = 255 - (start[2] ^ end[2]);
			mask[3] = 255 - (start[3] ^ end[3]);

			dwStart = (start[3] << 24) | (start[2] << 16) | (start[1] << 8) | start[0];
			dwEnd = (end[3] << 24) | (end[2] << 16) | (end[1] << 8) | end[0];
			dwMask = (mask[3] << 24) | (mask[2] << 16) | (mask[1] << 8) | mask[0];

			Print();
		}

		IP(struct in_addr in)
		{
			dwStart = in.s_addr;
			dwEnd = dwStart;
			dwMask = 4294967295UL;
		}

		bool IsEqual(const IP & r) const
		{
			return (dwStart == r.dwStart && dwEnd == r.dwEnd && dwMask == r.dwMask);
		}

		bool IsChildOf(IP & r)
		{
			if ((r.dwStart & r.dwMask) != (dwStart & r.dwMask))
				return false;

			uint32_t m = r.dwMask | dwMask;

			return (dwStart & ~m) == (dwStart & ~dwMask) && (dwEnd & ~m) == (dwEnd & ~dwMask);
		}

		int32_t hash()
		{
			return (dwStart & 0x000000FF);
		}

		void Print()
		{
			struct in_addr in_ip, in_mask, in_end;

			in_ip.s_addr = dwStart;
			in_mask.s_addr = dwMask;
			in_end.s_addr = dwEnd;

			fprintf(stderr, "\t%s", inet_ntoa(in_ip));
			fprintf(stderr, "\t%s", inet_ntoa(in_end));
			fprintf(stderr, "\t%s\tfirst %d\n", inet_ntoa(in_mask), hash());
		}

	protected:
		bool Read(const char * s, uint8_t * dest)
		{
			uint8_t bClass = 0;
			const char * p = s;

			while (bClass < 3)
			{
				char szNum[4];
				char * pDot = const_cast<char*>(strchr(p, '.'));

				if (!pDot)
					break;

				strlcpy(szNum, p, sizeof(szNum));

				str_to_number(dest[bClass++], szNum);
				p = pDot + 1;
			}

			if (bClass != 3)
			{
				fprintf(stderr, "error reading start %s\n", s);
				return false;
			}

			str_to_number(dest[bClass], p);
			return true;
		}

		uint32_t dwStart;
		uint32_t dwEnd;
		uint32_t dwMask;
};

std::map<int32_t, std::vector<IP> > mapBanIP;

bool LoadBanIP(const char * filename)
{
	auto fp = msl::file_ptr(filename, "r");
	if (!fp)
		return false;

	char buf[256];
	char start[256];
	char end[256];

	fprintf(stderr, "LOADING BANNED IP LIST\n");

	while (fgets(buf, 256, fp.get()))
	{
		*strchr(buf, '\n') = '\0';
		char * p = strchr(buf, '\t');

		if (!p)
		{
			strlcpy(start, buf, sizeof(start));
			*end = '\0';
		}
		else
		{
			char * p2 = strchr(p + 1, '\t');

			if (p2)
				*p2 = '\0';

			strlcpy(end, p + 1, sizeof(end));

			*p = '\0';
			strlcpy(start, buf, sizeof(start));
		}

		IP ip(start, end);

		auto it = mapBanIP.find(ip.hash());

		if (it == mapBanIP.end())
		{
			std::vector<IP> v;
			v.push_back(ip);
			mapBanIP.emplace(ip.hash(), v);
		}
		else
			it->second.push_back(ip);
	}

	return true;
}

bool IsBanIP(struct in_addr in)
{
	IP ip(in);

	auto it = mapBanIP.find(ip.hash());

	if (it == mapBanIP.end())
		return false;

	auto it2 = it->second.begin();

	while (it2 != it->second.end())
		if (ip.IsChildOf(*(it2++)))
			return true;

	return false;
}
