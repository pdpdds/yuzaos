#include "SGLRaster.h"
#include "SGLColor.h"
#include "SGLDevice.h"
#include "SGLConst.h"
#include "SGLState.h"
#include "SGLGraphicsPipeline.h"
#include "SGLTexture.h"
#include "SGLObject.h"
#include <SDL.h>
#include "SGLFaceSet.h"
#include <limits.h>
#include <math.h>

#define ADDRESS(x, y) (((unsigned char *)sglDevice.window->pixels + (x*sglDevice.windowDepth>>3) + (y*sglDevice.window->pitch)));
//삼각형 넓이
//Z 성분이 0 인 두 벡터의 외적
// | (x2 - x1)  (y2 - y1)   0 |
// | (x3 - x1)  (y3 - y1)   0 |
#define TRIAREA(x1, y1, x2, y2, x3, y3) ((x2 - x1) * (y3 - y1) - (x3 - x1) * (y2 - y1)) 

template<class T>
void swap(T& a, T& b)
{
	T c = a;
	a = b;
	b = c;
}
void sglClearColor(const SGLColor& color)
{
	sglClearSurface(color);
	if(sglGetState(SGL_ZBUFFER))
	{
		sglClearZBuffer(INT_MAX);
	}
}

void sglDrawPoint(int x, int y, const SGLColor& color)
{
#if SGL_CLIP
	//printf("(%d, %d)\n", x ,y);
	if( x < 0 || x >= sglDevice.windowWidth )
		return ;
	if( y < 0 || y >= sglDevice.windowHeight)
		return ;
#endif
	SDL_LockSurface(sglDevice.window);
		unsigned char* pixel = ADDRESS(x, y);
	if(sglGetState(SGL_ALPHABLEND))
	{
		unsigned char D = 255 - color.A;
		pixel[0] = (color.B*color.A+pixel[0]*D)>>8;
		pixel[1] = (color.G*color.A+pixel[1]*D)>>8;
		pixel[2] = (color.R*color.A+pixel[2]*D)>>8;
	}
	else
	{
		pixel[0] = color.B;
		pixel[1] = color.G;
		pixel[2] = color.R;
	}
	SDL_UnlockSurface(sglDevice.window);	
}

void sglDrawPointZ(int x, int y, float z, const SGLColor& color)
{
#if SGL_CLIP
	//printf("(%d, %d)\n", x ,y);
	if( x < 0 || x >= sglDevice.windowWidth )
		return ;
	if( y < 0 || y >= sglDevice.windowHeight)
		return ;
#endif
	if(sglDevice.zbuffer[x + y*sglDevice.windowWidth] == INT_MAX || (z*1000) < sglDevice.zbuffer[x + y*sglDevice.windowWidth])
	{
		SDL_LockSurface(sglDevice.window);
			unsigned char* pixel = ADDRESS(x, y);
		if(sglGetState(SGL_ALPHABLEND))
		{
			unsigned char D = 255 - color.A;
			pixel[0] = (color.B*color.A+pixel[0]*D)>>8;
			pixel[1] = (color.G*color.A+pixel[1]*D)>>8;
			pixel[2] = (color.R*color.A+pixel[2]*D)>>8;
		}
		else
		{
			pixel[0] = color.B;
			pixel[1] = color.G;
			pixel[2] = color.R;
		}
		SDL_UnlockSurface(sglDevice.window);	
	}
}

