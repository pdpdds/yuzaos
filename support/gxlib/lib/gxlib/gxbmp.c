/*******************************************************************************
프로젝트 : gxLib
모듈내용 : BITMAP 출력
변경일자 : 2008-11-22
작성자   : 장길석(jwjwmx@gmail.com)
저작권   : 주석 내용을 변경하지 않는 한 무료 제공
홈페이지 : http://forum.falinux.com
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gx.h>
#include <gxbmp.h>

#define BI_RGB          0
#define BI_RLE4         1
#define BI_RLE8         2
#define BI_BITFIELD     3

/********************************************************************* 1 pixel */

static int   b1_color( color_t color)
//-------------------------------------------------------------------------------
// 설명: 칼라 값을 구한다.
// 인수: red               0부터 255 사이의 red   값
//       green             0부터 255 사이의 green 값
//       blue              0부터 255 사이의 blue  값
{
   if ( 0 == color.red +color.green +color.blue)
      return   0;
   else
      return   1;
}

static void  b1_clear( bmp_t *bmp, color_t color)
//-------------------------------------------------------------------------------
{
   int      ncolor;

   printf( "구현해야 함: gxbmp.c -> b1_clear\n");
   ncolor   = b1_color( color);
   bmp = bmp; color = color;
}

static void b1_get_pixel( bmp_t *bmp, int coor_x, int coor_y, color_t *color)
//-------------------------------------------------------------------------------
{
   unsigned char *pdata;
   int            noffset;
   int            npixel;

   pdata    = bmp->data + (bmp->height -coor_y -1) * bmp->bytes_per_line + (coor_x /8);
   noffset  = coor_x %8;
   npixel   = (*pdata & (0x80U >> noffset)) >> (7 -noffset);

   /* palette lookup */
   color->blue    = bmp->palette[npixel].blue;
   color->green   = bmp->palette[npixel].green;
   color->red     = bmp->palette[npixel].red;
   color->alpha   = 255;
}

static void b1_set_pixel( bmp_t *bmp, int coor_x, int coor_y, color_t color)
//-------------------------------------------------------------------------------
{
   printf( "구현해야 함: gxbmp.c -> b1_set_pixel\n");
   bmp = bmp;     coor_x = coor_x;    coor_y = coor_y;    color = color;
}

static void  b1_hline( bmp_t *bmp, int x_1st, int x_2nd, int coor_y, color_t color)
//-------------------------------------------------------------------------------
// 설명: 수평선을 그린다.
{
   printf( "구현해야 함: gxbmp.c -> b1_hline\n");
   bmp = bmp;  x_1st = x_1st;   x_2nd = x_2nd;   coor_y = coor_y;    color = color;
}

static void  b1_vline( bmp_t *bmp, int coor_x, int y_1st, int y_2nd, color_t color)
//-------------------------------------------------------------------------------
// 설명: 수직선을 그린다.
{
   printf( "구현해야 함: gxbmp.c -> b1_vline\n");
   bmp = bmp;  coor_x = coor_x; y_1st = y_1st;   y_2nd = y_2nd;    color = color;
}

/********************************************************************* 4 pixel */

static int   b4_color( color_t color)
//-------------------------------------------------------------------------------
// 설명: 칼라 값을 구한다.
{
   printf( "구현해야 함: gxbmp.c -> b4_color\n");
   color = color;
   return  0;
}

static void  b4_clear( bmp_t *bmp, color_t color)
//-------------------------------------------------------------------------------
{
   printf( "구현해야 함: gxbmp.c -> b4_clear\n");
   bmp = bmp; color = color;
}
static void b4_get_pixel( bmp_t *bmp, int coor_x, int coor_y, color_t *color)
//-------------------------------------------------------------------------------
{
   unsigned char *pdata;
   int            noffset;
   int            npixel;

   pdata    = bmp->data + ( bmp->height -coor_y -1) * bmp->bytes_per_line + ( coor_x>>1);
   noffset  = ( coor_x & 0x01) << 2;

   npixel   = (*pdata & (0xF0U >> noffset)) >> (4-noffset);

   /* palette lookup */
   color->blue    = bmp->palette[npixel].blue;
   color->green   = bmp->palette[npixel].green;
   color->red     = bmp->palette[npixel].red;
   color->alpha   = 255;
}

