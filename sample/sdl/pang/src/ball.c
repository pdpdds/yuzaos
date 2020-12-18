#ifdef PC_VERSION
#include "PCSoundEngine.h"
#endif
#ifdef DREAMCAST_VERSION
#include "DCSoundEngine.h"
#endif

#include "PCGfxEngine.h"

#include "structures.h"
#include "collisions.h"
#include "actions.h"
#include "bonus.h"
#include "objets.h"
#include "ball.h"
#include "main.h"

/*
 Copie à l'écran l'image de la balle numéro i
*/
void showBall(int i)
{
    if (ball[i].utilise==1)
	{
     
//      fprintf(stderr,"posx: %d\n",(int)ball[i].posx);      	        
//      fprintf(stderr,"posy: %d\n",(int)ball[i].posy);      	        	        
                           
		moveSprite(ball[i].spriteno,(int)ball[i].posx,(int)ball[i].posy);

//      fprintf(stderr,"sposx: %d\n",sprite[0].posx);      	        
//      fprintf(stderr,"sposy: %d\n",sprite[0].posy);      	        	        
           				
		changeSpriteImage(ball[i].spriteno,ball[i].type);
		showSprite(ball[i].spriteno);
	}
	
}

/*
  Renvoie TOUCH_HORIZONTAL ou TOUCH_VERTICAL pour savoir si
  la balle à touché une plateforme sur le coté ou sur les surfaces horizontales
  sprite1 = sprite de la balle numéro b
  sprite2 = sprite de la plateforme
  b = n° de la balle
*/
int ballCollideWithPlatform(int sprite1,int sprite2,int b)
{
	int collided = 0;
	int rect1_x, rect1_y;
	int rect2_x, rect2_y;
	int i, j, k, l;
	int coorx_1 = sprite[sprite1].posx;
	int coory_1 = sprite[sprite1].posy;
	int coorx_2 = sprite[sprite2].posx;
	int coory_2 = sprite[sprite2].posy;
	int sprite1w = imageBank[sprite[sprite1].image].imagel;
	int sprite1h = imageBank[sprite[sprite1].image].imageh;
	int sprite2w = imageBank[sprite[sprite2].image].imagel;
	int sprite2h = imageBank[sprite[sprite2].image].imageh;
	
	if (sprite[sprite1].utilise==0) return 0;
	if (sprite[sprite2].utilise==0) return 0;

	/*Détection par bounding box
	Retourne 0 et sort de la fonction
	si les sprites ne possédent pas de zones superposées*/
	if(coorx_1 > coorx_2+imageBank[sprite[sprite2].image].imagel) return 0;
	if(coorx_1+imageBank[sprite[sprite1].image].imagel < coorx_2) return 0;
	if(coory_1 > coory_2+imageBank[sprite[sprite2].image].imageh) return 0;
	if(coory_1+imageBank[sprite[sprite1].image].imageh < coory_2) return 0;

	/*Le but des lignes suivantes est de définir un
	rectangle qui englobe la zone d'affichage
	commune aux deux sprites
	On traite les coordonnées x du rectangle*/

	if (coorx_1 < coorx_2)
	{
		rect1_x = coorx_2;
		if (coorx_1 + sprite1w >= coorx_2 + sprite2w)
			rect2_x = coorx_2 + sprite2w;
		else
			rect2_x = coorx_1 + sprite1w;
	}
	else
	{
		rect1_x = coorx_1;
		if (coorx_2 + sprite2w >= coorx_1 + sprite1w)
			rect2_x = coorx_1 + sprite1w;
		else
			rect2_x = coorx_2 + sprite2w;
	}

	/*On traite les coordonnées y du rectangle*/
	if (coory_1 < coory_2)
	{
		rect1_y = coory_2;
		if (coory_1 + sprite1h >= coory_2 + sprite2h)
			rect2_y = coory_2 + sprite2h;
		else
			rect2_y = coory_1 + sprite1h;
	}
	else
	{
		rect1_y = coory_1;
		if (coory_2 + sprite2h > coory_1 + sprite1h)
			rect2_y = coory_1 + sprite1h;
		else
			rect2_y = coory_2 + sprite2h;
	}

	if (SDL_MUSTLOCK(imageBank[sprite[sprite1].image].image)) SDL_LockSurface(imageBank[sprite[sprite1].image].image);
	if (SDL_MUSTLOCK(imageBank[sprite[sprite2].image].image)) SDL_LockSurface(imageBank[sprite[sprite2].image].image);


	/*Il ne reste plus qu'à tester pour chaque

	pixel du rectangle précèdemment défini si ses pixels

	sont transparents

	Un pixel non transparent signifie qu'un bout de sprite

	est present dans le rectangle

	et donc que les sprites sont en collision*/

	for (i = rect1_x - coorx_1, j = rect1_x - coorx_2;i < rect2_x - coorx_1; i++, j++)
	{
		for (k = rect1_y - coory_1, l = rect1_y - coory_2;k < rect2_y - coory_1; k++, l++)
		{
			
			if((CollideTransparentPixelTest(imageBank[sprite[sprite1].image].image , i , k)!=0)
			&& (CollideTransparentPixelTest(imageBank[sprite[sprite2].image].image , j , l))!=0)			
			{                        
				if (SDL_MUSTLOCK(imageBank[sprite[sprite1].image].image)) SDL_UnlockSurface(imageBank[sprite[sprite1].image].image);
				if (SDL_MUSTLOCK(imageBank[sprite[sprite2].image].image)) SDL_UnlockSurface(imageBank[sprite[sprite2].image].image);
                                                                                         
				collided = 1; 
			}
		}

	}

 	if (SDL_MUSTLOCK(imageBank[sprite[sprite1].image].image)) SDL_UnlockSurface(imageBank[sprite[sprite1].image].image);
	if (SDL_MUSTLOCK(imageBank[sprite[sprite2].image].image)) SDL_UnlockSurface(imageBank[sprite[sprite2].image].image);
	

	int lfinal = rect2_x-rect1_x;
	int hfinal = rect2_y-rect1_y;
	
	int ligne; /* 0 - egalite 1 - horizontal  2 - vertical */
	int cote=0; /* 3 -gauche 4 - droite */
		
	if (collided==1)
	{
		
		if (lfinal<hfinal) ligne=2;
		else if (lfinal>hfinal) ligne=1;
		else ligne=0;
		
		/* Coté droit ou gauche ? */
		if (ligne==2)
		{
			if (rect1_x==sprite[sprite2].posx) cote=3;
			else cote=4;			
		}
	}
	else
	{
		return 0;
	}
		
	/* si la balle va à droite*/
	if (ball[b].move>0)	
	{
		if (cote==4) return TOUCH_HORIZONTAL; /* elle touche un coté droit */		
		if (cote==3) return TOUCH_VERTICAL;		
	}
	else
	{
		if (cote==3) return TOUCH_HORIZONTAL;
		if (cote==4) return TOUCH_VERTICAL;
	}
		
	if (ligne==1) return TOUCH_HORIZONTAL;
	if (ligne==2) return TOUCH_VERTICAL;
	
	if (ligne==0) return TOUCH_VERTICAL;
	
	/* D'après les tests fait plus haut ya forcément une collision; mais bon on en renvoie une par défaut */
	return TOUCH_VERTICAL;    
    
    
    
}


