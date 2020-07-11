#ifndef __CSTATEMANAGER_H
#define __CSTATEMANAGER_H

#include <d3d9.h>
#include <d3dx9.h>

#include <vector>

#include "../eterBase/Singleton.h"

#define CHECK_D3DAPI(a)		\
{							\
	HRESULT hr = (a);		\
							\
	if (hr != S_OK)			\
		assert(!#a);		\
}

static const uint32_t STATEMANAGER_MAX_RENDERSTATES = 256;
static const uint32_t STATEMANAGER_MAX_TEXTURESTATES = 128;
static const uint32_t STATEMANAGER_MAX_SAMPLERSTATES = 128;
static const uint32_t STATEMANAGER_MAX_STAGES = 8;
static const uint32_t STATEMANAGER_MAX_VCONSTANTS = 96;
static const uint32_t STATEMANAGER_MAX_PCONSTANTS = 8;
static const uint32_t STATEMANAGER_MAX_TRANSFORMSTATES = 300;
static const uint32_t STATEMANAGER_MAX_STREAMS = 16;

class CStreamData
{
	public:
		CStreamData(LPDIRECT3DVERTEXBUFFER9 pStreamData = nullptr, uint32_t Stride = 0) : m_lpStreamData(pStreamData), m_Stride(Stride)
		{
		}

		bool operator == (const CStreamData& rhs) const
		{
			return ((m_lpStreamData == rhs.m_lpStreamData) && (m_Stride == rhs.m_Stride));
		}

		LPDIRECT3DVERTEXBUFFER9	m_lpStreamData;
		uint32_t					m_Stride;
};

// State types managed by the class
typedef enum eStateType
{
	STATE_MATERIAL = 0,
	STATE_RENDER,
	STATE_TEXTURE,
	STATE_TEXTURESTAGE,
	STATE_VSHADER,
	STATE_PSHADER,
	STATE_TRANSFORM,
	STATE_VCONSTANT,
	STATE_PCONSTANT,
	STATE_STREAM,
	STATE_INDEX
} eStateType;

class CStateID
{
	public:
		CStateID(eStateType Type, uint32_t dwValue0 = 0, uint32_t dwValue1 = 0)
			: m_Type(Type),
		m_dwValue0(dwValue0),
		m_dwValue1(dwValue1)
		{
		}

		CStateID(eStateType Type, uint32_t dwStage, D3DTEXTURESTAGESTATETYPE StageType)
			: m_Type(Type),
		m_dwStage(dwStage),
		m_TextureStageStateType(StageType)
		{
		}

		CStateID(eStateType Type, D3DRENDERSTATETYPE RenderType)
			: m_Type(Type),
		m_RenderStateType(RenderType)
		{
		}

		eStateType m_Type;

		union
		{
			uint32_t					m_dwValue0;
			uint32_t					m_dwStage;
			D3DRENDERSTATETYPE		m_RenderStateType;
			D3DTRANSFORMSTATETYPE	m_TransformStateType;
		};

		union
		{
			uint32_t						m_dwValue1;
			D3DTEXTURESTAGESTATETYPE	m_TextureStageStateType;
		};
};

typedef std::vector<CStateID> TStateID;

class CStateManagerState
{
	public:
		CStateManagerState(): m_dwPixelShader(0), m_dwVertexShader(0)
		{
		}

		void ResetState()
		{
			uint32_t i, y;

			for (i = 0; i < STATEMANAGER_MAX_RENDERSTATES; i++)
				m_RenderStates[i] = 0x7FFFFFFF;

			for (i = 0; i < STATEMANAGER_MAX_STAGES; i++) {
				for (y = 0; y < STATEMANAGER_MAX_TEXTURESTATES; y++)
					m_TextureStates[i][y] = 0x7FFFFFFF;

				for (y = 0; y < STATEMANAGER_MAX_SAMPLERSTATES; ++y)
					m_SamplerStates[i][y] = 0x7FFFFFFF;
			}
			for (i = 0; i < STATEMANAGER_MAX_STREAMS; i++)
				m_StreamData[i] = CStreamData();

			m_IndexData = nullptr;

			for (i = 0; i < STATEMANAGER_MAX_STAGES; i++)
				m_Textures[i] = nullptr;
			for (i = 0; i < STATEMANAGER_MAX_TRANSFORMSTATES; i++)
				D3DXMatrixIdentity(&m_Matrices[i]);

			for (i = 0; i < STATEMANAGER_MAX_VCONSTANTS; i++)
				m_VertexShaderConstants[i] = D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f);

			for (i = 0; i < STATEMANAGER_MAX_PCONSTANTS; i++)
				m_PixelShaderConstants[i] = D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f);

			m_dwPixelShader = 0;
			m_dwVertexShader = 0;
			m_dwVertexDeclaration = 0;
			m_dwFVF = D3DFVF_XYZ;
			m_bVertexProcessing = false;

			ZeroMemory(&m_Matrices, sizeof(D3DXMATRIX) * STATEMANAGER_MAX_TRANSFORMSTATES);
		}
		uint32_t					m_RenderStates[STATEMANAGER_MAX_RENDERSTATES];
		uint32_t					m_TextureStates[STATEMANAGER_MAX_STAGES][STATEMANAGER_MAX_TEXTURESTATES];
		uint32_t					m_SamplerStates[STATEMANAGER_MAX_STAGES][STATEMANAGER_MAX_SAMPLERSTATES];
		D3DXVECTOR4				m_VertexShaderConstants[STATEMANAGER_MAX_VCONSTANTS];
		D3DXVECTOR4				m_PixelShaderConstants[STATEMANAGER_MAX_PCONSTANTS];
		LPDIRECT3DBASETEXTURE9	m_Textures[STATEMANAGER_MAX_STAGES];
		LPDIRECT3DPIXELSHADER9  m_dwPixelShader;
		LPDIRECT3DVERTEXSHADER9 m_dwVertexShader;
		LPDIRECT3DVERTEXDECLARATION9 m_dwVertexDeclaration;
		uint32_t					m_dwFVF;

		D3DXMATRIX				m_Matrices[STATEMANAGER_MAX_TRANSFORMSTATES];

		D3DMATERIAL9			m_D3DMaterial;

		CStreamData				m_StreamData[STATEMANAGER_MAX_STREAMS];
		LPDIRECT3DINDEXBUFFER9 m_IndexData;

		bool					m_bVertexProcessing;
};

