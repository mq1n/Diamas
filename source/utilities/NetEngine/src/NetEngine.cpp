#include "../include/NetEngine.hpp"

#ifdef _WIN32
	#ifdef _DEBUG
		#pragma comment( lib, "cryptopp-static_debug.lib" )
	#else
		#pragma comment( lib, "cryptopp-static_release.lib" )
	#endif
#endif
