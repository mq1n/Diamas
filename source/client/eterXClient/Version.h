#pragma once
#define VER_FILE_VERSION __FILE_VERSION__
#define VER_FILE_VERSION_STR __PRODUCT_VERSION__
static inline std::string METIN2_GET_VERSION()
{
	return _GIT_VERSION_;
}
