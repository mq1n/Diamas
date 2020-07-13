#ifndef __INC_ETER2_ETERBASE_UTILS_H__
#define __INC_ETER2_ETERBASE_UTILS_H__

#include <Windows.h>
#include <cstdint>
#include <vector>
#include <string>
#define _USE_MATH_DEFINES
#include <math.h>

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)     \
		{                      \
			if (p)             \
			{                  \
				delete (p);    \
				(p) = nullptr; \
			}                  \
		}
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) \
		{                        \
			if (p)               \
			{                    \
				delete[](p);     \
				(p) = nullptr;   \
			}                    \
		}
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)     \
		{                       \
			if (p)              \
			{                   \
				(p)->Release(); \
				(p) = nullptr;  \
			}                   \
		}
#endif

#ifndef SAFE_FREE_GLOBAL
#define SAFE_FREE_GLOBAL(p)  \
		{                        \
			if (p)               \
			{                    \
				::GlobalFree(p); \
				(p) = nullptr;   \
			}                    \
		}
#endif

#ifndef SAFE_FREE_LIBRARY
#define SAFE_FREE_LIBRARY(p)  \
		{                         \
			if (p)                \
			{                     \
				::FreeLibrary(p); \
				(p) = nullptr;    \
			}                     \
		}
#endif

#define AssertLog(str) \
	TraceError(str);   \
	assert(!(str))

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                                      \
		((uint32_t)(uint8_t)(ch0) | ((uint32_t)(uint8_t)(ch1) << 8) | \
		((uint32_t)(uint8_t)(ch2) << 16) | ((uint32_t)(uint8_t)(ch3) << 24))
#endif // defined(MAKEFOURCC)

#ifndef IS_SET
#define IS_SET(flag,bit)                ((flag) & (bit))
#endif

#ifndef IS_SET_SINGLE_BIT
#define IS_SET_SINGLE_BIT(flag, bit)	(flag & (1 << bit)) != 0
#endif

#ifndef SET_BIT
#define SET_BIT(var,bit)                ((var) |= (bit))
#endif

#ifndef REMOVE_BIT
#define REMOVE_BIT(var,bit)             ((var) &= ~(bit))
#endif

#ifndef TOGGLE_BIT
#define TOGGLE_BIT(var,bit)             ((var) = (var) ^ (bit))
#endif

extern const char * CreateTempFileName(const char * c_pszPrefix = nullptr);
extern void			GetFilePathNameExtension(const char* c_szFile, int32_t len, std::string* pstPath, std::string* pstName, std::string* pstExt);
extern void			GetFileExtension(const char* c_szFile, int32_t len, std::string* pstExt);
extern void			GetFileNameParts(const char* c_szFile, int32_t len, char* pszPath, char* pszName, char* pszExt);
extern void			GetOldIndexingName(char * szName, int32_t Index);
extern void			GetIndexingName(char * szName, uint32_t Index);
extern void			GetOnlyFileName(const char * sz_Name, std::string & strFileName);
extern void			GetOnlyPathName(const char * sz_Name, std::string & OnlyPathName);
extern const char *	GetOnlyPathName(const char * c_szName);
bool				GetLocalFileName(const char * c_szGlobalPath, const char * c_szFullPathFileName, std::string * pstrLocalFileName);
extern void			GetExceptionPathName(const char * sz_Name, std::string & OnlyFileName);
extern void			GetWorkingFolder(std::string & strFileName);
extern void			StringLowers(char * pString);
extern void			StringPath(std::string & rString);
extern void			StringPath(char * pString);	// 모두 소문자로 만들고, \는 /로 바꾼다.
extern void			StringPath(const char * c_szSrc, char * szDest);	// 모두 소문자로 만들고, \는 /로 바꾼다.
extern void			StringPath(const char * c_szSrc, std::string & rString);	// 모두 소문자로 만들고, \는 /로 바꾼다.
extern void			PrintAsciiData(const void* data, int32_t bytes);
bool				IsFile(const char* filename);
bool				IsGlobalFileName(const char * c_szFileName);
int32_t					MIN(int32_t a, int32_t b);
int32_t					MAX(int32_t a, int32_t b);
int32_t					MINMAX(int32_t min, int32_t value, int32_t max);
float				fMIN(float a, float b);
float				fMAX(float a, float b);
float				fMINMAX(float min, float value, float max);
void				MyCreateDirectory(const char* path);
void				RemoveAllDirectory(const char * c_szDirectoryName);
bool				SplitLine(const char * c_szLine, const char * c_szDelimeter, std::vector<std::string> * pkVec_strToken);
const char *		_getf(const char* c_szFormat, ...);
PCHAR*				CommandLineToArgv( PCHAR CmdLine, int32_t* _argc );