static void b4_set_pixel( bmp_t *bmp, int coor_x, int coor_y, color_t color)
//-------------------------------------------------------------------------------
{
   bmp = bmp;  coor_x = coor_x;  coor_y = coor_y;  color = color;
}

static void  b4_hline( bmp_t *bmp, int x_1st, int x_2nd, int coor_y, color_t color)
//-------------------------------------------------------------------------------
// 설명: 수평선을 그린다.
{
   int      ncolor;

   printf( "구현해야 함: gxbmp.c -> b4_hline\n");
   ncolor  = b4_color( color);
   bmp = bmp;  x_1st = x_1st;   x_2nd = x_2nd; coor_y = coor_y;  color = color;
}

static void  b4_vline( bmp_t *bmp, int coor_x, int y_1st, int y_2nd, color_t color)
//-------------------------------------------------------------------------------
// 설명: 수직선을 그린다.
{
   printf( "구현해야 함: gxbmp.c -> b4_vline\n");
   bmp = bmp;  coor_x = coor_x;  y_1st = y_1st; y_2nd = y_2nd; color = color;
}

/********************************************************************* 8 pixel */

static int   b8_color( color_t color)
//-------------------------------------------------------------------------------
// 설명: 칼라 값을 구한다.
{
   printf( "구현해야 함: gxbmp.c -> b8_color\n");
   color = color;
   return  0;
}

static void  b8_clear( bmp_t *bmp, color_t color)
//-------------------------------------------------------------------------------
{
   printf( "구현해야 함: gxbmp.c -> b8_clear\n");
   bmp = bmp; color = color;
}
static void b8_get_pixel( bmp_t *bmp, int coor_x, int coor_y, color_t *color)
//-------------------------------------------------------------------------------
{
   unsigned char *pdata;
   int            npixel;

   pdata          = bmp->data +(bmp->height -coor_y -1) *bmp->bytes_per_line +coor_x;
   npixel         = *pdata;

   /* palette lookup */

   color->blue    = bmp->palette[npixel].blue;
   color->green   = bmp->palette[npixel].green;
   color->red     = bmp->palette[npixel].red;
   color->alpha   = 255;
}

static void b8_set_pixel( bmp_t *bmp, int coor_x, int coor_y, color_t color)
//-------------------------------------------------------------------------------
{
   printf( "구현해야 함: gxbmp.c -> b8_set_pixel\n");
   bmp = bmp;  coor_x = coor_x;  coor_y = coor_y;  color = color;
}

static void  b8_hline( bmp_t *bmp, int x_1st, int x_2nd, int coor_y, color_t color)
//-------------------------------------------------------------------------------
// 설명: 수평선을 그린다.
{
   int      ncolor;

   printf( "구현해야 함: gxbmp.c -> b8_hline\n");
   ncolor  = b8_color( color);
   bmp = bmp;  x_1st = x_1st;   x_2nd = x_2nd;   coor_y = coor_y;    color = color;
}

static void  b8_vline( bmp_t *bmp, int coor_x, int y_1st, int y_2nd, color_t color)
//-------------------------------------------------------------------------------
// 설명: 수직선을 그린다.
{
   printf( "구현해야 함: gxbmp.c -> b8_vline\n");
   bmp = bmp;  coor_x = coor_x; y_1st = y_1st;   y_2nd = y_2nd;    color = color;
}

/******************************************************************** 16 pixel */

