/** fMSX: portable MSX emulator ******************************/
/**                                                         **/
/**                         Hires.h                         **/
/**                                                         **/
/** This file contains screen drivers for >512 width        **/
/** displays.                                               **/
/**                                                         **/
/** Copyright (C) Vincent van Dam 2001                      **/
/**               Marat Fayzullin 1994-2001                 **/
/**               John Stiles     1996                      **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/

static int HiresFirstLine = 14;     /* First scanline in the XBuf */

static void  HiresSprites(byte Y,register pixel *Line);
static void  HiresColorSprites(byte Y,byte *ZBuf);
static pixel *HiresRefreshBorder(byte Y,pixel C);
static void  HiresClearLine(pixel *P,pixel C);
static pixel HiresYJKColor(int Y,int J,int K);

#ifndef NARROW
/** RefreshScreen() ******************************************/
/** Refresh screen. This function is called in the end of   **/
/** refresh cycle to show the entire screen.                **/
/*************************************************************/
void RefreshScreen(void) { PutImage(); }
#endif

/** ClearLine() **********************************************/
/** Clear 256 pixels from P with color C.                   **/
/*************************************************************/
static void HiresClearLine(register pixel *P,register pixel C)
{
  register int J;
  for(J=0;J<512;J++) P[J]=C;
}

/** YJKColor() ***********************************************/
/** Given a color in YJK format, return the corresponding   **/
/** palette entry.                                          **/
/*************************************************************/
static pixel HiresYJKColor(register int Y,register int J,register int K)
{
  register int R,G,B;
		
  R=Y+J;
  G=Y+K;
  B=((5*Y-2*J-K)/4);

  R=R<0? 0:R>31? 31:R;
  G=G<0? 0:G>31? 31:G;
  B=B<0? 0:B>31? 31:B;

  return(BPal[(R&0x1C)|((G&0x1C)<<3)|(B>>3)]);
}

/** RefreshBorder() ******************************************/
/** This function is called from RefreshLine#() to refresh  **/
/** the screen border. It returns a pointer to the start of **/
/** scanline Y in XBuf or 0 if scanline is beyond XBuf.     **/
/*************************************************************/
pixel *HiresRefreshBorder(register byte Y,register pixel C)
{
  register pixel *P;
  register int H,I;

  /* First line number in the buffer */
  if(!Y) HiresFirstLine=(ScanLines212? 14:22)+VAdjust;

  /* Return 0 if we've run out of the screen buffer due to overscan */
  if( (Y+HiresFirstLine)*2 >= HEIGHT ) return(0);

  /* Set up the transparent color */
  XPal[0]=(!BGColor||SolidColor0)? XPal0:XPal[BGColor];

  /* Start of the buffer */
  P=(pixel *)XBuf;

  /* Paint top of the screen */
  if(!Y) for(H=WIDTH*((HiresFirstLine-1)*2);H>=0;H=H-WIDTH*2) for(I=0;I<WIDTH;I++) P[H+I]=C;

  /* Start of the line */
  P+=WIDTH*((Y+HiresFirstLine)*2);

  /* Paint left/right borders */
  for(H=(WIDTH-512)/2+(HAdjust*2);H>0;H--) P[H-1]=C;
  for(H=(WIDTH-512)/2-(HAdjust*2);H>0;H--) P[WIDTH-H]=C;

  /* Paint bottom of the screen */
  H=ScanLines212? 212:192;
  if(Y==H-1) for(H=WIDTH*((HEIGHT/2-HiresFirstLine-H)*2);H>=0;H=H-WIDTH*2) for(I=0;I<WIDTH;I++) P[H+I]=C;

  /* Return pointer to the scanline in XBuf */
  return(P+(WIDTH-512)/2+(HAdjust*2));
}

