#include <stdio.h>
#include <stdlib.h>
#include <gx.h>

#define BUFFER_INC_SIZE     10

int gx_mosaic_draw( dc_t *dc_target, mosaic_t *mosaic,  int index)
{       
    mosaic_item_t  *item;
    int             coor_y, coor_x;
    color_t         color_item;
    color_t         color_table;
    color_t         color;
    
    if ( 0 <= mosaic->selected)
    {
        item = mosaic->items+mosaic->selected;
        gx_bitblt( mosaic->dc_canvas, item->left, item->top, mosaic->dc_fore, item->left, item->top, item->width, item->height);
    }                                       
    mosaic->selected = index;
    item        = mosaic->items+mosaic->selected;
    color_item  = item->color;
    
    for ( coor_y = item->top; coor_y < item->top+item->height; coor_y++)
    {
        for ( coor_x = item->left; coor_x < item->left+item->width; coor_x++)
        {
            gx_get_pixel( mosaic->dc_table, coor_x, coor_y, &color_table);            
            if (    ( color_table.red   == color_item.red   )
                &&  ( color_table.green == color_item.green )
                &&  ( color_table.blue  == color_item.blue  )
               )
            {
                gx_get_pixel( mosaic->dc_back, coor_x, coor_y, &color);            
            }
            else
            {
                gx_get_pixel( mosaic->dc_fore, coor_x, coor_y, &color);            
            }
            gx_set_pixel( mosaic->dc_canvas, coor_x, coor_y, color);            
        }
    }
    gx_bitblt( dc_target, mosaic->left, mosaic->top, mosaic->dc_canvas, 0, 0, mosaic->dc_canvas->width, mosaic->dc_canvas->height);
    return GX_TRUE;
}

int gx_mosaic_add( mosaic_t *mosaic, int left, int top, int width, int height, int color_x, int color_y)
{         
    if ( ( mosaic->count+1) == mosaic->buffer_count)
    {
        mosaic->buffer_count += BUFFER_INC_SIZE;
        mosaic->items        = realloc( mosaic->items, sizeof( mosaic_item_t) * mosaic->buffer_count);        
    }
    mosaic->items[mosaic->count].left   = left;
    mosaic->items[mosaic->count].top    = top;
    mosaic->items[mosaic->count].width  = width;
    mosaic->items[mosaic->count].height = height;
    gx_get_pixel( mosaic->dc_table, color_x, color_y, &mosaic->items[mosaic->count].color);
    mosaic->count++;                

    return GX_TRUE;
}

void gx_mosaic_close( mosaic_t *mosaic)
{         
    gx_release_dc( mosaic->dc_fore);
    gx_release_dc( mosaic->dc_back);
    gx_release_dc( mosaic->dc_table);
    gx_release_dc( mosaic->dc_canvas);
    free( mosaic->items);
    free( mosaic);
}

mosaic_t *gx_mosaic_create ( dc_t *dc_fore, dc_t *dc_back, dc_t *dc_table, int left, int top)
{         
    mosaic_t    *mosaic = NULL;
    
    mosaic = malloc( sizeof( mosaic_t));
    if ( NULL != mosaic)                                                        // 메모리 구하기에 성공
    {
        mosaic->items = malloc( sizeof( mosaic_item_t) *BUFFER_INC_SIZE);       // 셀별 아에템 정보를 위한 메모리 확보
        if ( NULL != mosaic->items)
        {
            mosaic->dc_canvas = gx_get_buffer_dc( dc_fore->width, dc_fore->height);
            if ( NULL != mosaic->dc_canvas)
            {
                mosaic->dc_fore         = dc_fore;
                mosaic->dc_back         = dc_back;
                mosaic->dc_table  = dc_table;
                mosaic->left            = left;
                mosaic->top             = top;                         
                mosaic->count           = 0;                         
                mosaic->buffer_count    = BUFFER_INC_SIZE;
                mosaic->selected        = -1;
                
                gx_bitblt( mosaic->dc_canvas, 0, 0, dc_fore, 0, 0, dc_fore->width, dc_fore->height);    
            }
            else
            {
                free( mosaic);     
                printf( "gx_mosaic_create() : no canvas handle.\n");        
            }                
        }
        else
        { 
            free( mosaic);     
            printf( "gx_mosaic_create() : out of memroy.\n");        
        }    
    }
    else
    {    
        printf( "gx_mosaic_create() : out of memroy.\n");        
    }    
    return mosaic;
}
