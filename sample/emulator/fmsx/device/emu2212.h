#ifndef _EMUSCC_H_
#define _EMUSCC_H_

#ifdef EMUSCC_DLL_EXPORTS
  #define EMUSCC_API __declspec(dllexport)
#elif  EMUSCC_DLL_IMPORTS
  #define EMUSCC_API __declspec(dllimport)
#else
  #define EMUSCC_API
#endif

#ifdef __cplusplus
extern "C" {
#endif
  
#include "emutypes.h"

#define SCC_CACHE_SIZE 1024 // 2^N only.

typedef struct {

  e_int32 out ;
  
  int masterVolume ;

  //e_int32 buf ;

  e_uint32 realstep ;
  e_uint32 scctime ;
  e_uint32 sccstep ;

  e_uint32 incr[5] ;

  e_uint8 save_9000 ;
  e_uint8 save_BFFE ;
  e_uint8 save_mode ;

  e_int8  wave[5][64] ;

  e_uint32 enable ;

  e_uint32 count[5] ;
  e_uint32 freq[5] ;
  e_uint32 phase[5] ;
  e_uint32 volume[5] ;
  e_uint32 offset[5] ;

  int ch_enable ;

  int cycle_4bit ;
  int cycle_8bit ;
  int refresh ;
  int rotate[5] ;

} SCC ;

EMUSCC_API void SCC_init(e_uint32 c, e_uint32 r);
EMUSCC_API void SCC_set_quality(e_uint32 q) ;

EMUSCC_API SCC *SCC_new(void) ;
EMUSCC_API void SCC_reset(SCC *scc) ;
EMUSCC_API void SCC_delete(SCC *scc) ;
EMUSCC_API void SCC_close() ;
EMUSCC_API e_int16 SCC_calc(SCC *scc) ;
EMUSCC_API e_int16 SCC_calcHQ(SCC *scc) ;
EMUSCC_API void SCC_write(SCC *scc, e_uint32 adr, e_uint32 val) ;
EMUSCC_API e_uint32 SCC_read(SCC *scc, e_uint32 adr) ;

#ifdef __cplusplus
}
#endif

#endif