/* 
   Teste les collisions avec les bords de l'écran 
*/

void initBalls()
{
	int i;
	for (i=0;i<MAX_BALL;i++)
	{
		ball[i].utilise = 0;
	}
}

void explodeABall(int a)
{
     playSound(4,1);
    /* Crée une nouvelle balle plus petite qui ira à gauche */
	int tmp2;
	tmp2 = createBall(ball[a].posx,ball[a].posy,ball[a].type+1,LEFT,UP);
	ball[tmp2].hauteurmax_cpt = ball[tmp2].hauteurmax-(20);
	ball[tmp2].vel=2.0;
	if (ball[a].bonus_parent!=0) ball[tmp2].bonus=ball[a].bonus_parent; 
    /* Crée une nouvelle balle plus petite qui ira à droite */	
	tmp2=createBall(ball[a].posx,ball[a].posy,ball[a].type+1,RIGHT,UP);
	ball[tmp2].hauteurmax_cpt = ball[tmp2].hauteurmax-(20);
	ball[tmp2].vel=2.0;
	/* Si la balle <a> qui explose contient un bonus, on le crée */
	if (ball[a].bonus!=0)
	{
       createBonus(ball[a].bonus,(int)ball[a].posx+10,(int)ball[a].posy+10);
	}
	/* On libère la balle (et le sprite associé) qui vient d'exploser */
	ball[a].utilise = 0;
	releaseSprite(ball[a].spriteno);
}

