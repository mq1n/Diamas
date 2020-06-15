#pragma once

#include "Model.h"
#include "Motion.h"

class CGraphicThing : public CResource
{
	public:
		using TRef = CRef<CGraphicThing>;

	public:
		static TType Type();

	public:
		CGraphicThing(const FileSystem::CFileName& filename);
		virtual ~CGraphicThing();

		virtual bool			CreateDeviceObjects();
		virtual void			DestroyDeviceObjects();

		bool					CheckModelIndex(int32_t iModel) const;
		CGrannyModel *			GetModelPointer(int32_t iModel);
		int32_t						GetModelCount() const;

		bool					CheckMotionIndex(int32_t iMotion) const;
		CGrannyMotion *			GetMotionPointer(int32_t iMotion);
		int32_t						GetMotionCount() const;
		
		int32_t						GetTextureCount() const;
		const char *			GetTexturePath(int32_t iTexture);

	protected:
		void					Initialize();

		bool					LoadModels();
		bool					LoadMotions();

	protected:
		bool					OnLoad(int32_t iSize, const void* c_pvBuf);
		void					OnClear();
		bool					OnIsEmpty() const;
		bool					OnIsType(TType type);

	protected:
		granny_file *			m_pgrnFile;
		granny_file_info *		m_pgrnFileInfo;

		granny_animation *		m_pgrnAni;

		CGrannyModel *			m_models;
		CGrannyMotion *			m_motions;
};