int sglClipLine(int* x1, int* y1, int* x2, int* y2)
{
	int point_1 = 0; int point_2 = 0;
	int clip_always = 0;
	int xi, yi;

	int right_edge = 0,
		left_edge = 0,
		top_edge = 0,
		bottom_edge = 0;
	int success = 0;

	float dx, dy;

	int clip_min_x = 0;
	int clip_max_x = sglDevice.windowWidth-1;
	int clip_min_y = 0;
	int clip_max_y = sglDevice.windowHeight-1;

	if( (*x1>=clip_min_x) && (*x1<=clip_max_x) &&
		(*y1>=clip_min_y) && (*y1<=clip_max_y))
		point_1 = 1;

	if( (*x2>=clip_min_x) && (*x2<=clip_max_x) &&
		(*y2>=clip_min_y) && (*y2<=clip_max_y))
		point_2 = 1;

	if(point_1 == 1 && point_2 == 1)
		return 1; //두 점다 영역 안에 있다

	if(point_1 == 0 && point_2 == 0)
	{//두 점 모두 영역 밖에 있을 때
		if( ((*x1<clip_min_x) && (*x2<clip_min_x)) ||
			((*x1>clip_max_x) && (*x2>clip_max_x)) ||
			((*y1<clip_min_y) && (*y2<clip_min_y)) ||
			((*y1>clip_max_y) && (*y2>clip_max_y)) )
		{
			return 0; //그릴 필요가 없음
		}
		//선이 화면에 걸쳐 있을 때
		clip_always = 1;
	}

	if((point_1 == 1) || (point_1==0 && point_2 == 0) )
	{//끝점이 영역 안에 있을 때
		dx = *x2 - *x1;
		dy = *y2 - *y1;

		if(*x2 > clip_max_x)
		{
			right_edge = 1;
			
			if(dx != 0)
				yi = (int)(0.5 + (dy/dx) * (clip_max_x - *x1) + *y1);
			else
				yi = -1;
		}
		else
		if(*x2 < clip_min_x)
		{
			left_edge = 1;

			if(dx!=0)
				yi = (int)(0.5 + (dy/dx) * (clip_min_x - *x1) + *y1);
			else
				yi = -1;
		}

		if(*y2 > clip_max_y)
		{
			bottom_edge = 1;

			if(dy!=0)
				xi = (int)(0.5 + (dx/dy) * (clip_max_y - *y1) + *x1);
			else
				xi = -1;
		}
		else
		if(*y2 < clip_min_y)
		{
			top_edge = 1;
			if(dy!=0)
				xi = (int)(0.5 + (dx/dy) * (clip_min_y - *y1) + *x1);
			else
				xi = -1;
		}

		if(right_edge == 1 && (yi>=clip_min_y && yi<=clip_max_y))
		{
			*x2 = clip_max_x;
			*y2 = yi;
			success = 1;
		}
		else
		if(left_edge==1 && (yi>=clip_min_y && yi<=clip_max_y))
		{
			*x2 = clip_min_x;
			*y2 = yi;
			success = 1;
		}
		
		if(bottom_edge==1 && (xi>=clip_min_x && xi<=clip_max_x))
		{
			*x2 = xi;
			*y2 = clip_max_y;
			success = 1;
		}
		else
		if(top_edge==1 && (xi>=clip_min_x && xi<=clip_max_x))
		{
			*x2 = xi;
			*y2 = clip_min_y;
			success = 1;
		}
	}

	right_edge = left_edge = top_edge = bottom_edge = 0;

	if ( (point_2==1) || (point_1==0 && point_2==0))
	{
		dx = *x1 - *x2;
		dy = *y1 - *y2;

		if (*x1 > clip_max_x)
		{
			right_edge = 1;
			if (dx!=0)
				yi = (int)(.5 + (dy/dx) * (clip_max_x - *x2) + *y2);
			else
				yi = -1;  
		} 
		else
		if (*x1 < clip_min_x)
		{
			left_edge = 1;
			if (dx!=0)
				yi = (int)(.5 + (dy/dx) * (clip_min_x - *x2) + *y2);
			else
				yi = -1;
		} 

		if (*y1 > clip_max_y)
		{
			bottom_edge = 1;
			if (dy!=0)
				xi = (int)(.5 + (dx/dy) * (clip_max_y - *y2) + *x2);
			else
				xi = -1;
		} 
		else
		if (*y1 < clip_min_y)
		{
			top_edge = 1;
			if (dy!=0)
				xi = (int)(.5 + (dx/dy) * (clip_min_y - *y2) + *x2);
			else	
				xi = -1;
		}

		if (right_edge==1 && (yi>=clip_min_y && yi<=clip_max_y) )
		{
			*x1 = clip_max_x;
			*y1 = yi;
			success = 1;
		}
		else
		if (left_edge==1 && (yi>=clip_min_y && yi<=clip_max_y) )
		{
			*x1 = clip_min_x;
			*y1 = yi;
			success = 1;
		} 

		if (bottom_edge==1 && (xi>=clip_min_x && xi<=clip_max_x) )
		{
			*x1 = xi;
			*y1 = clip_max_y;
			success = 1;
		}
		else
		if (top_edge==1 && (xi>=clip_min_x && xi<=clip_max_x) )
		{
			*x1 = xi;
			*y1 = clip_min_y;
			success = 1;
		}
	}
	return success;
}
void sglDrawHLine(int x, int y, int xx, const SGLColor& color)
{
	unsigned char* pixel = ADDRESS(x, y);
	unsigned char  D = 255 - color.A;
	int step = sglDevice.windowDepth>>3;
	if(xx < 0)
	{
		xx = -xx;
	}
	if(xx < x)
	{
		int t;
		t = x;
		x = xx;
		xx= t;
	}
	if(sglGetState(SGL_ALPHABLEND))
	{
		for(int i=0; i<=xx-x; i++)
		{
			//pixel[0] = (color.B*color.A+pixel[0]*D)>>8;
			//pixel[1] = (color.G*color.A+pixel[1]*D)>>8;
			//pixel[2] = (color.R*color.A+pixel[2]*D)>>8;
			//pixel  += step;
			sglDrawPoint(x+i, y, color);
		}
	}
	else
	{
		for(int i=0; i<=xx-x; i++)
		{
			//pixel[0] = color.B;
			//pixel[1] = color.G;
			//pixel[2] = color.R;
			//pixel  += step;
			sglDrawPoint(x+i, y, color);
		}
	}
}
void sglDrawVLine(int x, int y, int yy, const SGLColor& color)
{
	unsigned char* pixel = ADDRESS(x, y);
	unsigned char  D = 255 - color.A;
	int step = sglDevice.window->pitch;
	if(yy < 0)
	{
		yy = -yy;
	}
	if(yy < y)
	{
		int t;
		t = y;
		y = yy;
		yy= t;
	}
	if(sglGetState(SGL_ALPHABLEND))
	{
		for(int i=0; i<=yy-y; i++)
		{
			pixel[0] = (color.B*color.A+pixel[0]*D)>>8;
			pixel[1] = (color.G*color.A+pixel[1]*D)>>8;
			pixel[2] = (color.R*color.A+pixel[2]*D)>>8;
			pixel  += step;
		}
	}
	else
	{
		for(int i=0; i<=yy-y; i++)
		{
			pixel[0] = color.B;
			pixel[1] = color.G;
			pixel[2] = color.R;
			pixel  += step;
		}
	}
}
void sglDrawLine(int x0, int y0, int x1, int y1, const SGLColor& color)
{
	
	if(!sglClipLine(&x0, &y0, &x1, &y1))
		return ;

	/*if(y0==y1)
	{
		sglDrawHLine(x0, y0, x1, color);
		return ;
	}
	else 
	if(x0==x1)
	{
		sglDrawVLine(x0,y0, y1, color);
		return ;
	}*/


	int dx,
		dy,
		x_inc,
		y_inc,
		error = 0,
		index;

	unsigned char* pixel = ADDRESS(x0, y0);
	unsigned char  D = 255 - color.A;
	dx = x1 - x0;
	dy = y1 - y0;
	if(dx>=0)
	{
		x_inc = sglDevice.windowDepth>>3;
	}
	else
	{
		x_inc = -(sglDevice.windowDepth>>3);
		dx = -dx;
	}

	if(dy>=0)
	{
		y_inc = sglDevice.window->pitch;
	}
	else
	{
		y_inc = -sglDevice.window->pitch;
		dy = -dy;
	}

	
	if(dx>dy)
	{
		if(sglGetState(SGL_ALPHABLEND))
		{
			for(index=0; index<=dx; index++)
			{
				SDL_LockSurface(sglDevice.window);
				pixel[0] = (color.B*color.A+pixel[0]*D)>>8;
				pixel[1] = (color.G*color.A+pixel[1]*D)>>8;
				pixel[2] = (color.R*color.A+pixel[2]*D)>>8;
				SDL_UnlockSurface(sglDevice.window);

				error += dy;

				if(error>dx)
				{
					error-=dx;
					pixel += y_inc;
				}
				pixel+=x_inc;
			}
		}
		else
		{
			for(index=0; index<=dx; index++)
			{
				SDL_LockSurface(sglDevice.window);
				pixel[0] = color.B;
				pixel[1] = color.G;
				pixel[2] = color.R;
				SDL_UnlockSurface(sglDevice.window);

				error += dy;

				if(error>dx)
				{
					error-=dx;
					pixel += y_inc;
				}
				pixel+=x_inc;
			}
		}
	}
	else
	{
		if(sglGetState(SGL_ALPHABLEND))
		{
			for(index=0; index<=dy; index++)
			{
				SDL_LockSurface(sglDevice.window);
				pixel[0] = (color.B*color.A+pixel[0]*D)>>8;
				pixel[1] = (color.G*color.A+pixel[1]*D)>>8;
				pixel[2] = (color.R*color.A+pixel[2]*D)>>8;
				SDL_UnlockSurface(sglDevice.window);

				error+=dx;
				if(error>0)
				{
					error-=dy;
					pixel+=x_inc;
				}
				pixel+=y_inc;
			}
		}
		else
		{
			for(index=0; index<=dy; index++)
			{
				SDL_LockSurface(sglDevice.window);
				pixel[0] = color.B;
				pixel[1] = color.G;
				pixel[2] = color.R;
				SDL_UnlockSurface(sglDevice.window);

				error+=dx;
				if(error>0)
				{
					error-=dy;
					pixel+=x_inc;
				}
				pixel+=y_inc;
			}
		}
	}
}
void sglDrawTriangleWithGShade(int x1, int y1, int x2, int y2, int x3, int y3, const SGLColor& c1, const SGLColor& c2, const SGLColor& c3)
{

	/*
		0.5 * h * dist3(밑변) = S
		h = 2*S / dist3

	*/
	float dist3 = sqrtf((x3-x1)*(x3-x1)+(y3-y1)*(y3-y1));

	if(dist3 == 0.0f)
		return;

	float area = TRIAREA(x1, y1, x2, y2, x3, y3);
	float h = (float)fabs(2.0f * area / dist3);

	float px1=x1;
	float px2=x3;
	float py1=y1;
	float py2=y3;

	SGLColor pc,pc1,pc2,pc3;
	pc1 = c1;
	pc2 = c2;
	pc3 = c3;

	float dx1 = (x2-x1)/h; //x1 변량
	float dx2 = (x2-x3)/h; //x2 변량
	float dy1 = (y2-y1)/h; //y1 변량
	float dy2 = (y2-y3)/h; //y2 변량

	float px,py,dx,dy,dist,colp1,colp2,colp3;
	float j,k;

	SGLColor c;

	for(j=0;j<h;j++)
	{
		dist = sqrtf((px2-px1)*(px2-px1)+(py2-py1)*(py2-py1));

		if(dist>0)
		{
			px = px1;
			py = py1;
			dx = (px2-px1)/dist;
			dy = (py2-py1)/dist;

			for(k=0;k<dist;k++)
			{
				//barycentric coordinates, sum always equal to 1
				colp1  = ((x1 - px) * (y2 - py) - (x2 - px) * (y1 - py)) / area;
				colp3  = ((x2 - px) * (y3 - py) - (x3 - px) * (y2 - py)) / area;
				colp2  = 1.0f - colp1 - colp3; //((x3 - px) * (y1 - py) - (x1 - px) * (y3 - py)) / area;

				c.R = (unsigned char)(colp1 * (float)pc1.R + colp2 * (float)pc2.R + colp3 * (float)pc3.R);
				c.G = (unsigned char)(colp1 * (float)pc1.G + colp2 * (float)pc2.G + colp3 * (float)pc3.G);
				c.B = (unsigned char)(colp1 * (float)pc1.B + colp2 * (float)pc2.B + colp3 * (float)pc3.B);

				sglDrawPoint((int)px,(int)py,c);
				px+=dx;
				py+=dy;
			}
			px1+=dx1;
			px2+=dx2;
			py1+=dy1;
			py2+=dy2;
		}
	}
}
void sglDrawTriangleWithTexture(int textureId, int x1, int y1, int x2, int y2, int x3, int y3, float tex_x1, float tex_y1, float tex_x2, float tex_y2, float tex_x3, float tex_y3)
{
	SGLTexture *texture = gp.textures[textureId];
	float dist3 = sqrtf((x3-x1)*(x3-x1)+(y3-y1)*(y3-y1));
	float area = TRIAREA(x1, y1, x2, y2, x3, y3);
	float h = dist3 > 0.0f ? (float)fabs(2.0f * area / dist3) : 0.0f;

	float px1=x1;
	float px2=x3;
	float py1=y1;
	float py2=y3;

	float ptx1=tex_x1;
	float ptx2=tex_x3;
	float pty1=tex_y1;
	float pty2=tex_y3;

	float dx1 = (x2-x1)/h;
	float dx2 = (x2-x3)/h;
	float dy1 = (y2-y1)/h;
	float dy2 = (y2-y3)/h;

	float dtx1 = (tex_x2-tex_x1)/h;
	float dtx2 = (tex_x2-tex_x3)/h;
	float dty1 = (tex_y2-tex_y1)/h;
	float dty2 = (tex_y2-tex_y3)/h;

	float px,py,dx,dy,dist,ptx,pty,dtx,dty;
	float j,k;

	SGLColor c;

	for(j=0;j<h;j++)
	{
		dist = sqrtf((px2-px1)*(px2-px1)+(py2-py1)*(py2-py1));

		if(dist>0)
		{
			px = px1;
			py = py1;
			ptx=ptx1;
			pty=pty1;
			dx = (px2-px1)/dist;
			dy = (py2-py1)/dist;
			dtx = (ptx2-ptx1)/dist;
			dty = (pty2-pty1)/dist;

			for(k=0;k<dist;k++)
			{
				c = texture->getColorAtf(ptx, pty);

				sglDrawPoint((int)px,(int)py, c);
				px+=dx;
				py+=dy;
				ptx+=dtx;
				pty+=dty;
			}

			px1+=dx1;
			px2+=dx2;
			py1+=dy1;
			py2+=dy2;

			ptx1+=dtx1;
			ptx2+=dtx2;
			pty1+=dty1;
			pty2+=dty2;
		}
	}
}
void sglDrawTriangleWithFrame(int x1, int y1, int x2, int y2, int x3, int y3, const SGLColor& color)
{
	sglDrawLine(x1, y1, x2, y2, color);
	sglDrawLine(x2, y2, x3, y3, color);
	sglDrawLine(x3, y3, x1, y1, color);
}
void sglDrawTopTriangle(int x1,int y1, int x2,int y2, int x3,int y3, const SGLColor& color)
{
	float dx_right, 
		  dx_left, 
		  xs,xe,   
		  height;  

	int temp_x,    
		temp_y,
		right,     
		left;

	int poly_clip_min_x = 0,
		poly_clip_max_x = sglDevice.windowWidth-1,
		poly_clip_min_y = 0,
		poly_clip_max_y = sglDevice.windowHeight-1;


	if (x2 < x1)
	{
		temp_x = x2;
		x2     = x1;
		x1     = temp_x;
	}
	height = y3-y1;
	dx_left  = (x3-x1)/height;
	dx_right = (x3-x2)/height;

	xs = (float)x1;
	xe = (float)x2+(float)0.5;

	if (y1<poly_clip_min_y)
    {
		xs = xs+dx_left*(float)(-y1+poly_clip_min_y);
		xe = xe+dx_right*(float)(-y1+poly_clip_min_y);
		y1=poly_clip_min_y;
	}

	if (y3>poly_clip_max_y)
		y3=poly_clip_max_y;


	if (x1>=poly_clip_min_x && x1<=poly_clip_max_x &&
		x2>=poly_clip_min_x && x2<=poly_clip_max_x &&
		x3>=poly_clip_min_x && x3<=poly_clip_max_x)
    {
	    for (temp_y=y1; temp_y<=y3; temp_y++)
        {
			//sglDrawLine(xs, temp_y, xe, temp_y, color);
			sglDrawHLine(xs, temp_y, xe, color);
	        xs+=dx_left;
		    xe+=dx_right;
        }

	}
	else
	{
	   for (temp_y=y1; temp_y<=y3; temp_y++)
       {
			left  = (int)xs;
			right = (int)xe;

			xs+=dx_left;
			xe+=dx_right;
			if (left < poly_clip_min_x)
			{
				left = poly_clip_min_x;
				if (right < poly_clip_min_x)
					continue;
			}
			if (right > poly_clip_max_x)
			{
				right = poly_clip_max_x;
				if (left > poly_clip_max_x)
					continue;
			}
			//sglDrawLine(left, temp_y, right, temp_y, color);
			sglDrawHLine(left, temp_y, right, color);
	   }
	}
}

