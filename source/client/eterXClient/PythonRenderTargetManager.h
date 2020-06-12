#pragma once
#include "StdAfx.h"
#include "InstanceBase.h"

class CPythonRenderTargetManager : public CSingleton<CPythonRenderTargetManager>
{
	public:
		enum RenderTypes {
			RENDER_TYPE_SHOP_DECO,
			RENDER_TYPE_COSTUME_INVENTORY,
			RENDER_TYPE_ITEM_TOOLTIP,
			RENDER_TYPE_MAX_NUM,
		};

		CPythonRenderTargetManager();
		virtual ~CPythonRenderTargetManager();

		void Destroy();
		void InitializeRenderInstance(uint32_t index, const char* background_image);

		void Show(uint32_t index, bool show);
		void CreateModelInstance(uint32_t index, uint32_t race_vnum);
		
		void SetRace(uint32_t index, uint32_t race_vnum);
		void SetAlwaysRotate(uint32_t index, bool rotate);
		void SetRotation(uint32_t index, float rotation);
		void SetArmor(uint32_t index, uint32_t armor_vnum);
		void SetHair(uint32_t index, uint32_t hair_vnum);
		void SetWeapon(uint32_t index, uint32_t weapon_vnum);
		void SetMotion(uint32_t index, uint32_t motion_index);

		void RenderBackground();
		void Deform();
		void RenderModel();
		void Update();

		class CPythonRenderTarget : public CGraphicBase {
			public:
				CPythonRenderTarget();
				virtual ~CPythonRenderTarget();

				void Initialize();
				void Destroy();

				bool CreateModelInstance(uint32_t index);
				
				void SetRace(uint32_t index);
				void SetArmor(uint32_t armor_vnum);
				void SetHair(uint32_t hair_vnum);
				void SetWeapon(uint32_t weapon_vnum);
				void SetMotion(uint32_t motion_index);

				bool BuildBackround(const char* background_image);
				void RenderBackground();
				void Update();
				void Deform();
				void RenderModel();

				void SetAlwaysRotate(bool rotate) { m_bAlwaysRotate = rotate; }
				void SetRotation(float rotation) { m_fRotation = rotation; }
				void Show(bool show) { m_bShow = show; }
				bool IsShow() { return m_bShow; }

				void SetRenderIndex(uint32_t index) { m_dwRenderIndex = index; }

			private:
				CInstanceBase* m_pModel;
				CGraphicExpandedImageInstance* m_pImageBG;
				bool m_bAlwaysRotate;
				float m_fRotation;
				bool m_bShow;
				uint32_t m_dwRenderIndex;
		};

	private:
		std::map<uint32_t, CPythonRenderTarget*> m_renderInstances;
};
