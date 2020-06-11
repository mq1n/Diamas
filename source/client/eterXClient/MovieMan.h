#pragma once

// 페이드아웃 길이(초)
#define		MOVIEMAN_FADE_DURATION		1300
#define		MOVIEMAN_SKIPPABLE_YES		true
#define		MOVIEMAN_POSTEFFECT_FADEOUT	1

// 동영상 재생에 DX 7.0 사용
struct IDirectDraw;
struct IDirectDrawSurface;
struct IDirectDrawMediaStream;
struct IGraphBuilder;
struct IBasicAudio;
struct IMultiMediaStream;
struct IAMMultiMediaStream;

const GUID CLSID_MP43DMOCodec = { 0xcba9e78b, 0x49a3, 0x49ea, 0x93, 0xd4, 0x6b, 0xcb, 0xa8, 0xc4, 0xde, 0x07 };
const GUID CLSID_MP4VideoCodec = { 0x82ccd3e0, 0xf71a, 0x11d0, 0x9f, 0xe5, 0x0, 0x60, 0x97, 0x78, 0xea, 0x66 };
const GUID CLSID_MP3AudioCodec = { 0x38be3000, 0xdbf4, 0x11d0, 0x86, 0x0e, 0x0, 0xa0, 0x24, 0xcf, 0xef, 0x6d };	
const GUID CLSID_DIrectSoundRenderer = { 0x79376820, 0x07D0, 0x11CF, 0xA2, 0x4D, 0x0, 0x20, 0xAF, 0xD7, 0x97, 0x67 };

class CMovieMan : public CSingleton<CMovieMan>
{
public:
	CMovieMan()
		: m_movieWidth(0),
		  m_movieHeight(0),
		  m_usingRGB32(false),
		  m_pPrimarySurface(nullptr),
		  m_pBasicAudio(nullptr)
	{
		CoInitialize(nullptr);
	};

	virtual ~CMovieMan()
	{
		CoUninitialize();
	};

	void				ClearToBlack();
	void				PlayLogo(const char *pcszName);
	void				PlayIntro();
	BOOL                PlayTutorial(int32_t nIdx);

private:
	bool				m_usingRGB32;
	int32_t  				m_movieWidth;
	int32_t  				m_movieHeight;
	RECT				m_movieRect;
	IDirectDrawSurface*	m_pPrimarySurface;
	IBasicAudio*		m_pBasicAudio;

	void				FillRect(RECT& fillRect, uint32_t fillColor);
	inline void			GDIFillRect(RECT& fillRect, uint32_t fillColor);
	inline void			GDIBlt(IDirectDrawSurface *pSrcSurface, RECT *pDestRect);

	void				GetWindowRect(RECT& windowRect);
	void				CalcMovieRect(int32_t srcWidth, int32_t srcHeight, RECT& movieRect);
	void				CalcBackgroundRect(const RECT& movieRect, RECT& upperRect, RECT& lowerRect);

	BOOL				PlayMovie(const char *cpFileName, const bool bSkipAllowed = FALSE, const int32_t nPostEffectID = 0, const uint32_t dwPostEffectData = 0);

	HRESULT				BuildFilterGraphManually(
							WCHAR* wpFilename, 
							IAMMultiMediaStream *pAMStream, 
							const GUID FAR clsidSplitter, 
							const GUID FAR clsidVideoCodec,
							const GUID FAR clsidAudioCodec);

	HRESULT				RenderFileToMMStream(const char *cpFilename, IMultiMediaStream **ppMMStream, IDirectDraw *pDD);
	HRESULT				RenderStreamToSurface(IDirectDrawSurface *pSurface, IDirectDrawMediaStream *pDDStream, IMultiMediaStream *pMMStream, bool bSkipAllowed, int32_t nPostEffectID, uint32_t dwPostEffectData);
	HRESULT				RenderPostEffectFadeOut(IDirectDrawSurface *pSurface, int32_t fadeOutDuration, uint32_t fadeOutColor);

//#ifdef _DEBUG
//	HRESULT				AddToRot(IGraphBuilder* pGraphBuilder, uint32_t *pdwRegister);
//	void				RemoveFromRot(uint32_t pdwRegister);
//#endif
};