static int   b16_color( color_t color)
//-------------------------------------------------------------------------------
// 설명: R,G,B 값을 지정했을 때, DC에 해당하는 정수 칼라 값을 구한다.
// 참고: 16 bit 환경에 따라 shift=0:5:6:5, shift=0:11:5:0
// 또한: red, green, blue 값을 아래와 같이 구해야 옳다. 그러나 실행 속도를 위해 일반 상수로 치환했다.
//   red   >>= 8-bmp->bsize_red;
//   green >>= 8-bmp->bsize_green;
//   blue  >>= 8-bmp->bsize_blue;
//   return   ( red << bmp->boffset_red) | ( green << bmp->boffset_green) | blue << bmp->boffset_blue;
// 인수: red:0부터 255 사이의 red 값, green:green 값, blud:blue  값
// 반환: 정수 칼라 값
{
   color.red   >>= 3;
   color.green >>= 3;
   color.blue  >>= 3;

   return   ( color.red << 10) | ( color.green << 5) | color.blue;
}

static void  b16_clear( bmp_t *bmp, color_t color)
//-------------------------------------------------------------------------------
// 설명: Bitmap DC의 영역을 지정한 색상으로 채운다.
{
   unsigned short *ptr;
   int             ncolor;
   int             ndx;

   if ( 0 < color.alpha)
   {
      ncolor  = b16_color( color);
      ptr = (unsigned short *)bmp->data;

      for ( ndx = 0; ndx < bmp->dots; ndx++)
         *ptr++ = ncolor;
   }
}

static void b16_get_pixel( bmp_t *bmp, int coor_x, int coor_y, color_t *color)
//-------------------------------------------------------------------------------
{
   unsigned short *pdata;
   unsigned       *mask    = (unsigned *)( bmp->palette);

   pdata = (unsigned short *)(bmp->data +(bmp->height -coor_y -1) *bmp->bytes_per_line +( coor_x << 1));

   color->blue    = ((*pdata & mask[2]) >> bmp->boffset_blue ) << (8-bmp->bsize_blue );
   color->green   = ((*pdata & mask[1]) >> bmp->boffset_green) << (8-bmp->bsize_green);
   color->red     = ((*pdata & mask[0]) >> bmp->boffset_red  ) << (8-bmp->bsize_red  );
   color->alpha   = 255;
}

static void b16_set_pixel( bmp_t *bmp, int coor_x, int coor_y, color_t color)
//-------------------------------------------------------------------------------
{
   unsigned short *pdata;
   int             offset = (bmp->height -coor_y -1) *bmp->bytes_per_line +( coor_x << 1);

   pdata = (unsigned short *)(bmp->data +offset);
  *pdata = b16_color( color);
}


static void  b16_hline( bmp_t *bmp, int x_1st, int x_2nd, int coor_y, color_t color)
//-------------------------------------------------------------------------------
// 설명: 수평선을 그린다.
// 인수: x_1st,_2nd     수평을 긋기 위한 x 시작과 종료 좌표
//       coor_y         수직 y 좌표
//       color
{
   unsigned short *ptr;
   int             ncolor;
   int             ndx;

   ncolor  = b16_color( color);
   ptr   = (unsigned short *)bmp->mapped + bmp->width *( bmp->height -coor_y) +x_1st;
   for ( ndx = x_1st; ndx <= x_2nd; ndx++)
     *ptr++ = ncolor;
}

static void  b16_vline( bmp_t *bmp, int coor_x, int y_1st, int y_2nd, color_t color)
//-------------------------------------------------------------------------------
// 설명: 수직선을 그린다.
// 인수: bmp            Bitmap DC
//       coor_x         수직선을 긋기 위한 x 좌표
//       y_1st,_2nd     수직선을 긋기 위한 y 좌표
//       color          수직선의 색상
{
   unsigned short *ptr;
   int             ncolor;
   int             ndx;

   ncolor  = b16_color( color);
   ptr   = (unsigned short *)bmp->mapped + bmp->width * ( bmp->height-y_1st) +coor_x;
   for ( ndx = y_1st; ndx <= y_2nd; ndx++)
   {
     *ptr   = ncolor;
      ptr  -= bmp->width;
   }
}

/******************************************************************** 24 pixel */

