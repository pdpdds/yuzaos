#ifndef _GX_BMP_H_
#define _GX_BMP_H_

#include <gx.h>

typedef struct palette_t_ palette_t;
struct palette_t_
{
	unsigned char blue;
	unsigned char green;
	unsigned char red;
	unsigned char filter;
};

typedef struct bmp_t_ bmp_t;
struct bmp_t_                                                           
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
    void (*clear	  )( bmp_t *bmp, color_t color);					// 색으로 전체 칠하기
	void (*get_pixel )( bmp_t *bmp, int coor_x, int coor_y, color_t *color);	// 칼라 값을 읽어 오기
	void (*set_pixel )( bmp_t *bmp, int coor_x, int coor_y, color_t color 	 );	// 점 찍기
    void (*hline	  )( bmp_t *bmp, int x1st  , int x_2nd , int coor_y, color_t color);	// 수평선 긋기
    void (*vline	  )( bmp_t *bmp, int coor_x, int y_1st , int y_2nd , color_t color);	// 수직선 긋기

	int         file_size;                                              // ^  이미지 파일 사이즈
	int         reserved;                                               // |  예약 영역
	int         data_offset;                                            // |
	int         header_size;                                            // |
	int         img_width;                                              // |  이미지 폭
	int         img_height;                                             // |  이미지 높이
	short       cnt_planes;                                             // |  bmp 헤더 영역 52 bytes
	short       bpp;                                                    // |
	int         compression;                                            // |  압축 형식
	int         bitmap_size;                                            // |
	int         hres;                                                   // |
	int         vres;                                                   // |
	int         cnt_colors;                                             // |
	int         important_colors;                                       // v

	int            cnt_palette;                                         // 팔레트 개수
	palette_t     *palette;                                             // 팔레트 칼라 정보
	unsigned char *data;                                                // 이미지 영역의 포인터
	unsigned char *encoded_data;
	unsigned       bsize_blue  , bsize_green  , bsize_red;              // R,G,B 각 색상별 비트 크기
	unsigned       boffset_blue, boffset_green, boffset_red;            // R,G,B 각 색상별 값을 구하기 위한 쉬프트 횟수
};

extern dc_t   *gx_bmp_create( int width, int height, int Depth, int palette_size);// Bitmap 객체 생성
extern dc_t   *gx_bmp_open  ( char  *filename);             	        // Bmp 객체를 생성한 후에, 파일을 오픈
extern void    gx_bmp_close ( dc_t *dc);                              // 파일을 클로즈 및 bmp 객체 소멸
extern bmp_t  *gx_given_bmp_mastering( bmp_t * bmp , int width, int height, int depth, int palette_size);

#endif
