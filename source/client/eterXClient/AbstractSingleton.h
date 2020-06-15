#pragma once

template <typename T> 
class TAbstractSingleton
{ 
	static T * ms_singleton;
	
public: 
	TAbstractSingleton()
	{ 
		assert(!ms_singleton);
		ms_singleton = static_cast<T*>(this);
	} 

	TAbstractSingleton(const TAbstractSingleton &) = delete;
	TAbstractSingleton(TAbstractSingleton &&) = delete;
	TAbstractSingleton & operator=(const TAbstractSingleton &) = delete;
	TAbstractSingleton & operator=(TAbstractSingleton &&) = delete;
	
	virtual ~TAbstractSingleton()
	{ 
		assert(ms_singleton);
		ms_singleton = nullptr;
	}

	__forceinline static T & GetSingleton()
	{
		assert(ms_singleton!=nullptr);
		return (*ms_singleton);
	}	
};

template <typename T>
T * TAbstractSingleton<T>::ms_singleton = nullptr;
