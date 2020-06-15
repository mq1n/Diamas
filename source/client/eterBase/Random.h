#ifndef __INC_ETERBASE_RANDOM_H__
#define __INC_ETERBASE_RANDOM_H__

extern void				srandom(uint32_t seed);
extern uint32_t	random();
extern float			frandom(float flLow, float flHigh);
extern int32_t				random_range(int32_t from, int32_t to);
extern LPSTR GetRandomStringA(__in uint32_t uMin);

#endif