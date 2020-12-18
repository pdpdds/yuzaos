#include "structures.h"
#ifdef PC_VERSION
       #include "PCGfxEngine.h"
#endif
#include "main.h"

void initPlateformes()
{
	int i;
	for (i=0;i<MAX_PLATEFORME;i++)
	{
		pforme[i].utilise = 0;
	}
}

int createPlateforme(int posx,int posy,int typepf)
{
	int i;
	i=0;
	while ((pforme[i].utilise==1) && (i<MAX_PLATEFORME))
	{
		i++;
	}

	if (typepf==PF_INCASSABLE)
	{
		pforme[i].type = PF_INCASSABLE;
		pforme[i].posx = posx;
		pforme[i].posy = posy;
		pforme[i].hauteur = 8*scale;
		pforme[i].largeur = 32*scale;
		pforme[i].utilise = 1;
		pforme[i].bonus=0;
	
	}
	else if (typepf==PF_CASSABLE)
	{
		pforme[i].type = PF_CASSABLE;
		pforme[i].posx = posx;
		pforme[i].posy = posy;
		pforme[i].hauteur = 8*scale;
		pforme[i].largeur = 32*scale;
		pforme[i].utilise = 1;
		pforme[i].bonus=0;
	}
	else if (typepf==PF_CASSABLEV)
	{
		pforme[i].type = PF_CASSABLEV;
		pforme[i].posx = posx;
		pforme[i].posy = posy;
		pforme[i].hauteur = 32*scale;
		pforme[i].largeur = 8*scale;
		pforme[i].utilise = 1;
		pforme[i].bonus=0;	
	}
	else if (typepf==PF_INCASSABLEV)
	{
		pforme[i].type = PF_INCASSABLEV;
		pforme[i].posx = posx;
		pforme[i].posy = posy;
		pforme[i].hauteur = 32*scale;
		pforme[i].largeur = 8*scale;
		pforme[i].utilise = 1;	
		pforme[i].bonus=0;
	}
	else if (typepf==PF_MOYEN_INCASSABLE)
	{
		pforme[i].type = PF_MOYEN_INCASSABLE;
		pforme[i].posx = posx;
		pforme[i].posy = posy;
		pforme[i].hauteur = 8*scale;
		pforme[i].largeur = 16*scale;
		pforme[i].utilise = 1;	
		pforme[i].bonus=0;		
	}
	else if (typepf==PF_MOYEN_CASSABLE)
	{
		pforme[i].type = PF_MOYEN_CASSABLE;
		pforme[i].posx = posx;
		pforme[i].posy = posy;
		pforme[i].hauteur = 8*scale;
		pforme[i].largeur = 16*scale;
		pforme[i].utilise = 1;
		pforme[i].bonus=0;
	}
	else if (typepf==PF_MICRO_INCASSABLE)
	{
		pforme[i].type = PF_MICRO_INCASSABLE;
		pforme[i].posx = posx;
		pforme[i].posy = posy;
		pforme[i].hauteur = 8*scale;
		pforme[i].largeur = 8*scale;
		pforme[i].utilise = 1;
		pforme[i].bonus=0;
	}
	else if (typepf==PF_MICRO_CASSABLE)
	{
		pforme[i].type = PF_MICRO_CASSABLE;
		pforme[i].posx = posx;
		pforme[i].posy = posy;
		pforme[i].hauteur = 8*scale;
		pforme[i].largeur = 8*scale;
		pforme[i].utilise = 1;
		pforme[i].bonus=0;
	}
	
	pforme[i].spriteno = initFreeSprite(pforme[i].posx,pforme[i].posy,pforme[i].type);
	
	return i;
}

void showPlateforme(int i)
{
	if (pforme[i].utilise==1)
	{
		showSprite(pforme[i].spriteno);
	}
}