void sglDrawBottomTriangle(int x1,int y1, int x2,int y2, int x3,int y3,const SGLColor& color)
{
	float dx_right,    // the dx/dy ratio of the right edge of line
			dx_left,     // the dx/dy ratio of the left edge of line
			xs,xe,       // the starting and ending points of the edges
			height;      // the height of the triangle

	int temp_x,        // used during sorting as temps
		temp_y,
		right,         // used by clipping
		left;

	
	int poly_clip_min_x = 0,
		poly_clip_max_x = sglDevice.windowWidth-1,
		poly_clip_min_y = 0,
		poly_clip_max_y = sglDevice.windowHeight-1;

	if (x3 < x2)
	{
		temp_x = x2;
		x2     = x3;
		x3     = temp_x;
	} // end if swap

	height = y3-y1;
	dx_left  = (x2-x1)/height;
	dx_right = (x3-x1)/height;

	xs = (float)x1;
	xe = (float)x1+(float)0.5;

	if (y1<poly_clip_min_y)
	{
		xs = xs+dx_left*(float)(-y1+poly_clip_min_y);
		xe = xe+dx_right*(float)(-y1+poly_clip_min_y);
		y1=poly_clip_min_y;
	}

	if (y3>poly_clip_max_y)
		y3=poly_clip_max_y;

	if (x1>=poly_clip_min_x && x1<=poly_clip_max_x &&
		x2>=poly_clip_min_x && x2<=poly_clip_max_x &&
		x3>=poly_clip_min_x && x3<=poly_clip_max_x)
	{
		for (temp_y=y1; temp_y<=y3; temp_y++)
		{
			//sglDrawLine(xs, temp_y, xe, temp_y, color);
			sglDrawHLine(xs, temp_y, xe, color);
			xs+=dx_left;
			xe+=dx_right;
		}
	} 
	else
	{
		for (temp_y=y1; temp_y<=y3; temp_y++)
		{
			left  = (int)xs;
			right = (int)xe;
	
			xs+=dx_left;
			xe+=dx_right;

			if (left < poly_clip_min_x)
			{
				left = poly_clip_min_x;
				if (right < poly_clip_min_x)
					continue;
			}

			if (right > poly_clip_max_x)
			{
				right = poly_clip_max_x;
				if (left > poly_clip_max_x)
					continue;
			}
			//sglDrawLine(left, temp_y, right, temp_y, color);
			sglDrawHLine(left, temp_y, right, color);
		}
	}
}

