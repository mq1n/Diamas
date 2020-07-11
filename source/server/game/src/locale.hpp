#pragma once

extern "C"
{
	void locale_init(const char *filename); 
	const char *locale_find(const char *string);

#define LC_TEXT(str) locale_find(str)
};
