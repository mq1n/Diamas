#include "../include/NetEngine.hpp"

#ifdef _WIN32
	#ifdef _DEBUG
		#ifdef CI_BUILD
			#pragma comment( lib, "cryptopp_debug.lib" )
		#else
			#pragma comment( lib, "cryptopp-static_debug.lib" )
		#endif
	#else
		#ifdef CI_BUILD
			#pragma comment( lib, "cryptopp_release.lib" )
		#else
			#pragma comment( lib, "cryptopp-static_release.lib" )
		#endif
	#endif
#endif