void sglDrawTriangleWithFlatShade(int x1, int y1, int x2, int y2, int x3, int y3, const SGLColor& color)
{
	int temp_x,
		temp_y,
		new_x;

	int poly_clip_min_x = 0,
		poly_clip_max_x = sglDevice.windowWidth-1,
		poly_clip_min_y = 0,
		poly_clip_max_y = sglDevice.windowHeight-1;

	if ((x1==x2 && x2==x3)  ||  (y1==y2 && y2==y3))
		return;

	if (y2<y1)
	{
		temp_x = x2;
		temp_y = y2;
		x2     = x1;
		y2     = y1;
		x1     = temp_x;
		y1     = temp_y;
	}

	if (y3<y1)
	{
		temp_x = x3;
		temp_y = y3;
		x3     = x1;
		y3     = y1;
		x1     = temp_x;
		y1     = temp_y;
	}

	if (y3<y2)
	{
		temp_x = x3;
		temp_y = y3;
		x3     = x2;
		y3     = y2;
		x2     = temp_x;
		y2     = temp_y;
	}

	if ( y3<poly_clip_min_y || y1>poly_clip_max_y ||
		(x1<poly_clip_min_x && x2<poly_clip_min_x && x3<poly_clip_min_x) ||
		(x1>poly_clip_max_x && x2>poly_clip_max_x && x3>poly_clip_max_x) )
		return;

	if (y1==y2)
	{
		sglDrawTopTriangle(x1,y1,x2,y2,x3,y3,color);
	}
	else
	if (y2==y3)
	{
		sglDrawBottomTriangle(x1,y1,x2,y2,x3,y3,color);
	}
	else
	{
		new_x = x1 + (int)((float)(y2-y1)*(float)(x3-x1)/(float)(y3-y1));
		sglDrawBottomTriangle(x1,y1,new_x,y2,x2,y2,color);
		sglDrawTopTriangle(x2,y2,new_x,y2,x3,y3,color);
	}
}

