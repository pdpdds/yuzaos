#include "shoot.h"
#include "ball.h"
#include "collisions.h"
#include "objets.h"
#include "structures.h"
#include "main.h"
#ifdef PC_VERSION
       #include "PCGfxEngine.h"
#endif
#ifdef DREAMCAST_VERSIONc
       #include "DCGfxEngine.h"
#endif


int isCollideLadder(int r1x,int r1y,int r1h,int r1l,int r2x,int r2y,int r2h,int r2l)
{
    if(r1x+r1l < r2x) return 0;
    if(r1x > r2x+r2l) return 0;
    if(r1y+r1h < r2y) return 0;
    if(r1y > r2y+r2h) return 0;

    return(1);
}

void initPlayer()
{
    player.posx=150*scale;
    player.posy=(200-32)*scale;
    player.xbox=4*scale;
    player.ybox=2*scale;
    player.hbox=28*scale;        /* Hauteur et largeur de la boite de collision */
    player.lbox=18*scale;
    player.nblive=3;
    player.weapon=WEAPON_NORMAL_SHOOT;
    player.score=0;
    player.nbtir=0;        /* Nombre de tir en cours ... */
    player.etat=PLAYER_STOP;
    player.old_etat=-1;
    player.anim_courante=ANIM_RIGHT1;
    player.derniere_balle = 0;
    player.multiplicateur=1;
    player.shoot_timer=0;
    player.spriteno=0;
    player.bonus_boom=0;
    player.bonus_freeze=0;
    initSprite(player.spriteno,player.posx,player.posy,player.anim_courante);
    player.bonus_life=0;
    player.bonus_life_level=-1;
    player.bonus_protection=0;
    player.bonus_protection_timer=-1;
    player.en_descente=0;
}

void reInitPlayer()
{
    player.posx=150*scale;
    player.posy=(200-32)*scale;
    player.xbox=4*scale;
    player.ybox=2*scale;
    player.hbox=28*scale;        /* Hauteur et largeur de la boite de collision */
    player.lbox=18*scale;
    player.weapon=WEAPON_NORMAL_SHOOT;
    /*player.score=0;*/
    player.nbtir=0;        /* Nombre de tir en cours ... */
    player.etat=PLAYER_STOP;
    player.old_etat=-1;
    player.anim_courante=ANIM_RIGHT1;
    player.derniere_balle = 0;
    player.multiplicateur=1;
    player.shoot_timer=0;
    player.spriteno=0;
    player.bonus_boom=0;
    player.bonus_freeze=0;
    initSprite(player.spriteno,player.posx,player.posy,player.anim_courante);
    player.bonus_life=0;
    /* NE PAS REINITIALISER BONUS_LIFE_LEVEL ! */
    player.bonus_protection=0;
    player.bonus_protection_timer=-1;
    player.en_descente=0;
}

