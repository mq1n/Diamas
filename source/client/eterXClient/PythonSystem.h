#pragma once

class CPythonSystem : public CSingleton<CPythonSystem>
{
	public:
		enum EWindow
		{
			WINDOW_STATUS,
			WINDOW_INVENTORY,
			WINDOW_ABILITY,
			WINDOW_SOCIETY,
			WINDOW_JOURNAL,
			WINDOW_COMMAND,

			WINDOW_QUICK,
			WINDOW_GAUGE,
			WINDOW_MINIMAP,
			WINDOW_CHAT,

			WINDOW_MAX_NUM,
		};

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

		typedef struct SWindowStatus
		{
			int32_t		isVisible;
			int32_t		isMinimized;

			int32_t		ixPosition;
			int32_t		iyPosition;
			int32_t		iHeight;
		} TWindowStatus;

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
			uint8_t			voice_volume;

			int32_t				gamma;

			int32_t				isSaveID;
			char			SaveID[20];

			bool			bWindowed;
			bool			bDecompressDDS;
			bool			bNoSoundCard;
			bool			bUseDefaultIME;
			uint8_t			bSoftwareTiling;
			bool			bViewChat;
			bool			bAlwaysShowName;
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
		void							SetConfig(TConfig * set_config);
		TConfig *						GetConfig();
		void							ChangeSystem();

		// Interface
		bool							LoadInterfaceStatus();
		void							SaveInterfaceStatus();
		bool							isInterfaceConfig();
		const TWindowStatus &			GetWindowStatusReference(int32_t iIndex);

		uint32_t							GetWidth();
		uint32_t							GetHeight();
		uint32_t							GetBPP();
		uint32_t							GetFrequency();
		bool							IsSoftwareCursor();
		bool							IsWindowed();
		bool							IsViewChat();
		bool							IsAlwaysShowName();
		bool							IsShowDamage();
		bool							IsShowSalesText();
		bool							IsUseDefaultIME();
		bool							IsNoSoundCard();
		bool							IsAutoTiling();
		bool							IsSoftwareTiling();
		void							SetSoftwareTiling(bool isEnable);
		void							SetViewChatFlag(int32_t iFlag);
		void							SetAlwaysShowNameFlag(int32_t iFlag);
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

		// Window
		void							SaveWindowStatus(int32_t iIndex, int32_t iVisible, int32_t iMinimized, int32_t ix, int32_t iy, int32_t iHeight);

		// SaveID
		int32_t								IsSaveID();
		const char *					GetSaveID();
		void							SetSaveID(int32_t iValue, const char * c_szSaveID);

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
		int32_t								GetSoundVolume();
		void							SetMusicVolume(float fVolume);
		void							SetSoundVolumef(float fVolume);

		int32_t								GetDistance();
		int32_t								GetShadowLevel();
		void							SetShadowLevel(uint32_t level);

	protected:
		TResolution						m_ResolutionList[RESOLUTION_MAX_NUM];
		int32_t								m_ResolutionCount;

		TConfig							m_Config;
		TConfig							m_OldConfig;

		bool							m_isInterfaceConfig;
		PyObject *						m_poInterfaceHandler;
		TWindowStatus					m_WindowStatus[WINDOW_MAX_NUM];
};