static int b24_color( color_t color)
//-------------------------------------------------------------------------------
// 설명: R,G,B 값을 지정했을 때, DC에 해당하는 정수 칼라 값을 구한다.
// 인수: red:0부터 255 사이의 red 값, green:green 값, blud:blue  값
// 반환: 정수 칼라 값
{
   return  ( color.red << 16) | ( color.green << 8) | color.blue;
}

static void b24_clear( bmp_t *bmp, color_t color)
//-------------------------------------------------------------------------------
// 설명: Bitmap DC의 영역을 지정한 색상으로 채운다.
// 인수: bmp
//       color
{
   printf( "구현해야 함: gxbmp.c -> b24_clear\n");
   bmp = bmp; color = color;
}

static void b24_get_pixel( bmp_t *bmp, int coor_x, int coor_y, color_t *color)
//-------------------------------------------------------------------------------
{
   unsigned char *pdata;

   pdata          = bmp->data + ( bmp->height -coor_y -1)*bmp->bytes_per_line +coor_x*3;
   color->blue    = *pdata++;
   color->green   = *pdata++;
   color->red     = *pdata;
   color->alpha   = 255;
}

static void b24_set_pixel( bmp_t *bmp, int coor_x, int coor_y, color_t color)
//-------------------------------------------------------------------------------
{
   unsigned char *pdata;
   int            offset = ( bmp->height -coor_y -1)*bmp->bytes_per_line +coor_x*3;

   pdata          = bmp->data + offset;
  *pdata++        = color.blue;
  *pdata++        = color.green;
  *pdata          = color.red;
}

static void b24_hline( bmp_t *bmp, int x_1st, int x_2nd, int coor_y, color_t color)
//-------------------------------------------------------------------------------
// 설명: 수평선을 그린다.
{
   int      ncolor;

   printf( "구현해야 함: gxbmp.c -> b24_hline\n");
   ncolor  = b24_color( color);
   bmp = bmp;  x_1st = x_1st;   x_2nd = x_2nd;   coor_y = coor_y;    color = color;
}

static void b24_vline( bmp_t *bmp, int coor_x, int y_1st, int y_2nd, color_t color)
//-------------------------------------------------------------------------------
// 설명: 수직선을 그린다.
{
   printf( "구현해야 함: gxbmp.c -> b24_vline\n");
   bmp = bmp;  coor_x = coor_x; y_1st = y_1st;   y_2nd = y_2nd;    color = color;
}

/******************************************************************** 32 pixel */

static int b32_color( color_t color)
//-------------------------------------------------------------------------------
// 설명: R,G,B 값을 지정했을 때, DC에 해당하는 정수 칼라 값을 구한다.
// 인수: red:0부터 255 사이의 red 값, green:green 값, blud:blue  값
// 반환: 정수 칼라 값
{
   return  ( color.red << 16) | ( color.green << 8) | color.blue;
}

static void  b32_clear( bmp_t *bmp, color_t color)
//-------------------------------------------------------------------------------
{
   printf( "구현해야 함: gxbmp.c -> b32_clear\n");
   bmp = bmp; color = color;
}

static void b32_get_pixel( bmp_t *bmp, int coor_x, int coor_y, color_t *color)
//-------------------------------------------------------------------------------
{
   unsigned char *pdata;

   pdata          = bmp->data + ( bmp->height -coor_y -1)*bmp->bytes_per_line +coor_x*4;
   color->blue    = *pdata++;
   color->green   = *pdata++;
   color->red     = *pdata++;
   color->alpha   = *pdata;
}

static void b32_set_pixel( bmp_t *bmp, int coor_x, int coor_y, color_t color)
//-------------------------------------------------------------------------------
{
   unsigned char *pdata;
   int            offset = ( bmp->height -coor_y -1)*bmp->bytes_per_line +coor_x*4;

   pdata          = bmp->data + offset;
  *pdata++        = color.blue;
  *pdata++        = color.green;
  *pdata++        = color.red;
  *pdata          = color.alpha;
}

static void  b32_hline( bmp_t *bmp, int x_1st, int x_2nd, int coor_y, color_t color)
//-------------------------------------------------------------------------------
// 설명: 수평선을 그린다.
{
   int      ncolor;

   printf( "구현해야 함: gxbmp.c -> b32_hline\n");
   ncolor  = b32_color( color);
   bmp = bmp;  x_1st = x_1st;   x_2nd = x_2nd;   coor_y = coor_y;    color = color;
}

