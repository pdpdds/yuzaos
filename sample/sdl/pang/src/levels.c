#include "structures.h"
#include "ball.h"
#include "bonus.h"
#include "objets.h"
#include "shoot.h"
#include "plateforme.h"
#include "ladder.h"
#include "main.h"

void initLevel(int n)
{
	int i;
	
	initLadders();
	initShoot();
	initBalls();
	initBonus();
	initPlateformes(); 
	initObjets();
					


	// Pour l'instant testé jusko 21
	// n=21;

	if (n==1)
	{
		i=createBall((39-32)*scale,19*scale,BIG,LEFT,DOWN);
		ball[i].bonus = getRandomBonus();
//		ball[i].bonus = BONUS_LIFE;

		ball[i].bonus_parent = getRandomBonus();
	}
	if (n==2) 
	{
		i=createBall((120-32)*scale,18*scale,BIG,LEFT,DOWN);
		ball[i].bonus_parent = getRandomBonus();
		i=createPlateforme((161-32)*scale,81*scale,PF_CASSABLE);
		pforme[i].bonus = getRandomBonus();
		i=createPlateforme((193-32)*scale,81*scale,PF_CASSABLE);
		pforme[i].bonus = getRandomBonus();
	}
	if (n==3) 
	{
		createBall((76-32)*scale,32*scale,BIG,LEFT,DOWN);
		i=createBall((176-32)*scale,101*scale,SMALL,RIGHT,DOWN);
		ball[i].vel=0.5;
		i=createPlateforme((177-32)*scale,81*scale,PF_CASSABLE);
		pforme[i].bonus=getRandomBonus();
		i=createPlateforme((177-32)*scale,138*scale,PF_CASSABLE);
		pforme[i].bonus=getRandomBonus();
		createPlateforme((73-32)*scale,(81-8)*scale,PF_INCASSABLE);
		createPlateforme((281-32)*scale,(81-8)*scale,PF_INCASSABLE);
		/*createBall(184,23,BIG,LEFT,DOWN);
		createPlateformeCassable(160,100);*/
	}
		
	if (n==4)
	{
		i=createBall((39-32)*scale,19*scale,BIG,LEFT,DOWN);
		ball[i].bonus = getRandomBonus();
		ball[i].bonus_parent = getRandomBonus();
		createLadder((162-32)*scale,153*scale,12);
		createLadder((202-32)*scale,153*scale,12);
		createPlateforme((185-32)*scale,(153-1)*scale,PF_INCASSABLEV);
		createPlateforme((193-32)*scale,(153-1)*scale,PF_INCASSABLEV);
		player.posx=player.posx + 16*scale;
	}
	if (n==5)
	{
		i=createBall((49-32)*scale,70*scale,BIG,LEFT,DOWN);
		ball[i].bonus = WEAPON_DOUBLE_SHOOT;
		i=createBall(241*scale,120*scale,NORMAL,LEFT,DOWN);
		ball[i].bonus_parent = BONUS_FREEZE;
		createPlateforme((97-32)*scale,56*scale,PF_CASSABLEV);
		createPlateforme((97-32)*scale,81*scale,PF_CASSABLEV);
		i=createPlateforme((289-32)*scale,56*scale,PF_CASSABLEV);
		pforme[i].bonus = BONUS_BOOM;
		createPlateforme((289-32)*scale,81*scale,PF_CASSABLEV);
		createPlateforme((193-32)*scale,57*scale,PF_CASSABLEV);
	}
	if (n==6)
	{
		createPlateforme((81-32)*scale,65*scale,PF_MOYEN_INCASSABLE);
		createPlateforme((297-32)*scale,65*scale,PF_MOYEN_INCASSABLE);
		createPlateforme((185-32)*scale,65*scale,PF_MOYEN_CASSABLE);
		i=createBall((154-32)*scale,16*scale,BIG,LEFT,DOWN);
		ball[i].bonus = WEAPON_DOUBLE_SHOOT;
		createBall(192*scale-32*scale,16*scale,BIG,RIGHT,DOWN);
	}
	if (n==7)
	{
		createPlateforme(97*scale-32*scale-16*scale,49*scale,PF_MICRO_INCASSABLE);
		createPlateforme(105*scale-32*scale-16*scale,57*scale,PF_MICRO_INCASSABLE);
		createPlateforme(113*scale-32*scale-16*scale,65*scale,PF_MICRO_INCASSABLE);
		createPlateforme(129*scale-32*scale-16*scale,105*scale,PF_MICRO_INCASSABLE);
		createPlateforme(137*scale-32*scale-16*scale,121*scale,PF_MICRO_INCASSABLE);
		createPlateforme(305*scale-32*scale-16*scale,49*scale,PF_MICRO_INCASSABLE);
		createPlateforme(297*scale-32*scale-16*scale,57*scale,PF_MICRO_INCASSABLE);
		createPlateforme(289*scale-32*scale-16*scale,65*scale,PF_MICRO_INCASSABLE);
		createPlateforme(273*scale-32*scale-16*scale,105*scale,PF_MICRO_INCASSABLE);
		createPlateforme(265*scale-32*scale-16*scale,121*scale,PF_MICRO_INCASSABLE);
		createPlateforme(281*scale-32*scale-16*scale,73*scale,PF_INCASSABLEV);
		createPlateforme(121*scale-32*scale-16*scale,73*scale,PF_INCASSABLEV);
		createPlateforme(193*scale-32*scale-16*scale-5*scale,121*scale,PF_CASSABLE);
		createPlateforme(161*scale-32*scale-16*scale,121*scale,PF_MOYEN_CASSABLE);
		createPlateforme(232*scale-32*scale-16*scale,121*scale,PF_MOYEN_CASSABLE);
		i=createBall(205*scale-32*scale,31*scale,BIG,LEFT,DOWN);
		ball[i].bonus=getRandomBonus();
		ball[i].bonus_parent=getRandomBonus();
		i=createBall(161*scale-32*scale,85*scale,NORMAL,RIGHT,DOWN);
		ball[i].bonus_parent=WEAPON_DOUBLE_SHOOT;
		
	}
	if (n==8)
	{
		createPlateforme(65*scale-32*scale,97*scale,PF_INCASSABLE);
		createPlateforme(65*scale+32*scale-32*scale,97*scale,PF_INCASSABLE);
		createPlateforme(65*scale-32*scale,97*scale+8*scale,PF_INCASSABLE);
		createPlateforme(65*scale+32*scale-32*scale,97*scale+8*scale,PF_INCASSABLE);
		
		createPlateforme(257*scale-32*scale,97*scale,PF_INCASSABLE);
		createPlateforme(257*scale+32*scale-32*scale,97*scale,PF_INCASSABLE);
		createPlateforme(257*scale-32*scale,97*scale+8*scale,PF_INCASSABLE);
		createPlateforme(257*scale+32*scale-32*scale,97*scale+8*scale,PF_INCASSABLE);
		createPlateforme(185*scale-32*scale,49*scale,PF_INCASSABLEV);
		createPlateforme(193*scale-32*scale,49*scale,PF_INCASSABLEV);
		createPlateforme(185*scale-32*scale,129*scale,PF_INCASSABLEV);
		createPlateforme(193*scale-32*scale,129*scale,PF_INCASSABLEV);
		i=createBall(150*scale-32*scale,79*scale,NORMAL,RIGHT,DOWN);
		ball[i].bonus = BONUS_FREEZE;
		i=createBall(209*scale-32*scale,79*scale,NORMAL,RIGHT,DOWN);
		ball[i].bonus_parent=getRandomBonus();
	}

	if (n==9)
	{
		i=createBall(134*scale-32*scale-10*scale,61*scale,BIG,LEFT,DOWN);
		ball[i].bonus=getRandomBonus();
		ball[i].bonus_parent=getRandomBonus();
		i=createBall(209*scale-32*scale+10*scale,61*scale,BIG,RIGHT,DOWN);
		ball[i].bonus=getRandomBonus();
		ball[i].bonus_parent=getRandomBonus();
		
		createLadder(51*scale,184*scale,4);
		createPlateforme(73*scale,183*scale,PF_MOYEN_INCASSABLE);
		createPlateforme(73*scale,183*scale+8*scale,PF_MOYEN_INCASSABLE);
		createLadder(90*scale,184*scale,4);
		
		createLadder(129*scale,184*scale,4);
		createPlateforme(151*scale,183*scale,PF_MOYEN_INCASSABLE);
		createPlateforme(151*scale,183*scale+8*scale,PF_MOYEN_INCASSABLE);
		createLadder(169*scale,184*scale,4);
		
		createLadder(213*scale,184*scale,4);
		createPlateforme(235*scale,183*scale,PF_MOYEN_INCASSABLE);
		createPlateforme(235*scale,183*scale+8*scale,PF_MOYEN_INCASSABLE);
		createLadder(253*scale,184*scale,4);
		player.posx = player.posx + 15*scale;
	}
	if (n==10) /* BUG DE COLLISION A ANALYSER */
	{

		createPlateforme(57*scale-32*scale,89*scale,PF_MICRO_CASSABLE);
		createPlateforme(81*scale-32*scale,113*scale,PF_MICRO_CASSABLE);
		createPlateforme(153*scale-32*scale,89*scale,PF_MICRO_CASSABLE);
		i=createPlateforme(177*scale-32*scale,113*scale,PF_MICRO_CASSABLE);
		pforme[i].bonus = getRandomBonus();
		createPlateforme(249*scale-32*scale,89*scale,PF_MICRO_CASSABLE);
		createPlateforme(273*scale-32*scale,113*scale,PF_MICRO_CASSABLE);


		createPlateforme(105*scale-32*scale,89*scale,PF_MICRO_INCASSABLE);
		createPlateforme(128*scale-32*scale,113*scale,PF_MICRO_INCASSABLE);
		createPlateforme(201*scale-32*scale,89*scale,PF_MICRO_INCASSABLE);
		createPlateforme(225*scale-32*scale,113*scale,PF_MICRO_INCASSABLE);
		createPlateforme(296*scale-32*scale,89*scale,PF_MICRO_INCASSABLE);
		createPlateforme(320*scale-32*scale,113*scale,PF_MICRO_INCASSABLE);


		i=createBall(42*scale-32*scale,54*scale,NORMAL,RIGHT,DOWN);
		ball[i].bonus = getRandomBonus();
		i=createBall(279*scale-32*scale,45*scale,BIG,RIGHT,DOWN);
		ball[i].bonus_parent = getRandomBonus();
	}	
	if (n==11)
	{
		createPlateforme(41*scale-32*scale,57*scale+5*scale,PF_MOYEN_CASSABLE);
		createPlateforme(105*scale-32*scale,48*scale+5*scale,PF_MOYEN_CASSABLE);
		i=createPlateforme(185*scale-32*scale,33*scale+5*scale,PF_MOYEN_CASSABLE);
		pforme[i].bonus = BONUS_LIFE;
		createPlateforme(265*scale-32*scale,49*scale+5*scale,PF_MOYEN_CASSABLE);
		i=createPlateforme(329*scale-32*scale,57*scale+5*scale,PF_MOYEN_CASSABLE);
		pforme[i].bonus = getRandomBonus();
		createPlateforme(81*scale-32*scale,89*scale+5*scale,PF_MOYEN_CASSABLE);
		createPlateforme(160*scale-32*scale,81*scale+5*scale,PF_MOYEN_CASSABLE);
		createPlateforme(209*scale-32*scale,81*scale+5*scale,PF_MOYEN_CASSABLE);
		i=createPlateforme(289*scale-32*scale,89*scale+5*scale,PF_MOYEN_CASSABLE);
		pforme[i].bonus = getRandomBonus();
		createPlateforme(121*scale-32*scale,121*scale+5*scale,PF_MOYEN_CASSABLE);
		i=createPlateforme(250*scale-32*scale,121*scale+5*scale,PF_MOYEN_CASSABLE);
		pforme[i].bonus = getRandomBonus();
		createPlateforme(161*scale-32*scale,145*scale+5*scale,PF_MOYEN_CASSABLE);
		createPlateforme(209*scale-32*scale,145*scale+5*scale,PF_MOYEN_CASSABLE);
		createBall(119*scale-32*scale,20*scale,NORMAL,RIGHT,DOWN);
		i=createBall(217*scale-32*scale,37*scale,BIG,RIGHT,DOWN);
		ball[i].bonus_parent=getRandomBonus();
		
	}
	if (n==12)
	{
		i=createBall(39*scale-32*scale,19*scale,BIG,LEFT,DOWN);
		ball[i].bonus_parent = getRandomBonus();
		i=createBall(276*scale-32*scale,19*scale,BIG,RIGHT,DOWN);
		ball[i].bonus_parent = getRandomBonus();
		createLadder(177*scale-32*scale,56*scale,36);
		createPlateforme(177*scale-32*scale-32*scale,55*scale,PF_INCASSABLE);
		createPlateforme(199*scale-32*scale,55*scale,PF_INCASSABLE);
		
		createPlateforme(88*scale-32*scale+6*scale,160*scale-4*scale,PF_MOYEN_INCASSABLE);
		createPlateforme(88*scale-32*scale+6*scale,168*scale-4*scale,PF_MOYEN_INCASSABLE);
		createPlateforme(280*scale-32*scale-6*scale,160*scale-4*scale,PF_MOYEN_INCASSABLE);
		createPlateforme(280*scale-32*scale-6*scale,168*scale-4*scale,PF_MOYEN_INCASSABLE);
	}	
	if (n==13)
	{
		createPlateforme(8*scale,139*scale,PF_INCASSABLE);
		createPlateforme(50*scale,139*scale,PF_INCASSABLE);
		createPlateforme(104*scale,139*scale,PF_INCASSABLE);
		createPlateforme(136*scale,139*scale,PF_INCASSABLE);
		createPlateforme(180*scale,139*scale,PF_INCASSABLE);
		createPlateforme(234*scale,139*scale,PF_INCASSABLE);
		createPlateforme(280*scale,139*scale,PF_INCASSABLE);
		
		createPlateforme(49*scale,80*scale,PF_MOYEN_INCASSABLE);
		createPlateforme(248*scale,80*scale,PF_MOYEN_INCASSABLE);
		createPlateforme(108*scale,38*scale,PF_MOYEN_INCASSABLE);
		createPlateforme(192*scale,38*scale,PF_MOYEN_INCASSABLE);
		
		createLadder(212*scale,140*scale,5);
		createLadder(82*scale,140*scale,15);
		
		i=createBall(134*scale,60*scale,BIG,RIGHT,DOWN);
		ball[i].bonus = WEAPON_GLUE_SHOOT;
		ball[i].bonus_parent = getRandomBonus();
		ball[i].vel = ball[i].vel/2;
		
		i=createBall(20*scale,105*scale,NORMAL,RIGHT,DOWN);
		ball[i].bonus = getRandomBonus();
		ball[i].bonus_parent = WEAPON_GLUE_SHOOT;
		ball[i].vel = ball[i].vel/2;
	}


	if (n==14)
	{
		createPlateforme(65*scale,118*scale,PF_INCASSABLE);
		createPlateforme(119*scale,118*scale,PF_INCASSABLE);
		createPlateforme(151*scale,118*scale,PF_INCASSABLE);
		createPlateforme(183*scale,118*scale,PF_INCASSABLE);
		createPlateforme(215*scale,118*scale,PF_INCASSABLE);
		createPlateforme(8*scale,66*scale,PF_INCASSABLE);
		createPlateforme(40*scale,66*scale,PF_INCASSABLE);
		createPlateforme(248*scale,55*scale,PF_INCASSABLE);
		createPlateforme(280*scale,55*scale,PF_INCASSABLE);
		
		createLadder(97*scale,119*scale,5);
		createLadder(247*scale,119*scale,20);
		
		createPlateforme(75*scale,28*scale,PF_CASSABLEV);
		createPlateforme(54*scale,81*scale,PF_CASSABLEV);
		
		i=createBall(9*scale,9*scale,BIG,RIGHT,DOWN);
		ball[i].bonus = getRandomBonus();
		ball[i].bonus_parent = getRandomBonus();
		
		i=createBall(222*scale,76*scale,NORMAL,RIGHT,DOWN);
		ball[i].bonus = getRandomBonus();
		ball[i].bonus_parent = getRandomBonus();
	}	
	if (n==15)
	{
		i=createBall(68*scale,128*scale,NORMAL,LEFT,DOWN);
		ball[i].bonus = getRandomBonus();
		ball[i].bonus_parent = getRandomBonus();
		i=createBall(68*scale,22*scale,NORMAL,LEFT,DOWN);
		ball[i].bonus_parent = getRandomBonus();
		ball[i].vel = ball[i].vel / 2;
		i=createBall(246*scale,20*scale,NORMAL,RIGHT,DOWN);
		ball[i].vel = ball[i].vel / 2;
		ball[i].bonus = getRandomBonus();
		ball[i].bonus_parent = getRandomBonus();
		i=createBall(246*scale,135*scale,NORMAL,RIGHT,DOWN);
		ball[i].bonus_parent = getRandomBonus();
		
		createPlateforme(8*scale,109*scale,PF_INCASSABLE);
		createPlateforme(62*scale,109*scale,PF_INCASSABLE);
		createPlateforme(94*scale,109*scale,PF_INCASSABLE);
		createPlateforme(147*scale,109*scale,PF_INCASSABLE);
		createPlateforme(179*scale,109*scale,PF_INCASSABLE);
		createPlateforme(211*scale,109*scale,PF_INCASSABLE);
		createPlateforme(265*scale,109*scale,PF_INCASSABLE);
		
		createLadder(243*scale,109*scale,23);	
		createLadder(125*scale,109*scale,6);	
		createLadder(40*scale,109*scale,23);		
	}

	if (n==16)
	{
		int i;
		i=createBall(21*scale,14*scale,MICRO,RIGHT,DOWN);
		ball[i].bonus = getRandomBonus();
		createBall(25*scale,30*scale,MICRO,RIGHT,DOWN);
		createBall(29*scale,46*scale,MICRO,RIGHT,DOWN);
		createBall(33*scale,62*scale,MICRO,RIGHT,DOWN);
		createBall(36*scale,77*scale,MICRO,RIGHT,DOWN);
		createBall(41*scale,92*scale,MICRO,RIGHT,DOWN);
		createBall(45*scale,110*scale,MICRO,RIGHT,DOWN);
		createBall(50*scale,127*scale,MICRO,RIGHT,DOWN);
		
		i=createBall(320*scale-21*scale,14*scale,MICRO,LEFT,DOWN);
		ball[i].bonus = getRandomBonus();
		createBall(320*scale-25*scale,30*scale,MICRO,LEFT,DOWN);
		createBall(320*scale-29*scale,46*scale,MICRO,LEFT,DOWN);
		createBall(320*scale-33*scale,62*scale,MICRO,LEFT,DOWN);
		createBall(320*scale-36*scale,77*scale,MICRO,LEFT,DOWN);
		createBall(320*scale-41*scale,92*scale,MICRO,LEFT,DOWN);
		createBall(320*scale-45*scale,110*scale,MICRO,LEFT,DOWN);
		createBall(320*scale-50*scale,127*scale,MICRO,LEFT,DOWN);		
		
		for (i=0;i<MAX_BALL;i++)
		{
			if (ball[i].utilise==1)
			{
				ball[i].vel_cst = 10.0;
			}
		}
	}
	
	if (n==17)
	{
		int i;
		int tmp;
		for (i=0;i<9;i++)
		{
			createPlateforme((17+(i*32))*scale,29*scale,PF_CASSABLE);
		}
		
		i=createPlateforme(9*scale,29*scale,PF_MICRO_CASSABLE);
		pforme[i].bonus = BONUS_LIFE;
		i=createPlateforme(297*scale+8*scale,29*scale,PF_MICRO_CASSABLE);
		pforme[i].bonus = getRandomBonus();
		
		for (i=0;i<8;i++)
		{
			tmp=createBall((18+(i*8))*scale,18*scale,MICRO,RIGHT,DOWN);
			ball[tmp].vel=0.5;
		}
		for (i=0;i<8;i++)
		{
			tmp=createBall((230+(i*8))*scale,18*scale,MICRO,RIGHT,DOWN);
			ball[tmp].vel=0.5;
		}		
		
		i=createBall(69*scale,56*scale,BIG,LEFT,DOWN);
		ball[i].bonus_parent=WEAPON_DOUBLE_SHOOT;
		ball[i].vel=1.0;
		ball[i].vel_cst = 11.0;
		i=createBall(194*scale,64*scale,NORMAL,RIGHT,DOWN);
		ball[i].bonus_parent=WEAPON_DOUBLE_SHOOT;
		ball[i].vel=1.0;		
	}
	if (n==18) 
	{
		int tmp;
		for (i=0;i<4;i++)
		{
			tmp=createBall((18+(i*8))*scale,150*scale,MICRO,LEFT,DOWN);
			ball[tmp].vel_cst=8.0;
			if (i==4) ball[tmp].bonus = WEAPON_DOUBLE_SHOOT;
		}
		for (i=0;i<4;i++)
		{
			tmp=createBall((230+(i*8))*scale,150*scale,MICRO,RIGHT,DOWN);
			ball[tmp].vel_cst=8.0;
			if (i==4) ball[tmp].bonus = WEAPON_DOUBLE_SHOOT;
		}
		
		tmp = createPlateforme(65*scale,85*scale,PF_CASSABLE);
		pforme[tmp].bonus = WEAPON_DOUBLE_SHOOT;
		tmp = createPlateforme(220*scale,85*scale,PF_CASSABLE);
		pforme[tmp].bonus = getRandomBonus();
	}
	
	if (n==19)
	{
		i=createBall(77*scale,17*scale,BIG,RIGHT,DOWN);
		ball[i].bonus_parent = getRandomBonus();
		i=createBall(229*scale,19*scale,NORMAL,LEFT,DOWN);
		ball[i].bonus_parent = getRandomBonus();
		
		createLadder(8*scale,100*scale,25);
		createLadder(290*scale,100*scale,25);
		
		for (i=0;i<7;i++)
		{
			createPlateforme((47+(i*32))*scale,136*scale,PF_INCASSABLE);	
		}
	}
	if (n==20)
	{
		createPlateforme(64*scale-32*scale,48*scale,PF_MOYEN_CASSABLE);
		createPlateforme(304*scale-32*scale,48*scale,PF_MOYEN_CASSABLE);
		createPlateforme(160*scale-32*scale,140*scale,PF_INCASSABLE);
		createPlateforme(160*scale,140*scale,PF_INCASSABLE);
		i=createBall(124*scale-32*scale,27*scale,BIG,RIGHT,DOWN);
		ball[i].bonus = getRandomBonus();
		ball[i].bonus_parent = getRandomBonus();
		ball[i].vel = ball[i].vel / 2;
		i=createBall(225*scale-32*scale,27*scale,BIG,LEFT,DOWN);
		ball[i].vel = ball[i].vel / 2;
	}
	if (n==21)
	{
		createPlateforme(175*scale-32*scale-16*scale,127*scale,PF_CASSABLE);
		createPlateforme(175*scale-16*scale,127*scale,PF_CASSABLE);
		
		createPlateforme(185*scale-32*scale,65*scale,PF_MOYEN_INCASSABLE);
		
		createPlateforme(90*scale-32*scale,80*scale,PF_MOYEN_INCASSABLE);
		createPlateforme(286*scale-32*scale,80*scale,PF_MOYEN_INCASSABLE);
		
		i=createBall(124*scale-32*scale,27*scale,BIG,RIGHT,DOWN);
		ball[i].bonus = getRandomBonus();
		ball[i].bonus_parent = getRandomBonus();
		ball[i].vel = ball[i].vel / 2;
		i=createBall(225*scale-32*scale,27*scale,BIG,LEFT,DOWN);
		ball[i].vel = ball[i].vel / 2;
	}
	if (n==22)
	{
		for (i=0;i<7;i++)
		{
			createPlateforme((112-64+(i*32))*scale,104*scale,PF_INCASSABLE);
		}
		
		i=createBall((92-32)*scale,27*scale,BIG,RIGHT,DOWN);
		ball[i].bonus_parent = getRandomBonus();
		ball[i].vel = ball[i].vel / 2;
		i=createBall((250-32)*scale,27*scale,BIG,RIGHT,DOWN);
		ball[i].bonus_parent = getRandomBonus();
		ball[i].vel = ball[i].vel / 2;
	}
	if (n==23)
	{
		for (i=0;i<4;i++)
		{
			createPlateforme(81*scale-32*scale,(33+(i*32))*scale,PF_INCASSABLEV);
			createPlateforme(153*scale-32*scale,(33+(i*32))*scale,PF_CASSABLEV);
			createPlateforme(225*scale-32*scale,(33+(i*32))*scale,PF_CASSABLEV);
			createPlateforme(297*scale-32*scale,(33+(i*32))*scale,PF_INCASSABLEV);
		}
		player.posx = player.posx + 64*scale;
		i=createBall(197*scale-64*scale,20*scale,BIG,RIGHT,DOWN);
		ball[i].bonus = getRandomBonus();
		ball[i].bonus_parent = getRandomBonus();
	}
	if (n==24)
	{
		for (i=0;i<4;i++)
		{
			createPlateforme(81*scale-32*scale,(33+(i*32))*scale,PF_INCASSABLEV);
			/*createPlateforme(153-32,33+(i*32),PF_CASSABLEV);
			createPlateforme(225-32,33+(i*32),PF_CASSABLEV);*/
			createPlateforme(297*scale-32*scale,(33+(i*32))*scale,PF_INCASSABLEV);
		}
		/*player.posx = player.posx + 64 + 32 + 16;*/
		i=createBall(197*scale + 8*scale,20*scale,BIG,RIGHT,DOWN);
		ball[i].bonus = getRandomBonus();
		ball[i].bonus_parent = getRandomBonus();
		i=createBall(197*scale-64*scale-64*scale,20*scale,BIG,LEFT,DOWN);
		ball[i].bonus = getRandomBonus();
		ball[i].bonus_parent = getRandomBonus();
	}
	if (n==25)
	{
		i=createBall(50*scale,10*scale,BIG,LEFT,DOWN);
		ball[i].bonus = getRandomBonus();
		ball[i].bonus_parent = getRandomBonus();	
		i=createBall(50*scale,10*scale,BIG,RIGHT,DOWN);
		ball[i].bonus = getRandomBonus();
		ball[i].bonus_parent = getRandomBonus();
		createBall(200*scale,10*scale,MICRO,LEFT,DOWN);
		createBall(205*scale,15*scale,MICRO,LEFT,UP);
		createBall(210*scale,20*scale,MICRO,RIGHT,DOWN);
		createBall(215*scale,22*scale,MICRO,RIGHT,DOWN);
		createBall(220*scale,25*scale,MICRO,RIGHT,UP);
		createBall(225*scale,30*scale,MICRO,LEFT,DOWN);
		
		for (i=0;i<6;i++)
		{
			if (i>2)
			{
				createPlateforme(((337-32)-(i*8))*scale,(41+(i*8))*scale,PF_MICRO_INCASSABLE);
				createPlateforme(((41-32)+(i*8))*scale,(41+(i*8))*scale,PF_MICRO_INCASSABLE);
			}
		}
		
		createLadder((332-32-22+10)*scale,(41+(2*8))*scale,36*scale);
		createLadder(((41-32)+2)*scale,(41+(2*8))*scale,36*scale);
		
		i=createPlateforme((337-32-16)*scale-(6*8)*scale-8*scale,41*scale+(6*8)*scale,PF_CASSABLE);
		i=createPlateforme((41-32)*scale+(6*8)*scale,41*scale+(5*8)*scale,PF_CASSABLE);
		
		
		createPlateforme((41-32)*scale+(6*8)*scale+32*scale,41*scale+(6*8)*scale,PF_MOYEN_INCASSABLE);
		pforme[i].bonus = getRandomBonus();
		createPlateforme((337-32)*scale-(6*8)*scale-32*scale-8*scale,41*scale+(6*8)*scale,PF_MOYEN_INCASSABLE);
		pforme[i].bonus = BONUS_LIFE;
		
		for (i=0;i<3;i++)
		{
			createPlateforme((248-32-8)*scale-(i*8)*scale,97*scale+(i*8)*scale,PF_MICRO_INCASSABLE);
			createPlateforme((137-32)*scale+(i*8)*scale,97*scale+(i*8)*scale,PF_MICRO_INCASSABLE);
		}
		
		createPlateforme(161*scale-32*scale,121*scale,PF_MOYEN_INCASSABLE);
		createPlateforme(209*scale-32*scale,121*scale,PF_MOYEN_INCASSABLE);
	}
	if (n==26)
	{
		createBall(39*scale-32*scale,19*scale,BIG,LEFT,DOWN);
		for (i=0;i<3;i++)
		{
			createPlateforme(34*scale+(i*32)*scale-22*scale+16*scale,159*scale,PF_CASSABLE);
			createPlateforme(184*scale+(i*32)*scale,159*scale,PF_CASSABLE);
		}	
		createPlateforme(34*scale+(3*32)*scale-22*scale+16*scale,159*scale,PF_INCASSABLE);
		createPlateforme(152*scale,159*scale,PF_INCASSABLE);
		createPlateforme(280*scale,159*scale,PF_INCASSABLE);
		
		createLadder(130*scale,155*scale+4*scale,10*scale);
		
		createPlateforme(8*scale,102*scale,PF_CASSABLE);
		for (i=0;i<6;i++)
		{
			createPlateforme(41*scale+(i*32)*scale,102*scale,PF_INCASSABLE);
		}
		createPlateforme(232*scale,102*scale,PF_MOYEN_INCASSABLE);
		createPlateforme(249*scale,102*scale,PF_CASSABLE);
		createPlateforme(281*scale,102*scale,PF_INCASSABLE);
		
		createLadder(162*scale,99*scale+4*scale,14*scale);
		
	}
	if (n==30)
	{
		for (i=0;i<4;i++)
		{
			createPlateforme(81*scale-32*scale,33*scale+(i*32)*scale,PF_INCASSABLEV);
			createPlateforme(153*scale-32*scale,33*scale+(i*32)*scale,PF_CASSABLEV);
			createPlateforme(225*scale-32*scale,33*scale+(i*32)*scale,PF_CASSABLEV);
			createPlateforme(297*scale-32*scale,33*scale+(i*32)*scale,PF_INCASSABLEV);
		}
		player.posx = player.posx + 64*scale;
		i=createBall(197*scale-64*scale,20*scale,BIG,RIGHT,DOWN);
		ball[i].bonus = getRandomBonus();
		ball[i].bonus_parent = getRandomBonus();
		i=createBall(197*scale-64*scale-64*scale,20*scale,BIG,LEFT,DOWN);
		ball[i].bonus = getRandomBonus();
		ball[i].bonus_parent = getRandomBonus();


	}
	if (n==38)
	{
		for (i=0;i<4;i++)
		{
			createPlateforme(81*scale-32*scale,33*scale+(i*32)*scale,PF_INCASSABLEV);
			createPlateforme(153*scale-32*scale,33*scale+(i*32)*scale,PF_CASSABLEV);
			createPlateforme(225*scale-32*scale,33*scale+(i*32)*scale,PF_CASSABLEV);
			createPlateforme(297*scale-32*scale,33*scale+(i*32)*scale,PF_INCASSABLEV);
		}
		player.posx = player.posx + 64*scale + 32*scale + 16*scale;
		i=createBall(197*scale + 8*scale,20*scale,BIG,RIGHT,DOWN);
		ball[i].bonus = getRandomBonus();
		ball[i].bonus_parent = getRandomBonus();
		i=createBall(197*scale-64*scale,20*scale,BIG,RIGHT,DOWN);
		ball[i].bonus = getRandomBonus();
		ball[i].bonus_parent = getRandomBonus();
		i=createBall(197*scale-64*scale-64*scale,20*scale,BIG,LEFT,DOWN);
		ball[i].bonus = getRandomBonus();
		ball[i].bonus_parent = getRandomBonus();
	}
}
