#pragma once
#ifdef _WIN32
#include <Windows.h>
#include <typeinfo>
#endif
#include <cassert>

template <typename T>
class CSingleton
{
	static T* ms_singleton;

public:
	CSingleton()
	{
#ifdef _WIN32
		if (ms_singleton)
			MessageBoxA(nullptr, typeid(T).name(), "CSingleton() DECLARED MORE THAN ONCE", MB_ICONEXCLAMATION | MB_OK);
#endif

		assert(!ms_singleton);
		ms_singleton = static_cast<T*>(this);
	}

	virtual ~CSingleton()
	{
#ifdef _WIN32
		if (!ms_singleton)
			MessageBoxA(nullptr, typeid(T).name(), "~CSingleton() FREED AT RUNTIME", MB_ICONEXCLAMATION | MB_OK);
#endif

		assert(ms_singleton);
		ms_singleton = nullptr;
	}

	__forceinline static T& Instance()
	{
#ifdef _WIN32
		if (!ms_singleton)
			MessageBoxA(nullptr, typeid(T).name(), "CSingleton::Instance() NEVER DECLARED", MB_ICONEXCLAMATION | MB_OK);
#endif

		assert(ms_singleton);
		return (*ms_singleton);
	}

	__forceinline static T* InstancePtr()
	{
		return (ms_singleton);
	}
};

template <typename T>
T* CSingleton<T>::ms_singleton = nullptr;
