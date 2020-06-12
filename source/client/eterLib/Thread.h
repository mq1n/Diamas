#ifndef __INC_ETERLIB_THREAD_H__
#define __INC_ETERLIB_THREAD_H__

class CThread
{
	public:
		CThread();
		bool Create(void * arg);
		
	protected:
		static uint32_t CALLBACK	EntryPoint(void * pThis);

		virtual uint32_t			Setup() = 0;				// Execute�� �ҷ����� ���� �ҷ�����.
		virtual uint32_t			Execute(void * arg) = 0;	// ���� �����尡 �ϴ� ���� ���� ��

		uint32_t					Run(void * arg);

		void *					Arg() const		{ return m_pArg; }
		void					Arg(void * arg) { m_pArg = arg; }
		
		HANDLE					m_hThread;

	private:
		void *					m_pArg;
		uint32_t				m_uThreadID;
};

#endif