static void  b32_vline( bmp_t *bmp, int coor_x, int y_1st, int y_2nd, color_t color)
//-------------------------------------------------------------------------------
// 설명: 수직선을 그린다.
{
   printf( "구현해야 함: gxbmp.c -> b32_vline\n");
   bmp = bmp;  coor_x = coor_x;  y_1st = y_1st; y_2nd = y_2nd; color = color;
}

/*******************************************************************************/

static void calculate_boffset(bmp_t *bmp)
//-------------------------------------------------------------------------------
{
   int i;
   unsigned *mask = (unsigned *)(bmp->palette);
   unsigned  temp;

   /* red */
   temp = mask[0];
   for(i = 0; i < 32; i++)
   {
      if(temp & 0x01)
         break;
      temp >>= 1;
   }
   bmp->boffset_red = i;
   for(i = 0; i < 32; i++)
   {
      if(temp & 0x800000UL)
         break;
      temp <<= 1;
   }
   bmp->bsize_red = 32-i;

   /* green */
   temp = mask[1];
   for(i = 0; i < 32; i++)
   {
      if(temp & 0x01)
         break;
      temp >>= 1;
   }
   bmp->boffset_green = i;
   for(i = 0; i < 32; i++)
   {
      if(temp & 0x800000UL)
         break;
      temp <<= 1;
   }
   bmp->bsize_green = 32-i;

   /* blue */
   temp = mask[2];
   for(i = 0; i < 32; i++)
   {
      if(temp & 0x01)
         break;
      temp >>= 1;
   }
   bmp->boffset_blue = i;
   for(i = 0; i < 32; i++)
   {
      if (temp & 0x800000UL)
         break;
      temp <<= 1;
   }
   bmp->bsize_blue = 32-i;
}

static void rle8_decoding( bmp_t *bmp)
//-------------------------------------------------------------------------------
{
   unsigned char *pdata    = bmp->data;
   unsigned char *pend     = pdata + bmp->width*bmp->height;
   unsigned char *penc_data = bmp->encoded_data;
   unsigned char  c_byte;
   int            ndx, jdx;

   while( 1 )
   {
      if(pdata >= pend)
         break;
      c_byte = *penc_data++;
      if( 0 == c_byte) /* escape */
      {
         c_byte = *penc_data++;
         if( 0 == c_byte) /* end of line */
         {
            ndx = pdata - bmp->data;
            ndx %= bmp->width;
            for( ; ndx < bmp->width; ndx++)
               pdata++;
         }
         else if( c_byte == 1) /* end of bitmap */
         {
            return;
         }
         else if( c_byte == 2) /* delta */
         {
            jdx = *penc_data++; /* right */
            ndx = *penc_data++; /* down */
            ndx = jdx +ndx *bmp->width;
            while(0 < ndx--)
               pdata++;
         }
         else /* absolute mode */
         {
            c_byte = *penc_data++;
            while( 0 < c_byte--)
               *pdata++ = *penc_data++;
            /* word boundary */
            while((unsigned)penc_data & 0x01)
               penc_data++;
         }
      }
      else
      {
         while( c_byte--)
            *pdata++ = *penc_data;
         penc_data++;
      }
   }
}

static int read_data(FILE *fp, bmp_t *bmp)
//-------------------------------------------------------------------------------
{
   fseek(fp, bmp->data_offset, SEEK_SET);

   if (bmp->compression == BI_RGB || bmp->compression == BI_BITFIELD)
   {
      bmp->data = (unsigned char *)malloc(bmp->bitmap_size);
      fread(bmp->data, 1, bmp->bitmap_size, fp);
   }
   else
   {                     
      bmp->encoded_data = (unsigned char *)malloc(bmp->bitmap_size);
      bmp->data         = (unsigned char *)malloc(bmp->width*bmp->height*bmp->bpp/8);
      memset(bmp->encoded_data, 0, bmp->bitmap_size);
      fread( bmp->encoded_data, 1, bmp->bitmap_size, fp);
      if(bmp->compression == BI_RLE4)
         return GX_FALSE;                                               /* rle4_decoding is not supported ! */
      else
         rle8_decoding( bmp);
   }
   return GX_TRUE;
}

