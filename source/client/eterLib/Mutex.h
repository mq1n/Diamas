#ifndef __INC_ETERLIB_MUTEX_H__
#define __INC_ETERLIB_MUTEX_H__
#include <mutex>
class Mutex
{
	public:
		Mutex();	
		~Mutex();
		
		void Lock();
		void Unlock();

	private:
		std::mutex lock;		
};

#endif
