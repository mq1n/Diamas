#pragma once

template <typename T> 
class TAbstractSingleton
{ 
	static T * ms_singleton;
	
public: 
	TAbstractSingleton()
	{ 
		assert(!ms_singleton);
		int32_t offset = (int32_t) (T*) 1 - (int32_t) (CSingleton <T>*) (T*) 1; 
		ms_singleton = (T*) ((int32_t) this + offset);
	} 

	virtual ~TAbstractSingleton()
	{ 
		assert(ms_singleton);
		ms_singleton = 0; 
	}

	__forceinline static T & GetSingleton()
	{
		assert(ms_singleton!=nullptr);
		return (*ms_singleton);
	}	
};

template <typename T> T * TAbstractSingleton <T>::ms_singleton = 0;