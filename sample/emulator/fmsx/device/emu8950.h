#ifndef _EMU8950_H_
#define _EMU8950_H_

#include "emuadpcm.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef EMU8950_DLL_EXPORTS
  #define EMU8950_API __declspec(dllexport)
#elif  EMU8950_DLL_IMPORTS
  #define EMU8950_API __declspec(dllimport)
#else
  #define EMU8950_API
#endif

#define PI 3.14159265358979

#include "emutypes.h"

/* voice data */
typedef struct {
  e_uint32 TL,FB,EG,ML,AR,DR,SL,RR,KR,KL,AM,PM,WF ;
} OPL_PATCH ;

/* slot */
typedef struct {

  e_int32 type ;          /* 0 : modulator 1 : carrier */

  /* OUTPUT */
  e_int32 feedback ;
  e_int32 output[5] ;      /* Output value of slot */

  /* for Phase Generator (PG) */
  e_uint32 *sintbl ;    /* Wavetable */
  e_uint32 phase ;      /* Phase */
  e_uint32 dphase ;     /* Phase increment amount */
  e_uint32 pgout ;      /* output */

  /* for Envelope Generator (EG) */
  e_int32 fnum ;          /* F-Number */
  e_int32 block ;         /* Block */
  e_uint32 tll ;	      /* Total Level + Key scale level*/
  e_uint32 rks ;        /* Key scale offset (Rks) */
  e_int32 eg_mode ;       /* Current state */
  e_uint32 eg_phase ;   /* Phase */
  e_uint32 eg_dphase ;  /* Phase increment amount */
  e_uint32 egout ;      /* output */

  /* LFO (refer to OPL->*) */
  e_int32 *plfo_am ;
  e_int32 *plfo_pm ;

  OPL_PATCH *patch;  

} OPL_SLOT ;

/* Channel */
typedef struct {

  e_int32 key_status ;
  e_int32 alg ;
  OPL_SLOT *mod, *car ;

} OPL_CH ;

/* OPL */
typedef struct {

  ADPCM *adpcm;

  e_uint32 adr ;

  e_int32 output[2] ;

  /* Register */
  unsigned char reg[0xff] ; 
  e_int32 slot_on_flag[18] ;

  /* Rythm Mode : 0 = OFF, 1 = ON */
  e_int32 rythm_mode ;

  /* Pitch Modulator */
  e_int32 pm_mode ;
  e_uint32 pm_phase ;

  /* Amp Modulator */
  e_int32 am_mode ;
  e_uint32 am_phase ;

  /* Noise Generator */
  e_uint32 noise_seed ;

  /* Channel & Slot */
  OPL_CH *ch[9] ;
  OPL_SLOT *slot[18] ;

  e_int32 mask[10] ; /* mask[9] = RYTHM */

} OPL ;

EMU8950_API void OPL_init(e_uint32 clk, e_uint32 rate) ;
EMU8950_API void OPL_close(void) ;
EMU8950_API OPL *OPL_new(void) ;
EMU8950_API void OPL_reset(OPL *opl) ;
EMU8950_API void OPL_delete(OPL *opl) ;
EMU8950_API void OPL_writeReg(OPL *opl, e_uint32 reg, e_uint32 val) ;
EMU8950_API e_int16 OPL_calc(OPL *opl) ;
EMU8950_API void OPL_writeIO(OPL *opl, e_uint32 adr, e_uint32 val) ;
EMU8950_API e_uint32 OPL_readIO(OPL *opl) ;
EMU8950_API e_uint32 OPL_status(OPL *opl) ;
#ifdef __cplusplus
}
#endif



#endif