void sglDrawObject(const SGLObject& obj)
{
	SGLVertex v[4];
	int texIdCount = -1;
	if(obj.numVertices == 0)
		return;
	
	if(obj.numVertices > 0 && obj.vplist.size() > 0)
	{//sglVertexf() 로만 정의한 경우이다
		if(obj.objectType == SGL_TRIANGLES)
		{
			//3개씩 끊어 읽는다.
			for(int j=0; j<obj.numVertices; j += 3)
			{
				v[0] = obj.cameraVertices[j+0];
				v[1] = obj.cameraVertices[j+1];
				v[2] = obj.cameraVertices[j+2];
				if(sglGetShadeMode() == SGL_FRAME)
				{
					sglDrawTriangleWithFrame(v[0].pos.x, v[0].pos.y, v[1].pos.x, v[1].pos.y, v[2].pos.x, v[2].pos.y, v[0].color);
				}
				else
				if(sglGetShadeMode() == SGL_FLAT_SHADE)
				{
					sglDrawTriangleWithFlatShade(v[0].pos.x, v[0].pos.y, v[1].pos.x, v[1].pos.y, v[2].pos.x, v[2].pos.y, v[0].color);
				}
				else
				if(sglGetShadeMode() == SGL_GOURAUD_SHADE)
				{
					sglDrawTriangleWithGShade(v[0].pos.x, v[0].pos.y, v[1].pos.x, v[1].pos.y, v[2].pos.x, v[2].pos.y, v[0].color, v[1].color, v[2].color);
				}
				else
				if(sglGetShadeMode() == SGL_TEXTURE)
				{
					texIdCount++;
					if(texIdCount >= obj.textureIds.size())
					{
						texIdCount = obj.textureIds.size()-1;
					}
					sglDrawTriangleWithTexture(obj.textureIds[texIdCount], v[0].pos.x, v[0].pos.y, v[1].pos.x, v[1].pos.y, v[2].pos.x, v[2].pos.y, v[0].u, v[0].v, v[1].u, v[1].v, v[2].u, v[2].v);
				}
		}	
		}
		else
		if(obj.objectType == SGL_QUADS)
		{
			//4개씩 끊어 읽는다.
			for(int j=0; j<obj.numVertices; j += 4)
			{
				v[0] = obj.cameraVertices[j+0];
				v[1] = obj.cameraVertices[j+1];
				v[2] = obj.cameraVertices[j+2];
				v[3] = obj.cameraVertices[j+3];
				if(sglGetShadeMode() == SGL_FRAME)
				{
					sglDrawTriangleWithFrame(v[0].pos.x, v[0].pos.y, v[1].pos.x, v[1].pos.y, v[2].pos.x, v[2].pos.y, v[0].color);
					sglDrawTriangleWithFrame(v[0].pos.x, v[0].pos.y, v[2].pos.x, v[2].pos.y, v[3].pos.x, v[3].pos.y, v[0].color);
				}
				else
				if(sglGetShadeMode() == SGL_FLAT_SHADE)
				{
					sglDrawTriangleWithFlatShade(v[0].pos.x, v[0].pos.y, v[1].pos.x, v[1].pos.y, v[2].pos.x, v[2].pos.y, SGLColor(255, 255, 255));
					sglDrawTriangleWithFlatShade(v[0].pos.x, v[0].pos.y, v[2].pos.x, v[2].pos.y, v[3].pos.x, v[3].pos.y, SGLColor(255, 252, 255));
				}
				else
				if(sglGetShadeMode() == SGL_GOURAUD_SHADE)
				{
					sglDrawTriangleWithGShade(v[0].pos.x, v[0].pos.y, v[1].pos.x, v[1].pos.y, v[2].pos.x, v[2].pos.y, v[0].color, v[1].color, v[2].color);
					sglDrawTriangleWithGShade(v[0].pos.x, v[0].pos.y, v[2].pos.x, v[2].pos.y, v[3].pos.x, v[3].pos.y, v[0].color, v[2].color, v[3].color);
				}
				else
				if(sglGetShadeMode() == SGL_TEXTURE)
				{
					texIdCount++;
					if(texIdCount >= obj.textureIds.size())
					{
						texIdCount = obj.textureIds.size()-1;
					}
					sglDrawTriangleWithTexture(obj.textureIds[texIdCount], v[0].pos.x, v[0].pos.y, v[1].pos.x, v[1].pos.y, v[2].pos.x, v[2].pos.y, v[0].u, v[0].v, v[1].u, v[1].v, v[2].u, v[2].v);
					sglDrawTriangleWithTexture(obj.textureIds[texIdCount], v[0].pos.x, v[0].pos.y, v[2].pos.x, v[2].pos.y, v[3].pos.x, v[3].pos.y, v[0].u, v[0].v, v[2].u, v[2].v, v[3].u, v[3].v);
				}
			}	
		}
	}
	else
	if(obj.numVertices > 0 && obj.plist.size() > 0)
	{//인덱스를 이용하여 정의한 경우이다
		if(obj.objectType == SGL_TRIANGLES)
		{
			for(int i=0; i<obj.numPolygons; i++)
			{
				v[0] = obj.cameraVertices[obj.plist[i].indexList[0]];
				v[1] = obj.cameraVertices[obj.plist[i].indexList[1]];
				v[2] = obj.cameraVertices[obj.plist[i].indexList[2]];
				if(sglGetShadeMode() == SGL_FRAME)
				{
					sglDrawTriangleWithFrame(v[0].pos.x, v[0].pos.y, v[1].pos.x, v[1].pos.y, v[2].pos.x, v[2].pos.y, v[0].color);
				}
				else
				if(sglGetShadeMode() == SGL_FLAT_SHADE)
				{
					sglDrawTriangleWithFlatShade(v[0].pos.x, v[0].pos.y, v[1].pos.x, v[1].pos.y, v[2].pos.x, v[2].pos.y, v[0].color);
				}
				else
				if(sglGetShadeMode() == SGL_GOURAUD_SHADE)
				{
					sglDrawTriangleWithGShade(v[0].pos.x, v[0].pos.y, v[1].pos.x, v[1].pos.y, v[2].pos.x, v[2].pos.y, v[0].color, v[1].color, v[2].color);
				}
				else
				if(sglGetShadeMode() == SGL_TEXTURE)
				{
					texIdCount++;
					if(texIdCount >= obj.textureIds.size())
					{
						texIdCount = obj.textureIds.size()-1;
					}
					sglDrawTriangleWithTexture(obj.textureIds[texIdCount], v[0].pos.x, v[0].pos.y, v[1].pos.x, v[1].pos.y, v[2].pos.x, v[2].pos.y, v[0].u, v[0].v, v[1].u, v[1].v, v[2].u, v[2].v);
				}
			}
		}
		else
		if(obj.objectType == SGL_QUADS)
		{
			for(int i=0; i<obj.numPolygons; i++)
			{
				v[0] = obj.cameraVertices[obj.plist[i].indexList[0]];
				v[1] = obj.cameraVertices[obj.plist[i].indexList[1]];
				v[2] = obj.cameraVertices[obj.plist[i].indexList[2]];
				v[3] = obj.cameraVertices[obj.plist[i].indexList[3]];
				if(sglGetShadeMode() == SGL_FRAME)
				{
					sglDrawTriangleWithFrame(v[0].pos.x, v[0].pos.y, v[1].pos.x, v[1].pos.y, v[2].pos.x, v[2].pos.y, v[0].color);
					sglDrawTriangleWithFrame(v[0].pos.x, v[0].pos.y, v[2].pos.x, v[2].pos.y, v[3].pos.x, v[3].pos.y, v[0].color);
				}
				else
				if(sglGetShadeMode() == SGL_FLAT_SHADE)
				{
					sglDrawTriangleWithFlatShade(v[0].pos.x, v[0].pos.y, v[1].pos.x, v[1].pos.y, v[2].pos.x, v[2].pos.y, SGLColor(255, 255, 255));
					sglDrawTriangleWithFlatShade(v[0].pos.x, v[0].pos.y, v[2].pos.x, v[2].pos.y, v[3].pos.x, v[3].pos.y, SGLColor(255, 252, 255));
				}
				else
				if(sglGetShadeMode() == SGL_GOURAUD_SHADE)
				{
					sglDrawTriangleWithGShade(v[0].pos.x, v[0].pos.y, v[1].pos.x, v[1].pos.y, v[2].pos.x, v[2].pos.y, v[0].color, v[1].color, v[2].color);
					sglDrawTriangleWithGShade(v[0].pos.x, v[0].pos.y, v[2].pos.x, v[2].pos.y, v[3].pos.x, v[3].pos.y, v[0].color, v[2].color, v[3].color);
				}
				else
				if(sglGetShadeMode() == SGL_TEXTURE)
				{
					texIdCount++;
					if(texIdCount >= obj.textureIds.size())
					{
						texIdCount = obj.textureIds.size()-1;
					}
					sglDrawTriangleWithTexture(obj.textureIds[texIdCount], v[0].pos.x, v[0].pos.y, v[1].pos.x, v[1].pos.y, v[2].pos.x, v[2].pos.y, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f);//v[0].u, v[0].v, v[1].u, v[1].v, v[2].u, v[2].v);
					sglDrawTriangleWithTexture(obj.textureIds[texIdCount], v[0].pos.x, v[0].pos.y, v[2].pos.x, v[2].pos.y, v[3].pos.x, v[3].pos.y, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f);//v[0].u, v[0].v, v[2].u, v[2].v, v[3].u, v[3].v);
				}
			}
		}
	}
}

