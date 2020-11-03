/****************************************************************************

  emu2212.c -- S.C.C. emulator by Mitsutaka Okazaki 2001

  2001 09-30 : Version 1.00
  2001 10-03 : Version 1.01 -- Added SCC_set_quality().

*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "emu2212.h"

#if defined(_MSC_VER)
#define INLINE __forceinline
#elif defined(__GNUC__)
#define INLINE __inline__
#else
#define INLINE
#endif

#define GETA_BITS 22

static e_uint32 clk, rate ,base_incr, HQ ;

EMUSCC_API void SCC_init(e_uint32 c, e_uint32 r)
{
	clk = c ;
	rate = r ;
  SCC_set_quality(0) ;
}

EMUSCC_API void SCC_set_quality(e_uint32 q)
{
  if(q)
    base_incr = 2<<GETA_BITS ;
  else if(rate!=0)
    base_incr = (e_uint32)((double)clk*(1<<GETA_BITS)/rate) ;

  HQ = q ;
}

EMUSCC_API SCC *SCC_new(void)
{
  SCC *scc ;

	scc = malloc(sizeof(SCC)) ;
	if( scc == NULL ) return NULL ;

	return scc ;
}

EMUSCC_API void SCC_reset(SCC *scc)
{

  int i,j ;

  if(scc==NULL) return ;

  scc->save_9000 = 0x3f ;
  scc->save_BFFE = 0 ;
  scc->save_mode = 0 ;

  for(i=0;i<5;i++)
  {
    for(j=0;j<5;j++) scc->wave[i][j] = 0 ;
    scc->count[i] = 0 ;
    scc->freq[i] = 0 ;
    scc->phase[i] = 0 ;
    scc->volume[i] = 0 ;
    scc->offset[i] = 0 ;
    scc->rotate[i] = 0 ;
  }

  scc->enable = 1 ;
  scc->ch_enable = 0xff ;

  scc->cycle_4bit = 0 ;
  scc->cycle_8bit = 0 ;
  scc->refresh = 0 ;

  scc->out = 0 ;

	scc->realstep = (e_uint32)((1<<31)/rate) ;
	scc->sccstep =  (e_uint32)((1<<31)/(clk/2)) ;
	scc->scctime = 0 ;

  return ;

}


EMUSCC_API void SCC_delete(SCC *scc)
{
  if(scc!=NULL) free(scc) ;
}

EMUSCC_API void SCC_close()
{
}

INLINE static e_int16 calc(SCC *scc)
{
  int i ;
  e_int32 mix = 0 ;

  for(i=0;i<5;i++){

    if(!((scc->ch_enable>>i)&1)) continue ;

    scc->count[i] = (scc->count[i] + scc->incr[i]) ;

	  if(scc->count[i]&(1<<(GETA_BITS+5)))
    {
	    scc->count[i]&=((1<<(GETA_BITS+5))-1) ;
      scc->offset[i] = (scc->offset[i]+31)&scc->rotate[i] ;
	  }

    scc->phase[i] = ((scc->count[i]>>(GETA_BITS))+scc->offset[i])&0x1f ;
    mix += ((((e_int8)(scc->wave[i][scc->phase[i]]) * (e_int8)scc->volume[i]))) >> 4 ;
 
  }

  return (e_int16)(mix<<4) ; 
}

EMUSCC_API e_int16 SCC_calc(SCC *scc)
{
  if(!HQ) return calc(scc) ;
  /* Simple rate converter */
  while (scc->realstep > scc->scctime)
  {
    scc->scctime += scc->sccstep ;
    scc->out += calc(scc) ;
	  scc->out >>= 1 ;
  }

  scc->scctime = scc->scctime - scc->realstep ;
  
  return scc->out ; 
}

INLINE void check_enable(SCC *scc)
{
  if((scc->save_BFFE==0x20)&&(scc->save_9000 == 0x80)) scc->enable = 2 ;
  else if((scc->save_BFFE==0x00)&&(scc->save_9000 == 0x3F)) scc->enable = 1 ;
  else scc->enable = 0 ;
}