void explodeAllBall()
{
	/* On trouve une grosse boule, si elle existe on l'explose */
	int sortie=0;
	int trouve=-1;
	int b=0;
	while (sortie==0)
	{
		if ((ball[b].type==BIG) && (ball[b].utilise==1))
		{
			trouve = b;
			sortie=1;
		}
		b++;
		if (b==MAX_BALL) sortie=1;
	}
	
	/* Si on l'a trouvé, on l'explose ! */
	if (trouve!=-1)
	{
		/* BOOM */
		explodeABall(trouve);
	}
	else /* Sinon on continue de chercher pour une normale */
	{
		b=0;
		sortie=0;
		while (sortie==0)
		{
			if ((ball[b].type==NORMAL) && (ball[b].utilise==1))
			{
				trouve = b;
				sortie=1;				
			}
			b++;
			if (b==MAX_BALL) sortie=1;
		}
		
		if (trouve!=-1)
		{
			/* BOOM */
			explodeABall(trouve);
		}
		else
		{
			b=0;
			sortie=0;
			while (sortie==0)
			{
				if ((ball[b].type==SMALL) && (ball[b].utilise==1))
				{
					trouve = b;
					sortie=1;
				}
				b++;
				if (b==MAX_BALL) sortie=1;
			}			
			if (trouve!=-1)
			{
				/* BOOM */
				explodeABall(trouve);
			}
			else
			{
				/* Plus rien à exploser */
				player.bonus_boom=0;
			}
		}
	}
}

int createBigBall(double posx,double posy,int hdirection,int vdirection)
{
	int i;
	i=0;
	while ((ball[i].utilise==1) && (i<MAX_BALL))
	{
		i++;
	}
	
	ball[i].posx= posx;
	ball[i].posy= posy;
	ball[i].hauteurmax= 180*scale;
	ball[i].hauteurmax_cpt= 160*scale;
	ball[i].speedx= 1;
	ball[i].speedy= 3;
	ball[i].lr= RIGHT;
	ball[i].ud= DOWN;
	ball[i].coefdivaccell= 10;
	ball[i].speed_cpt= 0;	
	ball[i].utilise= 1;	
	ball[i].xbox = 6*scale;
	ball[i].ybox = 4*scale;
	ball[i].hbox = (40-6)*scale;
	ball[i].lbox = (48-10)*scale;
	ball[i].type = BIG;
	ball[i].suspend = 0;
	ball[i].bonus = 0;
	ball[i].bonus_parent = 0;
	ball[i].spriteno = initFreeSprite((int)ball[i].posx,(int)ball[i].posy,BIG);
	ball[i].last_posx=posx;
	ball[i].last_posy=posy;
	
#ifdef RES320X240
	ball[i].vel = -10.0;
	ball[i].vel_cst = 12.5;
#else
	ball[i].vel = -10;
	ball[i].vel_cst = 17.5;
#endif
	ball[i].move=2;

	if (hdirection==LEFT)
	{
		ball[i].lr=LEFT;
		ball[i].move = -ball[i].move;
	}
	if (vdirection==UP) 
	{
		ball[i].ud=UP;
		ball[i].vel = -ball[i].vel;
	}

	ball[i].nbtouch = 0;
	ball[i].y_a_til_eu_collision_avant=0;

	return i;
}

