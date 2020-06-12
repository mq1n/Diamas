#pragma once
#include "ReferenceObject.h"
#include <string>
#include <FileSystemIncl.hpp>

class CResource : public CReferenceObject
{
	public:
		typedef uint32_t TType;

		enum EState
		{
			STATE_EMPTY,
			STATE_ERROR,			
			STATE_EXIST,
			STATE_LOAD,
			STATE_FREE
		};

	public:
		void			Clear();

		static TType	StringToType(const char* c_szType);
		static TType	Type();

		void			Load();
		void			Reload();

		virtual bool	CreateDeviceObjects();
		virtual void	DestroyDeviceObjects();

	public:
		CResource(const FileSystem::CFileName& filename);
		virtual ~CResource();

		static void		SetDeleteImmediately(bool isSet = false);

		// is loaded?
		bool			IsData() const;
		bool			IsEmpty() const;
		bool			IsType(TType type);

		std::string GetFileNameString() const { return m_stFileName.GetPathA(); }
		const FileSystem::CFileName& GetFilename() const { return m_stFileName; }

		virtual bool	OnLoad(int32_t iSize, const void * c_pvBuf) = 0;

	protected:
		void			SetFileName(const FileSystem::CFileName& filename);

		virtual void	OnClear() = 0;
		virtual bool	OnIsEmpty() const = 0;
		virtual bool	OnIsType(TType type) = 0;

		virtual void	OnConstruct();
		virtual void	OnSelfDestruct();

	protected:
		FileSystem::CFileName		m_stFileName;
		//char *			m_pszFileName;
		//uint32_t			m_dwLoadCostMiliiSecond;
		EState			me_state;

	protected:
		static bool		ms_bDeleteImmediately;
};
