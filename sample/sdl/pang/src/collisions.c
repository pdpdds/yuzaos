#include "structures.h"
#ifdef PC_VERSION
       #include "PCGfxEngine.h"
#endif
#ifdef DREAMCAST_VERSION
       #include "DCGfxEngine.h"
#endif

#include "main.h"


int checkBallCollisionWithBorder(int i)
{
    int isCollide = 0;
	/* Collision avec le bas ? */
	if (ball[i].posy >= 200*scale - imageBank[sprite[ball[i].spriteno].image].imageh)
	{
		ball[i].vel = ball[i].vel_cst;
		ball[i].posy = 200*scale - imageBank[sprite[ball[i].spriteno].image].imageh;
		isCollide = 1;
	}
			
	/* Collision avec le haut ? */
	if (ball[i].posy<8*scale)
	{
		ball[i].posy=8*scale;	
		isCollide = 1;
	}

	/* Gauche */
	if (ball[i].posx <= 8*scale)
	{
		ball[i].move = -ball[i].move;
		ball[i].posx = 8*scale;		
		isCollide = 1;		
	}

	/* Droite */
	if (ball[i].posx>= (320- 8) *scale  - imageBank[sprite[ball[i].spriteno].image].imagel)
	{
		ball[i].move = -ball[i].move;
		ball[i].posx = (320 - 8)*scale  - imageBank[sprite[ball[i].spriteno].image].imagel;	
		isCollide = 1;		
	}     
	
	return isCollide;
}

int isCollide(int XRectangle1,int YRectangle1,int HRectangle1,int LRectangle1,int XRectangle2,int YRectangle2,int HRectangle2,int LRectangle2)
{
	if (XRectangle1+LRectangle1 < XRectangle2) return(0);
	if (XRectangle1 > XRectangle2+LRectangle2 ) return(0);
	if (YRectangle1+HRectangle1 < YRectangle2 ) return(0);
	if (YRectangle1 > YRectangle2+HRectangle2 ) return(0);    

	return(1);
} 
