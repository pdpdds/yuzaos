#ifdef PC_VERSION
#include "PCSoundEngine.h"
#endif

#ifdef DC_VERSION
#include "DCSoundEngine.h"
#endif

#include "PCGfxEngine.h"
#include "structures.h"
#include "collisions.h"
#include "bonus.h"
#include "main.h"

void initShoot()
{
	int i;
	for (i=0;i<MAX_SHOOT;i++)
	{
		shoot[i].utilise = 0;
	}
}

void createShoot(int type)
{
	int i;
	i=0;
	while ((shoot[i].utilise==1) && (i<MAX_SHOOT))
	{
		i++;
	}

	if ((type==WEAPON_NORMAL_SHOOT) || (type==WEAPON_DOUBLE_SHOOT) || (type==WEAPON_GLUE_SHOOT))
	{
		shoot[i].posx=player.posx+12*scale;
		shoot[i].posy=player.posy;/*-30;*/
		shoot[i].xbox=0*scale;
		shoot[i].ybox=0*scale;
		shoot[i].hbox=0*scale;
		shoot[i].lbox=9*scale;
		shoot[i].type=type;
		shoot[i].utilise=1;
		shoot[i].duree=-1;
		shoot[i].posy_depart = player.posy+32*scale;
		playSound(3,0);
	}
	
}


void updateShoot(int i)
{
	int p;
	if (shoot[i].utilise==1)
	{
		if ((shoot[i].type==WEAPON_NORMAL_SHOOT) || (shoot[i].type==WEAPON_DOUBLE_SHOOT))
		{
			shoot[i].posy = shoot[i].posy - 2*scale;
			shoot[i].hbox=(shoot[i].posy_depart) - shoot[i].posy;
			if (shoot[i].posy<8*scale)
			{
				shoot[i].utilise = 0;
				player.nbtir--;
			}
		}
		if (shoot[i].type==WEAPON_GLUE_SHOOT)
		{
			if (shoot[i].duree==-1) shoot[i].posy = shoot[i].posy - 2*scale;
			shoot[i].hbox=(shoot[i].posy_depart) - shoot[i].posy;
			if (shoot[i].posy<8*scale)
			{
				if (shoot[i].duree==-1) shoot[i].duree = 120;
				shoot[i].posy=8*scale;
			}
		}
		if (shoot[i].duree>0) shoot[i].duree--;
		if (shoot[i].duree==0)
		{
			shoot[i].duree=-1;
			shoot[i].utilise = 0;
			player.nbtir--;
		}
		for (p=0;p<MAX_PLATEFORME;p++)
		{
			if (pforme[p].utilise==1)
			{
				if (isCollide(shoot[i].posx+shoot[i].xbox,shoot[i].posy+shoot[i].ybox,shoot[i].hbox-5*scale,shoot[i].lbox,pforme[p].posx,pforme[p].posy,pforme[p].hauteur,pforme[p].largeur))
				{
					// Si c un shoot normal ou un double shoot quelque soit la plateforme en collide ca dégage
					if ((shoot[i].type==WEAPON_DOUBLE_SHOOT) || (shoot[i].type==WEAPON_NORMAL_SHOOT))
					{
						if (shoot[i].utilise==1)
						{
							shoot[i].utilise = 0;
							player.nbtir--;
							if (player.nbtir<0) player.nbtir=0;
						}
					}
					// Si c un grappin et un PF_INCASSABLE on laisse le grapin.
					if (shoot[i].type==WEAPON_GLUE_SHOOT)
					{
						if ((pforme[p].type==PF_INCASSABLE) || (pforme[p].type==PF_INCASSABLEV) || (pforme[p].type==PF_MOYEN_INCASSABLE) || (pforme[p].type==PF_MICRO_INCASSABLE))
						{
							if (shoot[i].duree==-1) shoot[i].duree = 120;
						}
					}
					// Si c une plateforme normale, quelquesoit le shoot, on le dégage.
					if ((pforme[p].type==PF_CASSABLE) || (pforme[p].type==PF_CASSABLEV) || (pforme[p].type==PF_MOYEN_CASSABLE) || (pforme[p].type==PF_MICRO_CASSABLE))
					{
						if (shoot[i].utilise==1)
						{
							shoot[i].utilise = 0;
							player.nbtir--;						
							if (player.nbtir<0) player.nbtir=0;
						}
						pforme[p].utilise=0;
						releaseSprite(pforme[p].spriteno);
						if (pforme[p].bonus!=0)
						{
						 	if (pforme[p].bonus==BONUS_LIFE)
						 	{
								if ((player.bonus_life==0) && (player.bonus_life_level!=currentLevel))	
								{
									createBonus(pforme[p].bonus,pforme[p].posx+4*scale,pforme[p].posy);
								}
							}
							else
							{
								createBonus(pforme[p].bonus,pforme[p].posx+4*scale,pforme[p].posy);
							}
						}
					}
				}
			}
		}
	}
	if (player.nbtir<0) player.nbtir=0;
}

void showShoot(int i)
{
	if (shoot[i].utilise==1)
	{
		if ((shoot[i].type==WEAPON_NORMAL_SHOOT) || (shoot[i].type==WEAPON_DOUBLE_SHOOT))
		{
//			GpTransBlt(NULL, &gpDraw[nflip], shoot[i].posx, shoot[i].posy, shoot[i].lbox,shoot[i].hbox, (unsigned char*)sprite1, 305, 11, 320, 240,INVISIBLE_COLOR);
         blitImageToScreen(1,305*scale,11*scale,shoot[i].lbox,shoot[i].hbox,shoot[i].posx,shoot[i].posy,shoot[i].lbox,shoot[i].hbox,320*scale,240*scale);
		}
		if (shoot[i].type==WEAPON_GLUE_SHOOT)
		{
			if ((shoot[i].duree>0) && (shoot[i].duree<30) && (gbl_timer%2==0))
			{} else	blitImageToScreen(1,293*scale,11*scale,shoot[i].lbox,shoot[i].hbox,shoot[i].posx,shoot[i].posy,shoot[i].lbox,shoot[i].hbox,320*scale,240*scale);
//			{} else	GpTransBlt(NULL, &gpDraw[nflip], shoot[i].posx, shoot[i].posy, shoot[i].lbox,shoot[i].hbox, (unsigned char*)sprite1, 293, 11, 320, 240,INVISIBLE_COLOR);
		}
	}
}
