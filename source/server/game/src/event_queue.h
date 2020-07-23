#pragma once
#include "../../../common/defines.h"

#ifdef M2_USE_POOL
#include "pool.h"
#endif

// TODO: concurrentqueue (quantumcore)

struct TQueueElement
{
	LPEVENT	pvData;
	int32_t		iStartTime;
	int32_t		iKey;
	bool	bCancel;
};

class CEventQueue
{
	public:
		struct FuncQueueComp
		{
			bool operator () (TQueueElement * left, TQueueElement * right) const
			{
				return (left->iKey > right->iKey);
			}
		};

	public:
		CEventQueue();
		~CEventQueue();

		TQueueElement *	Enqueue(LPEVENT data, int32_t duration, int32_t pulse);
		TQueueElement *	Dequeue();
		void		Delete(TQueueElement * pElem);
		void		Requeue(TQueueElement *, int32_t key);
		int32_t		GetTopKey();
		int32_t		Size();

	protected:
		void		Destroy();

	private:
		std::priority_queue<TQueueElement *, std::vector<TQueueElement *>, FuncQueueComp> m_pq_queue;

#ifdef M2_USE_POOL
		ObjectPool<TQueueElement> pool_;
#endif
};
