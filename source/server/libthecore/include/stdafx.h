#pragma once

#if defined(__GNUC__)
#define INLINE __inline__
#elif defined(_MSC_VER)
#define INLINE inline
#endif

#include <random>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <thread>
#include <chrono>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <tchar.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <conio.h>
#include <process.h>
#include <limits.h>
#include <math.h>
#include <locale.h>
#include <io.h>
#include <direct.h>
#include <fcntl.h>

#define S_ISDIR(m)	(m & _S_IFDIR)
#define snprintf _snprintf

#define __USE_SELECT__

#define PATH_MAX _MAX_PATH

// C runtime library adjustments
#define strlcat(dst, src, size) strcat_s(dst, size, src)
#define strlcpy(dst, src, size) strncpy_s(dst, size, src, _TRUNCATE)
#define strtoull(str, endptr, base) _strtoui64(str, endptr, base)
#define strcasecmp(s1, s2) _stricmp(s1, s2)
#define strncasecmp(s1, s2, n) _strnicmp(s1, s2, n)
#define atoll(str) _atoi64(str)
#define localtime_r(timet, result) localtime_s(result, timet)
#define strtok_r(s, delim, ptrptr) strtok_s(s, delim, ptrptr)

// dummy declaration of non-supported signals
#define SIGUSR1     30  /* user defined signal 1 */
#define SIGUSR2     31  /* user defined signal 2 */

inline void usleep(unsigned long usec) {
	::Sleep(usec / 1000);
}
inline unsigned sleep(unsigned sec) {
	::Sleep(sec * 1000);
	return 0;
}


#else

#ifndef __FreeBSD__
#define __USE_SELECT__
#ifdef __CYGWIN__
#define _POSIX_SOURCE 1
#endif
#endif

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <dirent.h>

#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>

#include <sys/signal.h>
#include <sys/wait.h>

#include <pthread.h>
#include <semaphore.h>

#ifdef __FreeBSD__
#include <sys/event.h>
#endif

#endif

#ifdef _WIN32
#define isdigit iswdigit
#define isspace iswspace
#define isalpha iswalpha
#endif

#define ishan(ch)       (((ch) & 0xE0) > 0x90)
#define ishanasc(ch)    (isascii(ch) || ishan(ch))
#define ishanalp(ch)    (isalpha(ch) || ishan(ch))
#define isnhdigit(ch)   (!ishan(ch) && isdigit(ch))
#define isnhspace(ch)   (!ishan(ch) && isspace(ch))

#include "typedef.h"
#include "heart.h"
#include "fdwatch.h"
#include "socket.h"
#include "buffer.h"
#include "signal.h"
#include "log.h"
#include "main.h"
#include "utils.h"
#include "crypt.h"
#include "random.h"
#include "../../../common/file_ptr.h"
