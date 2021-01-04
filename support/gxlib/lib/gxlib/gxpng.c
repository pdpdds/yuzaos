/*******************************************************************************
프로젝트 : gxLib
모듈내용 : PNG 출력
변경일자 : 2008-11-22
작성자   : 장길석(jwjwmx@gmail.com)
저작권   : 주석 내용을 변경하지 않는 한 무료 제공
홈페이지 : http://forum.falinux.com
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <png.h>
#include <gx.h>
#include <gxpng.h>

typedef unsigned char byte;

typedef struct rgb_t_ rgb_t;
struct rgb_t_
{
    byte red;
    byte green;
    byte blue;
};


static void  ch3_clear( png_t *png, color_t color)
//-------------------------------------------------------------------------------
{
    byte    *image_data;
    rgb_t   *ptr;                                                        // 3 바이트 크기의 포인터를 생성               
    rgb_t    color_rgb;                                                  // 3 바이트 크기의 칼라값을 만든다. 
    int      ndx;
                          
    color_rgb.red   = color.red;
    color_rgb.green = color.green;
    color_rgb.blue  = color.blue;

    image_data  = png->mapped;
    ptr         = (rgb_t *)image_data;

    for ( ndx = 0; ndx < png->width*png->height; ndx++)                         
       *ptr++ = color_rgb;
}

static void ch3_get_pixel( png_t *png, int coor_x, int coor_y, color_t *color)
//-------------------------------------------------------------------------------
{
    byte    *image_data;
    byte    *ptr;
    
    image_data      = png->mapped;
    ptr             = image_data + coor_y*png->bytes_per_line +coor_x*3;
    
    color->red      = *ptr++;
    color->green    = *ptr++;
    color->blue     = *ptr;
    color->alpha    = 255;
}                      

static void ch3_set_pixel( png_t *png, int coor_x, int coor_y, color_t color)
//-------------------------------------------------------------------------------
{
    byte    *image_data;
    byte    *ptr;
    
    image_data  = png->mapped;
    ptr         = image_data + coor_y*png->bytes_per_line +coor_x*3;

   *ptr++   = color.red  ;
   *ptr++   = color.green;
   *ptr     = color.blue;
}

static void  ch3_hline( png_t *png, int x_1st, int x_2nd, int coor_y, color_t color)
//-------------------------------------------------------------------------------
// 설명: 수평선을 그린다.
{
    byte    *image_data;
    byte    *ptr;
    int      ndx;
                
    image_data  = png->mapped;
    ptr         = image_data + coor_y*png->bytes_per_line +x_1st*3;
    
    for ( ndx = x_1st; ndx <= x_2nd; ndx++)
    {
       *ptr++ = color.red  ;
       *ptr++ = color.green;
       *ptr++ = color.blue ;
    }
}

static void  ch3_vline( png_t *png, int coor_x, int y_1st, int y_2nd, color_t color)
//-------------------------------------------------------------------------------
// 설명: 수직선을 그린다.
{
    byte    *image_data;
    byte    *ptr;
    byte    *ptr_bck;
    int      ndx;
                 
    image_data  = png->mapped;
    ptr         = image_data + y_1st*png->bytes_per_line +coor_x*3;
    
    for ( ndx = y_1st; ndx <= y_2nd; ndx++)
    {
        ptr_bck = ptr;
       *ptr++ = color.red  ;
       *ptr++ = color.green;
       *ptr   = color.blue ;
        ptr   = ptr_bck +png->bytes_per_line;
    }
}


static void  ch4_clear( png_t *png, color_t color)
//-------------------------------------------------------------------------------
{                 
    byte       *image_data;
    color_t    *ptr;
    int         ndx;
    
    image_data  = png->mapped;
    ptr         = (color_t *)image_data;
    
    for ( ndx = 0; ndx < png->width*png->height; ndx++)
       *ptr++ = color;
}

static void ch4_get_pixel( png_t *png, int coor_x, int coor_y, color_t *color)
//-------------------------------------------------------------------------------
{
    byte    *image_data;
    byte    *ptr;
                                
    image_data  = png->mapped;
    ptr         = image_data + coor_y*png->bytes_per_line +coor_x*4;
    
    color->red     = *ptr++;
    color->green   = *ptr++;
    color->blue    = *ptr++;
    color->alpha   = *ptr;
}

static void ch4_set_pixel( png_t *png, int coor_x, int coor_y, color_t color)
//-------------------------------------------------------------------------------
{
    byte    *image_data;
    byte    *ptr;
    
    image_data  = png->mapped;
    ptr         = image_data + coor_y*png->bytes_per_line +coor_x*4;

   *ptr++ = color.red  ;
   *ptr++ = color.green;
   *ptr++ = color.blue ;
   *ptr   = color.alpha;
}

static void  ch4_hline( png_t *png, int x_1st, int x_2nd, int coor_y, color_t color)
//-------------------------------------------------------------------------------
// 설명: 수평선을 그린다.
{
    byte    *image_data;
    byte    *ptr;
    int      ndx;
    
    image_data  = png->mapped;
    ptr         = image_data + coor_y*png->bytes_per_line +x_1st*4;
    
    for ( ndx = x_1st; ndx <= x_2nd; ndx++)
    {
       *ptr++ = color.red  ;
       *ptr++ = color.green;
       *ptr++ = color.blue ;
       *ptr++ = color.alpha;
    }
}

static void  ch4_vline( png_t *png, int coor_x, int y_1st, int y_2nd, color_t color)
//-------------------------------------------------------------------------------
// 설명: 수직선을 그린다.
{
    byte    *image_data;
    byte    *ptr;
    byte    *ptr_bck;
    int      ndx;
    
    image_data  = png->mapped;
    ptr         = image_data + y_1st*png->bytes_per_line +coor_x*4;
    
    for ( ndx = y_1st; ndx <= y_2nd; ndx++)
    {
        ptr_bck = ptr;
       *ptr++ = color.red  ;
       *ptr++ = color.green;
       *ptr++ = color.blue ;
       *ptr   = color.alpha;
        ptr   = ptr_bck +png->bytes_per_line;
    }
}

static void  release_dc( dc_t *dc)
//-------------------------------------------------------------------------------
{
   png_t  *png  = (png_t *)dc;

   if ( NULL == png)             return;
   if ( NULL != png->mapped)     free( png->mapped);
   free( png);
}

static void free_png_resource( png_t *png, FILE *fp)
//-------------------------------------------------------------------------------
// 설명:
{
   release_dc( (dc_t *)png);
   if ( fp)    fclose(fp);
}

void  gx_png_close( dc_t *dc)
//-------------------------------------------------------------------------------
// 설명: PNG 파일 사용 종료
{                 
    release_dc( dc);
}

dc_t  *gx_png_create( int width, int height)
// 설명: 파일없이 PNG 객체를 생성
// 참고: 채널 3과 4를 분리하여 가상 함수 지정
{                   
    png_t   *png      = NULL;
    byte    *image_data = NULL;
    int      image_rowbytes;
                    
    if ( NULL == ( png = malloc( sizeof( png_t))) )
    {
        printf( "gx_png_create() : out of memory.\n");
        free_png_resource( png, 0);
        return NULL;
    }
    memset( png, 0, sizeof( png_t));
    
    png->dc_type        = DCTYPE_PNG;
    png->coor_x         = 0;                                            // LineTo를 위한 좌표를 0으로 초기화
    png->coor_y         = 0;
    png->pen_color      = gx_color( 255, 255, 255, 255);                // 기본 펜 색상은 백색
    png->brush_color    = gx_color(   0,   0,   0, 255);                // 기본 브러쉬 색상은 검정
    png->font_color     = gx_color( 255, 255, 255, 255);                // 기본 글씨 색상은 백색
    png->font           = NULL;
    png->release_dc     = release_dc;                                   // 소멸자 가상 함수
    
    
    png->width          = width;
    png->height         = height;
    png->colors         = 8;                                            // 우선 상수로 처리, bit_depth값을 넣어야
    png->color_type     = 6;                                            // 우선 상수로 처리,
    png->bcolor.alpha   = 255;

    image_rowbytes      = 4 * width;                                    // r,g,b,a각각을 위해 픽셀당 4바이트 지정           
    
    png->bytes_per_line = image_rowbytes;                               // 라인당 바이트 개수
    png->bytes          = image_rowbytes*height;                        // 전체 바이트 개수

    if ( NULL == (image_data = (byte*)malloc(png->bytes)))
    {
        printf( "gx_png_create() : out of memory.\n");
        free_png_resource( png, 0);
        return NULL;
    }
    memset( image_data, 0, png->bytes);                                 // 이미지 영역을 초기화한다.              

    //////////////////////////////////////////////////////////////////////////////
                               
    png->mapped         = image_data;
    png->clear          = ch4_clear;
    png->get_pixel      = ch4_get_pixel;
    png->set_pixel      = ch4_set_pixel;
    png->hline          = ch4_hline;
    png->vline          = ch4_vline;
    png->bits_per_pixel = 4 * 8;                                                // r,g,b,a각각을 위해 픽셀당 4바이트 지정           
    
    return ( dc_t *)png;
}

dc_t *gx_png_open( char *filename)
// 설명: PNG 파일을 읽어 들임
// 참고: 채널 3과 4를 분리하여 가상 함수 지정
{
    static png_t  *png;                                                 // static : might be clobbered by 'longjmp' or 'vfork' warinig 메시지를 없애기 위해
    FILE          *fp;
    
    byte           sig[8];
    png_structp    png_ptr  = NULL;
    png_infop      info_ptr = NULL;
    
    png_uint_32    ihdr_width, ihdr_height;
    int            bit_depth, color_type;
    png_color_16p  pBackground	= NULL;
    
    byte          *image_data = NULL;
    int            image_channels, image_rowbytes;
    double         display_exponent  = 1.0 * 2.2;
    
    double         gamma;
    png_bytepp     row_pointers = NULL;
    int            i;
    
    if ( NULL == ( png = malloc( sizeof( png_t))) )
    {
        printf( "gx_png_open() : out of memory.\n");
        free_png_resource( png, 0);
        return NULL;
    }
    memset( png, 0, sizeof( png_t));
                                                                        
    png->dc_type         = DCTYPE_PNG;
    png->coor_x          = 0;                                           // LineTo를 위한 좌표를 0으로 초기화
    png->coor_y          = 0;
    png->pen_color       = gx_color( 255, 255, 255, 255);               // 기본 펜 색상은 백색
    png->brush_color     = gx_color(   0,   0,   0, 255);               // 기본 브러쉬 색상은 검정
    png->font_color      = gx_color( 255, 255, 255, 255);               // 기본 글씨 색상은 백색
    png->font            = NULL;
    png->release_dc      = release_dc;                                  // 소멸자 가상 함수
    
    
    if ( NULL == ( fp = fopen( filename, "rb")) )                       // 파일이 없거나 열기에 실패했다면
    {
        printf( "gx_png_open() : no file->%s\n", filename);
        free_png_resource( png, fp);
        return NULL;
    }
    fread(sig, 1, 8, fp);
    if ( !png_check_sig( sig, 8))                                       // PNG 시그네쳐 에러
    {
        printf( "gx_png_open() : signature error.\n");
        free_png_resource( png, fp);
        return NULL;
    }
    png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);   // 사용자 에러 정의 핸들러를 모두 NULL로 설정
    if ( !png_ptr)
    {
        printf( "gx_png_open() : out of memory.\n");
        free_png_resource( png, fp);
        return NULL;
    }
    
    info_ptr = png_create_info_struct( png_ptr);
    if ( !info_ptr)
    {
        printf( "gx_png_open() : out of memory.\n");
        free_png_resource( png, fp);
        return NULL;
    }
    
    if ( setjmp( png_jmpbuf(png_ptr)))
    {
        printf( "gx_png_open() : header info error.\n");
        png_destroy_read_struct( &png_ptr, &info_ptr, NULL);
        free_png_resource( png, fp);
        return NULL;
    }
    
    png_init_io( png_ptr, fp);
    png_set_sig_bytes( png_ptr, 8);                                               // 이미 8 바이트의 시그네쳐 값을 읽었으므로 포인터 이동
    png_read_info( png_ptr, info_ptr);                                            // 이미지 데이터 전까지 PNG 정보를 읽어 들임
    
    /* alternatively, could make separate calls to png_get_image_width(),
     * etc., but want bit_depth and color_type for later [don't care about
     * compression_type and filter_type => NULLs] */
    
    png_get_IHDR(png_ptr, info_ptr, &ihdr_width, &ihdr_height, &bit_depth, &color_type, NULL, NULL, NULL);
    png->width        = ihdr_width;
    png->height       = ihdr_height;
    png->colors       = bit_depth;
    png->color_type   = color_type;
    
       //백그라운 칼라를 구한다. 또는 사용자가 직접 지정할 수 도 있지만 여기서는 제외
    
    if ( setjmp( png_jmpbuf( png_ptr))) 
    {
        printf( "gx_png_open() : processing error.\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        free_png_resource( png, fp);
        return NULL;
    }
//  if ( !png_get_valid( png_ptr, info_ptr, PNG_INFO_bKGD))
//  {
//     printf( "fails due to no bKGD chunk\n");
//     png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
//     return;
//  }
    png_get_bKGD(png_ptr, info_ptr, &pBackground);
    
    /* however, it always returns the raw bKGD data, regardless of any
     * bit-depth transformations, so check depth and adjust if necessary */
    
     if( pBackground != NULL )
     {
         if ( bit_depth == 16)
         {
            png->bcolor.red      = pBackground->red   >> 8;
            png->bcolor.green    = pBackground->green >> 8;
            png->bcolor.blue     = pBackground->blue  >> 8;
         }
         else if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
         {
            if      ( bit_depth == 1)  png->bcolor.red   = png->bcolor.green = png->bcolor.blue = pBackground->gray? 255 : 0;
            else if ( bit_depth == 2)  png->bcolor.red   = png->bcolor.green = png->bcolor.blue = (255/ 3) * pBackground->gray;
            else /* bit_depth == 4 */  png->bcolor.red   = png->bcolor.green = png->bcolor.blue = (255/15) * pBackground->gray;
         }
         else
         {
            png->bcolor.red   = (byte)pBackground->red;
            png->bcolor.green = (byte)pBackground->green;
            png->bcolor.blue  = (byte)pBackground->blue;
         }
     }
     png->bcolor.alpha    = 255;
    
    // 이제 이미지 데이터를 읽어 들인다.
    
    /* setjmp() must be called in every function that calls a PNG-reading
     * libpng function */
    
    if  (setjmp( png_jmpbuf(png_ptr)))
    {
        printf( "gx_png_open() : processing error.\n");
        png_destroy_read_struct( &png_ptr, &info_ptr, NULL);
        free_png_resource( png, fp);
        return NULL;
    }
    
    /* expand palette images to RGB, low-bit-depth grayscale images to 8 bits,
     * transparency chunks to full alpha channel; strip 16-bit-per-sample
     * images to 8 bits per sample; and convert grayscale to RGB[A] */
    
    if (color_type == PNG_COLOR_TYPE_PALETTE)
       png_set_expand(png_ptr);
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
       png_set_expand(png_ptr);
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
       png_set_expand(png_ptr);
    if (bit_depth == 16)
       png_set_strip_16(png_ptr);
    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
       png_set_gray_to_rgb(png_ptr);
    
    /* unlike the example in the libpng documentation, we have *no* idea where
     * this file may have come from--so if it doesn't have a file gamma, don't
     * do any correction ("do no harm") */
    
    if ( png_get_gAMA(png_ptr, info_ptr, &gamma))
       png_set_gamma(png_ptr, display_exponent, gamma);
    
    /* all transformations have been registered; now update info_ptr data,
     * get rowbytes and channels, and allocate image memory */
    
    png_read_update_info( png_ptr, info_ptr);
    
    image_rowbytes      = png_get_rowbytes(png_ptr, info_ptr);
    image_channels      = (int)png_get_channels(png_ptr, info_ptr);
    
    png->bytes_per_line = image_rowbytes;                               // 라인당 바이트 개수
    png->bytes          = image_rowbytes*png->height;                   // 전체 바이트 개수
    
    if ( NULL == (image_data = (byte*)malloc(png->bytes)))
    {
        printf( "gx_png_open() : out of memory.\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        free_png_resource( png, fp);
        return NULL;
    }
    if ( NULL == (row_pointers = (png_bytepp)malloc(sizeof( png_bytep)*png->height)))
    {
        printf( "gx_png_open() : out of memory.\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        free(image_data);
        free_png_resource( png, fp);
        return NULL;
    }
    
    /* set the individual row_pointers to point at the correct offsets */
    for (i = 0;  i < png->height; i++)
       row_pointers[i] = image_data + image_rowbytes*i;
    
    /* now we can go ahead and just read the whole image */
    
    png_read_image( png_ptr, row_pointers);
    
    /* and we're done!  (png_read_end() can be omitted if no processing of
     * post-IDAT text/time/etc. is desired) */
    
    free( row_pointers);
    row_pointers = NULL;
    png_read_end( png_ptr, NULL);
    
    //////////////////////////////////////////////////////////////////////////////
    
    png->mapped = image_data;
    if ( image_channels == 3)
    {
       png->clear           = ch3_clear;
       png->get_pixel       = ch3_get_pixel;
       png->set_pixel       = ch3_set_pixel;
       png->hline           = ch3_hline;
       png->vline           = ch3_vline;
       png->bits_per_pixel  = 3 * 8;
    }
    else /* if (image_channels == 4) */
    {
       png->clear           = ch4_clear;
       png->get_pixel       = ch4_get_pixel;
       png->set_pixel       = ch4_set_pixel;
       png->hline           = ch4_hline;
       png->vline           = ch4_vline;
       png->bits_per_pixel  = 4 * 8;
    }
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(fp);
    
    return (dc_t *)png;
}
