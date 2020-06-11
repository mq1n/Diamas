#ifndef __INC_ETERIMAGELIB_IMAGE_H__
#define __INC_ETERIMAGELIB_IMAGE_H__

#include <Windows.h>
#include <string>
#include <cstdint>

#pragma pack(push)
#pragma pack(1)
struct TGA_HEADER
{
	char idLen;			// 0
	char palType;		// 파레트있으면 1, 없음 0
	char imgType;		// 파레트있으면 1, 없음 2
	uint16_t colorBegin;	// 0
	uint16_t colorCount;	// 파레트 있으면 256, 없음 0
	char palEntrySize;	// 파레트 있으면 24, 없음 0
	uint16_t left;
	uint16_t top;
	uint16_t width;
	uint16_t height;
	char colorBits;
	char desc;
};
#define IMAGEDESC_ORIGIN_MASK		0x30
#define IMAGEDESC_TOPLEFT			0x20
#define IMAGEDESC_BOTLEFT			0x00
#define IMAGEDESC_BOTRIGHT			0x10
#define IMAGEDESC_TOPRIGHT			0x30
#pragma pack(pop)

class CImageC
{
	public:
		CImageC();
		CImageC(CImageC & image);

		virtual ~CImageC();

		void				Destroy();

		void				Create(int32_t width, int32_t height);

		void				Clear(uint32_t color = 0);

		int32_t					GetWidth() const;
		int32_t					GetHeight() const;

		uint32_t *				GetBasePointer();
		uint32_t *				GetLinePointer(int32_t line);

		void				PutImage(int32_t x, int32_t y, CImageC* pImage);
		void				FlipTopToBottom();

		void				SetFileName(const char* c_szFileName);

		const std::string &	GetFileNameString();

		bool				IsEmpty() const;

	protected:
		void				Initialize();

	protected:
		uint32_t *				m_pdwColors;
		int32_t					m_width;
		int32_t					m_height;

		std::string			m_stFileName;
};

#endif