/** Sprites() ************************************************/
/** This function is called from RefreshLine#() to refresh  **/
/** sprites in SCREENs 1-3.                                 **/
/*************************************************************/
void HiresSprites(register byte Y,register pixel *Line)
{
  register pixel *P,C;
  register byte H,*PT,*AT;
  register unsigned int M;
  register int L,K;

  /* Assign initial values before counting */
  H=Sprites16x16? 16:8;
  C=0;M=0;L=0;
  AT=SprTab-4;
  Y+=VScroll;

  /* Count displayed sprites */
  do
  {
    M<<=1;AT+=4;L++;    /* Iterating through SprTab      */
    K=AT[0];            /* K = sprite Y coordinate       */
    if(K==208) break;   /* Iteration terminates if Y=208 */
    if(K>256-H) K-=256; /* Y coordinate may be negative  */

    /* Mark all valid sprites with 1s, break at MAXSPRITE1 sprites */
    if((Y>K)&&(Y<=K+H)) { M|=1;C++;if(C==MAXSPRITE1) break; }
  }
  while(L<32);

  /* Draw all marked sprites */
  for(;M;M>>=1,AT-=4)
    if(M&1)
    {
      C=AT[3];                  /* C = sprite attributes */
      L=C&0x80? AT[1]-32:AT[1]; /* Sprite may be shifted left by 32 */
      C&=0x0F;                  /* C = sprite color */

      if((L<256)&&(L>-H)&&C)
      {
        K=AT[0];                /* K = sprite Y coordinate */
        if(K>256-H) K-=256;     /* Y coordinate may be negative */

        P=Line+L*2;
        PT=SprGen+((int)(H>8? AT[2]&0xFC:AT[2])<<3)+Y-K-1;
        C=XPal[C];

        /* Mask 1: clip left sprite boundary */
        K=L>=0? 0x0FFFF:(0x10000>>-L)-1;
        /* Mask 2: clip right sprite boundary */
        if(L>256-H) K^=((0x00200>>(H-8))<<(L-257+H))-1;
        /* Get and clip the sprite data */
        K&=((int)PT[0]<<8)|(H>8? PT[16]:0x00);

        /* Draw left 8 pixels of the sprite */
        if(K&0xFF00)
        {
          if(K&0x8000) P[0]=P[1]=C;if(K&0x4000) P[2]=P[3]=C;
          if(K&0x2000) P[4]=P[5]=C;if(K&0x1000) P[6]=P[7]=C;
          if(K&0x0800) P[8]=P[9]=C;if(K&0x0400) P[10]=P[11]=C;
          if(K&0x0200) P[12]=P[13]=C;if(K&0x0100) P[14]=P[15]=C;
        }

        /* Draw right 8 pixels of the sprite */
        if(K&0x00FF)
        {
          if(K&0x0080) P[16]=P[17]=C;if(K&0x0040) P[18]=P[19]=C;
          if(K&0x0020) P[20]=P[21]=C;if(K&0x0010) P[22]=P[23]=C;
          if(K&0x0008) P[24]=P[25]=C;if(K&0x0004) P[26]=P[27]=C;
          if(K&0x0002) P[28]=P[29]=C;if(K&0x0001) P[30]=P[31]=C;
        }
      }
    }
}

