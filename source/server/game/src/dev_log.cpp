#include "stdafx.h"
#include "config.h"
#include "dev_log.h"

#ifndef IS_SET
#	define IS_SET(flag,bit)		((flag) & (bit))
#endif	// IS_SET

#ifndef SET_BIT
#	define SET_BIT(var,bit)		((var) |= (bit))
#endif	// SET_BIT

#ifndef REMOVE_BIT
#	define REMOVE_BIT(var,bit)		((var) &= ~(bit))
#endif	// REMOVE_BIT

#ifndef TOGGLE_BIT
#	define TOGGLE_BIT(var,bit)		((var) = (var) ^ (bit))
#endif	// TOGGLE_BIT

static int32_t	s_log_mask = 0xffffffff;

void dev_log(const char *file, int32_t line, const char *function, int32_t level, const char *fmt, ...)
{
	if (!g_bIsTestServer || !IS_SET(s_log_mask, level))
		return;

	static char	buf[1024*1024];	// 1M
	int32_t			fd;
    char        strtime[64+1];
    const char        *strlevel;
    int32_t         mon, day, hour, min, sec;
    int32_t         nlen;
    va_list     args;
	struct tm curr_tm;
	time_t time_s;

	time_s = time(0);
	curr_tm = *localtime(&time_s);

	// ---------------------------------------
	// open file
	// ---------------------------------------
#ifndef _WIN32
	fd = ::open("DEV_LOG.log", O_WRONLY|O_APPEND|O_CREAT, 0666);
#else
	fd = ::_open("DEV_LOG.log", _O_WRONLY|_O_APPEND|_O_CREAT, 0666);
#endif

	if (fd < 0)
		return;

    // ---------------------------------------
    // set time string
    // ---------------------------------------
    mon         = curr_tm.tm_mon + 1;
    day         = curr_tm.tm_mday;
    hour        = curr_tm.tm_hour;
    min         = curr_tm.tm_min;
    sec         = curr_tm.tm_sec;

	nlen = snprintf(strtime, sizeof(strtime), "%02d%02d %02d:%02d.%02d", mon, day, hour, min, sec);

	if (nlen < 0 || nlen >= static_cast<int32_t>(sizeof(strtime)))
		nlen = sizeof(strtime) - 1;

	strtime[nlen - 2] = '\0';

    // ---------------------------------------
    // get level string
    // ---------------------------------------
#define GET_LEVEL_STRING(level) case L_##level : strlevel = #level; break
    switch ( level ) {
        GET_LEVEL_STRING ( WARN );
        GET_LEVEL_STRING ( ERR );
        GET_LEVEL_STRING ( CRIT );
        GET_LEVEL_STRING ( INFO );

        GET_LEVEL_STRING ( MIN );
        GET_LEVEL_STRING ( MAX );

        GET_LEVEL_STRING ( LIB0 );
        GET_LEVEL_STRING ( LIB1 );
        GET_LEVEL_STRING ( LIB2 );
        GET_LEVEL_STRING ( LIB3 );

        GET_LEVEL_STRING ( DEB0 );
        GET_LEVEL_STRING ( DEB1 );
        GET_LEVEL_STRING ( DEB2 );
        GET_LEVEL_STRING ( DEB3 );

        GET_LEVEL_STRING ( USR0 );
        GET_LEVEL_STRING ( USR1 );
        GET_LEVEL_STRING ( USR2 );
        GET_LEVEL_STRING ( USR3 );

        default : strlevel = "UNKNOWN"; break;
    }
#undef GET_LEVEL_STRING

	nlen = snprintf(buf, sizeof(buf), "%s %-4s (%-15s,%4d,%-24s) ",
			strtime, strlevel, file, line, function);

	if (nlen < 0 || nlen >= (int32_t) sizeof(buf))
		return;

	// ---------------------------------------
	// write_log
	// ---------------------------------------
	va_start(args, fmt);
	int32_t vlen = vsnprintf(buf + nlen, sizeof(buf) - (nlen + 2), fmt, args);
	va_end(args);

	if (vlen < 0 || vlen >= (int32_t) sizeof(buf) - (nlen + 2))
		nlen += (sizeof(buf) - (nlen + 2)) - 1;
	else
		nlen += vlen;

	buf[nlen++] = '\n';
	buf[nlen] = 0;

#ifndef _WIN32
	::write(fd, buf, nlen);
	::close(fd);
#else
	::_write(fd, buf, nlen);
	::_close(fd);
#endif
}

void dev_log_add_level(int32_t level)
{
	SET_BIT(s_log_mask, level);
}

void dev_log_del_level(int32_t level)
{
	REMOVE_BIT(s_log_mask, level);
}

void dev_log_set_level(int32_t mask)
{
	s_log_mask = mask;
}