int createNormalBall(double posx,double posy,int hdirection,int vdirection)
{
	int i;
	i=0;
	while ((ball[i].utilise==1) && (i<MAX_BALL))
	{
		i++;
	}
	
	ball[i].posx= posx;
	ball[i].posy= posy;
	ball[i].hauteurmax= 120*scale;
	ball[i].hauteurmax_cpt= 100*scale;
	ball[i].speedx= 1;
	ball[i].speedy= 3;
	ball[i].lr= RIGHT;
	ball[i].ud= DOWN;
	ball[i].coefdivaccell= 20;
	ball[i].speed_cpt= 0;	
	ball[i].utilise= 1;
	ball[i].xbox = 3*scale;
	ball[i].ybox = 3*scale;	
	ball[i].hbox = (26-6);
	ball[i].lbox = (32-6)*scale;
	ball[i].type = NORMAL;
	ball[i].suspend = 0;
	ball[i].bonus = 0;
	ball[i].bonus_parent = 0;
	ball[i].spriteno = initFreeSprite((int)posx,(int)posy,NORMAL);

#ifdef RES320X240
	ball[i].vel = -10.0;
	ball[i].vel_cst = 10.0;
#else
	ball[i].vel = -10.0;
	ball[i].vel_cst = 14.0;
#endif
	ball[i].move=2;
	
		ball[i].last_posx=posx;
	ball[i].last_posy=posy;
	
	if (hdirection==LEFT)
	{
		ball[i].lr=LEFT;
		ball[i].move = -ball[i].move;
	}
	if (vdirection==UP) 
	{
		ball[i].ud=UP;
		ball[i].vel = -ball[i].vel;
	}
   	ball[i].nbtouch = 0; 
	ball[i].y_a_til_eu_collision_avant=0;
	return i;
}

int createSmallBall(double posx,double posy,int hdirection,int vdirection)
{
	int i;
	i=0;
	while ((ball[i].utilise==1) && (i<MAX_BALL))
	{
		i++;
	}
	
	ball[i].posx= posx;
	ball[i].posy= posy;
	ball[i].hauteurmax= 80*scale;
	ball[i].hauteurmax_cpt= 60*scale;
	ball[i].speedx= 1;
	ball[i].speedy= 3;
	ball[i].lr= RIGHT;
	ball[i].ud= DOWN;
	ball[i].coefdivaccell= 30;
	ball[i].speed_cpt= 0;	
	ball[i].utilise= 1;
	ball[i].xbox = 1*scale;
	ball[i].ybox = 1*scale;	
	ball[i].hbox = (14-3)*scale;
	ball[i].lbox = (16-3)*scale;
	ball[i].type = SMALL;
	ball[i].suspend = 0;
	ball[i].bonus = 0;
	ball[i].bonus_parent = 0;
	ball[i].spriteno = initFreeSprite((int)ball[i].posx,(int)ball[i].posy,SMALL);
	
#ifdef RES320X240
	ball[i].vel = -8.0;
	ball[i].vel_cst = 8.0;
#else
	ball[i].vel = -8.0;
	ball[i].vel_cst = 11.5;
#endif
	ball[i].move=2;
	
		ball[i].last_posx=posx;
	ball[i].last_posy=posy;
	
	if (hdirection==LEFT)
	{
		ball[i].lr=LEFT;
		ball[i].move = -ball[i].move;
	}
	if (vdirection==UP) 
	{
		ball[i].ud=UP;
		ball[i].vel = -ball[i].vel;
	}
	ball[i].nbtouch = 0;
	ball[i].y_a_til_eu_collision_avant=0;
	return i;
}

int createMicroBall(double posx,double posy,int hdirection,int vdirection)
{
	int i;
	
	i=0;
	while ((ball[i].utilise==1) && (i<MAX_BALL))
	{
		i++;
	}
	
	ball[i].posx= posx;
	ball[i].posy= posy;
	ball[i].hauteurmax= 60*scale;
	ball[i].hauteurmax_cpt= 30*scale;
	ball[i].speedx= 1;
	ball[i].speedy= 3;
	ball[i].lr= RIGHT;
	ball[i].ud= DOWN;
	ball[i].coefdivaccell= 40;
	ball[i].speed_cpt= 0;	
	ball[i].utilise= 1;
	ball[i].xbox = 1*scale;
	ball[i].ybox = 1*scale;	
	ball[i].hbox = (5)*scale;
	ball[i].lbox = (6)*scale;
	ball[i].type = MICRO;
	ball[i].suspend = 0;
	ball[i].bonus = 0;
	ball[i].bonus_parent = 0;
	ball[i].spriteno = initFreeSprite((int)ball[i].posx,(int)ball[i].posy,MICRO);
	
#ifdef RES320X240
	ball[i].vel = -6.5;
	ball[i].vel_cst = 6.5;
#else
	ball[i].vel = -8.5;
	ball[i].vel_cst = 8.5;
#endif

	ball[i].move=2;
	
	ball[i].last_posx=posx;
	ball[i].last_posy=posy;
	
	if (hdirection==LEFT) 
	{
		ball[i].lr=LEFT;
		ball[i].move = -ball[i].move;
	}
	if (vdirection==UP) 
	{
		ball[i].ud=UP;
		ball[i].vel = -ball[i].vel;
	}
	ball[i].nbtouch = 0;
	ball[i].y_a_til_eu_collision_avant=0;
	return i;
}

