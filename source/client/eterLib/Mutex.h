#ifndef __INC_ETERLIB_MUTEX_H__
#define __INC_ETERLIB_MUTEX_H__

class Mutex
{
	public:
		Mutex();	
		~Mutex();
		
		void Lock();
		void Unlock();

	private:
		CRITICAL_SECTION lock;		
};

#endif