class CStateManager : public CSingleton<CStateManager>
{
	public:
		CStateManager(LPDIRECT3DDEVICE9 lpDevice);
		virtual ~CStateManager();

		void	SetDefaultState();
		void	Restore();

		bool	BeginScene();
		void	EndScene();
		void	SaveMaterial();
		void	SaveMaterial(const D3DMATERIAL9 * pMaterial);
		void	RestoreMaterial();
		void	SetMaterial(const D3DMATERIAL9 * pMaterial);
		void	GetMaterial(D3DMATERIAL9 * pMaterial);

		void	SetLight(uint32_t index, CONST D3DLIGHT9* pLight);
		void	GetLight(uint32_t index, D3DLIGHT9* pLight);
		void	SaveRenderState(D3DRENDERSTATETYPE Type, uint32_t dwValue);
		void	RestoreRenderState(D3DRENDERSTATETYPE Type);
		void	SetRenderState(D3DRENDERSTATETYPE Type, uint32_t Value);
		void	GetRenderState(D3DRENDERSTATETYPE Type, uint32_t * pdwValue);
		void	SaveTexture(uint32_t dwStage, LPDIRECT3DBASETEXTURE9 pTexture);
		void	RestoreTexture(uint32_t dwStage);
		void	SetTexture(uint32_t dwStage, LPDIRECT3DBASETEXTURE9 pTexture);
		void	GetTexture(uint32_t dwStage, LPDIRECT3DBASETEXTURE9 * ppTexture);
		void	SaveTextureStageState(uint32_t dwStage, D3DTEXTURESTAGESTATETYPE Type, uint32_t dwValue);
		void	RestoreTextureStageState(uint32_t dwStage, D3DTEXTURESTAGESTATETYPE Type);
		void	SetTextureStageState(uint32_t dwStage, D3DTEXTURESTAGESTATETYPE Type, uint32_t dwValue);
		void	GetTextureStageState(uint32_t dwStage, D3DTEXTURESTAGESTATETYPE Type, uint32_t * pdwValue);
		void	SetBestFiltering(uint32_t dwStage); 
		void	SaveSamplerState(uint32_t dwStage, D3DSAMPLERSTATETYPE Type, uint32_t dwValue);
		void	RestoreSamplerState(uint32_t dwStage, D3DSAMPLERSTATETYPE Type);
		void	SetSamplerState(uint32_t dwStage, D3DSAMPLERSTATETYPE Type, uint32_t dwValue);
		void	GetSamplerState(uint32_t dwStage, D3DSAMPLERSTATETYPE Type, uint32_t * pdwValue);
		void	SaveVertexShader(LPDIRECT3DVERTEXSHADER9 dwShader);
		void	RestoreVertexShader();
		void	SetVertexShader(LPDIRECT3DVERTEXSHADER9 dwShader);
		void	GetVertexShader(LPDIRECT3DVERTEXSHADER9 * pdwShader);
		void	SaveVertexDeclaration(LPDIRECT3DVERTEXDECLARATION9 dwShader);
		void	RestoreVertexDeclaration();
		void	SetVertexDeclaration(LPDIRECT3DVERTEXDECLARATION9 dwShader);
		void	GetVertexDeclaration(LPDIRECT3DVERTEXDECLARATION9 * pdwShader);
		void	SaveFVF(uint32_t dwShader);
		void	RestoreFVF();
		void	SetFVF(uint32_t dwShader);
		void	GetFVF(uint32_t * pdwShader);
		void	SavePixelShader(LPDIRECT3DPIXELSHADER9 dwShader);
		void	RestorePixelShader();
		void	SetPixelShader(LPDIRECT3DPIXELSHADER9 dwShader);
		void	GetPixelShader(LPDIRECT3DPIXELSHADER9 * pdwShader);
		void SaveTransform(D3DTRANSFORMSTATETYPE Transform, const D3DMATRIX* pMatrix);
		void RestoreTransform(D3DTRANSFORMSTATETYPE Transform);
		void SaveVertexProcessing(bool IsON);
		void RestoreVertexProcessing();
		void SetTransform(D3DTRANSFORMSTATETYPE Type, const D3DMATRIX* pMatrix);
		void GetTransform(D3DTRANSFORMSTATETYPE Type, D3DMATRIX * pMatrix);
		void SaveVertexShaderConstant(uint32_t dwRegister, CONST void* pConstantData, uint32_t dwConstantCount);
		void RestoreVertexShaderConstant(uint32_t dwRegister, uint32_t dwConstantCount);
		void SetVertexShaderConstant(uint32_t dwRegister, CONST void* pConstantData, uint32_t dwConstantCount);
		void SavePixelShaderConstant(uint32_t dwRegister, CONST void* pConstantData, uint32_t dwConstantCount);
		void RestorePixelShaderConstant(uint32_t dwRegister, uint32_t dwConstantCount);
		void SetPixelShaderConstant(uint32_t dwRegister, CONST void* pConstantData, uint32_t dwConstantCount);

