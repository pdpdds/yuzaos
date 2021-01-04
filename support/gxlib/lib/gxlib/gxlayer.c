#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gx.h>
#include <gxlayer.h>

#define BUFFER_INC_SIZE     10

static int is_in_invalid_rect( layer_rect_t *lay_rect, layer_rect_t *inv_rect)
{                           
    if ( inv_rect->top     > lay_rect->bottom   )       return GX_FALSE;
    if ( inv_rect->left    > lay_rect->right    )       return GX_FALSE;
    if ( inv_rect->right   < lay_rect->left     )       return GX_FALSE;
    if ( inv_rect->bottom  < lay_rect->top      )       return GX_FALSE;

    return GX_TRUE;
}

dc_t *gx_layer_get_dc( layer_t *layer, int ndx_layer)
{
    layer_item_t   *item;

    if ( layer->count > ndx_layer)
    {
        item    = layer->items+ndx_layer;
        return item->dc;
    }
    else
        return NULL;
}

static int is_no_invalid( layer_t *layer, int chk_left, int chk_top, int chk_right, int chk_bottom)
{
    layer_item_t   *item;
    int             item_right;
    int             item_bottom;
    int             ndx;
    

    for ( ndx = 0; ndx < layer->count; ndx++)
    {
        item    = layer->items+ndx;
        if ( item->invalid)
        {
            item_right  = item->left +item->dc->width;
            item_bottom = item->top  +item->dc->height;

            if (   ( item->left  <= chk_left  ) 
                && ( item->top   <= chk_top   )
                && ( item_right  >= chk_right )
                && ( item_bottom >= chk_bottom) )   return GX_FALSE;
        }
    }            
    return GX_TRUE;
}

void gx_layer_invalid( layer_t *layer, int ndx_layer)
{
    layer_item_t   *item;
                                                           
    if ( layer->count > ndx_layer)
    {
        item            = layer->items+ndx_layer;
        item->invalid   = is_no_invalid( layer, item->left, item->top, item->left+item->dc->width, item->top+item->dc->height);
    }
}

void gx_layer_visible( layer_t *layer, int ndx, int visible)
{
    layer_item_t   *item;

    if ( ndx < layer->count)
    {
        item            = layer->items+ndx;
        if ( item->visible != visible)
        {
            item->visible   = visible;
            gx_layer_invalid( layer, ndx);
        }
    }
}

void gx_layer_redraw( dc_t *dc_target, dc_t *dc_canvas, layer_t *layer)
{
    layer_item_t   *item;
    int             ndx;

    for ( ndx = layer->count-1; 0 <= ndx; ndx--)                        // 제일 하단 레이어부터 그려 나간다.
    {
        item            = layer->items+ndx;
        item->invalid   = GX_FALSE;

        if ( item->visible)
        {
            gx_bitblt( dc_canvas, item->left, item->top, ( dc_t *)item->dc, 0, 0, item->dc->width, item->dc->height);
        }
    }
    if ( NULL != dc_target)
        gx_bitblt( dc_target, 0, 0, dc_canvas, 0, 0, dc_canvas->width, dc_canvas->height);    // 배경 전체를 다시 그린다.
}

void gx_layer_draw( dc_t *dc_target, dc_t *dc_canvas, layer_t *layer)
{
    layer_item_t   *item;
    int             ndx;
    int             ndx_layer;
    int             coor_left;
    int             coor_top;
    int             coor_right;
    int             coor_bottom;
    layer_rect_t    inv_rect;
    layer_rect_t    lay_rect;

    for ( ndx_layer = layer->count-1; 0 <= ndx_layer; ndx_layer--)
    {
        item    = layer->items+ndx_layer;
        if ( item->invalid)
        {
            inv_rect.left   = item->left; 
            inv_rect.top    = item->top;
            inv_rect.right  = inv_rect.left +item->dc->width  -1;
            inv_rect.bottom = inv_rect.top  +item->dc->height -1;

            for ( ndx = layer->count-1; 0 <= ndx; ndx--)                                // 제일 하단 레이어부터 그려 나간다.
            {
                item    = layer->items+ndx;
                if ( item->visible)
                {
                    lay_rect.left    = item->left; 
                    lay_rect.top     = item->top;
                    lay_rect.right   = lay_rect.left +item->dc->width -1;
                    lay_rect.bottom  = lay_rect.top  +item->dc->height-1;
                    
                    if ( is_in_invalid_rect( &lay_rect, &inv_rect))
                    {                                           
                        if ( lay_rect.left   < inv_rect.left  ) coor_left   = inv_rect.left  ;
                        else                                    coor_left   = lay_rect.left  ;
                        if ( lay_rect.top    < inv_rect.top   ) coor_top    = inv_rect.top   ;
                        else                                    coor_top    = lay_rect.top   ;
                        if ( lay_rect.right  < inv_rect.right ) coor_right  = lay_rect.right ;
                        else                                    coor_right  = inv_rect.right ;
                        if ( lay_rect.bottom < inv_rect.bottom) coor_bottom = lay_rect.bottom;
                        else                                    coor_bottom = inv_rect.bottom;
                            
                        gx_bitblt( dc_canvas, coor_left, coor_top, ( dc_t *)item->dc, coor_left -item->left, coor_top -item->top, coor_right-coor_left+1, coor_bottom-coor_top+1);
                    }
                }
            }
        }
    }            
    if ( NULL != dc_target)
    {            
        for ( ndx_layer = layer->count-1; 0 <= ndx_layer; ndx_layer--)
        {
            item    = layer->items+ndx_layer;
            if ( item->invalid)
            {
                item->invalid   = GX_FALSE;
                gx_bitblt( dc_target, item->left, item->top, dc_canvas, item->left, item->top, item->dc->width, item->dc->height);
            }
        }                
    }                        
}

int gx_layer_add( layer_t *layer, dc_t *dc, int left, int top, int visible)
{
    if ( ( layer->count+1) == layer->buffer_count)
    {
        layer->buffer_count += BUFFER_INC_SIZE;
        layer->items        = realloc( layer->items, sizeof( layer_item_t) * layer->buffer_count);
    }
    layer->items[layer->count].dc       = dc;
    layer->items[layer->count].left     = left;
    layer->items[layer->count].top      = top;
    layer->items[layer->count].visible = visible;
    layer->items[layer->count].invalid  = GX_TRUE;

    layer->count++;
    return layer->count-1;
}

void gx_layer_close( layer_t *layer)
{
    layer_item_t   *item;
    int             ndx;

    for ( ndx = 0; ndx < layer->count; ndx++)
    {
        item    = layer->items+ndx;
        gx_release_dc( item->dc);
    }
    free( layer->items);
    free( layer);
}

layer_t *gx_layer_create( void)
{
    layer_t    *layer = NULL;

    layer = malloc( sizeof( layer_t));
    if ( NULL != layer)                                                 // 메모리 구하기에 성공
    {
        layer->items = malloc( sizeof( layer_item_t) *BUFFER_INC_SIZE); // 셀별 아에템 정보를 위한 메모리 확보, BUFFER_INC_SIZE: 버퍼가 모자를 때 한 번에 증가시키는 량
        if ( NULL != layer->items)
        {
            layer->count            = 0;                                // 추가된 아이템 개수 확인
            layer->buffer_count     = BUFFER_INC_SIZE;                  // 아이템을 담을 수 있는 아이템 버퍼 크기
        }
        else
        {
            free( layer);
            layer   = NULL;
            printf( "gx_layer_create() : out of memory.\n");        
        }
    }
    else
    {
        printf( "gx_layer_create() : out of memory.\n");        
    }
    return layer;
}
