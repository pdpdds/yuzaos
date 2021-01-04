/*******************************************************************************
프로젝트 : gxLib
모듈내용 : gxLib에서 JPEG 이미지 출력
변경일자 : 2008-11-23
작성자   : 푸른수염(ssseo88@chol.com)
수정자   : 장길석( jwjw, jwjwmx@gmail.com)
저작권   : 주석 내용을 변경하지 않는 한 무료 제공
수정내용 :
            - 2009-10-25
                JPG출력을 128 팔레트 사용에서 투루칼라로 출력하도록 수정       
홈페이지 : http://forum.falinux.com
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include "gx.h"
#include "gxbmp.h"
#include "gxjpg.h"
#else
#include <gx.h>
#include <gxbmp.h>
#include <gxjpg.h>
#endif

#include <jpeglib.h>

#include <setjmp.h>

//#define BI_RGB          0
//#define BI_RLE4         1
//#define BI_RLE8         2
//#define BI_BITFIELD     3
//
//typedef unsigned char   uch;
//typedef unsigned short  ush;
//typedef unsigned int    uln;
//typedef unsigned long   ulg;

//typedef struct
//{
//  unsigned char red;
//  unsigned char green;
//   unsigned char blue;
//} rgb_color_struct;

struct ima_error_mgr {
  struct jpeg_error_mgr pub;    /* "public" fields */

  jmp_buf setjmp_buffer;    /* for return to caller */
};

typedef struct ima_error_mgr *ima_error_ptr;


// struct Iterator__                                                            // jwjw : 필요 없어서 주석 처리
// {                                                                            // jwjw : 필요 없어서 주석 처리
//   int               Itx, Ity;                       // Counters              // jwjw : 필요 없어서 주석 처리
//   int               Stepx, Stepy;                                            // jwjw : 필요 없어서 주석 처리
//   unsigned char    *IterImage;                      //  Image pointer        // jwjw : 필요 없어서 주석 처리
//   jpg_t            *ima;                                                     // jwjw : 필요 없어서 주석 처리
// };                                                                           // jwjw : 필요 없어서 주석 처리
// typedef struct Iterator__ Iterator;                                          // jwjw : 필요 없어서 주석 처리

//static int IterItOK ( Iterator* iter )
//{
//  if ( iter->Itx <= iter->ima->width && iter->Ity <= iter->ima->height )
//   return TRUE;
//  else
//   return FALSE;
//}

//static void IterReset( Iterator* iter )
//{
//  iter->IterImage = iter->ima->data;
//  iter->Itx = iter->Ity = 0;
//}

//static int IterNextRow(Iterator* iter )
//{
//  if (++(iter->Ity) >= iter->ima->height) return 0;
//  iter->IterImage += iter->ima->bytes_per_line;
//  return 1;
//}

//static int IterPrevRow(Iterator* iter )                                       // jwjw : 필요 없어서 주석 처리
//{                                                                             // jwjw : 필요 없어서 주석 처리
//  if (--(iter->Ity) < 0) return 0;                                            // jwjw : 필요 없어서 주석 처리
//  iter->IterImage -= iter->ima->bytes_per_line;                               // jwjw : 필요 없어서 주석 처리
//  return 1;                                                                   // jwjw : 필요 없어서 주석 처리
//}                                                                             // jwjw : 필요 없어서 주석 처리
                                                                              
//static void IterUpset(Iterator* iter )                                        // jwjw : 필요 없어서 주석 처리
//{                                                                             // jwjw : 필요 없어서 주석 처리
//  iter->Itx         = 0;                                                      // jwjw : 필요 없어서 주석 처리
//  iter->Ity         = iter->ima->height-1;                                    // jwjw : 필요 없어서 주석 처리
//  iter->IterImage   = iter->ima->data + iter->ima->bytes_per_line*(iter->ima->height-1);  // jwjw : 필요 없어서 주석 처리
//}

