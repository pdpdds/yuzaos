/*
@file     main.c
@date     2012/01/09
@author   장길석 jks@falinux.com  FALinux.Co.,Ltd.
@brief    ./images에 있는 모든 이미지 파일을 화면에 출력
@todo
@bug
@remark
@warning

저작권    에프에이리눅스(주)
          외부공개 금지

 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <gx.h>
#include <gxjpg.h>
#include <gxpng.h>

/** ----------------------------------------------------------------------------
@brief  버퍼 크기 정의
@remark
 -----------------------------------------------------------------------------*/
#define MAX_BUFF_SIZE   1024
#define MAX_DEV_NANE    100

/** ----------------------------------------------------------------------------
@brief  지역 변수 정의
@remark
 -----------------------------------------------------------------------------*/
static dc_t         *dc_screen;                                         // 화면 Device Context
static dc_t         *dc_buffer;                                         // 화면 깜빡임을 피하기 위한 버퍼 DC
static font_t 		*hfont_1;
static font_t 		*hfont_2;

static char  buff[MAX_BUFF_SIZE];                                       // 1차 함수에서만 사용할 수 있는 버퍼
static char  fb_dev_name[MAX_DEV_NANE];

#define	VIEW_LEFT	    100
#define	VIEW_TOP	    50
#define	VIEW_RIGHT	    500
#define VIEW_BOTTOM	    200
#define	VIEW_WIDTH	    ( VIEW_RIGHT  	- VIEW_LEFT)
#define VIEW_HEIGHT	    ( VIEW_BOTTOM	- VIEW_TOP )

static  int ndx_counter = 0;

static void display_text( void){

    // 특정 지역 특정 색으로 채움
	gx_clear_area( dc_buffer, VIEW_LEFT, VIEW_TOP, VIEW_RIGHT, VIEW_BOTTOM, gx_color( 0, 0, 0, 255));

	// counter 출력
    dc_buffer->font = hfont_2;
    dc_buffer->font_color   = gx_color(    0, 255, 255, 255);
    gx_text_out( dc_buffer, VIEW_LEFT, VIEW_TOP+50, "counter=");

    sprintf( buff, "%d", ndx_counter++);
	dc_buffer->font	= hfont_1;
	dc_buffer->font_color	= gx_color( 255,    0, 255, 255);
	gx_text_out( dc_buffer, VIEW_LEFT+190, VIEW_TOP+40, buff);

	// falinux 문자열 출력

    if ( ndx_counter % 5){
        dc_buffer->font = hfont_2;
        dc_buffer->font_color   = gx_color( 255, 255, 255, 255);
        gx_text_out( dc_buffer, VIEW_LEFT, VIEW_TOP+120, "www.falinux.com");
    }
    else {
        dc_buffer->font = hfont_1;
        dc_buffer->font_color   = gx_color( 255, 255, 0  , 255);
        gx_text_out( dc_buffer, VIEW_LEFT, VIEW_TOP+120, "---------------");
    }

    gx_bitblt( dc_screen, VIEW_LEFT, VIEW_TOP, dc_buffer, VIEW_LEFT, VIEW_TOP, VIEW_WIDTH, VIEW_HEIGHT);
}


/** ----------------------------------------------------------------------------
@brief  main()
@remark -
@param  -
@return -
 -----------------------------------------------------------------------------*/
int   main  ( int argc, char *argv[]){

    if( 2 > argc){                                                      //  인수가 없다면 Frame Buffer의 기본 Device Name은 /dev/fb
        strcpy( fb_dev_name, "/dev/fb");
    } else {
        strcpy( fb_dev_name, argv[1]);
    }

    if  ( GX_SUCCESS != gx_open( fb_dev_name)           )   return 1;   // gxLib 초기화
    if  ( NULL == ( dc_screen = gx_get_screen_dc() ) 	)   return 1;   // 화면 출력을 위한 screen DC 구함
    if  ( NULL == ( dc_buffer
                    = gx_get_compatible_dc( dc_screen) ))   return 1;   // 화면 깜빡임을 없애기 위한 버퍼 DC

    gx_clear( dc_screen, gx_color( 0, 0, 0, 255));
    gx_clear( dc_buffer, gx_color( 0, 0, 0, 255));

    printf( "font loading\n");
    if ( NULL == ( hfont_1 = gx_open_font( "gulim12.bdf")) )   return 1;
    if ( NULL == ( hfont_2 = gx_open_font( "nbold32.bdf")) )   return 1;

    printf( "running....\n");
    printf( "screen widht= %d\n"      , dc_screen->width);              // 화면 폭과 넓이를 출력
    printf( "screen color depth= %d\n", dc_screen->colors);

    while( 1){
        display_text();
        usleep( 100 * 1000);                                            // 200 msec 대기
    }

    gx_release_dc( dc_screen);
    gx_close();

    return   0;
}
