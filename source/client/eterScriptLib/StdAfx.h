#pragma once

#include "../eterLib/StdAfx.h"
#include "../eterGrnLib/StdAfx.h"

// python include
extern "C"
{
#ifdef AT
	#undef AT
#endif

#include <python27/python.h>

#ifdef AT
	#undef AT
#endif
}

#include "PythonUtils.h"
#include "PythonLauncher.h"
#include "Resource.h"

void initdbg();
