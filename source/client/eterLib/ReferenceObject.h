#pragma once

class CReferenceObject
{
	public:
		CReferenceObject();
		virtual ~CReferenceObject();

		void AddReference();
		void AddReferenceOnly();
		void Release();
		
		int32_t GetReferenceCount();

		bool canDestroy();
		
	protected:		
		virtual void OnConstruct();
		virtual void OnSelfDestruct();

	private:
		int32_t m_refCount;
		bool m_destructed;
};
