#include "stdlib.h"
#include "structures.h"
#include "collisions.h"
#include "main.h"
#include "PCGfxEngine.h"
#include <math.h>


void initBonus()
{
	int i;
	for (i=0;i<MAX_BONUS;i++)
	{
		bonus[i].utilise = 0;
	}
}

int getRandomBonus()
{
	int i;
	i=10;
	
	while (i>4)
	{
		i = rand();
		i = i - ((int)(i/10))*10;
	}
		
	if ( (i==0) ) return BONUS_BOOM;
	if ( (i==1) ) return BONUS_FREEZE;
	if ( (i==2) ) return BONUS_PROTECTION;
	if ( (i==3) ) return WEAPON_DOUBLE_SHOOT;
	if ( (i==4) ) return WEAPON_GLUE_SHOOT;
	
    return 0;
}

int createBonus(int type,int x,int y)
{
	int i;
	i=0;
	while ((bonus[i].utilise==1) && (i<MAX_BONUS))
	{
		i++;
	}

	bonus[i].posx=x;
	bonus[i].posy=y;
	bonus[i].xbox=0*scale;
	bonus[i].ybox=0*scale;
	bonus[i].hbox=18*scale;
	bonus[i].lbox=18*scale;
	bonus[i].type=type;
	bonus[i].utilise=1;
	bonus[i].etat = DOWN;
	bonus[i].duree_de_vie=200;

	return i;
}

void updateBonus(int i)
{
	int p;
	int collide=0;
	
	if (bonus[i].utilise==1)
	{
		/* On check les collision avec les plateformes */
		for (p=0;p<MAX_PLATEFORME;p++)
		{
			if (pforme[p].utilise==1)
			{
				collide += isCollide(bonus[i].xbox+bonus[i].posx,bonus[i].ybox+bonus[i].posy,bonus[i].hbox,bonus[i].lbox,pforme[p].posx,pforme[p].posy,pforme[p].hauteur,pforme[p].largeur);
			}
		}
		/* Si il y ?collision, l'?at passe ?stop ! */
		if (collide>0)
		{
			bonus[i].etat = STOP;
		}
		else
		{
			bonus[i].etat = DOWN;	
		}
				
		/* On check les collision avec le bas */
		if (bonus[i].posy+bonus[i].hbox>200*scale) 
		{
			//bonus[i].posy=200*scale-bonus[i].hbox;
			bonus[i].etat = STOP;
		}

		if (bonus[i].etat==DOWN) bonus[i].posy+=2;
		if (bonus[i].etat==STOP) bonus[i].duree_de_vie--;
		if (bonus[i].duree_de_vie<0) bonus[i].utilise=0;
	}
}

void showBonus(int i)
{
	if (bonus[i].utilise==1)
	{

//		if (bonus[i].type==WEAPON_DOUBLE_SHOOT) GpTransLRBlt(NULL, &gpDraw[nflip], bonus[i].posx, bonus[i].posy, 18,18, (unsigned char*)sprite1, 53, 191, 320, 240,INVISIBLE_COLOR);
//		else if (bonus[i].type==WEAPON_GLUE_SHOOT) GpTransLRBlt(NULL, &gpDraw[nflip], bonus[i].posx, bonus[i].posy, 18,18, (unsigned char*)sprite1, 11, 191, 320, 240,INVISIBLE_COLOR);
///		else if (bonus[i].type==BONUS_BOOM) GpTransLRBlt(NULL, &gpDraw[nflip], bonus[i].posx, bonus[i].posy, 18,18, (unsigned char*)sprite1, 116, 191, 320, 240,INVISIBLE_COLOR);
///		else if (bonus[i].type==BONUS_FREEZE) GpTransLRBlt(NULL, &gpDraw[nflip], bonus[i].posx, bonus[i].posy, 18,18, (unsigned char*)sprite1, 32, 191, 320, 240,INVISIBLE_COLOR);
//		else if (bonus[i].type==BONUS_LIFE) GpTransLRBlt(NULL, &gpDraw[nflip], bonus[i].posx, bonus[i].posy, 18,18, (unsigned char*)sprite1, 74, 191, 320, 240,INVISIBLE_COLOR);
//		else if (bonus[i].type==BONUS_PROTECTION) GpTransLRBlt(NULL, &gpDraw[nflip], bonus[i].posx, bonus[i].posy, 18,18, (unsigned char*)sprite1, 158, 191, 320, 240,INVISIBLE_COLOR);

		if (bonus[i].type==WEAPON_DOUBLE_SHOOT) blitImageToScreen(1,53*scale,191*scale,18*scale,18*scale,bonus[i].posx,bonus[i].posy,18*scale,18*scale,320*scale,240*scale);
		else if (bonus[i].type==WEAPON_GLUE_SHOOT) blitImageToScreen(1,11*scale,191*scale,18*scale,18*scale,bonus[i].posx,bonus[i].posy,18*scale,18*scale,320*scale,240*scale);
		else if (bonus[i].type==BONUS_BOOM) blitImageToScreen(1,116*scale,191*scale,18*scale,18*scale,bonus[i].posx,bonus[i].posy,18*scale,18*scale,320*scale,240*scale);
		else if (bonus[i].type==BONUS_FREEZE) blitImageToScreen(1,32*scale,191*scale,18*scale,18*scale,bonus[i].posx,bonus[i].posy,18*scale,18*scale,320*scale,240*scale);
		else if (bonus[i].type==BONUS_LIFE) blitImageToScreen(1,74*scale,191*scale,18*scale,18*scale,bonus[i].posx,bonus[i].posy,18*scale,18*scale,320*scale,240*scale);
		else if (bonus[i].type==BONUS_PROTECTION) blitImageToScreen(1,158*scale,191*scale,18*scale,18*scale,bonus[i].posx,bonus[i].posy,18*scale,18*scale,320*scale,240*scale);

	}
}
