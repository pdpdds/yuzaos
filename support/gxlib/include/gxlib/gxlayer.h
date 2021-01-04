#ifndef _GX_LAYER_H_
#define _GX_LAYER_H_

typedef struct layer_t_ layer_t;

typedef struct
{
    dc_t           *dc;             // 레이어가 가지고 있는 Device Context
    int             left;           // 레이어의 위치
    int             top;            // 레이어의 위치
    int             visible;        // 레이어 출력 여부
    int             invalid;        // 화면 갱신 영역
} layer_item_t;

struct layer_t_
{                           
    layer_item_t   *items;          // 레이어 아이템 목록
    int             count;          // 추가된 레이어 개수    
    int             buffer_count;   // 레이어를 추가하기 위한 버퍼의 실제 크기
};

typedef struct 
{                           
    int     left;
    int     top;
    int     right;
    int     bottom;
} layer_rect_t;


// 레이어를 생성
extern layer_t  *gx_layer_create( void);
// 레이어를 소멸
extern void      gx_layer_close( layer_t *layer);
// 새로운 레이어를 추가
extern int       gx_layer_add( layer_t *layer, dc_t *dc, int left, int top, int visible);
// 특정 레이어의 visible 속성을 변경
extern void      gx_layer_visible( layer_t *layer, int ndx, int visible);
// 레이어 전체를 지정 DC에 출력
extern void      gx_layer_redraw( dc_t *dc_target, dc_t *dc_canvas, layer_t *layer);
// 레이어 전체를 지정 DC에 출력
extern void      gx_layer_draw( dc_t *dc_target, dc_t *dc_canvas, layer_t *layer);
// 레이어에 대한 DC를 구한다.
extern dc_t     *gx_layer_get_dc( layer_t *layer, int ndx_layer);
// 레이어에 대해 갱신 영역으로 지정한다.
extern void      gx_layer_invalid( layer_t *layer, int ndx_layer);

#endif
