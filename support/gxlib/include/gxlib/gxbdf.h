#ifndef __GXBDF_H__
#define __GXBDF_H__

/// 폰트를 사용하기 위한 열기를 한다.
extern font_t *gx_open_font( char *font_filename);
/// DC에 폰트를 지정한다.
extern void gx_set_font( dc_t *dc, font_t *font);
/// DC에 문자열을 출력한다. * 주의: coo_y는 문자열 출력의 하단 좌표. 즉, coor_x=좌측 좌표, coor_y=하단 좌표
extern int gx_text_out( dc_t *dc, int coor_x, int coor_y, char *text);

#endif
