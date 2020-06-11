/*
 *    Filename: queue.c
 * Description: ť ó��
 *
 *      Author: ������ (aka. ��, Cronan), �ۿ��� (aka. myevan, ���ڷ�)
 */
#include "stdafx.h"

#include "event_queue.h"

CEventQueue::CEventQueue()
{
}

CEventQueue::~CEventQueue()
{
	Destroy();
}

void CEventQueue::Destroy()
{
	while (!m_pq_queue.empty())
	{
		TQueueElement * pElem = m_pq_queue.top();
		m_pq_queue.pop();

		Delete(pElem);
	}
}

TQueueElement * CEventQueue::Enqueue(LPEVENT pvData, int32_t duration, int32_t pulse)
{
#ifdef M2_USE_POOL
	TQueueElement * pElem = pool_.Construct();
#else
	TQueueElement * pElem = M2_NEW TQueueElement;
#endif

	pElem->pvData = pvData;
	pElem->iStartTime = pulse;
	pElem->iKey = duration + pulse;
	pElem->bCancel = FALSE;

	m_pq_queue.push(pElem);
	return pElem;
}

TQueueElement * CEventQueue::Dequeue()
{
	if (m_pq_queue.empty())
		return nullptr;

	TQueueElement * pElem = m_pq_queue.top();
	m_pq_queue.pop();
	return pElem;
}

void CEventQueue::Delete(TQueueElement * pElem)
{
#ifdef M2_USE_POOL
	pool_.Destroy(pElem);
#else
	M2_DELETE(pElem);
#endif
}

int32_t CEventQueue::GetTopKey()
{
	if (m_pq_queue.empty())
		return INT_MAX;

	return m_pq_queue.top()->iKey;
}

int32_t CEventQueue::Size()
{
	return m_pq_queue.size();
}