static int read_palette(FILE *fp, bmp_t *bmp)
//-------------------------------------------------------------------------------
{
   int size;

   size = fread( bmp->palette, sizeof( palette_t), bmp->cnt_palette, fp);
   if ( size != bmp->cnt_palette)
      return GX_FALSE;

   return GX_TRUE;
}

static int read_header(FILE *fp, bmp_t *bmp)
//-------------------------------------------------------------------------------
{
   int size;
   int remnant;
   unsigned char ID[2];

   ID[0] = fgetc( fp);                                                           // ID 체크
   ID[1] = fgetc( fp);
   if(ID[0] != 'B' || ID[1] != 'M')
      return GX_FALSE;

   if ( 52 != ( size = fread( &bmp->file_size, 1, 52, fp)))                       // bmp 헤더 정보를 모두 읽어 들인다.
      return GX_FALSE;

   bmp->cnt_palette  = ( bmp->data_offset-54) >> 2;
   bmp->width        = bmp->img_width;
   bmp->height       = bmp->img_height;

   size     = bmp->img_width * bmp->bpp /8;
   remnant  = size %4;
   if (remnant == 0)
      bmp->bytes_per_line = size;
   else
      bmp->bytes_per_line = size +(4 -remnant);

   if ( 0 == bmp->bitmap_size)                                                    // 만일 압축이 안된 상태라면 bitmap_size 는 0 이 됨
      bmp->bitmap_size = bmp->height*bmp->bytes_per_line;

   return GX_TRUE;
}

static int set_header( bmp_t *bmp, int width, int height, int depth, int palette_size)
//-------------------------------------------------------------------------------
{
   int size;
   int remnant;

   bmp->width        = width;
   bmp->height       = height;
   bmp->img_width    = width;
   bmp->img_height   = height;
   bmp->dots         = width * height;
   bmp->data         = NULL;
   bmp->palette      = NULL;
   bmp->compression  = BI_RGB;                                                   // BITMAP 압축 방식은 RGB값

   if ( 0 < palette_size)                                                         // 팔레트 사이즈가 지정되어 있다면
   {
      bmp->cnt_palette  = palette_size;
      bmp->palette      = ( palette_t *)malloc( sizeof( palette_t) * bmp->cnt_palette);
      memset( bmp->palette, 0, sizeof( palette_t) * bmp->cnt_palette);
   }

   bmp->bpp = depth;
   size     = bmp->img_width * bmp->bpp /8;
   remnant  = size %4;

   if ( 0 == remnant)
      bmp->bytes_per_line = size;
   else
      bmp->bytes_per_line = size + (4-remnant);

   bmp->bitmap_size  = bmp->height * bmp->bytes_per_line;
   bmp->bytes        = bmp->bitmap_size;
   
   return GX_TRUE;
}

static void  release_dc( dc_t *dc)
//-------------------------------------------------------------------------------
{
   bmp_t  *bmp  = (bmp_t *)dc;

   if ( NULL == bmp)             return;
   if ( NULL != bmp->palette)    free( bmp->palette);
   if ( NULL != bmp->data   )    free( bmp->data   );
   free( bmp);
}

static void free_bmp_resource( bmp_t *bmp, FILE *fp)
//-------------------------------------------------------------------------------
{
   release_dc( ( dc_t *)bmp);
   if ( fp)    fclose(fp);
}

