/*
 * gxframe_s3c6410.c
 *
 *  Created on: 2011. 5. 12.
 *      Author: jwjw
 *      참고 파일 : \\eabi\staff\freefrug\kernel\linux-2.6.29\drivers\video\samsung\s3cfb_fimd4x.c
 *      참고 파일 : \\eabi\staff\freefrug\kernel\linux-2.6.29\drivers\video\samsung\s3cfb.h
 */

#include <stdio.h>
#include <stdlib.h>                                                              // malloc srand
#include <string.h>                                                              // abs
#include <stdarg.h>

#include <unistd.h>                                                              // open/close
#include <fcntl.h>                                                               // O_RDWR
#include <sys/ioctl.h>                                                           // ioctl
#include <sys/mman.h>                                                            // mmap PROT_
#include <linux/fb.h>

#include <gx.h>
#include <gxframe.h>

//typedef struct {
//  unsigned int colval_red;
//  unsigned int colval_green;
//  unsigned int colval_blue;
//} s3cfb_color_val_info_t;


//  color_val_info.colval_red   = 0x1f;
//  color_val_info.colval_green = 0;
//  color_val_info.colval_blue  = 0;

//  ioctl( fb_1->fd, _IOW( 'F', 305, s3cfb_color_val_info_t), &color_val_info);                     // Color Key Setup

//printf( "color_val_info.colval_red  =%d\n", color_val_info.colval_red  );
//printf( "color_val_info.colval_green=%d\n", color_val_info.colval_green);
//printf( "color_val_info.colval_blue =%d\n", color_val_info.colval_blue );

typedef struct {
    int direction;
    unsigned int compkey_red;
    unsigned int compkey_green;
    unsigned int compkey_blue;
} s3cfb_color_key_info_t;


//-------------------------------------------------------------------------------
// 설명   v_sync 타임을 기다린다.
// 인수   a_filedesc  : 화면 출력 파일 디스크립터
// 반환   음수 : 실행 오류. 대표적인 예) -EDEFAULT( -14)
static int wait_v_sync( int a_filedesc){

    int n_arg = 0;

    return ioctl( a_filedesc, _IOW( 'F', 32, unsigned int), &n_arg);                             // FBIO_WAITFORVSYNC
}
//-------------------------------------------------------------------------------
// 설명: 지정한 프레임을 화면에 출력
// 인수: a_fb : 화면에 출력할 프레임 버퍼
int gx_show_frame( frame_buffer_t   *a_fb)
{
    if ( 0 > wait_v_sync( a_fb->fd) ){
        printf( "wait v sync error\n");
        return -1;
    }
    ioctl( a_fb->fd, _IO( 'F', 201), NULL);                                     // OSD Start
    return 0;
}

//-------------------------------------------------------------------------------
// 설명: 프레임을 화면에서 감춤
// 인수: a_fb : 화면에 출력할 프레임 버퍼
int gx_hide_frame( frame_buffer_t   *a_fb)
{
    if ( 0 > wait_v_sync( a_fb->fd) ){
        printf( "wait v sync error\n");
        return -1;
    }
    ioctl(a_fb->fd, _IO('F', 202), NULL); // OSD Stop
    return 0;
}

//-------------------------------------------------------------------------------
// 설명: 프레임에서 색상으로 특정 영역을 투명하게 처리 시작
// 인수: a_fb : 화면에 출력할 프레임 버퍼
int gx_start_frame_trans( frame_buffer_t *a_fb)
{
    if ( 0 > wait_v_sync( a_fb->fd) ){
        printf( "wait v sync error\n");
        return -1;
    }
    ioctl( a_fb->fd, _IO( 'F', 300), NULL);                                 // Color Key Stop
    return 0;
}

//-------------------------------------------------------------------------------
// 설명: 프레임의 투명 영역 처리를 중지
// 인수: a_fb : 화면에 출력할 프레임 버퍼
int gx_stop_frame_trans( frame_buffer_t *a_fb)
{
    if ( 0 > wait_v_sync( a_fb->fd) ){
        printf( "wait v sync error\n");
        return -1;
    }
    ioctl( a_fb->fd, _IO( 'F', 301), NULL);                                 // Color Key Stop
    return 0;
}
//-------------------------------------------------------------------------------
// 설명: 프레임의 일부 영역을 특정 색으로 투명 영역을 지정
// 인수: a_fb : 화면에 출력할 프레임 버퍼
// 인수: a_color  : 투명하게 지정할 색상
int gx_set_frame_trans_color( frame_buffer_t *a_fb, color_t a_color)
{
    s3cfb_color_key_info_t  color_key_info;

    color_key_info.direction        = 0;
    switch( a_fb->colors)
    {
    case 16 :
        color_key_info.compkey_red      = a_color.red   >> 3;
        color_key_info.compkey_green    = a_color.green >> 2;
        color_key_info.compkey_blue     = a_color.blue  >> 3;
        break;
    default :
        color_key_info.compkey_red      = a_color.red;
        color_key_info.compkey_green    = a_color.green;
        color_key_info.compkey_blue     = a_color.blue;
        break;
    }

    if ( 0 > wait_v_sync( a_fb->fd) ){
        printf( "wait v sync error\n");
        return -1;
    }
    ioctl( a_fb->fd, _IOW( 'F', 304, s3cfb_color_key_info_t), &color_key_info); // Color Key Setup

    return gx_start_frame_trans( a_fb);
}
