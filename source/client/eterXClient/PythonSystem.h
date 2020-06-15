#pragma once

class CPythonSystem : public CSingleton<CPythonSystem>
{
	public:
		enum
		{
			FREQUENCY_MAX_NUM  = 30,
			RESOLUTION_MAX_NUM = 100
		};

		typedef struct SResolution
		{
			uint32_t	width;
			uint32_t	height;
			uint32_t	bpp;		// bits per pixel (high-color = 16bpp, true-color = 32bpp)

			uint32_t	frequency[20];
			uint8_t	frequency_count;
		} TResolution;

		typedef struct SConfig
		{
			uint32_t			width;
			uint32_t			height;
			uint32_t			bpp;
			uint32_t			frequency;

			bool			is_software_cursor;
			bool			is_object_culling;
			int32_t				iDistance;
			int32_t				iShadowLevel;

			float			music_volume;
		float voice_volume;

			int32_t				gamma;

			bool			bWindowed;
			bool			bDecompressDDS;
			bool			bNoSoundCard;
			bool			bUseDefaultIME;
			bool			bViewChat;
			uint8_t			bShowNameFlag;
			bool			bShowDamage;
			bool			bShowSalesText;
#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBAIFLAG)
			bool			bShowMobAIFlag;
#endif
#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBLEVEL)
			bool			bShowMobLevel;
#endif
		} TConfig;

	public:
		CPythonSystem();
		virtual ~CPythonSystem();

		void Clear();
		void SetInterfaceHandler(PyObject * poHandler);
		void DestroyInterfaceHandler();

		// Config
		void							SetDefaultConfig();
		bool							LoadConfig();
		bool							SaveConfig();
		void							ApplyConfig();
		void SetConfig(const TConfig * pNewConfig);
		TConfig *						GetConfig();
		void							ChangeSystem();

		uint32_t							GetWidth();
		uint32_t							GetHeight();
		uint32_t							GetBPP();
		uint32_t							GetFrequency();
		bool							IsSoftwareCursor();
		bool							IsWindowed();
		bool							IsViewChat();
		uint8_t							GetShowNameFlag();
		bool							IsAlwaysShowName();
		bool							IsShowDamage();
		bool							IsShowSalesText();
		bool							IsUseDefaultIME();
		bool							IsNoSoundCard();
		void							SetViewChatFlag(int32_t iFlag);
		void							SetShowNameFlag(int32_t iFlag);
		void							SetShowDamageFlag(int32_t iFlag);
		void							SetShowSalesTextFlag(int32_t iFlag);
#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBAIFLAG)
		bool							IsShowMobAIFlag();
		void							SetShowMobAIFlagFlag(int32_t iFlag);
#endif
#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBLEVEL)
		bool							IsShowMobLevel();
		void							SetShowMobLevelFlag(int32_t iFlag);
#endif
		/// Display
		void							GetDisplaySettings();

		int32_t								GetResolutionCount();
		int32_t								GetFrequencyCount(int32_t index);
		bool							GetResolution(int32_t index, OUT uint32_t *width, OUT uint32_t *height, OUT uint32_t *bpp);
		bool							GetFrequency(int32_t index, int32_t freq_index, OUT uint32_t *frequncy);
		int32_t								GetResolutionIndex(uint32_t width, uint32_t height, uint32_t bpp);
		int32_t								GetFrequencyIndex(int32_t res_index, uint32_t frequency);
		bool							isViewCulling();

		// Sound
		float							GetMusicVolume();
		float							GetSoundVolume();
		void							SetMusicVolume(float fVolume);
		void							SetSoundVolume(float fVolume);

		int32_t								GetDistance();
		int32_t								GetShadowLevel();
		void							SetShadowLevel(uint32_t level);

	protected:
		TResolution						m_ResolutionList[RESOLUTION_MAX_NUM];
		int32_t								m_ResolutionCount;

		TConfig							m_Config;
		TConfig							m_OldConfig;

		PyObject *						m_poInterfaceHandler;
};