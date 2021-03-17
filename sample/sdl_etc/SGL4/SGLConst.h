#ifndef SGLCONST_H
#define SGLCONST_H

#define SGL_CLIP			1

#define SGL_PI				3.141592f
#define SGL_RAD				0.0174533f
#define SGL_ZBUFFER			0
#define SGL_ZDEPTHSORT		1
#define SGL_LIGHT			2
#define SGL_TEXTURE			3
#define SGL_BACKFACE_CULL	4
#define SGL_OBJECT_CULL		5
#define SGL_FRAME			6
#define SGL_FLAT_SHADE		7
#define SGL_GOURAUD_SHADE	8
#define SGL_ALPHABLEND		9
#define SGL_CLIP_3D			10
#define SGL_AMBIENT			11
#define SGL_DIFFUSE			12

//오브젝트 타입
#define SGL_TRIANGLES		3	//정점3개 루프의 인덱스로도 쓰인다.
#define SGL_QUADS			4	//정점4개 루프의 인덱스로도 쓰인다.

//행렬 스택
#define	SGL_MATRIXSTACK_COUNT	2
#define SGL_MODELVIEW			0
#define SGL_PROJECTION			1

//색상
#define SGL_BLACK				SGLColor(0, 0, 0)
#define SGL_GRAY				SGLColor(128, 128, 128)


#endif