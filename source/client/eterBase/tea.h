#pragma once
#include <cstdint>
#ifdef __cplusplus
extern "C"
{
#endif

	/* TEA is a 64-bit symmetric block cipher with a 128-bit key, developed
   by David J. Wheeler and Roger M. Needham, and described in their
   paper at <URL:http://www.cl.cam.ac.uk/ftp/users/djw3/tea.ps>.

   This implementation is based on their code in
   <URL:http://www.cl.cam.ac.uk/ftp/users/djw3/xtea.ps> */

#define TEA_KEY_LENGTH 16
	int32_t tea_encrypt(uint32_t * dest, const uint32_t * src, const uint32_t * key, int32_t size);
	int32_t tea_decrypt(uint32_t * dest, const uint32_t * src, const uint32_t * key, int32_t size);

#ifdef __cplusplus
}
#endif