static bmp_t *setup_bitmap( bmp_t *bmp)
//-------------------------------------------------------------------------------
// 설명: bmp 정보를 완성
{                           
    bmp->dc_type            = DCTYPE_BMP;                               // DC 형태를 BITMAP로
    bmp->coor_x             = 0;                                        // LineTo를 위한 좌표를 0으로 초기화
    bmp->coor_y             = 0;
    bmp->pen_color          = gx_color( 255, 255, 255, 255);            // 기본 펜 색상은 백색
    bmp->brush_color        = gx_color(   0,   0,   0, 255);            // 기본 브러쉬 색상은 검정
    bmp->font_color         = gx_color( 255, 255, 255, 255);            // 기본 글씨 색상은 백색
    bmp->release_dc         = release_dc;                               // 소멸자 가상 함수
    bmp->mapped             = bmp->data;                                // 이미지 부분의 메모리 포인터
    bmp->colors             = bmp->bpp;                                 // 칼라 깊이
    bmp->font               = NULL; 
    
    switch(bmp->bpp)
    {
    case  1:                 
            bmp->clear      = b1_clear;
            bmp->get_pixel  = b1_get_pixel;
            bmp->set_pixel  = b1_set_pixel;
            bmp->vline      = b1_vline;
            bmp->hline      = b1_hline;
            break;          
    case  4:                
            bmp->clear      = b4_clear;
            bmp->get_pixel  = b4_get_pixel;
            bmp->set_pixel  = b4_set_pixel;
            bmp->vline      = b4_vline;
            bmp->hline      = b4_hline;
            break;          
    case  8:                
            bmp->clear      = b8_clear;
            bmp->get_pixel  = b8_get_pixel;
            bmp->set_pixel  = b8_set_pixel;
            bmp->vline      = b8_vline;
            bmp->hline      = b8_hline;
            break;          
    case  16:               
            bmp->clear      = b16_clear;
            bmp->get_pixel  = b16_get_pixel;
            bmp->set_pixel  = b16_set_pixel;
            bmp->vline      = b16_vline;
            bmp->hline      = b16_hline;
    
            if( bmp->compression == BI_RGB)                                      // 보통 BI_RGB
            {
                unsigned *mask;
                
                if( NULL != bmp->palette) /* something wrong */
                {
                    printf( "setup_bitmap() : palette info error.\n");
                    free_bmp_resource( bmp, NULL);
                    return NULL;
                }
                mask = (unsigned *)malloc(sizeof(unsigned)*3);
                mask[2]             = 0x001F; /* blue mask */
                mask[1]             = 0x03E0; /* green mask */
                mask[0]             = 0x7C00; /* red mask */
                bmp->palette        = ( palette_t *)mask;
                bmp->boffset_blue   = 0;
                bmp->boffset_green  = 5;
                bmp->boffset_red    = 10;
                bmp->bsize_blue     = 5;
                bmp->bsize_green    = 5;
                bmp->bsize_red      = 5;
            }                       
            else  /* BI_BITFIELD */
            {
                if  ( bmp->palette == NULL) /* something wrong */
                {
                    printf( "setup_bitmap() : palette info error.\n");
                    free_bmp_resource( bmp, NULL);
                    return NULL;
                }
                calculate_boffset(bmp);
            }
            break;
    case  24:               
            bmp->clear      = b24_clear;
            bmp->get_pixel  = b24_get_pixel;
            bmp->set_pixel  = b24_set_pixel;
            bmp->vline      = b24_vline;
            bmp->hline      = b24_hline;
            break;          
    case  32:               
            bmp->clear      = b32_clear;
            bmp->get_pixel  = b32_get_pixel;
            bmp->set_pixel  = b32_set_pixel;
            bmp->vline      = b32_vline;
            bmp->hline      = b32_hline;
            if  (bmp->compression == BI_RGB)
            {
                unsigned *mask;
                
                if  ( bmp->palette != NULL) /* something wrong */
                {
                    printf( "setup_bitmap() : palette info error.\n");
                    free_bmp_resource( bmp, NULL);
                    return NULL;
                }
                
                mask                 = (unsigned *)malloc(sizeof(unsigned)*3);
                mask[2]              = 0x000000FF; /* blue mask */
                mask[1]              = 0x0000FF00; /* green mask */
                mask[0]              = 0x00FF0000; /* red mask */
                bmp->palette         = ( palette_t *)mask;
                bmp->boffset_blue    = 0;
                bmp->boffset_green   = 8;
                bmp->boffset_red     = 16;
                bmp->bsize_blue      = 8;
                bmp->bsize_green     = 8;
                bmp->bsize_red       = 8;
            }
            else /* BI_BITFILED */
            {
                if ( bmp->palette == NULL) /* something wrong */
                {
                    printf( "setup_bitmap() : palette info error.\n");
                    free_bmp_resource( bmp, NULL);
                    return NULL;
                }
                calculate_boffset( bmp);
            }
            break;
    default:
            printf( "setup_bitmap() : color depth error.\n");
            free_bmp_resource( bmp, NULL);
            return NULL;
    }
    return bmp;
}