/** ColorSprites() *******************************************/
/** This function is called from RefreshLine#() to refresh  **/
/** color sprites in SCREENs 4-8. The result is returned in **/
/** ZBuf, whose size must be 304 bytes (32+256+16).         **/
/*************************************************************/
void HiresColorSprites(register byte Y,byte *ZBuf)
{
  register byte C,H,J,OrThem;
  register byte *P,*PT,*AT;
  register int L,K;
  register unsigned int M;

  /* Clear ZBuffer and exit if sprites are off */
  memset(ZBuf+32,0,256);
  if(SpritesOFF) return;

  /* Assign initial values before counting */
  H=Sprites16x16? 16:8;
  C=0;M=0;L=0;
  AT=SprTab-4;
  OrThem=0x00;

  /* Count displayed sprites */
  do
  {
    M<<=1;AT+=4;L++;          /* Iterating through SprTab      */
    K=AT[0];                  /* Read Y from SprTab            */
    if(K==216) break;         /* Iteration terminates if Y=216 */
    K=(byte)(K-VScroll);      /* Sprite's actual Y coordinate  */
    if(K>256-H) K-=256;       /* Y coordinate may be negative  */

    /* Mark all valid sprites with 1s, break at MAXSPRITE2 sprites */
    if((Y>K)&&(Y<=K+H)) { M|=1;C++;if(C==MAXSPRITE2) break; }
  }
  while(L<32);

  /* Draw all marked sprites */
  for(;M;M>>=1,AT-=4)
    if(M&1)
    {
      K=(byte)(AT[0]-VScroll); /* K = sprite Y coordinate */
      if(K>256-H) K-=256;      /* Y coordinate may be negative */

      J=Y-K-1;
      C=SprTab[-0x0200+((AT-SprTab)<<2)+J];
      OrThem|=C&0x40;

      if(C&0x0F)
      {
        PT=SprGen+((int)(H>8? AT[2]&0xFC:AT[2])<<3)+J;
        P=ZBuf+AT[1]+(C&0x80? 0:32);
        C&=0x0F;
        J=PT[0];

        if(OrThem&0x20)
        {
          if(J&0x80) P[0]|=C;if(J&0x40) P[1]|=C;
          if(J&0x20) P[2]|=C;if(J&0x10) P[3]|=C;
          if(J&0x08) P[4]|=C;if(J&0x04) P[5]|=C;
          if(J&0x02) P[6]|=C;if(J&0x01) P[7]|=C;
          if(H>8)
          {
            J=PT[16];
            if(J&0x80) P[8]|=C; if(J&0x40) P[9]|=C;
            if(J&0x20) P[10]|=C;if(J&0x10) P[11]|=C;
            if(J&0x08) P[12]|=C;if(J&0x04) P[13]|=C;
            if(J&0x02) P[14]|=C;if(J&0x01) P[15]|=C;
          }
        }
        else
        {
          if(J&0x80) P[0]=C;if(J&0x40) P[1]=C;
          if(J&0x20) P[2]=C;if(J&0x10) P[3]=C;
          if(J&0x08) P[4]=C;if(J&0x04) P[5]=C;
          if(J&0x02) P[6]=C;if(J&0x01) P[7]=C;
          if(H>8)
          {
            J=PT[16];
            if(J&0x80) P[8]=C; if(J&0x40) P[9]=C;
            if(J&0x20) P[10]=C;if(J&0x10) P[11]=C;
            if(J&0x08) P[12]=C;if(J&0x04) P[13]=C;
            if(J&0x02) P[14]=C;if(J&0x01) P[15]=C;
          }
        }
      }

      /* Update overlapping flag */
      OrThem>>=1;
    }
}

/** RefreshLineF() *******************************************/
/** Dummy refresh function called for non-existing screens. **/
/*************************************************************/
void HiresRefreshLineF(register byte Y)
{
  register pixel *P;
  P=HiresRefreshBorder(Y,XPal[BGColor]);
  if(P) HiresClearLine(P,XPal[BGColor]);
}

/** RefreshLine0() *******************************************/
/** Refresh line Y (0..191/211) of SCREEN0.                 **/
/*************************************************************/
void HiresRefreshLine0(register byte Y)
{
  register pixel *P,FC,BC;
  register byte X,*T,*G;

  BC=XPal[BGColor];
  P=HiresRefreshBorder(Y,BC);
  if(!P) return;

  if(!ScreenON) HiresClearLine(P,BC);
  else
  {
    P[0]=P[1] =P[2] =P[3] =P[4] =P[5] =P[6] =P[7] =P[8] =
    P[9]=P[10]=P[11]=P[12]=P[13]=P[14]=P[15]=P[16]=P[17]=BC;

    G=(UseFont&&FontBuf? FontBuf:ChrGen)+((Y+VScroll)&0x07);
    T=ChrTab+40*(Y>>3);
    FC=XPal[FGColor];
    P+=18;

    for(X=0;X<40;X++,T++,P+=12)
    {
      Y=G[(int)*T<<3];
      P[0]=P[1]=Y&0x80? FC:BC;P[2] =P[3] =Y&0x40? FC:BC;
      P[4]=P[5]=Y&0x20? FC:BC;P[6] =P[7] =Y&0x10? FC:BC;
      P[8]=P[9]=Y&0x08? FC:BC;P[10]=P[11]=Y&0x04? FC:BC;
    }

    P[0]=P[1]=P[2]=P[3] =P[4] =P[5] =P[6] =
    P[7]=P[8]=P[9]=P[10]=P[11]=P[12]=P[13]=BC;
  }
}

