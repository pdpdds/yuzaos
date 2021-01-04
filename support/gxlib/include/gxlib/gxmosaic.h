#ifndef _GX_MOSAIC_H_
#define _GX_MOSAIC_H_

typedef struct
{
    int     left;
    int     top;
    int     width;
    int     height;
    
    color_t color;   
} mosaic_item_t;

typedef struct
{
    dc_t           *dc_fore;    
    dc_t           *dc_back;    
    dc_t           *dc_table;
    dc_t           *dc_canvas;
    int             left;
    int             top;
    int             count;
    int             buffer_count;
    int             selected;
    mosaic_item_t  *items;
        
} mosaic_t;

extern mosaic_t *gx_mosaic_create ( dc_t *dc_fore, dc_t *dc_back, dc_t *dc_color_table, int left, int top);
extern int       gx_mosaic_add( mosaic_t *mosaic, int left, int top, int width, int height, int color_x, int color_y);
extern int       gx_mosaic_draw( dc_t *dc_target, mosaic_t *mosaic,  int index);
extern void      gx_mosaic_close( mosaic_t *mosaic);

#endif
