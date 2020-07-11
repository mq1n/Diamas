#pragma once
#include "../../common/service.h"
#include <intrusive_ptr.h>

#ifdef M2_USE_POOL
#include "pool.h"
#endif

/**
 * Base class for all event info data
 */
struct event_info_data 
{
	event_info_data() {}
	virtual ~event_info_data() {}

#ifdef M2_USE_POOL
	static void* operator new(size_t size) {
		return pool_.Acquire(size);
	}
	static void operator delete(void* p, size_t size) {
		pool_.Release(p, size);
	}
private:
	static MemoryPool pool_;
#endif
};
	
typedef struct event EVENT;
typedef eastl::intrusive_ptr<EVENT> LPEVENT;
typedef int32_t (*TEVENTFUNC) (LPEVENT event, int32_t processing_time);

#define EVENTFUNC(name)	int32_t (name) (LPEVENT event, int32_t processing_time)
#define EVENTINFO(name) struct name : public event_info_data

struct TQueueElement;

struct event
{
	event() : 
		func(nullptr), info(nullptr), q_el(nullptr), ref_count(0), is_force_to_end(0), is_processing(0)
	{
	}
	~event() 
	{
		if (info != nullptr) 
		{
#ifdef M2_USE_POOL
			delete info;
#else
			M2_DELETE(info);
#endif
		}
	}
	TEVENTFUNC			func;
	event_info_data* 	info;
	TQueueElement *		q_el;
	char				is_force_to_end;
	char				is_processing;

	size_t ref_count;
};

extern void intrusive_ptr_add_ref(EVENT* p);
extern void intrusive_ptr_release(EVENT* p);

template<class T> // T should be a subclass of event_info_data
T* AllocEventInfo() 
{
#ifdef M2_USE_POOL
	return new T;
#else
	return M2_NEW T;
#endif
}

extern void		event_destroy();
extern int32_t		event_process(int32_t pulse);
extern int32_t		event_count();

#define event_create(func, info, when) event_create_ex(func, info, when)
extern LPEVENT	event_create_ex(TEVENTFUNC func, event_info_data* info, int32_t when);
extern void		event_cancel(LPEVENT * event);			// 이벤트 취소
extern int32_t		event_processing_time(LPEVENT event);	// 수행 시간 리턴
extern int32_t		event_time(LPEVENT event);			// 남은 시간 리턴
extern void		event_reset_time(LPEVENT event, int32_t when);	// 실행 시간 재 설정
