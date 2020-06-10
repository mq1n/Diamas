#include "stdafx.h"

#ifdef _DEBUG
	#pragma comment(linker, "/NODEFAULTLIB:libcmt.lib")
#endif

#pragma comment( lib, "version.lib" )
#pragma comment( lib, "dbghelp.lib" )
#pragma comment( lib, "mss32.lib" )
#pragma comment( lib, "winmm.lib" )
#pragma comment( lib, "imm32.lib" )
#pragma comment( lib, "uuid.lib" )
#pragma comment( lib, "oldnames.lib" )
#pragma comment( lib, "SpeedTreeRT.lib" )
#pragma comment( lib, "dinput8.lib" )
#pragma comment( lib, "ws2_32.lib" )
#pragma comment( lib, "strmiids.lib" )
#pragma comment( lib, "dmoguids.lib" )

#ifdef _DEBUG
	#pragma comment( lib, "cryptopp-static_debug.lib" )
	#pragma comment( lib, "python27_debug.lib" )
	#pragma comment( lib, "DevIL_debug.lib" )
	#pragma comment( lib, "fmt_debug.lib" )
#else
	#pragma comment( lib, "cryptopp-static.lib" )
	#pragma comment( lib, "python27.lib" )
	#pragma comment( lib, "DevIL.lib" )
	#pragma comment( lib, "fmt.lib" )
#endif

#pragma comment( lib, "FSLib.lib" )
#pragma comment( lib, "granny2.11.8.0_static.lib" )
#pragma comment( lib, "CrashRpt1403.lib" )

#pragma comment(lib, "d3d8/d3d8.lib")
#pragma comment(lib, "d3d8/d3dx8.lib")
#pragma comment(lib, "d3d8/ddraw.lib")
#pragma comment(lib, "d3d8/dxguid.lib")
