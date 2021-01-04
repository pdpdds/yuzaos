/*******************************************************************************
프로젝트 :  gxLib
버전번호 :  0.0.1
모듈내용 :  bmp, jpg, png 파일을 처리
작성자   :  장길석(jwjwmx@gmail.com)
홈페이지 :  http://forum.falinux.com
변경일자 :
           2011-05-11	작성 시작

*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>                                                              // malloc srand
#include <string.h>                                                              // abs
#include <stdarg.h>

#include <unistd.h>                                                              // open/close
#include <fcntl.h>                                                               // O_RDWR
#include <sys/ioctl.h>                                                           // ioctl
#include <sys/mman.h>                                                            // mmap PROT_
#include <linux/fb.h>

#include <gxpng.h>
#include <gxjpg.h>
#include <gx.h>
#include <gxfile.h>

int gx_open_file( dc_t *dc, char *filename)
//-------------------------------------------------------------------------------
// 설명: 인수로 지정한 filename의 이미지 파일을 화면에 출력
// 참고: BitBlt를 빠르게 처리하기 위해 DC Type를 DCTYPE_SCREEN으로 지정한다.
// 인수: width       버퍼의 폭
//       height      버퍼의 높이
// 반환: 버퍼 Device Context 핸들
{
    dc_t    *bmp;
    dc_t    *png;
    dc_t    *jpg;
    int      sz_filename;
    char     file_ext[4];

    sz_filename = strlen(filename);
    if ( 4 > sz_filename)
    {
        return GXERR_FILE_NAME;
    }


    memcpy( file_ext, filename+strlen(filename)-3, 3);
    file_ext[3] = '\0';

    if ( 0 == strcasecmp( file_ext, "bmp"))
    {
        bmp = gx_bmp_open( filename);
        if ( NULL == bmp)
            printf( "gx_open_file() : %s is not exists.\n", filename);
        else
        {
            gx_bitblt( dc, 0, 0, ( dc_t *)bmp, 0, 0, bmp->width, bmp->height);
            gx_bmp_close( bmp);
        }
    }
    else if ( 0 == strcasecmp( "jpg", file_ext))
    {
        jpg = gx_jpg_open( filename);
        if ( NULL == jpg)
            printf( "gx_open_file() : %s is not exists.\n", filename);
        else
        {
            gx_bitblt( dc, 0, 0, ( dc_t *)jpg, 0, 0, jpg->width, jpg->height);
            gx_jpg_close( jpg);
        }
    }
    else if ( 0 == strcasecmp( "png", file_ext))
    {
        png = gx_png_open( filename);
        if ( NULL == png)
            printf( "gx_open_file() : %s is not exists.\n", filename);
        else
        {
            gx_bitblt( dc, 0, 0, ( dc_t *)png, 0, 0, png->width, png->height);
            gx_png_close( png);
        }
    }
    else
    {
        printf( "gx_open_file() : %s is not valid graphic format.\n", file_ext);
        return GXERR_FILE_NAME;
    }
    return GXERR_NONE;
}