template<typename T>
T EL_DegreeToRadian(T degree)
{
	const T PI = T(M_PI);
	return T(PI*degree/180.0f);
}

template<typename T>
void ELPlainCoord_GetRotatedPixelPosition(T centerX, T centerY, T distance, T rotDegree, T* pdstX, T* pdstY)
{	
	T rotRadian=EL_DegreeToRadian(rotDegree);
	*pdstX = centerX + distance * T(sin(static_cast<double>(rotRadian)));
	*pdstY = centerY + distance * T(cos(static_cast<double>(rotRadian)));
}

template<typename T>
T EL_SignedDegreeToUnsignedDegree(T fSrc)
{
	if (fSrc<0.0f)
		return T(360.0+T(fmod(fSrc, 360.0)));
	
	return T(fmod(fSrc, 360.0));
}

template<typename T>
T ELRightCoord_ConvertToPlainCoordDegree(T srcDegree)
{
	return T(fmod(450.0 - srcDegree, 360.0));
}

template<typename C>
void string_join(const std::string& sep, const C& container, std::string* ret)
{
	uint32_t capacity = sep.length() * container.size() - 1;
    
    // calculate string sequence
    {
        for (auto i = container.begin(); i != container.end(); ++i)
            capacity += (*i).length();
    }

    std::string buf;
    buf.reserve(capacity);

    // join strings
    {
        auto cur = container.begin();
        auto end = container.end();
        --end;

        while (cur != end)
        {
            buf.append(*cur++);
            buf.append(sep);
        }
        buf.append(*cur);
    }

    swap(*ret, buf);
}

__forceinline int32_t htoi(const wchar_t *s, int32_t size)
{
    const wchar_t *t = s;
    int32_t x = 0, y = 1;
    s += size;

    while (t <= --s)
    {
		if (L'0' <= *s && *s <= L'9')
            x += y * (*s - L'0');
        else if (L'a' <= *s && *s <= L'f')
            x += y * (*s - L'a' + 10);
        else if (L'A' <= *s && *s <= L'F')
            x += y * (10 + *s - L'A');
        else
            return -1; /* invalid input! */
        y <<= 4;
    }

    return x;
}

__forceinline int32_t htoi(const char *s, int32_t size)
{
    const char *t = s;
    int32_t x = 0, y = 1;
    s += size;

    while (t <= --s)
    {
		if ('0' <= *s && *s <= '9')
            x += y * (*s - '0');
        else if ('a' <= *s && *s <= 'f')
            x += y * (*s - 'a' + 10);
        else if ('A' <= *s && *s <= 'F')
            x += y * (10 + *s - 'A');
        else
            return -1; /* invalid input! */
        y <<= 4;
    }

    return x;
}

__forceinline int32_t htoi(const char *s)
{
    const char *t = s;
    int32_t x = 0, y = 1;
    s += strlen(s);

    while (t <= --s)
    {
		if ('0' <= *s && *s <= '9')
            x += y * (*s - '0');
        else if ('a' <= *s && *s <= 'f')
            x += y * (*s - 'a' + 10);
        else if ('A' <= *s && *s <= 'F')
            x += y * (10 + *s - 'A');
        else
            return -1; /* invalid input! */
        y <<= 4;
    }

    return x;
}

typedef std::vector<std::string> TTokenVector;

void StringExceptCharacter(std::string * pstrString, const char * c_szCharacter);

extern void GetExcutedFileName(std::string & r_str);

struct ci_less
{
	// case-independent (ci) compare_less binary function
	struct nocase_compare
	{
		bool operator() (const unsigned char& c1, const unsigned char& c2) const {
			return tolower(c1) < tolower(c2);
		}
	};
	bool operator() (const std::string& s1, const std::string& s2) const {
		return std::lexicographical_compare
		(s1.begin(), s1.end(),   // source range
			s2.begin(), s2.end(),   // dest range
			nocase_compare());  // comparison
	}
};

#endif