////////////////////////// AD - for interlace ///////////////////////////////
//static void IterSetY(Iterator* iter ,int y)
//{
//  if ((y < 0) || (y > iter->ima->height)) return;
//  iter->Ity = y;
//  iter->IterImage = iter->ima->data + iter->ima->bytes_per_line*y;
//}

/////////////////////////////////////////////////////////////////////////////

//static void IterSetRow( Iterator* iter ,unsigned char *buf, int n )           // jwjw : 필요 없어서 주석 처리
//{                                                                             // jwjw : 필요 없어서 주석 처리
//// Here should be bcopy or memcpy                                             // jwjw : 필요 없어서 주석 처리
//  //_fmemcpy(IterImage, (void far *)buf, n);                                  // jwjw : 필요 없어서 주석 처리
//	int i;                                                                      // jwjw : 필요 없어서 주석 처리
//  if (n<0 || n > iter->ima->width )                                           // jwjw : 필요 없어서 주석 처리
//	 n = iter->ima->width;                                                      // jwjw : 필요 없어서 주석 처리
//                                                                              // jwjw : 필요 없어서 주석 처리
//  for (i=0; i<n; i++)                                                         // jwjw : 필요 없어서 주석 처리
//	  iter->IterImage[i] = buf[i];                                              // jwjw : 필요 없어서 주석 처리
//}                                                                             // jwjw : 필요 없어서 주석 처리

//static void IterGetRow(Iterator* iter ,unsigned char *buf, int n)
//{
//  int i;
//  for (i=0; i<n; i++) buf[i] = iter->IterImage[i];
//}

//static unsigned char* IterGet(Iterator* iter )
//{
//  return iter->IterImage;
//}

//static int IterNextByte(Iterator* iter )
//{
//  if (++(iter->Itx )< iter->ima->width)
//   return 1;
//  else
//   if (++(iter->Ity) < iter->ima->height)
//   {
//      iter->IterImage += iter->ima->bytes_per_line;
//      iter->Itx = 0;
//      return 1;
//   } else
//      return 0;
//}
//
//static int IterPrevByte(Iterator* iter )
//{
//  if (--(iter->Itx) >= 0)
//   return 1;
//  else
//   if (--(iter->Ity) >= 0)
//   {
//      iter->IterImage -= iter->ima->bytes_per_line;
//      iter->Itx = 0;
//      return 1;
//   } else
//      return 0;
//}
//
//static int IterNextStep(Iterator* iter )
//{
//  iter->Itx += iter->Stepx;
//  if (iter->Itx < iter->ima->bytes_per_line)
//   return 1;
//  else {
//   iter->Ity += iter->Stepy;
//   if (iter->Ity < iter->ima->height)
//   {
//      iter->IterImage += iter->ima->bytes_per_line;
//      iter->Itx = 0;
//      return 1;
//   } else
//      return 0;
//  }
//}
//
//static int IterPrevStep(Iterator* iter )
//{
//  iter->Itx -= iter->Stepx;
//  if (iter->Itx >= 0)
//   return 1;
//  else {
//   iter->Ity -= iter->Stepy;
//   if (iter->Ity >= 0 && iter->Ity < iter->ima->height)
//   {
//      iter->IterImage -= iter->ima->bytes_per_line;
//      iter->Itx = 0;
//      return 1;
//   } else
//      return 0;
//  }
//}


/*
 * Here's the routine that will replace the standard error_exit method:
 */

void ima_jpeg_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  ima_error_ptr myerr = (ima_error_ptr) cinfo->err;

  char buffer[JMSG_LENGTH_MAX];

  /* Create the message */
  myerr->pub.format_message (cinfo, buffer);

  /* Send it to stderr, adding a newline */
//        AfxMessageBox(buffer);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}



static void CreateGrayColourMap( palette_t* palette , int n )
{
  int i;
  unsigned char g;

  for (i=0; i<n; i++)
  {
    g = (unsigned char)(256*i/n);
    palette[i].blue = palette[i].green = palette[i].red = g;
    palette[i].filter = 0;
  }
}

