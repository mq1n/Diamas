#pragma once
#include <cstdint>

#ifndef _WIN32

	typedef int32_t		socket_t;

#else

	typedef SOCKET		socket_t;

	#if !defined(_W64)
		#if !defined(__midl) && (defined(_X86_) || defined(_M_IX86)) && _MSC_VER >= 1300
			#define _W64 __w64
		#else
			#define _W64
		#endif
	#endif

#endif