void sglDrawFaceSet(const SGLFaceSet& faceset)
{
	int texIdCount = -1;
	for(int i=0; i<faceset.size(); i++)
	{
		const SGLFace& f = faceset[i];
		if(f.numPoints == 3)
		{
			if(sglGetShadeMode() == SGL_FRAME)
			{
				sglDrawTriangleWithFrame(f.v[0].pos.x, f.v[0].pos.y, f.v[1].pos.x, f.v[1].pos.y, f.v[2].pos.x, f.v[2].pos.y, f.v[0].color);
			}
			else
			if(sglGetShadeMode() == SGL_FLAT_SHADE)
			{
				sglDrawTriangleWithFlatShade(f.v[0].pos.x, f.v[0].pos.y, f.v[1].pos.x, f.v[1].pos.y, f.v[2].pos.x, f.v[2].pos.y, f.v[0].color);
			}
			else
			if(sglGetShadeMode() == SGL_GOURAUD_SHADE)
			{
				sglDrawTriangleWithGShade(f.v[0].pos.x, f.v[0].pos.y, f.v[1].pos.x, f.v[1].pos.y, f.v[2].pos.x, f.v[2].pos.y, f.v[0].color, f.v[1].color, f.v[2].color);
			}
			else
			if(sglGetShadeMode() == SGL_TEXTURE)
			{
				texIdCount++;
				if(texIdCount >= f.obj->textureIds.size())
				{
					texIdCount = f.obj->textureIds.size()-1;
				}
				sglDrawTriangleWithTexture(f.obj->textureIds[texIdCount], f.v[0].pos.x, f.v[0].pos.y, f.v[1].pos.x, f.v[1].pos.y, f.v[2].pos.x, f.v[2].pos.y, f.v[0].u, f.v[0].v, f.v[1].u, f.v[1].v, f.v[2].u, f.v[2].v);
			}
		}
		else
		{
			if(sglGetShadeMode() == SGL_FRAME)
			{
				sglDrawTriangleWithFrame(f.v[0].pos.x, f.v[0].pos.y, f.v[1].pos.x, f.v[1].pos.y, f.v[2].pos.x, f.v[2].pos.y, f.v[0].color);
				sglDrawTriangleWithFrame(f.v[0].pos.x, f.v[0].pos.y, f.v[2].pos.x, f.v[2].pos.y, f.v[3].pos.x, f.v[3].pos.y, f.v[0].color);
			}
			else
			if(sglGetShadeMode() == SGL_FLAT_SHADE)
			{
				sglDrawTriangleWithFlatShade(f.v[0].pos.x, f.v[0].pos.y, f.v[1].pos.x, f.v[1].pos.y, f.v[2].pos.x, f.v[2].pos.y, f.v[0].color);
				sglDrawTriangleWithFlatShade(f.v[0].pos.x, f.v[0].pos.y, f.v[2].pos.x, f.v[2].pos.y, f.v[3].pos.x, f.v[3].pos.y, f.v[0].color);
			}
			else
			if(sglGetShadeMode() == SGL_GOURAUD_SHADE)
			{
				sglDrawTriangleWithGShade(f.v[0].pos.x, f.v[0].pos.y, f.v[1].pos.x, f.v[1].pos.y, f.v[2].pos.x, f.v[2].pos.y, f.v[0].color, f.v[1].color, f.v[2].color);
				sglDrawTriangleWithGShade(f.v[0].pos.x, f.v[0].pos.y, f.v[2].pos.x, f.v[2].pos.y, f.v[3].pos.x, f.v[3].pos.y, f.v[0].color, f.v[1].color, f.v[2].color);
			}
			else
			if(sglGetShadeMode() == SGL_TEXTURE)
			{
				texIdCount++;
				if(texIdCount >= f.obj->textureIds.size())
				{
					texIdCount = f.obj->textureIds.size()-1;
				}
				sglDrawTriangleWithTexture(f.obj->textureIds[texIdCount], f.v[0].pos.x, f.v[0].pos.y, f.v[1].pos.x, f.v[1].pos.y, f.v[2].pos.x, f.v[2].pos.y, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f);
				sglDrawTriangleWithTexture(f.obj->textureIds[texIdCount], f.v[0].pos.x, f.v[0].pos.y, f.v[2].pos.x, f.v[2].pos.y, f.v[3].pos.x, f.v[3].pos.y, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f);
			}	
		}
	}
}