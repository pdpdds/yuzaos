#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gx.h>
#include <gxpng.h>
#include <gximagelist.h>

image_list_t *gx_imagelist_create( char  *filename, int width, int height)
// 설명: png 파일을 이용하여 이미지 리스트 생성
{
    image_list_t   *list = NULL;
    dc_t           *png_sour;
    int             coor_x  , coor_y;
    int             wid_sour, hgt_sour;
    int             cnt_x, cnt_y;
    int             ndx;

    if  ( !( png_sour = gx_png_open( filename)))
        return NULL;
    else
    {
        cnt_x   = png_sour->width  / width;
        cnt_y   = png_sour->height / height;
        list    = malloc( sizeof( image_list_t));
        list->count = cnt_x *cnt_y;
        list->array = malloc( sizeof( png_t *) *list->count);

        coor_y    = 0;
        wid_sour  = png_sour->width;
        hgt_sour  = png_sour->height;

        ndx = 0;
        while( (coor_y+height) <= hgt_sour)
        {
            coor_x      = 0;
            while( (coor_x+width) <= wid_sour)
            {
                list->array[ndx] = gx_png_create( width, height);
                gx_bitblt( list->array[ndx], 0, 0, png_sour, coor_x, coor_y, width, height);
                coor_x += width;
                ndx++;
            }
            coor_y += height;
        }
        gx_png_close( png_sour);
    }
    return  list;
}

dc_t *gx_imagelist_get( image_list_t *list, int index)
// 설명: 인덱스 번호에 해당하는 이미지 파일 반환
{
    if ( index < list->count)
        return list->array[index];
    else
        return NULL;
}

void gx_imagelist_close( image_list_t *list)
// 설명: 이미지 리스트 객체 소멸
{
    int     ndx;

    if ( NULL != list)
    {
        for ( ndx = 0; ndx < list->count; ndx++)
            gx_png_close( gx_imagelist_get( list, ndx));
        free( list);
    }
}
