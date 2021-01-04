#ifndef _GX_IMAGE_LIST_H_
#define _GX_IMAGE_LIST_H_

#include <gx.h>
#include <gxpng.h>

typedef struct image_list_t_ image_list_t;
struct image_list_t_
{
   int   count;
   
   dc_t **array;
};

extern image_list_t  *gx_imagelist_create( char  *filename, int width, int height); // png 파일을 이용하여 이미지 리스트 생성
extern dc_t *gx_imagelist_get( image_list_t *list, int index);                 // 인덱스 번호에 해당하는 이미지 파일 반환
extern void gx_imagelist_close( image_list_t *list);                            // 이미지 리스트 객체 소멸

#endif
