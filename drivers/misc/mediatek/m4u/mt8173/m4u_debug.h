#ifndef __M4U_DEBUG_H__
#define __M4U_DEBUG_H__

extern __attribute__((weak)) int ddp_mem_test(void);
extern __attribute__((weak)) int __ddp_mem_test(unsigned int *pSrc, unsigned int pSrcPa,
			    unsigned int *pDst, unsigned int pDstPa,
			    int need_sync);
#endif