void updatePlayer()
{
    int i;
    //unsigned char keydata; // evilo

    //int isOnLadder=0; // evilo
    int whereIsPlayer=999;
                /* 0 - En bas de l'?ran */
                /* 1 - En bas d'une ?helle */
                /* 2 - Sur une Echelle */
                /* 3 - En haut d'une echelle */
                /* 4 - Sur une plateforme */
                /* 5 - Dans le vide */
    int echelleCollide=-1;
    int plateformeCollide=-1;
    int plateformeCollideBlock=-1;

    for (i=0;i<MAX_ECHELLE;i++)
    {
        /* Si le joueur est sur une base (en bas) --> BAS */
        /* Si les pieds du joueur sont en haut de l'echelle --> HAUT */
        /* SINON Si le joueur est sur un milieu UNIQUEMENT */

        if (ech[i].utilise==1)
        {
            if ((isCollideLadder(player.posx+12*scale,player.posy+30*scale,2*scale,10*scale,ech[i].pad_milieu_x,ech[i].pad_milieu_y,ech[i].pad_milieu_hauteur,ech[i].pad_milieu_largeur))
		&& (player.en_descente<3))
            {
                whereIsPlayer = 2;
                echelleCollide=i;
            }
            /* bas */
            if (isCollideLadder(player.posx+12*scale,player.posy+30*scale,2*scale,10*scale,ech[i].pad_bas_x,ech[i].pad_bas_y,ech[i].pad_bas_hauteur,ech[i].pad_bas_largeur)
		&& (player.en_descente<3))
            {
                whereIsPlayer = 1;
                echelleCollide=i;
            } /* haut */
            if (isCollideLadder(player.posx+12*scale,player.posy+30*scale,2*scale,10*scale,ech[i].pad_haut_x,ech[i].pad_haut_y,ech[i].pad_haut_hauteur,ech[i].pad_haut_largeur)
		&& (player.en_descente<3))
            {
                whereIsPlayer = 3;
                echelleCollide=i;
                player.en_descente=0;
            }
        }
    }

    for (i=0;i<MAX_PLATEFORME;i++)
    {
            if (pforme[i].utilise==1)
            {
                if (whereIsPlayer>3)
                {
                    while ((isCollideLadder(player.posx+12*scale,player.posy+30*scale,2*scale,6*scale,pforme[i].posx,pforme[i].posy,pforme[i].hauteur,pforme[i].largeur)))
                    {
                        whereIsPlayer = 4;
                        plateformeCollide=i;
                        player.en_descente=0;
                        player.posy--;
                    }
                }
            }
    }

    /*sprintf(debug,"%d",whereIsPlayer);*/

    /*GpRectFill(NULL, &gpDraw[nflip], player.posx+2,player.posy+30,28,2,232);
    GpRectFill(NULL, &gpDraw[nflip], player.posx+5,player.posy+15,22,10,232);
    GpRectFill(NULL, &gpDraw[nflip], player.posx+5,player.posy,15,25,232);*/

    /* Si on est pas sur une ?helle ou est on ? */
    if (whereIsPlayer==999)
    {
        if (player.posy==(200-32)*scale)
        {
            whereIsPlayer=0;
            player.en_descente=0;
        }
        else
        {
            whereIsPlayer=5;
        }
    }

    if (whereIsPlayer==5)
    {
        player.posy++;
        player.en_descente++;
        player.posy++; //doppelt ?
        player.en_descente++; //doppelt ?
    }
    if (whereIsPlayer==4)
    {
        player.posy=pforme[plateformeCollide].posy - 32 *scale;
    }

    /*player.old_etat = player.etat;*/
    if (( keyLeft==1) && (player.anim_courante!=ANIM_SHOOT) && (whereIsPlayer!=2))
    {
        player.xbox=8*scale;
        player.ybox=2*scale;
        player.hbox=28*scale;        /* Hauteur et largeur de la boite de collision */
        player.lbox=22*scale;

        if (gbl_timer%3==0) 
            player.posx-=2*scale;
        else
            player.posx-=1*scale; //doppelt ?
	
        int sortie=0;
        while (sortie==0)
        {
            plateformeCollideBlock=-1;
            for (i=0;i<MAX_PLATEFORME;i++)
            {
                if (pforme[i].utilise==1)
                {
                    if((isCollideLadder(player.posx+5*scale,player.posy+6*scale,19*scale,2*scale,pforme[i].posx,pforme[i].posy,pforme[i].hauteur,pforme[i].largeur)))
                    {
                        plateformeCollideBlock=i;
                    }
                }
            }
            if (plateformeCollideBlock==-1)
            {
                sortie=1;
            }
            else
            {
                if (gbl_timer%3==0)
                    player.posx+=2*scale;
		else
                    player.posx+=1*scale;
            }
        }

        player.old_etat = player.etat;
        player.etat=PLAYER_LEFT;
    }else
    if (( keyRight ) && (player.anim_courante!=ANIM_SHOOT) && (whereIsPlayer!=2))
    {
        player.xbox=4*scale;
        player.ybox=2*scale;
        player.hbox=28*scale;        /* Hauteur et largeur de la boite de collision */
        player.lbox=18*scale;

        if (gbl_timer%3==0)
            player.posx+=2*scale;
        else
            player.posx+=1*scale;
	
        int sortie=0;
        while (sortie==0)
        {
            plateformeCollideBlock=-1;
            for (i=0;i<MAX_PLATEFORME;i++)
            {
                if (pforme[i].utilise==1)
                {
                      if ((isCollideLadder(player.posx+25*scale,player.posy+6*scale,19*scale,2*scale,pforme[i].posx,pforme[i].posy,pforme[i].hauteur,pforme[i].largeur)))
                    {
                        plateformeCollideBlock=i;
                    }
                }
            }
            if (plateformeCollideBlock==-1)
            {
                sortie=1;
            }
            else
            {
                if (gbl_timer%3==0)
                    player.posx-=2*scale;
                else
                    player.posx-=1*scale;
            }
        }

        player.old_etat = player.etat;
        player.etat=PLAYER_RIGHT;
    }else if ((keyUp) && (whereIsPlayer>0) && (whereIsPlayer<3))
    {
        player.posy--;
        player.posx=ech[echelleCollide].posx;
        player.old_etat = player.etat;
        player.etat = PLAYER_LADDER;
    }
    else if ((keyDown) && (whereIsPlayer>0) && (whereIsPlayer<4))
    {
        player.posy+=1*scale;
        player.posx=ech[echelleCollide].posx;
        player.old_etat = player.etat;
        player.etat = PLAYER_LADDER;
    }
    else
    {
        if (whereIsPlayer!=2)
        {
            player.etat=PLAYER_STOP;
            player.anim_courante = ANIM_STOP;
        }
        else
        {
            player.etat=PLAYER_STOP;
            player.anim_courante = ANIM_LADDER1;
        }
    }

    if (keyAction4==1)
    {
//        keydata = GpKeyGet();
    }

    if ((( keyAction1==1 ) || ( keyAction2==1 )) && (player.shoot_timer==0))

    {
        player.xbox=2*scale;
        player.ybox=2*scale;
        player.hbox=30*scale;        /* Hauteur et largeur de la boite de collision */
        player.lbox=30*scale;
        if ((player.nbtir<1) && (player.weapon==WEAPON_NORMAL_SHOOT))
        {
            createShoot(WEAPON_NORMAL_SHOOT);
            player.nbtir ++ ;
            player.anim_courante=ANIM_SHOOT;
            player.anim_cpt=5;
            player.shoot_timer=1;
        } else if ((player.nbtir<1) && (player.weapon==WEAPON_GLUE_SHOOT))
        {
            createShoot(WEAPON_GLUE_SHOOT);
            player.nbtir ++ ;
            player.anim_courante=ANIM_SHOOT;
            player.anim_cpt=5;
            player.shoot_timer=1;
        } else if ((player.nbtir<2) && (player.weapon==WEAPON_DOUBLE_SHOOT) && (player.shoot_timer==0))
        {
            createShoot(WEAPON_DOUBLE_SHOOT);
            player.nbtir ++ ;
            player.anim_courante=ANIM_SHOOT;
            player.anim_cpt=5;
            player.shoot_timer=1;
        }
    }

/* Pause TODO !! */
//    if (GpKeyGet() & GPC_VK_START)
//    {
//        while (GpKeyGet() & GPC_VK_START) {}
//        while (!(GpKeyGet() & GPC_VK_START)) {}
//        while (GpKeyGet() & GPC_VK_START) {}
//    }

    /* Pour empecher le tir automatique ! */
    if ((keyAction1==0) && (keyAction2==0)) player.shoot_timer=0;

    /* Test des collisions droite gauche*/
    if (player.posx<8*scale) player.posx=8*scale;
    if (player.posx>283*scale) player.posx=283*scale;

    if (player.posy>(200-32)*scale) player.posy=(200-32)*scale;

    if (!(CHEAT))
    {
        if (player.bonus_freeze==0)
        {
            for (i=0;i<MAX_BALL;i++)
            {
                if (ball[i].utilise==1)
                {
                    if (isSpriteCollide(player.spriteno,ball[i].spriteno)==1)
//        	keyAction4 = 1;
                    {
                        if (player.bonus_protection==1)
                        {
                            if (player.bonus_protection_timer==-1)
                            {
                                player.bonus_protection_timer=20;
                            }
                            explodeABall(i);
                        }

                        // si on est pas deja mort
                        else if (gbl_evt!=EVT_LOOSE_LIFE)
                        {
                            gbl_evt = EVT_LOOSE_LIFE;
//                            GpPcmStop();
                            sound_etat=0;
                            player.nblive-=1;
                        }
                    }
                }
            }
        }
    }


    /* Test des collisions avec les bonus */
    for (i=0;i<MAX_BONUS;i++)
    {
        if (bonus[i].utilise==1)
        {
            if (isCollide(player.xbox+player.posx,player.ybox+player.posy,player.hbox,player.lbox,bonus[i].xbox+bonus[i].posx,bonus[i].ybox+bonus[i].posy,bonus[i].hbox,bonus[i].lbox)==1)

            {
                if (bonus[i].type==WEAPON_DOUBLE_SHOOT)
                {
                    player.weapon=WEAPON_DOUBLE_SHOOT;
                    bonus[i].utilise=0;
                }
                if (bonus[i].type==WEAPON_GLUE_SHOOT)
                {
                    player.weapon=WEAPON_GLUE_SHOOT;
                    bonus[i].utilise=0;
                }
                else if (bonus[i].type==BONUS_BOOM)
                {
                    player.bonus_boom=1;
//                    player.bonus_freeze=0;
                    bonus[i].utilise=0;
                }
                else if (bonus[i].type==BONUS_FREEZE)
                {
                    player.bonus_freeze=300;
                    player.bonus_boom=0;
                    bonus[i].utilise=0;
                }
                else if (bonus[i].type==BONUS_LIFE)
                {
                    player.bonus_life=1;
                    player.bonus_life_level = currentLevel;
                    player.nblive+=1;
                    bonus[i].utilise=0;
                    createObjet(OBJ_1UP,player.posx,player.posy,0);
                }
                else if (bonus[i].type==BONUS_PROTECTION)
                {
                    player.bonus_protection=1;
                    player.bonus_protection_timer = -1;
                    bonus[i].utilise=0;
                }
            }
        }
    }

    if ((player.bonus_boom==1) && (gbl_timer%10==0)) explodeAllBall();
    if (player.bonus_freeze>0) player.bonus_freeze--;
    if (player.bonus_protection_timer!=-1) {player.bonus_protection_timer--;}

    if (player.bonus_protection_timer==0){player.bonus_protection=0;player.bonus_protection_timer=-1;};

    /* Mise ?jour de l'animation */
    if (player.anim_courante!=ANIM_SHOOT)
    {
        if (player.etat==PLAYER_RIGHT)
        {
            if (player.etat==player.old_etat)
            {
                if (gbl_timer%5==0)
                {
                    player.anim_courante++;

                    if (player.anim_courante>ANIM_RIGHT3) player.anim_courante = ANIM_RIGHT1;
                }
            }
            else
            {
                player.anim_courante = ANIM_RIGHT1;
            }
        }
        if (player.etat==PLAYER_LEFT)
        {
            if (player.etat==player.old_etat)
            {
                if (gbl_timer%5==0)
                {
                    player.anim_courante++;
                    if (player.anim_courante>ANIM_LEFT3) player.anim_courante = ANIM_LEFT1;
                }
            }
            else
            {
                player.anim_courante = ANIM_LEFT1;
            }
        }
        if (player.etat==PLAYER_LADDER)
        {
            if (player.etat==player.old_etat)
            {
                if (gbl_timer%5==0)
                {
                    player.anim_courante++;
                    if (player.anim_courante>ANIM_LADDER2) player.anim_courante = ANIM_LADDER1;
                }
            }
            else
            {
                player.anim_courante = ANIM_LADDER1;
            }
        }
    }

    if (player.anim_courante==ANIM_SHOOT)
    {
        player.anim_cpt--;
        if (player.anim_cpt==0)
        {
            if (player.old_etat==PLAYER_RIGHT) player.anim_courante = ANIM_RIGHT1;
            else if (player.old_etat==PLAYER_LEFT) player.anim_courante = ANIM_LEFT1;
            else player.anim_courante = ANIM_LADDER1;
        }
    }

}

void showPlayer()
{
        moveSprite(0,player.posx,player.posy);
        changeSpriteImage(0,player.anim_courante);
        if ((player.anim_courante==ANIM_RIGHT1) || (player.anim_courante==ANIM_RIGHT2) || (player.anim_courante==ANIM_RIGHT3))
            showSprite(player.spriteno);
        else
            showSprite(player.spriteno);

}