/** RefreshLine1() *******************************************/
/** Refresh line Y (0..191/211) of SCREEN1, including       **/
/** sprites in this line.                                   **/
/*************************************************************/
void HiresRefreshLine1(register byte Y)
{
  register pixel *P,FC,BC;
  register byte K,X,*T,*G;

  P=HiresRefreshBorder(Y,XPal[BGColor]);
  if(!P) return;

  if(!ScreenON) HiresClearLine(P,XPal[BGColor]);
  else
  {
    Y+=VScroll;
    G=(UseFont&&FontBuf? FontBuf:ChrGen)+(Y&0x07);
    T=ChrTab+((int)(Y&0xF8)<<2);

    for(X=0;X<32;X++,T++,P+=16)
    {
      K=ColTab[*T>>3];
      FC=XPal[K>>4];
      BC=XPal[K&0x0F];
      K=G[(int)*T<<3];
      P[0] =P[1] =K&0x80? FC:BC;P[2] =P[3] =K&0x40? FC:BC;
      P[4] =P[5] =K&0x20? FC:BC;P[6] =P[7] =K&0x10? FC:BC;
      P[8] =P[9] =K&0x08? FC:BC;P[10]=P[11]=K&0x04? FC:BC;
      P[12]=P[13]=K&0x02? FC:BC;P[14]=P[15]=K&0x01? FC:BC;
    }

    if(!SpritesOFF) HiresSprites(Y,P-512);
  }
}

/** RefreshLine2() *******************************************/
/** Refresh line Y (0..191/211) of SCREEN2, including       **/
/** sprites in this line.                                   **/
/*************************************************************/
void HiresRefreshLine2(register byte Y)
{
  register pixel *P,FC,BC;
  register byte K,X,*T,*PGT,*CLT;
  register int J;

  P=HiresRefreshBorder(Y,XPal[BGColor]);
  if(!P) return;

  if(!ScreenON) HiresClearLine(P,XPal[BGColor]);
  else
  {
    Y+=VScroll;
    T=ChrTab+((int)(Y&0xF8)<<2);
    J=((int)(Y&0xC0)<<5)+(Y&0x07);
    PGT=ChrGen+J;
    CLT=ColTab+J;

    for(X=0;X<32;X++,T++,P+=16)
    {
      J=(int)*T<<3;
      K=CLT[J&ColTabM];
      FC=XPal[K>>4];
      BC=XPal[K&0x0F];
      K=PGT[J&ChrGenM];
      P[0]= P[1]= K&0x80? FC:BC;P[2] =P[3] =K&0x40? FC:BC;
      P[4]= P[5]= K&0x20? FC:BC;P[6] =P[7] =K&0x10? FC:BC;
      P[8]= P[9]= K&0x08? FC:BC;P[10]=P[11]=K&0x04? FC:BC;
      P[12]=P[13]=K&0x02? FC:BC;P[14]=P[15]=K&0x01? FC:BC;
    }

    if(!SpritesOFF) HiresSprites(Y,P-512);
  }
}

/** RefreshLine3() *******************************************/
/** Refresh line Y (0..191/211) of SCREEN3, including       **/
/** sprites in this line.                                   **/
/*************************************************************/
void HiresRefreshLine3(register byte Y)
{
  register pixel *P;
  register byte X,K,*T,*G;

  P=HiresRefreshBorder(Y,XPal[BGColor]);
  if(!P) return;

  if(!ScreenON) HiresClearLine(P,XPal[BGColor]);
  else
  {
    Y+=VScroll;
    T=ChrTab+((int)(Y&0xF8)<<2);
    G=ChrGen+((Y&0x1C)>>2);

    for(X=0;X<32;X++,T++,P+=16)
    {
      K=G[(int)*T<<3];
      P[0] =P[1] =P[2] =P[3]= XPal[K>>4];
      P[4] =P[5] =P[6] =P[7]= XPal[K>>4];
      P[8] =P[9] =P[10]=P[11]=XPal[K&0x0F];
      P[12]=P[13]=P[14]=P[15]=XPal[K&0x0F];

	}

    if(!SpritesOFF) HiresSprites(Y,P-512);
  }
}