static void  release_dc( dc_t *dc)
//-------------------------------------------------------------------------------
{
   jpg_t  *jpg  = (jpg_t *)dc;

   if ( NULL == jpg)             return;
   if ( NULL != jpg->palette)    free( jpg->palette);
   if ( NULL != jpg->data   )    free( jpg->data   );
   free( jpg );
}

static void free_jpg_resource( jpg_t *jpg, FILE *fp)
//-------------------------------------------------------------------------------
{
   release_dc( ( dc_t *)jpg );
   if ( fp)    fclose(fp);
}

static void copy_1byte( unsigned char *pdata, unsigned char *buff, int width)
//-------------------------------------------------------------------------------
// 설명: 도트 당 칼라 바이트가 한 개 바이트일 경우 한개 라인씩 복사
{
    int     ndx;

    for ( ndx = 0; ndx < width; ndx++)
    {
        *pdata++    = *buff++;
    }
}

static void copy_3byte( unsigned char *pdata, unsigned char *buff, int width)
//-------------------------------------------------------------------------------
// 설명: 도트 당 칼라 바이트가 세 개 바이트일 경우 한개 라인씩 복사
{
    char    r_color;
    char    g_color;
    char    b_color;
    int     ndx;

    for ( ndx = 0; ndx < width; ndx++)
    {
        b_color     = *buff++;
        g_color     = *buff++;
        r_color     = *buff++;
        *pdata++    = r_color;
        *pdata++    = g_color;
        *pdata++    = b_color;
    }
}

void  gx_jpg_close( dc_t *dc)
//-------------------------------------------------------------------------------
// 설명: PNG 파일 사용 종료
{
   release_dc( dc);
}

