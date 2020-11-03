/****************************************************************************

  emu76489.c -- SN76489 emulator by Mitsutaka Okazaki 2001

  2001 08-13 : Version 1.00
  2001 10-03 : Version 1.01 -- Added SNG_set_quality().

  References: 
    SN76489 data sheet   
    sn76489.c            -- in MAME

*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "emu76489.h"

#if defined(_MSC_VER)
#define INLINE __forceinline
#elif defined(__GNUC__)
#define INLINE __inline__
#else
#define INLINE
#endif

static e_uint32 voltbl[16]=
{
  0xff,0xcb,0xa1,0x80,0x65,0x50,0x40,0x33,0x28,0x20,0x19,0x14,0x10,0x0c,0x0a,0x00
} ;

#define GETA_BITS 24
static e_uint32 clk, rate, base_incr, HQ ;

EMU76489_API void SNG_init(e_uint32 c, e_uint32 r)
{
	clk = c ;
	rate = r ;
  SNG_set_quality(0) ;
}

EMU76489_API void SNG_set_quality(e_uint32 q)
{
  if(q)
    base_incr = 1<<GETA_BITS ;
  else if(rate!=0)
    base_incr = (e_uint32)((double)clk * (1<<GETA_BITS) / (16*rate) ) ;

  HQ = q ;
}

EMU76489_API SNG *SNG_new()
{
	SNG *sng ;

	sng = malloc(sizeof(SNG)) ;
	if( sng == NULL ) return NULL ;

  return sng ;
}

EMU76489_API void SNG_reset(SNG *sng)
{
	int i ;

  sng->base_count = 0 ;

	for(i=0;i<3;i++){
		sng->count[i] = 0 ;
		sng->freq[i] = 0 ;
		sng->edge[i] = 0 ;
		sng->volume[i] = 0x0f ;
    sng->mute[i] = 0 ;
	}

  sng->adr = 0 ;

	sng->realstep = (e_uint32)((1<<31)/rate) ;
	sng->sngstep =  (e_uint32)((1<<31)/(clk/16)) ;
	sng->sngtime = 0 ;

	sng->noise_seed = 0xffff ;
	sng->noise_count = 0 ;
	sng->noise_freq = 0 ;
  sng->noise_volume = 0x0f ;
  sng->noise_mode = 0 ;

	sng->out = 0 ;
}

EMU76489_API void SNG_delete(SNG *sng)
{
	free(sng) ;
}

EMU76489_API void SNG_close()
{
}

EMU76489_API void SNG_writeIO(SNG *sng, e_uint32 val)
{
  if(val&0x80)
  {
    sng->adr = (val&0x70) >> 4 ;
    switch(sng->adr)
    {
	  case 0: case 2: case 4:
		  sng->freq[sng->adr>>1] = (sng->freq[sng->adr>>1]&0x3F0) | (val&0x0F) ;
 		  break ;

    case 1: case 3: case 5:
      sng->volume[(sng->adr-1)>>1] = val & 0xF ;
      break ;

    case 6:
      sng->noise_mode = (val&4)>>2 ;

      if((val&0x03)==0x03)
        sng->noise_freq = sng->freq[2] ;
      else
        sng->noise_freq = 32 << (val&0x03) ;

      if(sng->noise_freq == 0) sng->noise_freq = 1 ;

      sng->noise_seed = 0xf35 ;
      break ;

    case 7:
      sng->noise_volume = val & 0x0f ;
      break ;
    }
  }
  else
  {
    sng->freq[sng->adr>>1] = ((val&0x3F)<<4) | (sng->freq[sng->adr>>1]&0x0F) ;
  }
}

INLINE static e_int16 calc(SNG *sng)
{

  int i ;
  e_uint32 incr ;
  e_int16 mix = 0 ;

  sng->base_count += base_incr ;
  incr = (sng->base_count >> GETA_BITS) ;
  sng->base_count &= (1 << GETA_BITS) - 1 ;

	/* Noise */
  sng->noise_count += incr ;
	if(sng->noise_count&0x100)
  {
		if(sng->noise_seed&1)
    {
      if(sng->noise_mode)
        sng->noise_seed^=0x12000 ; /* White */
      else
        sng->noise_seed^=0x08000 ; /* Periodic */
    }
    sng->noise_seed >>= 1 ;
		sng->noise_count -= sng->noise_freq ;
	}

	if(sng->noise_seed&1)
    mix = voltbl[sng->noise_volume] ;

	/* Tone */
	for(i=0;i<3;i++)
  {
    sng->count[i] += incr ;
		if(sng->count[i]&0x400)
    {
       if(sng->freq[i]>1)
       {
         sng->edge[i] = !sng->edge[i] ;
         sng->count[i] -= sng->freq[i] ;
       }
       else
       {
         sng->edge[i] = 1 ;
       }
    }

  	if(sng->edge[i]&&!sng->mute[i])
    {
      mix += voltbl[sng->volume[i]] ;
    }
	}

	return mix ;

}

EMU76489_API e_int16 SNG_calc(SNG *sng)
{
  if(!HQ) return calc(sng) << 4 ;
  /* Simple rate converter */
  while (sng->realstep > sng->sngtime)
  {
    sng->sngtime += sng->sngstep ;
    sng->out += calc(sng) ;
	  sng->out >>= 1 ;
  }

  sng->sngtime = sng->sngtime - sng->realstep ;
  
  return (e_int16)( sng->out << 4 ) ; 
}