/** RefreshLine4() *******************************************/
/** Refresh line Y (0..191/211) of SCREEN4, including       **/
/** sprites in this line.                                   **/
/*************************************************************/
void HiresRefreshLine4(register byte Y)
{
  register pixel *P,FC,BC;
  register byte I,X,C,*T,*R;
  register int J;
  byte ZBuf[304];

  P=HiresRefreshBorder(Y,XPal[BGColor]);
  if(!P) return;

  if(!ScreenON) HiresClearLine(P,XPal[BGColor]);
  else
  {
    HiresColorSprites(Y,ZBuf);
    R=ZBuf+32;
    Y+=VScroll;
    T=ChrTab+((int)(Y&0xF8)<<2);

    for(X=0;X<32;X++,R+=8,P+=16,T++)
    {
      J=((int)(Y&0xC0)<<5)+((int)*T<<3)+(Y&0x07);
      I=ColTab[J&ColTabM];
      FC=XPal[I>>4];
      BC=XPal[I&0x0F];
      I=ChrGen[J&ChrGenM];

      C=R[0];P[0]= P[1]= C? XPal[C]:(I&0x80)? FC:BC;
      C=R[1];P[2]= P[3]= C? XPal[C]:(I&0x40)? FC:BC;
      C=R[2];P[4]= P[5]= C? XPal[C]:(I&0x20)? FC:BC;
      C=R[3];P[6]= P[7]= C? XPal[C]:(I&0x10)? FC:BC;
      C=R[4];P[8]= P[9]= C? XPal[C]:(I&0x08)? FC:BC;
      C=R[5];P[10]=P[11]=C? XPal[C]:(I&0x04)? FC:BC;
      C=R[6];P[12]=P[13]=C? XPal[C]:(I&0x02)? FC:BC;
      C=R[7];P[14]=P[15]=C? XPal[C]:(I&0x01)? FC:BC;
    }
  }
}

/** RefreshLine5() *******************************************/
/** Refresh line Y (0..191/211) of SCREEN5, including       **/
/** sprites in this line.                                   **/
/*************************************************************/
void HiresRefreshLine5(register byte Y)
{
  register pixel *P;
  register byte I,X,*T,*R;
  byte ZBuf[304];

  P=HiresRefreshBorder(Y,XPal[BGColor]);
  if(!P) return;

  if(!ScreenON) HiresClearLine(P,XPal[BGColor]);
  else
  {
    HiresColorSprites(Y,ZBuf);
    R=ZBuf+32;
    T=ChrTab+(((int)(Y+VScroll)<<7)&ChrTabM&0x7FFF);

    for(X=0;X<16;X++,R+=16,P+=32,T+=8)
    {
      I=R[0]; P[0] =P[1] =XPal[I? I:T[0]>>4];
      I=R[1]; P[2] =P[3] =XPal[I? I:T[0]&0x0F];
      I=R[2]; P[4] =P[5] =XPal[I? I:T[1]>>4];
      I=R[3]; P[6] =P[7] =XPal[I? I:T[1]&0x0F];
      I=R[4]; P[8] =P[9] =XPal[I? I:T[2]>>4];
      I=R[5]; P[10]=P[11]=XPal[I? I:T[2]&0x0F];
      I=R[6]; P[12]=P[13]=XPal[I? I:T[3]>>4];
      I=R[7]; P[14]=P[15]=XPal[I? I:T[3]&0x0F];
      I=R[8]; P[16]=P[17]=XPal[I? I:T[4]>>4];
      I=R[9]; P[18]=P[19]=XPal[I? I:T[4]&0x0F];
      I=R[10];P[20]=P[21]=XPal[I? I:T[5]>>4];
      I=R[11];P[22]=P[23]=XPal[I? I:T[5]&0x0F];
      I=R[12];P[24]=P[25]=XPal[I? I:T[6]>>4];
      I=R[13];P[26]=P[27]=XPal[I? I:T[6]&0x0F];
      I=R[14];P[28]=P[29]=XPal[I? I:T[7]>>4];
      I=R[15];P[30]=P[31]=XPal[I? I:T[7]&0x0F];
    }
  }
}

