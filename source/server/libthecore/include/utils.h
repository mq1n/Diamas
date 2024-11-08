#ifndef __INC_LIBTHECORE_UTILS_H__
#define __INC_LIBTHECORE_UTILS_H__

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define SAFE_FREE(p)		{ if (p) { free( (void *) p);		(p) = NULL;  } }
#define SAFE_DELETE(p)		{ if (p) { delete (p);			(p) = NULL;  } }
#define SAFE_DELETE_ARRAY(p)	{ if (p) { delete[] (p);		(p) = NULL;  } }
#define SAFE_RELEASE(p)		{ if (p) { (p)->Release();		(p) = NULL;  } }

#define LOWER(c)	(((c)>='A'  && (c) <= 'Z') ? ((c)+('a'-'A')) : (c))
#define UPPER(c)	(((c)>='a'  && (c) <= 'z') ? ((c)+('A'-'a')) : (c))

#define STRNCPY(dst, src, len)          do {strncpy(dst, src, len); dst[len] = '\0'; } while(0)

    extern char *	str_dup(const char * source);	// 메모리 할당 해서 source 복사 한거 리턴
    extern void		printdata(const unsigned char * data, int bytes); // data를 hex랑 ascii로 출력 (패킷 분석 등에 쓰임)

#define core_dump()	core_dump_ex(__FILE__, __LINE__, __FUNCTION__)
    extern void		core_dump_ex(const char *who, long line, char const* function);	// 肄붿뼱瑜� 媛뺤젣濡� �옢�봽

#define TOKEN(string) if (!strcasecmp(token_string, string))
    // src = 토큰 : 값
    extern void		parse_token(char * src, char * token, char * value);

    extern void		trim_and_lower(const char * src, char * dest, size_t dest_size);

    // 문자열을 소문자로
    extern void		lower_string(const char * src, char * dest, size_t dest_len);

    // arg1이 arg2로 시작하는가? (대소문자 구별하지 않음)
    extern int		is_abbrev(char *arg1, char *arg2);

    // a와 b의 시간이 얼마나 차이나는지 리턴
    extern struct timeval *	timediff(const struct timeval *a, const struct timeval *b);

    // a의 시간에 b의 시간을 더해 리턴
    extern struct timeval *	timeadd(struct timeval *a, struct timeval *b);

    // 현재 시간 curr_tm으로 부터 days가 지난 날을 리턴
    extern struct tm *		tm_calc(const struct tm *curr_tm, int days);

    extern int MAX(int a, int b);
    extern int MIN(int a, int b); 
    extern int MINMAX(int min, int value, int max);

	float	fnumber(float from, float to);

    extern void		thecore_sleep(struct timeval * timeout);	// timeout만큼 프로세스 쉬기
    extern uint32_t	thecore_random();				// 랜덤 함수

    extern float	get_float_time();
    extern uint32_t	get_unix_ms_time();

    extern char *	time_str(time_t ct);

	// memory
#define CREATE(result, type, number)  do { \
	if (!((result) = (type *) calloc ((number), sizeof(type)))) \
	{ perror("malloc failure"); abort(); } } while (0)

#define RECREATE(result, type, number) do { \
	if (!((result) = (type *) realloc ((result), sizeof(type) * (number))))\
	{ perror("realloc failure"); abort(); } } while (0)


    // Next 와 Prev 가 있는 리스트에 추가
#define INSERT_TO_TW_LIST(item, head, prev, next)   \
    if (!(head))                                    \
    {                                               \
	head         = item;                        \
	    (head)->prev = (head)->next = NULL;         \
    }                                               \
    else                                            \
    {                                               \
	(head)->prev = item;                        \
	    (item)->next = head;                    \
	    (item)->prev = NULL;                    \
	    head         = item;                    \
    }

#define REMOVE_FROM_TW_LIST(item, head, prev, next)	\
    if ((item) == (head))           			\
    {                               			\
	if (((head) = (item)->next))			\
	    (head)->prev = NULL;    			\
    }                    				\
    else                 				\
    {                    				\
	if ((item)->next)				\
	    (item)->next->prev = (item)->prev;		\
							\
	if ((item)->prev)				\
	    (item)->prev->next = (item)->next;		\
    }


#define INSERT_TO_LIST(item, head, next)            \
    (item)->next = (head);                      \
	(head) = (item);                            \

#define REMOVE_FROM_LIST(item, head, next)          \
	if ((item) == (head))                       \
	    head = (item)->next;                     \
	else                                        \
	{                                           \
	    temp = head;                            \
		\
		while (temp && (temp->next != (item)))  \
		    temp = temp->next;                  \
			\
			if (temp)                               \
			    temp->next = (item)->next;          \
	}                                           \

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                                      \
                ((uint32_t)(uint8_t) (ch0       ) | ((uint32_t)(uint8_t) (ch1) <<  8) | \
                 ((uint32_t)(uint8_t) (ch2) << 16) | ((uint32_t)(uint8_t) (ch3) << 24))
#endif // defined(MAKEFOURCC)

#ifdef __cplusplus
}
#endif	// __cplusplus

extern int		number_ex(int from, int to, const char *file, int line); // from�쑝濡� 遺��꽣 to源뚯���옒 �옖�옢 媛� 由ы꽩
#define number(from, to) number_ex(from, to, __FILE__, __LINE__)

// _countof for gcc/g++
#if !defined(_countof)
#if !defined(__cplusplus)
#define _countof(_Array) (sizeof(_Array) / sizeof(_Array[0]))
#else
extern "C++"
{
	    template <typename _CountofType, size_t _SizeOfArray>
			        char (*__countof_helper(_CountofType (&_Array)[_SizeOfArray]))[_SizeOfArray];
#define _countof(_Array) sizeof(*__countof_helper(_Array))
}
#endif
#endif

#ifdef _WIN32
extern void gettimeofday(struct timeval* t, struct timezone* dummy);
#endif

#include <string>
#include <memory>

template<typename ... Args>
std::string string_format(const std::string& format, Args ... args)
{
	std::size_t size = snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
	std::unique_ptr<char[]> buf(new char[size]);
	snprintf(buf.get(), size, format.c_str(), args ...);
	return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

#endif	// __INC_UTILS_H__
