#include "string.h"
#include "structures.h"
#include "PCGfxEngine.h"
#include "main.h"


void showNumber(char *string,unsigned int x,unsigned int y)

{

	unsigned char i,temp;
	int positionxduzero=148*scale;
	int positionyduzero=226*scale;
	
	temp=strlen(string);
	for(i=0;i<temp;i++)
		{       
          if ( ((*string-48)*13) == 0 )
          {
                    blitImageToScreen(1,((*string-48)*14*scale)+positionxduzero,positionyduzero,13*scale,13*scale,x+(i*13),y*scale,13*scale,13*scale,320*scale,240*scale);
          }
          else
          {
                    blitImageToScreen(1,((*string-48)*14*scale)+positionxduzero+1,positionyduzero,13*scale,13*scale,x+(i*13),y,13*scale,13*scale,320*scale,240*scale);             
          }
          
          string++;
		}
}

void initObjets()
{
	int i;
	for (i=0;i<MAX_OBJETS;i++)
	{
		obj[i].utilise = 0;
	}
}

void createObjet(int type,int x,int y,int value)
{

	int i;
	i=0;
	while ((obj[i].utilise==1) && (i<MAX_OBJETS) )
	{
		i++;
	}
	
	obj[i].type=type;
	obj[i].posx=x;
	obj[i].posy=y;
	obj[i].value=value;
	obj[i].utilise=1;
	
	if (type==OBJ_MUL)
	{
		obj[i].cpt=20;
	} else if (type==OBJ_1UP)
	{
		obj[i].cpt=100;
	} else if ((type==OBJ_EXPL_BIG)||(type==OBJ_EXPL_NORMAL)||(type==OBJ_EXPL_SMALL)||(type==OBJ_EXPL_MICRO)) 
	{
		obj[i].cpt=20;
	}
}

void showObjet(int i)
{

	if (obj[i].utilise==1)
	{
		if (obj[i].type==OBJ_MUL)
		{
			if (obj[i].cpt>0)
			{
//   			GpTextOut(NULL, &gpDraw[nflip], obj[i].posx, obj[i].posy, "x", INVISIBLE_COLOR);
				char chaine[3];
				sprintf(chaine, "%d", obj[i].value);	
				blitImageToScreen(1,247*scale,90*scale,7*scale,9*scale,obj[i].posx,obj[i].posy,7*scale,9*scale,320*scale,240*scale);	
				
                showNumber(chaine,obj[i].posx+8, obj[i].posy);
//				GpTextOut(NULL, &gpDraw[nflip], obj[i].posx+8, obj[i].posy, chaine, INVISIBLE_COLOR);

                obj[i].posy--;
				obj[i].cpt--;
			}
			else
			{
				obj[i].utilise=0;
			}
		} else 
		if (obj[i].type==OBJ_1UP)
		{
			if (obj[i].cpt>0)
			{
//				GpTextOut(NULL, &gpDraw[nflip], obj[i].posx, obj[i].posy, "1UP", INVISIBLE_COLOR);
                blitImageToScreen(1,240*scale,12*scale,21*scale,13*scale,obj[i].posx,obj[i].posy,21*scale,13*scale,320*scale,240*scale);	
				obj[i].cpt--;
				obj[i].posy--;
			}
			else
			{
				obj[i].utilise=0;
			}
		}		
	}
}
