#ifndef __GX_PANEL_H__
#define __GX_PANEL_H__

typedef struct panel_t_ panel_t;

typedef struct
{
    dc_t           *dc;             // 레이어가 가지고 있는 Device Context
    int             left;           // 레이어의 위치
    int             top;            // 레이어의 위치
    int             visibled;       // 레이어 출력 여부
} panel_item_t;

struct panel_t_
{                           
    dc_t           *dc_output;      // 출력 대상 Device Context
    dc_t           *dc_canvas;      // 레이어의 자체 출력 Device Context
    dc_t           *dc_backgnd;     // 레이어의 자체 출력 Device Context
    panel_item_t   *items;          // 레이어 아이템 목록
    int             left;           // 레이어가 dc_output에서의 위치
    int             top;            // 레이어가 dc_output에서의 위치
    int             width;          // 레이어의 폭
    int             height;         // 레이어의 높이
    int             inv_left;       // 갱신 필요 영역 left
    int             inv_top;        // 갱신 필요 영역 top
    int             inv_right;      // 갱신 필요 영역 right
    int             inv_bottom;     // 갱신 필요 영역 bottom
    int             count;          // 추가된 레이어 개수    
    int             buffer_count;   // 레이어를 추가하기 위한 버퍼의 실제 크기
    int             manual_refresh; // 레이어가 변동이 있을 때, 자동으로 화면으로 출력할지의 여부
};

// 레이어를 생성
extern panel_t  *gx_panel_create ( dc_t *dc, int left, int top, int width, int height);
// 레이어를 소멸
extern void      gx_panel_close( panel_t *panel);
// 새로운 레이어를 추가
extern int       gx_panel_add( panel_t *panel, dc_t *dc, int left, int top, int visibled);
// 전체 레이어를 화면에 출력
extern void      gx_panel_redraw( panel_t *panel);
// 특정 레이어의 visible 속성을 변경
extern void      gx_panel_visible( panel_t *panel, int ndx, int visibled);
// 특정 레이어의 위치를 더하기 값으로 이동
extern void      gx_panel_move( panel_t *panel, int ndx, int inc_x, int inc_y);
// 특정 레이어의 위치를 지정된 좌표로 이동
extern void      gx_panel_move_to( panel_t *panel, int ndx, int coor_x, int coor_y);
// 레이어 출력을 수동 또는 자도으로 변경
extern void      gx_panel_manual( panel_t *panel, int manual);

#endif
