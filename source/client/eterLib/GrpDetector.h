#pragma once
#include <string>
#include <d3d8.h>

typedef BOOL (*PFNCONFIRMDEVICE) (D3DCAPS8& rkD3DCaps, uint32_t uBehavior, D3DFORMAT eD3DFmt);

enum
{
	D3DDEVICETYPE_HAL,
	D3DDEVICETYPE_REF,
	D3DDEVICETYPE_NUM,
};

struct D3D_SModeInfo
{
	uint32_t m_uScrWidth;     
	uint32_t m_uScrHeight;   
	uint32_t m_uScrDepthBit;
 	uint32_t m_dwD3DBehavior;
	D3DFORMAT m_eD3DFmtPixel;
	D3DFORMAT m_eD3DFmtDepthStencil;

	VOID GetString(std::string* pstEnumList);
};

class D3D_CAdapterDisplayModeList
{
	public:
		D3D_CAdapterDisplayModeList() {}
		~D3D_CAdapterDisplayModeList() {}
		VOID Build(IDirect3D8& rkD3D, D3DFORMAT eD3DFmtDefault, uint32_t iAdapter);

		uint32_t GetDisplayModeNum();
		uint32_t GetPixelFormatNum();
		
		const D3DDISPLAYMODE&	GetDisplayModer(uint32_t iD3DDM);
		const D3DFORMAT&		GetPixelFormatr(uint32_t iD3DFmt);

	protected:
		enum
		{			
			D3DDISPLAYMODE_MAX = 100,
			D3DFORMAT_MAX = 20,

			FILTEROUT_LOWRESOLUTION_WIDTH = 640,
			FILTEROUT_LOWRESOLUTION_HEIGHT = 480,
		};

	protected:
		D3DDISPLAYMODE m_akD3DDM[D3DDISPLAYMODE_MAX];
		D3DFORMAT m_aeD3DFmt[D3DFORMAT_MAX];

		uint32_t m_uD3DDMNum;
		uint32_t m_uD3DFmtNum;
		
};

class D3D_CDeviceInfo
{
	public:
		D3D_CDeviceInfo() {}
		~D3D_CDeviceInfo() {}
		BOOL Build(IDirect3D8& rkD3D, uint32_t iAdapter, uint32_t iDevType, D3D_CAdapterDisplayModeList& rkD3DADMList, PFNCONFIRMDEVICE pfnConfirmDevice);
		BOOL Find(uint32_t uScrWidth, uint32_t uScrHeight, uint32_t uScrDepthBits, BOOL isWindowed, uint32_t* piD3DModeInfo);	

		uint32_t GetD3DModeInfoNum();
		
		VOID GetString(std::string* pstEnumList);
		
		BOOL FindDepthStencilFormat(IDirect3D8& rkD3D, uint32_t iAdapter, D3DDEVTYPE DeviceType, D3DFORMAT TargetFormat, D3DFORMAT* pDepthStencilFormat);

		D3D_SModeInfo& GetD3DModeInfor(uint32_t iD3DModeInfo);
		D3D_SModeInfo* GetD3DModeInfop(uint32_t iD3DModeInfo);
				
	protected:
		enum
		{
			D3DMODEINFO_NUM = 150,
		};

	
	protected:
		const TCHAR* m_szDevDesc;

		D3DDEVTYPE	m_eD3DDevType;
		D3DCAPS8	m_kD3DCaps; 			
		BOOL		m_canDoWindowed;

		uint32_t		m_iCurD3DModeInfo;
		uint32_t		m_uD3DModeInfoNum;
		D3D_SModeInfo	m_akD3DModeInfo[D3DMODEINFO_NUM];

	
		BOOL		m_isWindowed;

		D3DMULTISAMPLE_TYPE m_eD3DMSTWindowed;
		D3DMULTISAMPLE_TYPE m_eD3DMSTFullscreen;

	
	protected:
		static const CHAR*		msc_aszD3DDevDesc[D3DDEVICETYPE_NUM];
		static const D3DDEVTYPE msc_aeD3DDevType[D3DDEVICETYPE_NUM];		
};

class D3D_CAdapterInfo
{
	public:
		D3D_CAdapterInfo() {}
		~D3D_CAdapterInfo() {}
		BOOL Find(uint32_t uScrWidth, uint32_t uScrHeight, uint32_t uScrDepthBits, BOOL isWindowed, uint32_t* piD3DModeInfo, uint32_t* piD3DDevInfo);

		BOOL Build(IDirect3D8& rkD3D, uint32_t iAdapter, PFNCONFIRMDEVICE pfnConfirmDevice);
		VOID GetString(std::string* pstEnumList);

		D3DADAPTER_IDENTIFIER8& GetIdentifier()
		{
			return m_kD3DAdapterIdentifier;
		}
		
		D3DDISPLAYMODE&	GetDesktopD3DDisplayModer();
		D3DDISPLAYMODE*	GetDesktopD3DDisplayModep();
		
		D3D_CDeviceInfo*	GetD3DDeviceInfop(uint32_t iD3DDevInfo);
		D3D_SModeInfo*		GetD3DModeInfop(uint32_t iD3DDevInfo, uint32_t iD3DModeInfo);		

	protected:
		enum
		{
			D3DDEVICEINFO_NUM = 5,
		};
		
		
	protected:
		D3DADAPTER_IDENTIFIER8	m_kD3DAdapterIdentifier;
		D3DDISPLAYMODE			m_kD3DDMDesktop;

		uint32_t			m_iCurD3DDevInfo;
		uint32_t			m_uD3DDevInfoNum;
		D3D_CDeviceInfo	m_akD3DDevInfo[D3DDEVICEINFO_NUM];	
	
};

class D3D_CDisplayModeAutoDetector
{
	public:
		D3D_CDisplayModeAutoDetector();
		~D3D_CDisplayModeAutoDetector();

		BOOL Find(uint32_t uScrWidth, uint32_t uScrHeight, uint32_t uScrDepthBits, BOOL isWindowed, uint32_t* piD3DModeInfo, uint32_t* piD3DDevInfo, uint32_t* piD3DAdapterInfo);
		BOOL Build(IDirect3D8& rkD3D, PFNCONFIRMDEVICE pfnConfirmDevice);

		D3D_CAdapterInfo*	GetD3DAdapterInfop(uint32_t iD3DAdapterInfo);
		D3D_SModeInfo*		GetD3DModeInfop(uint32_t iD3DAdapterInfo, uint32_t iD3DDevInfo, uint32_t iD3DModeInfo);

		VOID GetString(std::string* pstEnumList);

	protected:
		enum
		{
			D3DADAPTERINFO_NUM = 10,
		};

	protected:
		D3D_CAdapterInfo	m_akD3DAdapterInfo[D3DADAPTERINFO_NUM];
		uint32_t				m_uD3DAdapterInfoCount;
};