EMUSCC_API e_uint32 SCC_read(SCC *scc, e_uint32 adr)
{
  if((adr == 0xBFFE)||(adr == 0xBFFF)) return scc->save_BFFE ;
  if(adr == 0x9000) return scc->save_9000 ;

  if(scc->enable==0) return 0 ;

  if(((0x9800<=adr)&&(adr<0x9880))||((0xB800<=adr)&&(adr<0xB8A0)))
  {
    return scc->wave[(adr&0xe0)>>5][adr&0x1f] ;
  }
  else if (((0x9880<=adr)&&(adr<0x988A))||((0xB8A0<=adr)&&(adr<0xB8AA)))
  {
    if(adr&1) return scc->freq[(adr&0x0f)>>1]>>8 ;
    else return scc->freq[(adr&0x0f)>>1]&0xff ;
  }
  else if(((0x988A<=adr)&&(adr<0x988F))||((0xB8AA<=adr)&&(adr<0xB8AF)))
  {
    return scc->volume[(adr&0x0f)-0x0a] ;
  }
  else if((adr==0x988F)||(adr==0xB8AF))
  {
    return scc->ch_enable ;
  }
  else if(((0x98C0<=adr)&&(adr<0x98FF))||((0xB8C0<=adr)&&(adr<0xB8DF)))
  {
    return scc->save_mode ;
  }

  return 0 ;
}

EMUSCC_API void SCC_write(SCC *scc, e_uint32 adr, e_uint32 val)
{
  int ch ;
  e_uint32 freq ;

  val = val & 0xFF ;

  if((adr == 0xBFFE)||(adr == 0xBFFF))
  {
    scc->save_BFFE = (e_uint8)val ;
    check_enable(scc) ;
    return ;
  }
  
  if(adr == 0x9000)
  {
    scc->save_9000 = (e_uint8)val ;
    check_enable(scc) ;
    return ;
  }
  
  if(scc->enable==0) return ;

  if(((0x9800<=adr)&&(adr<0x9880))||((0xB800<=adr)&&(adr<0xB8A0)))
  {

    ch = (adr&0xe0)>>5 ;
    if(!scc->rotate[ch])
    {
      scc->wave[ch][adr&0x1f] = (e_int8)val ;
      if((scc->enable&1)&&(ch==3)) scc->wave[4][adr&0x1f] = (e_int8)val ;
    }

  }
  else if(((0x9880<=adr)&&(adr<0x988A))||((0xB8A0<=adr)&&(adr<0xB8AA)))
  {

    ch = (adr&0x0f)>>1 ;
    if(adr&1) scc->freq[ch] = ((val&0xf)<<8) | (scc->freq[ch]&0xff);
    else scc->freq[ch] = (scc->freq[ch]&0xf00) | (val&0xff) ;
    if(scc->refresh) scc->count[ch] = 0 ;
    freq = scc->freq[ch] ;
    if(scc->cycle_8bit) freq &= 0xff ;
    if(scc->cycle_4bit) freq >>= 8 ; 
    if(freq <= 8) scc->incr[ch] = 0 ; else scc->incr[ch] = base_incr/(freq+1) ;

  }
  else if(((0x988A<=adr)&&(adr<0x988F))||((0xB8AA<=adr)&&(adr<0xB8AF)))
  {

    ch = ((adr&0x0f)-0x0a) ;
    scc->volume[ch] = (e_uint8)(val&0xf) ;

  }
  else if((adr==0x988F)||(adr==0xB8AF))
  {

    scc->ch_enable = (e_uint8)val&31 ;

  }
  else if(((0x98C0<=adr)&&(adr<0x98FF))||((0xB8C0<=adr)&&(adr<0xB8DF)))
  {
    scc->save_mode = (e_uint8) val ;
    scc->cycle_4bit = val&1 ;
    scc->cycle_8bit = val&2 ;
    scc->refresh = val&32 ;
    if(val&64) for(ch=0;ch<5;ch++) scc->rotate[ch] = 0x1F ;
    else for(ch=0;ch<5;ch++) scc->rotate[ch] = 0 ;
    if(val&128) scc->rotate[3] = scc->rotate[4] = 0x1F ;
  }

  return ;
}
