#pragma once

#include "../eterLib/StdAfx.h"
#include "../eterBase/StdAfx.h"

#pragma warning(disable : 4244)

#define IS_VALID_HANDLE(handle)				(handle && handle != INVALID_HANDLE_VALUE)

typedef _Return_type_success_(return >= 0) LONG NTSTATUS;