/** RefreshLine8() *******************************************/
/** Refresh line Y (0..191/211) of SCREEN8, including       **/
/** sprites in this line.                                   **/
/*************************************************************/
void HiresRefreshLine8(register byte Y)
{
  static byte SprToScr[16] =
  {
    0x00,0x02,0x10,0x12,0x80,0x82,0x90,0x92,
    0x49,0x4B,0x59,0x5B,0xC9,0xCB,0xD9,0xDB
  };
  register pixel *P;
  register byte C,X,*T,*R;
  byte ZBuf[304];

  P=HiresRefreshBorder(Y,BPal[VDP[7]]);
  if(!P) return;

  if(!ScreenON) HiresClearLine(P,BPal[VDP[7]]);
  else
  {
    HiresColorSprites(Y,ZBuf);
    R=ZBuf+32;
    T=ChrTab+(((int)(Y+VScroll)<<8)&ChrTabM&0xFFFF);

    for(X=0;X<32;X++,T+=8,R+=8,P+=16)
    {
      C=R[0];P[0] =P[1] =BPal[C? SprToScr[C]:T[0]];
      C=R[1];P[2] =P[3] =BPal[C? SprToScr[C]:T[1]];
      C=R[2];P[4] =P[5] =BPal[C? SprToScr[C]:T[2]];
      C=R[3];P[6] =P[7] =BPal[C? SprToScr[C]:T[3]];
      C=R[4];P[8] =P[9] =BPal[C? SprToScr[C]:T[4]];
      C=R[5];P[10]=P[11]=BPal[C? SprToScr[C]:T[5]];
      C=R[6];P[12]=P[13]=BPal[C? SprToScr[C]:T[6]];
      C=R[7];P[14]=P[15]=BPal[C? SprToScr[C]:T[7]];
    }
  }
}

/** RefreshLine10() ******************************************/
/** Refresh line Y (0..191/211) of SCREEN10/11, including   **/
/** sprites in this line.                                   **/
/*************************************************************/
void HiresRefreshLine10(register byte Y)
{
  register pixel *P;
  register byte C,X,*T,*R;
  register int J,K;
  byte ZBuf[304];

  P=HiresRefreshBorder(Y,BPal[VDP[7]]);
  if(!P) return;

  if(!ScreenON) HiresClearLine(P,BPal[VDP[7]]);
  else
  {
    HiresColorSprites(Y,ZBuf);
    R=ZBuf+32;
    T=ChrTab+(((int)(Y+VScroll)<<8)&ChrTabM&0xFFFF);

    /* Draw first 4 pixels */
    C=R[0];P[0]=P[1]=C? XPal[C]:BPal[VDP[7]];
    C=R[1];P[2]=P[3]=C? XPal[C]:BPal[VDP[7]];
    C=R[2];P[4]=P[5]=C? XPal[C]:BPal[VDP[7]];
    C=R[3];P[6]=P[7]=C? XPal[C]:BPal[VDP[7]];
    R+=4;P+=8;

    for(X=0;X<63;X++,T+=4,R+=4,P+=8)
    {
      K=(T[0]&0x07)|((T[1]&0x07)<<3);
      if(K&0x20) K-=64;
      J=(T[2]&0x07)|((T[3]&0x07)<<3);
      if(J&0x20) J-=64;

      C=R[0];Y=T[0]>>3;P[0]=P[1]=C? XPal[C]:Y&1? XPal[Y>>1]:HiresYJKColor(Y,J,K);
      C=R[1];Y=T[1]>>3;P[2]=P[3]=C? XPal[C]:Y&1? XPal[Y>>1]:HiresYJKColor(Y,J,K);
      C=R[2];Y=T[2]>>3;P[4]=P[5]=C? XPal[C]:Y&1? XPal[Y>>1]:HiresYJKColor(Y,J,K);
      C=R[3];Y=T[3]>>3;P[6]=P[7]=C? XPal[C]:Y&1? XPal[Y>>1]:HiresYJKColor(Y,J,K);
    }
  }
}

