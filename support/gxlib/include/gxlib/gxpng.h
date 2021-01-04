#ifndef _GX_PNG_H_
#define _GX_PNG_H_

#include <gx.h>

typedef struct png_t_ png_t;
struct png_t_
{                  
    char        dc_type;                                                // DC의 형태로 Screen, Bitmap을 구분한다.
    int         width;                                                  // 도트 단위의 폭
    int         height;                                                 // 도트 단위의 높이
    int         dots;                                                   // 전체 도트 갯수 width * height
    int         bytes;                                                  // 메모리의 전체 Byte 크기
    int         colors;                                                 // 칼라 깊이
    int         bytes_per_line;                                         // 라인당 바이트 개수
    int         bits_per_pixel;                                         // 비트당 픽셀 개수
    int         coor_x;                                                 // 이전에 그리기 했던 마지막 좌표
    int         coor_y;                                                 // 이전에 그리기 했던 마지막 좌표
    color_t     pen_color;                                              // 현재의 펜 칼라
    color_t     brush_color;                                            // 현재의 브러쉬 칼라
    color_t     font_color;                                             // 글씨 색상          
    font_t     *font;                                                   // 문자열 출력을 위한 글꼴
    void       *mapped;                                                 // 메모리 매핑된 포인터
    
    void (*release_dc)( dc_t *dc);                                      // Device Context 소멸 및 관련 메모리를 삭제
    void (*clear    )( png_t *png, color_t color);                      // 색으로 전체 칠하기
    void (*get_pixel)( png_t *png, int coor_x, int coor_y, color_t   *color );   // 칼라 값을 읽어 오기
    void (*set_pixel)( png_t *png, int coor_x, int coor_y, color_t    color );   // 점 찍기
    void (*hline    )( png_t *png, int x1st  , int x_2nd , int coor_y, color_t color);// 수평선 긋기
    void (*vline    )( png_t *png, int coor_x, int y_1st , int y_2nd , color_t color);// 수직선 긋기
    
    color_t  bcolor;                                                    // 배경 칼라 중 빨강
    int   color_type;
};

extern dc_t   *gx_png_open  ( char  *filename);             	        // png 객체를 생성한 후에, 파일을 오픈
extern dc_t   *gx_png_create( int   width, int height);     	        // png 객체를 파일없이 생성
extern void    gx_png_close ( dc_t *dc);                                // 파일을 클로즈 및 png 객체 소멸

#endif
