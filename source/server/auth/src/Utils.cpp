#include "stdafx.h"
#include "Utils.h"

bool CopyStringSafe(char *dst, const std::string &src, uint32_t size) {
	if (size != 0) {
		auto cnt = std::min(src.length(), size - 1);
		std::memcpy(dst, src.data(), cnt);
		dst[cnt] = '\0';
		return cnt != size - 1;
	} else {
		dst[0] = '\0';
		return true;
	}
}