/** RefreshLine12() ******************************************/
/** Refresh line Y (0..191/211) of SCREEN12, including      **/
/** sprites in this line.                                   **/
/*************************************************************/
void HiresRefreshLine12(register byte Y)
{
  register pixel *P;
  register byte C,X,*T,*R;
  register int J,K;
  byte ZBuf[304];

  P=HiresRefreshBorder(Y,BPal[VDP[7]]);
  if(!P) return;

  if(!ScreenON) HiresClearLine(P,BPal[VDP[7]]);
  else
  {
    HiresColorSprites(Y,ZBuf);
    R=ZBuf+32;
    T=ChrTab+((int)((Y+VScroll)&ChrTabM)<<8);

    if(HScroll512&&(HScroll>255)) T=(byte *)((int)T^0x10000);
    T+=HScroll&0xFC;

    /* Draw first 4 pixels */
    C=R[0];P[0]=P[1]=C? XPal[C]:BPal[VDP[7]];
    C=R[1];P[2]=P[3]=C? XPal[C]:BPal[VDP[7]];
    C=R[2];P[4]=P[5]=C? XPal[C]:BPal[VDP[7]];
    C=R[3];P[6]=P[7]=C? XPal[C]:BPal[VDP[7]];
    R+=4;P+=8;

    for(X=1;X<64;X++,T+=4,R+=4,P+=8)
    {
      K=(T[0]&0x07)|((T[1]&0x07)<<3);
      if(K&0x20) K-=64;
      J=(T[2]&0x07)|((T[3]&0x07)<<3);
      if(J&0x20) J-=64;

      C=R[0];P[0]=P[1]=C? XPal[C]:HiresYJKColor(T[0]>>3,J,K);
      C=R[1];P[2]=P[3]=C? XPal[C]:HiresYJKColor(T[1]>>3,J,K);
      C=R[2];P[4]=P[5]=C? XPal[C]:HiresYJKColor(T[2]>>3,J,K);
      C=R[3];P[6]=P[7]=C? XPal[C]:HiresYJKColor(T[3]>>3,J,K);
    }
  }
}

/** RefreshLine6() *******************************************/
/** Refresh line Y (0..191/211) of SCREEN6, including       **/
/** sprites in this line.                                   **/
/*************************************************************/
void HiresRefreshLine6(register byte Y)
{
  register pixel *P;
  register byte X,*T,*R,C;
  byte ZBuf[304];

  P=HiresRefreshBorder(Y,XPal[BGColor&0x03]);
  if(!P) return;

  if(!ScreenON) HiresClearLine(P,XPal[BGColor&0x03]);
  else
  {
    HiresColorSprites(Y,ZBuf);
    R=ZBuf+32;
    T=ChrTab+(((int)(Y+VScroll)<<7)&ChrTabM&0x7FFF);

    for(X=0;X<32;X++)
    {
      C=R[0];P[0] =XPal[C? C: T[0]>>6];
      C=R[0];P[1] =XPal[C? C:(T[0]>>4)&0x03];
      C=R[1];P[2] =XPal[C? C:(T[0]>>2)&0x03];
      C=R[1];P[3] =XPal[C? C: T[0]&0x03];
      C=R[2];P[4] =XPal[C? C: T[1]>>6];
      C=R[2];P[5] =XPal[C? C:(T[1]>>4)&0x03];
      C=R[3];P[6] =XPal[C? C:(T[1]>>2)&0x03];
      C=R[3];P[7] =XPal[C? C: T[1]&0x03];
      C=R[4];P[8] =XPal[C? C: T[2]>>6];
      C=R[4];P[9] =XPal[C? C:(T[2]>>4)&0x03];
      C=R[5];P[10]=XPal[C? C:(T[2]>>2)&0x03];
      C=R[5];P[11]=XPal[C? C: T[2]&0x03];
      C=R[6];P[12]=XPal[C? C: T[3]>>6];
      C=R[6];P[13]=XPal[C? C:(T[3]>>4)&0x03];
      C=R[7];P[14]=XPal[C? C:(T[3]>>2)&0x03];
      C=R[7];P[15]=XPal[C? C: T[3]&0x03];
      R+=8;P+=16;T+=4;
    }
  }
}
  