dc_t *gx_bmp_create( int width, int height, int depth, int palette_size)
//-------------------------------------------------------------------------------
{
    bmp_t    *bmp;
    
    if  ( NULL == ( bmp = malloc( sizeof( bmp_t))) )
    {
        printf( "gx_bmp_create() : out of memory.\n");
        free_bmp_resource( bmp, NULL);
        return NULL;
    }
    memset( bmp, 0, sizeof( bmp_t));
    
    set_header( bmp, width, height, depth, palette_size);
           
    if  ( NULL == ( bmp->data = malloc( bmp->bitmap_size)) )
    {
        printf( "gx_bmp_create() : out of memory.\n");
        free_bmp_resource( bmp, NULL);
        return NULL;
    }
    
    return ( dc_t *)setup_bitmap( bmp);
}

bmp_t *gx_given_bmp_mastering( bmp_t *bmp , int width, int height, int depth, int palette_size)
//-------------------------------------------------------------------------------
{                                   
    set_header( bmp, width, height, depth, palette_size);
    
    if ( NULL == ( bmp->data = malloc( bmp->bitmap_size)) )
    {
        printf( "gx_given_bmp_mastering() : out of memory.\n");
        free_bmp_resource( bmp, NULL);
        return NULL;
    }
    
    return setup_bitmap( bmp);
}

void  gx_bmp_close( dc_t *dc)
//-------------------------------------------------------------------------------
// 설명: bmp 사용을 종료
{
    release_dc( dc);
}

dc_t *gx_bmp_open( char *filename)
//-------------------------------------------------------------------------------
// 설명: bmp 파일을 읽어 들임
// 참고: bmp의 칼라 깊이에 맞추어 가상 함수 지정
// 인수: fil
{            
    FILE    *fp    = NULL;
    bmp_t   *bmp   = NULL;
    
    if  ( NULL == ( bmp = malloc( sizeof( bmp_t))) )
    {                
        printf( "gx_bmp_open(): out of memory.\n");
        free_bmp_resource( bmp, NULL);
        return NULL;
    }
    
    memset( bmp, 0, sizeof( bmp_t));
    
    fp = fopen( filename, "r+b");
    if  ( NULL == ( fp) )
    {
        printf( "gx_bmp_open(): no file.\n");
        free_bmp_resource( bmp, fp);
        return NULL;
    }
    
    if ( !read_header( fp, bmp))
    {
        printf( "gx_bmp_open(): header info error.\n");
        free_bmp_resource( bmp, fp);
        return ( dc_t *)bmp;
    }
    
    if  ( 0 != bmp->cnt_palette)
    {
        bmp->palette = ( palette_t *)malloc( sizeof( palette_t) * bmp->cnt_palette);
        memset( bmp->palette, 0, sizeof( palette_t) * bmp->cnt_palette);
        if ( !read_palette(fp, bmp))
        {
            printf( "gx_bmp_open(): palette info error.\n");
            free_bmp_resource( bmp, fp);
            return NULL;
        }
    }
    
    if  ( !read_data(fp, bmp))
    {
        printf( "gx_bmp_open(): read error.\n");
        free_bmp_resource( bmp, fp);
        return NULL;
    }
    
    fclose(fp);
    return ( dc_t *)setup_bitmap( bmp);
}
