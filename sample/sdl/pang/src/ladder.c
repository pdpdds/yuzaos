#include "structures.h"
#include "main.h"
#include "PCGfxEngine.h"

void initLadders()
{
	int i;
	for (i=0;i<MAX_ECHELLE;i++)
	{
		ech[i].utilise = 0;
	}
}

/* Mini : 3 Barreau !! */
int createLadder(int x,int y,int nbBarreau)
{
	int i;
	i=0;
	while ((ech[i].utilise==1) && (i<MAX_ECHELLE))
	{
		i++;
	}

	ech[i].utilise=1;
	ech[i].posx=x;
	ech[i].posy=y;
	ech[i].hauteur=4*nbBarreau*scale;
	ech[i].largeur=22*scale;
	ech[i].nbBarreau=nbBarreau;

	ech[i].pad_haut_x = x;
	ech[i].pad_haut_y = y-4*scale;
	ech[i].pad_haut_largeur = ech[i].largeur;
	ech[i].pad_haut_hauteur = 4*scale;

	ech[i].pad_milieu_x = x;
	ech[i].pad_milieu_y = y;
	ech[i].pad_milieu_largeur = ech[i].largeur;
	ech[i].pad_milieu_hauteur = ech[i].hauteur-4*scale;

	ech[i].pad_bas_x = x;
	ech[i].pad_bas_y = y + ech[i].hauteur-4*scale;
	ech[i].pad_bas_largeur = ech[i].largeur;
	ech[i].pad_bas_hauteur = 4*scale;

	return i;
}

void showLadder(int i)
{
	if (ech[i].utilise==1) 
	{	
		int e;

		for (e=0;e<ech[i].nbBarreau;e++)
		{
			/*GpTransBlt(NULL, &gpDraw[nflip], ech[i].posx, ech[i].posy+(e*4), 22,4, (unsigned char*)sprite1, 71, 53, 320, 240,INVISIBLE_COLOR);*/
            
            blitImageToScreen(1,71*scale,53*scale,22*scale,4*scale,ech[i].posx,ech[i].posy+(e*4*scale),22*scale,4*scale,320*scale,240*scale);

		}
	}
}