/** RefreshLine7() *******************************************/
/** Refresh line Y (0..191/211) of SCREEN7, including       **/
/** sprites in this line.                                   **/
/*************************************************************/
void HiresRefreshLine7(register byte Y)
{
  register pixel *P;
  register byte C,X,*T,*R;
  byte ZBuf[304];

  P=HiresRefreshBorder(Y,XPal[BGColor]);
  if(!P) return;

  if(!ScreenON) HiresClearLine(P,XPal[BGColor]);
  else
  {
    HiresColorSprites(Y,ZBuf);
    R=ZBuf+32;
    T=ChrTab+(((int)(Y+VScroll)<<8)&ChrTabM&0xFFFF);

    for(X=0;X<32;X++,R+=8,P+=16,T+=8)
    {
      C=R[0];P[0] =XPal[C? C:T[0]>>4];
      C=R[0];P[1] =XPal[C? C:T[0]&0x0F];
      C=R[1];P[2] =XPal[C? C:T[1]>>4];
      C=R[1];P[3] =XPal[C? C:T[1]&0x0F];
      C=R[2];P[4] =XPal[C? C:T[2]>>4];
      C=R[2];P[5] =XPal[C? C:T[2]&0x0F];
      C=R[3];P[6] =XPal[C? C:T[3]>>4];
      C=R[3];P[7] =XPal[C? C:T[3]&0x0F];
      C=R[4];P[8] =XPal[C? C:T[4]>>4];
      C=R[4];P[9] =XPal[C? C:T[4]&0x0F];
      C=R[5];P[10]=XPal[C? C:T[5]>>4];
      C=R[5];P[11]=XPal[C? C:T[5]&0x0F];
      C=R[6];P[12]=XPal[C? C:T[6]>>4];
      C=R[6];P[13]=XPal[C? C:T[6]&0x0F];
      C=R[7];P[14]=XPal[C? C:T[7]>>4];
      C=R[7];P[15]=XPal[C? C:T[7]&0x0F];
    }  
  }
}

/** RefreshLineTx80() ****************************************/
/** Refresh line Y (0..191/211) of TEXT80.                  **/
/*************************************************************/
void HiresRefreshLineTx80(register byte Y)
{
  register pixel *P,FC,BC;
  register byte X,M,*T,*C,*G;

  BC=XPal[BGColor];
  P=HiresRefreshBorder(Y,BC);
  if(!P) return;

  if(!ScreenON) HiresClearLine(P,BC);
  else
  {
    P[0]=P[1] =P[2] =P[3] =P[4] =P[5] =P[6] =P[7] =P[8] =
    P[9]=P[10]=P[11]=P[12]=P[13]=P[14]=P[15]=P[16]=P[17]=BC;

    G=(UseFont&&FontBuf? FontBuf:ChrGen)+((Y+VScroll)&0x07);
    T=ChrTab+80*(Y>>3);
    C=ColTab+10*(Y>>3);
    P+=18;

    for(X=0,M=0x00;X<80;X++,T++,P+=6)
    {
      if(!(X&0x07)) M=*C++;
      if(M&0x80) { FC=XPal[XFGColor];BC=XPal[XBGColor]; }
      else       { FC=XPal[FGColor];BC=XPal[BGColor]; }
      M<<=1;
      Y=*(G+((int)*T<<3));
      P[0]=Y&0x80? FC:BC;P[1]=Y&0x40? FC:BC;
      P[2]=Y&0x20? FC:BC;P[3]=Y&0x10? FC:BC;
      P[4]=Y&0x08? FC:BC;P[5]=Y&0x04? FC:BC;   
	}

    P[0]=P[1]=P[2]=P[3] =P[4] =P[5] =P[6] =
    P[7]=P[8]=P[9]=P[10]=P[11]=P[12]=P[13]=BC;

  }
}

