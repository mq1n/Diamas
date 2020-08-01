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
#pragma comment( lib, "wtsapi32.lib" )
#pragma comment( lib, "Netapi32.lib" )
#pragma comment( lib, "DevIL.lib" )
#pragma comment( lib, "discord-rpc.lib" )

#ifdef _DEBUG
	#pragma comment( lib, "python27_debug.lib" )
	#pragma comment( lib, "lzo2_debug.lib" )
#else
	#pragma comment( lib, "python27.lib" )
	#pragma comment( lib, "lzo2.lib" )
#endif

//#pragma comment( lib, "libcef.lib" )
//#pragma comment( lib, "libcef_dll_wrapper.lib" )

#pragma comment( lib, "FSLib.lib" )
#pragma comment( lib, "granny2.11.8.0_static.lib" )
#pragma comment( lib, "CrashRpt1403.lib" )

#pragma comment(lib, "d3d9/d3d9.lib")
#pragma comment(lib, "d3d9/d3dx9.lib")
#pragma comment(lib, "d3d9/ddraw.lib")
#pragma comment(lib, "d3d9/dxguid.lib")
#pragma comment(lib, "d3d9/dxerr.lib")