dc_t *gx_jpg_open( char  *filename )
{
    static jpg_t   *jpg;                                                // static : might be clobbered by 'longjmp' or 'vfork' warinig 메시지를 없애기 위해
    static FILE    *fp      = NULL;                                     // static : might be clobbered by 'longjmp' or 'vfork' warinig 메시지를 없애기 위해
//    Iterator        iter;                                             // jwjw : 필요 없어서 주석 처리
    struct          jpeg_decompress_struct cinfo;
    struct          ima_error_mgr jerr;
    JSAMPARRAY      buffer;
    int             row_stride;
    unsigned char  *pbmp_buffer;
    void          (*copy_bitmap)( unsigned char *pdata, unsigned char *buff, int width);

    if ( NULL == ( jpg = malloc( sizeof( jpg_t))) )
    {                    
        printf( "gx_jpg_open() : out of memory.\n");        
        free_jpg_resource( jpg, fp);
        return NULL;
    }
    memset( jpg, 0, sizeof( jpg_t));

//    memset( &iter, 0x00, sizeof(Iterator));                           // jwjw : 필요 없어서 주석 처리
//    iter.ima = jpg;                                                   // jwjw : 필요 없어서 주석 처리

    fp = fopen((const char *)filename, "rb");
    if ( NULL == ( fp) )                                                // 파일이 없거나 열기에 실패했다면
    {
        printf( "gx_jpg_open() : no file.\n");        
        free_jpg_resource( jpg, fp);
        return NULL;
    }
    cinfo.err           = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = ima_jpeg_error_exit;

    if ( setjmp(jerr.setjmp_buffer))
    {
        printf( "gx_jpg_open() : processing error.\n");        
        free_jpg_resource( jpg, fp);
        jpeg_destroy_decompress(&cinfo);
        
        return NULL;
    }
    jpeg_create_decompress( &cinfo);
    jpeg_stdio_src( &cinfo, fp);                                        
    jpeg_read_header( &cinfo, TRUE);

    if ( JCS_GRAYSCALE == cinfo.jpeg_color_space)                       // jwjw: Gray Scale 이미지일 경우 메모리 크기를 줄여 저장하기 위해 팔레트를 생성한다
    {
        cinfo.quantize_colors   = TRUE;                                 // jwjw: TRUE - 팔레트를 생성
        cinfo.out_color_space   = JCS_GRAYSCALE;                        // jwjw: 도트당 칼라값이 한 개의 바이트로 지정하도록. cinfo.output_components 는 1 이됨
        jpeg_calc_output_dimensions( &cinfo);                           // jwjw:
        cinfo.desired_number_of_colors = 256;                           // jwjw: 256 - TRUE Color 또는 Direct Color 로 구한다

        jpeg_start_decompress( &cinfo);                                         
        gx_given_bmp_mastering((bmp_t*)jpg, cinfo.image_width, cinfo.image_height, 8*cinfo.output_components, cinfo.actual_number_of_colors);
        CreateGrayColourMap( jpg->palette ,256);                                
    }
    else                                                                // 칼라 이미지일 경우 팔레트를 생성하지 않고 True Color로 출력
    {
        cinfo.quantize_colors   = FALSE;                                // jwjw: FALSE - 팔레트를 생성 하지 않게
        cinfo.out_color_space   = JCS_RGB;                              // jwjw:
        jpeg_calc_output_dimensions( &cinfo);                           // jwjw:
        cinfo.desired_number_of_colors = 256;                           // jwjw: 256 - TRUE Color 또는 Direct Color 로 구한다

        jpeg_start_decompress( &cinfo);                                         
        gx_given_bmp_mastering( (bmp_t*)jpg, cinfo.image_width, cinfo.image_height, 8*cinfo.output_components, cinfo.actual_number_of_colors);
//        SetPalette( jpg->palette, cinfo.actual_number_of_colors, cinfo.colormap[0], cinfo.colormap[1], cinfo.colormap[2]);    // jwjw: 칼라에서 팔레트를 사용하지 않게 하기 위해 주석 처리
    }
    jpg->dc_type        = DCTYPE_JPG;                                   // DC 형태를 JPEG으로
    jpg->coor_x         = 0;                                            // LineTo를 위한 좌표를 0으로 초기화
    jpg->coor_y         = 0;                                            
    jpg->pen_color      = gx_color( 255, 255, 255, 255);                // 기본 펜 색상은 백색
    jpg->brush_color    = gx_color(   0,   0,   0, 255);                // 기본 브러쉬 색상은 검정
    jpg->font_color     = gx_color( 255, 255, 255, 255);                // 기본 글씨 색상은 백색
    jpg->font           = NULL;
    jpg->release_dc     = release_dc;                                   // 소멸자 가상 함수
    row_stride          = cinfo.output_width * cinfo.output_components;
    buffer              = ( *cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);

//    IterUpset(&iter);                                                 // jwjw : 필요 없어서 주석 처리

    switch( cinfo.output_components)
    {                
    case 1 :    copy_bitmap = copy_1byte;   break;                      // jwjw : 토트 당 칼라 값이 한 개의 바이트일 때를 지정
    case 3 :    copy_bitmap = copy_3byte;   break;                      // jwjw : 토트 당 칼라 값이 세 개의 바이트일 때를 지정
    default:    copy_bitmap = copy_1byte;                               // jwjw : 이외는 에러가 발생하지 않도록 함수 포인터는 지정
                printf( "JPEG ERROR : no assigned copy bitmap function.\n");    // jwjw : 함수 포인터가 제대로 지정되지 않았음을 에러 출력
                break;
    }
    
    pbmp_buffer = jpg->data + jpg->bytes_per_line *jpg->height;         // jwjw : gx_given_bmp_mastering()에서 생성된 bmp_t 메모리에 JPEG 이미지를 복사 준비
    while ( cinfo.output_scanline < cinfo.output_height)
    {
        jpeg_read_scanlines( &cinfo, buffer, 1);
        pbmp_buffer -= row_stride;                                      // jwjw : bitmap의 이미지 저장 순서는  
        copy_bitmap( pbmp_buffer, buffer[0], jpg->width);

//        IterSetRow( &iter, buffer[0], row_stride);                    // jwjw : 필요 없어서 주석 처리
//        IterPrevRow( &iter );                                         // jwjw : 필요 없어서 주석 처리
    }

    jpeg_finish_decompress( &cinfo);
    jpeg_destroy_decompress( &cinfo);

    fclose( fp);
    
    return ( dc_t *)jpg;
}