		void SaveStreamSource(uint32_t StreamNumber, LPDIRECT3DVERTEXBUFFER9 pStreamData, uint32_t Stride);
		void RestoreStreamSource(uint32_t StreamNumber);
		void SetStreamSource(uint32_t StreamNumber, LPDIRECT3DVERTEXBUFFER9 pStreamData, uint32_t Stride);

		void SaveIndices(LPDIRECT3DINDEXBUFFER9 pIndexData);
		void RestoreIndices();
		void SetIndices(LPDIRECT3DINDEXBUFFER9 pIndexData);

		HRESULT DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, uint32_t StartVertex, uint32_t PrimitiveCount);
		HRESULT DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, uint32_t PrimitiveCount, const void* pVertexStreamZeroData, uint32_t VertexStreamZeroStride);
		HRESULT DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, uint32_t minIndex, uint32_t NumVertices, uint32_t startIndex, uint32_t primCount, INT baseVertexIndex = 0);
		HRESULT DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, uint32_t MinVertexIndex, uint32_t NumVertexIndices, uint32_t PrimitiveCount, CONST void * pIndexData, D3DFORMAT IndexDataFormat, CONST void * pVertexStreamZeroData, uint32_t VertexStreamZeroStride);
		// Codes For Debug
		uint32_t GetRenderState(D3DRENDERSTATETYPE Type);

		bool LoadShader(const char *pszFilename, LPD3DXEFFECT &pEffect);

		void EnableAntiAlias(bool bEnable);
		bool IsAntiAliasEnabled() const;

		inline LPDIRECT3DDEVICE9 GetDevice() { return m_lpD3DDev; }

	private:
		void SetDevice(LPDIRECT3DDEVICE9 lpDevice);

	private:
		CStateManagerState	m_ChipState;
		CStateManagerState	m_CurrentState;
		CStateManagerState	m_CopyState;
		TStateID			m_DirtyStates;
		bool				m_bForce;
		bool				m_bScene;
		uint32_t				m_dwBestMinFilter;
		uint32_t				m_dwBestMagFilter;
		LPDIRECT3DDEVICE9	m_lpD3DDev;
		bool				m_bEnableGlobalAntialiasing;

#ifdef _DEBUG

		BOOL				m_bRenderStateSavingFlag[STATEMANAGER_MAX_RENDERSTATES];
		BOOL				m_bTextureStageStateSavingFlag[STATEMANAGER_MAX_STAGES][STATEMANAGER_MAX_TEXTURESTATES];
		BOOL				m_bSamplerStateSavingFlag[STATEMANAGER_MAX_STAGES][STATEMANAGER_MAX_SAMPLERSTATES];
		BOOL				m_bTransformSavingFlag[STATEMANAGER_MAX_TRANSFORMSTATES];
#endif // _DEBUG
};

#define STATEMANAGER (CStateManager::Instance())

#endif