int createBall(double posx,double posy,int typeBall,int hdirection,int vdirection)
{
	if (typeBall==BIG) return(createBigBall(posx,posy,hdirection,vdirection));
	if (typeBall==NORMAL) return(createNormalBall(posx,posy,hdirection,vdirection));
	if (typeBall==SMALL) return(createSmallBall(posx,posy,hdirection,vdirection));
	if (typeBall==MICRO) return(createMicroBall(posx,posy,hdirection,vdirection));
    return 0;
}

void updateBalls(int i)
{		
/*
     Si (ball[i].utilise == 1)
     {
        Tester si on est en collision de plateforme
        {
        }
        Tester si on est en collision de bordure
        {
        }
        Tester si on est en collision de balle
        {
        }
        
        Si (il n'y à pas eu de collision)
        {
            ball[i].last_posx = ball[i].posx;
			ball[i].last_posy = ball[i].posy; 
        }
        
        Si (le joueur n'a pas de bonus freeze)
        {
               bouger la balle <i>
        }
     }
*/


	int p=0;
	int sortie=0;
	int sortie2=0;
	int nbcollide=0;
	int collide=0;
	int totalcollide = 0;	
	
	int y_a_t_il_eu_collision_de_bord = 0;
    
    int y_a_t_il_eu_collision = 0;	

	if (ball[i].utilise==1)
	{	
		/* il faut récupérer la derniere position sans aucune collision 
           Pour cela on teste si on est en collision, si ce n'est pas le
           cas, on mets à jour last_posx et last_posy. Sinon, on ne mets 
           rien à jour et on à les bonnes dernières coordonnées de la position
           sans collision.
        */

		for (p=0;p<MAX_PLATEFORME;p++)	
		{
			if (pforme[p].utilise==1)
			{
				collide += ballCollideWithPlatform(ball[i].spriteno,pforme[p].spriteno,i);
			}
		}
				
		if (collide!=0)
		{
		   y_a_t_il_eu_collision = 1;
        }
		
		for (p=0;p<MAX_SHOOT;p++)
		{
			if (shoot[p].utilise==1)
			{
				collide = isCollide(ball[i].xbox+(int)ball[i].posx,ball[i].ybox+(int)ball[i].posy,ball[i].hbox,ball[i].lbox,shoot[p].posx+shoot[p].xbox,shoot[p].posy+shoot[p].ybox,shoot[p].hbox,shoot[p].lbox);
				if (collide!=0) 
				{
					shoot[p].utilise=0;
					player.nbtir--;
					if (ball[i].type<MICRO) 
					{
						int tmp;
						tmp = createBall(ball[i].posx,ball[i].posy,ball[i].type+1,LEFT,UP);
						
						if (player.bonus_freeze>0)
						{
							int old_posx = ball[i].posx;
							int old_posy = ball[i].posy;
							ball[tmp].posx = ball[i].posx+imageBank[sprite[ball[tmp].spriteno].image].imagel;
							moveSprite(ball[tmp].spriteno,(int)ball[i].posx+imageBank[sprite[ball[tmp].spriteno].image].imagel,(int)ball[i].posy);
							int plate;
							int platec=0;
							for (plate=0;plate<MAX_PLATEFORME;plate++)
							{
								platec-=ballCollideWithPlatform(ball[tmp].spriteno,pforme[plate].spriteno,tmp);
							}
							if (platec!=0) moveSprite(ball[tmp].spriteno,(int)old_posx,(int)old_posy);
						}
							
						ball[tmp].hauteurmax_cpt = ball[tmp].hauteurmax-20*scale;
						ball[tmp].vel=2.0;
						if (ball[i].bonus_parent!=0) ball[tmp].bonus=ball[i].bonus_parent;							
						tmp=createBall(ball[i].posx,ball[i].posy,ball[i].type+1,RIGHT,UP);
						ball[tmp].hauteurmax_cpt = ball[tmp].hauteurmax-20*scale;
						ball[tmp].vel=2.0;						
						
					}
					/* score ! */
					if (player.derniere_balle==ball[i].type)
					{
						player.multiplicateur++;
					}
					else
					{
						player.multiplicateur=1;	
					}
					if (ball[i].type==MICRO)
					{
						player.score = player.score + (4*player.multiplicateur);
					}
					else if (ball[i].type==SMALL)
					{
						player.score = player.score + (3*player.multiplicateur);
					}							
					else if (ball[i].type==NORMAL)
					{
						player.score = player.score + (2*player.multiplicateur);
					}							
					else if (ball[i].type==BIG)
					{
						player.score = player.score + (1*player.multiplicateur);
					}
					player.derniere_balle = ball[i].type;
					if (player.multiplicateur>1)
					{
						createObjet(OBJ_MUL,(int)ball[i].posx,(int)ball[i].posy,player.multiplicateur);
					}
					/* il fo détruire la balle EN DERNIER ! Sinon une autre sera reprise à ça position */
					ball[i].utilise = 0;
					if (ball[i].bonus!=0)
					{
						if (ball[i].bonus==BONUS_LIFE)
						{
							if ((player.bonus_life==0) && (player.bonus_life_level!=currentLevel))	
							{
								createBonus(ball[i].bonus,(int)ball[i].posx+5*scale,(int)ball[i].posy+5*scale);
							}
						}
						else
						{
							createBonus(ball[i].bonus,(int)ball[i].posx+5*scale,(int)ball[i].posy+5*scale);
						}
					}
					releaseSprite(ball[i].spriteno);
					playSound(4,0);
//					GpPcmPlay((unsigned short*)ball_explode,sizeof(ball_explode), 0);	
				}
			}
		}
			
		
		if (player.bonus_freeze==0)
		{
    		sortie=0;
			sortie2=0;
			nbcollide=0;
			collide=0;
			totalcollide = 0;
			p=0;
			while (sortie2==0)
			{				
				if (pforme[p].utilise==1)
				{	
					collide = ballCollideWithPlatform(ball[i].spriteno,pforme[p].spriteno,i);
					totalcollide = collide;					
					if (collide!=0)
					{
                        			y_a_t_il_eu_collision=1;
						ball[i].posx = ball[i].last_posx;
						ball[i].posy = ball[i].last_posy;						
						moveSprite(ball[i].spriteno,(int)ball[i].posx,(int)ball[i].posy);						
						sortie2=1;
					}					
				}
				p++;
				if (p==MAX_PLATEFORME) sortie2=1;	
			}
			
			
			if (totalcollide==TOUCH_HORIZONTAL)
			{
				ball[i].vel = -ball[i].vel;
				if (ball[i].vel==0) ball[i].move = -ball[i].move;
			}
			
			
			if (totalcollide==TOUCH_VERTICAL)
			{
				ball[i].move = -ball[i].move;
			}		
		
		
		    y_a_t_il_eu_collision_de_bord = checkBallCollisionWithBorder(i); 
		    if (y_a_t_il_eu_collision_de_bord!=0) y_a_t_il_eu_collision=1;

       		if (y_a_t_il_eu_collision==0)
       		{
       			ball[i].last_posx = ball[i].posx;
       			ball[i].last_posy = ball[i].posy;
			ball[i].y_a_til_eu_collision_avant=0;
			ball[i].nbtouch = 0;
       		}
		else
		{
			if (ball[i].y_a_til_eu_collision_avant==1)
			{
				ball[i].nbtouch++;
			}
			if (ball[i].y_a_til_eu_collision_avant==0)
			{
				ball[i].y_a_til_eu_collision_avant=1;
			}
		}
			
		if (ball[i].nbtouch>5) ball[i].posx = ball[i].posx + ball[i].move; 
						
			ball[i].posx = ball[i].posx + ball[i].move;
			ball[i].vel = ball[i].vel - GRAV;
			ball[i].posy = ball[i].posy - ball[i].vel;
        }
//	        fprintf(s	tderr,"move: %d\n",(int)ball[i].move);            
//	        fprintf(stderr,"vel: %d\n",(int)ball[i].vel);            	        
//	        fprintf(stderr,"posx: %d\n",(int)ball[i].posx);      	        
//	        fprintf(stderr,"posy: %d\n",(int)ball[i].posy);      	        	        

	}
}

