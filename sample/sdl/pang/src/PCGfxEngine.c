
#include <SDL.h>
#include "PCGfxEngine.h"
#include "structures.h"
#include <SDL_mixer.h>
#include <SDL_image.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#ifdef RES320X240
//#include <SDL_rotozoom.h>
#endif

// Nouvelle version !!
SDL_Surface *screen;
// SDL_Surface *ScreenSurface;
SDL_Joystick *stick;
SDL_Surface *imagesBMP[10];

int mapTiles[MAX_MAP][MAX_MAP_WIDTH][MAX_MAP_HEIGHT];
int currentWorldMapX[MAX_MAP];
int currentWorldMapY[MAX_MAP];

Uint32 sstart;
Uint32 ssend;

int keyRight = 0;
int keyLeft = 0;
int keyUp = 0;
int keyDown = 0;
int keyQuit = 0;

int keyAction1 = 0;
int keyAction2 = 0;
int keyAction3 = 0;
int keyAction4 = 0;

int keyActionPause = 0;

int fpsshow = 0;

/*
typedef struct si {
	int imageh;	// Hauteur
	int imagel; // Largeur
	SDL_Surface *image; // L'image en elle m?e
} simage;

typedef struct ss{
	int utilise;	// Sprite utilis??
	int posx;		// Position x
	int posy;		// Position y
	int image;		// Image courante
	int animation[10][20]; // 10 animations de 20 frames (-1 = on boucle )
	int animation_speed[10];   // vitesse de l'animation
	int current_animation; // animation courante -1 non anim?	int current_animation_frame; // image courante
	int intern1;	// interne
} ssprite;

typedef struct st {
	int image;	// Image repr?entant la tile
	int attr1;	// Attribut 1
	int attr2;	// Attribut 2
	int attr3;	// Attribut 3
} stile;
*/
ssprite sprite[MAX_SPRITE]; // Les sprites
simage imageBank[MAX_IMAGE]; // Les images d?oup?s des BMP charg?
stile tiles[MAX_TILE]; // Les tiles

void initTileEngine()
{
 int m,x,y;
 for (m=0;m<MAX_MAP;m++)
 for (x=0;x<MAX_MAP_WIDTH;x++)
 for (y=0;y<MAX_MAP_HEIGHT;y++)
     mapTiles[m][x][y]=0; 
}

// DEPRECATED !
// Prends une portion d'une des images BMP et la colle en vid? 
void blitImageToScreen(int imageBMPNo,int sx,int sy,int sw,int sh,int dx,int dy,int dw,int dh,int ttw,int tth)
{
			
            SDL_Rect dest;
            dest.x = dx;
            dest.y = dy;
            dest.h = dh;
            dest.w = dw;
                 
            SDL_Rect src;
            src.x = sx;
            src.y = sy;
            src.h = sh;
            src.w = sw;     

            SDL_BlitSurface(imagesBMP[imageBMPNo],&src,screen,&dest);


}
// DEPRECATED !

void blitBMPImageToScreen(int imageBMPNo,int sx,int sy,int sw,int sh,int dx,int dy,int dw,int dh,int ttw,int tth)
{
            SDL_Rect dest;
            dest.x = dx;
            dest.y = dy;
            dest.h = dh;
            dest.w = dw;
                 
            SDL_Rect src;
            src.x = sx;
            src.y = sy;
            src.h = sh;
            src.w = sw;     

            SDL_BlitSurface(imagesBMP[imageBMPNo],&src,screen,&dest);
}

void blitImageBankToScreen(int imageBankNo,int dx,int dy)
{
            SDL_Rect dest;
            dest.x = dx;
            dest.y = dy;
            dest.h = imageBank[imageBankNo].imageh;
            dest.w = imageBank[imageBankNo].imagel;
                 
            SDL_Rect src;
            src.x = 0;
            src.y = 0;
            src.h = imageBank[imageBankNo].imageh;
            src.w = imageBank[imageBankNo].imagel;   

            SDL_BlitSurface(imageBank[imageBankNo].image,&src,screen,&dest);
}

void createTile(int n,int imageNo,int a1,int a2,int a3)
{
	tiles[n].image = imageNo;
	tiles[n].attr1 = a1;
	tiles[n].attr2 = a2;
    tiles[n].attr3 = a3;

	/*    fprintf(stderr,"je cr? la tile : %d avec l'image %d\n",n,imageNo);	   */
}

void setTileInMap(int mapNo,int x,int y,int tileNo)
{
     mapTiles[mapNo][x][y]=tileNo;
}

void setWorldView(int mapNo,int x,int y)
{
    currentWorldMapX[mapNo] = x;
    currentWorldMapY[mapNo] = y;     
}

void mapDraw(int mapNo)
{
   int map_drawx = currentWorldMapX[mapNo];
   int map_drawy = currentWorldMapY[mapNo];
   int i, j;
   int mapx, mapy;
   int map_xoff, map_yoff;

   mapx = map_drawx / 16;
   mapy = map_drawy / 16;

   map_xoff = map_drawx & 15;
   map_yoff = map_drawy & 15;

   for(i = 0; i < 16; i++) {
      for(j = 0; j < 21; j++) {         
         if ( (j+mapx<MAX_MAP_WIDTH) && (i+mapy<MAX_MAP_HEIGHT) && (j+mapx>=0) && (i+mapy>=0) )
         {		 		
				blitImageBankToScreen(tiles[mapTiles[mapNo][j+mapx][i+mapy]].image,j * 16 - map_xoff, i * 16 - map_yoff);
         }       
      }
   }
}

void initGfxEngine()
{
	//if (SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) != 0)
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
	{
		printf("Unable to initialize SDL: %s\n", SDL_GetError());
		
#ifndef GP2X_VERSION
		//chdir("/usr/gp2x");
		//execl("/usr/gp2x/gp2xmenu", "/usr/gp2x/gp2xmenu", NULL);    
#endif
		
		exit(1);
	}
	//stick=SDL_JoystickOpen(0);
	//SDL_JoystickEventState(SDL_ENABLE);
	//fprintf(stdout,"Found %d joysticks\n",SDL_NumJoysticks());

     //atexit(SDL_Quit);

#ifdef RES320X240
    // ScreenSurface = SDL_SetVideoMode(320, 480, 16, SDL_SWSURFACE);
    // screen = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, 16, 0, 0, 0, 0);
    screen = SDL_SetVideoMode(320, 240, 16, SDL_SWSURFACE);

    printf("Resolution 320x240\n");
#else
    // screen = SDL_SetVideoMode(640,480,32,SDL_FULLSCREEN|SDL_HWSURFACE|SDL_DOUBLEBUF);
     screen = SDL_SetVideoMode(640,480,16,SDL_SWSURFACE);
#endif

     SDL_ShowCursor(0);
     if (screen == NULL) 
	 {
	printf("Failed to create the \"screen\" surface, exiting now\n");
#ifndef GP2X_VERSION
		//chdir("/usr/gp2x");
		//execl("/usr/gp2x/gp2xmenu", "/usr/gp2x/gp2xmenu", NULL);    
#endif
         exit(1);
     }
}

// image 1 et 3 transparente
int loadBmp(char *path,char *filename,char * pathfilename,char *pathdc,int noImage)
{        
		
    SDL_FreeSurface(imagesBMP[noImage]);

    SDL_Surface *temp ;
		
	if ((temp = IMG_Load(pathfilename)) == NULL) {
		fprintf(stderr, "Error: %s\n", SDL_GetError());

#ifndef GP2X_VERSION
		//chdir("/usr/gp2x");
		//execl("/usr/gp2x/gp2xmenu", "/usr/gp2x/gp2xmenu", NULL);    
#endif
		exit(1);
	}
		//if (SDL_MUSTLOCK(temp)) SDL_LockSurface(temp);

#ifdef RES320X240
    SDL_Surface *tmpS=NULL;
    tmpS=zoomSurface(temp,0.5,0.5,0);  // scale the image x0.5 when loaded into memory
    temp=tmpS;
#endif

    imagesBMP[noImage] = SDL_ConvertSurface(temp, screen->format,SDL_SWSURFACE);
	//if (SDL_MUSTLOCK(temp)) SDL_UnlockSurface(temp);
	SDL_FreeSurface(temp);
       

    if ( (noImage==1) || (noImage==3) ) SDL_SetColorKey(imagesBMP[noImage], (SDL_SRCCOLORKEY|SDL_RLEACCEL), SDL_MapRGB(imagesBMP[noImage]->format,255,255,255));

    return 0;
}

void flipScreen()
{
    SDL_Flip(screen);
   //  {
   //    if(SDL_MUSTLOCK(ScreenSurface)) SDL_LockSurface(ScreenSurface);
   //    int x, y;
   //    uint32_t *s = (uint32_t*)screen->pixels;
   //    uint32_t *d = (uint32_t*)ScreenSurface->pixels;
   //    		for(uint8_t y2 = 0; y2 < 240; y2++, s += 160, d += 320) 
			// memmove(d, s, 1280); // double-line fix by pingflood, 2018
   //    /*for(y=0; y<240; y++){
   //      for(x=0; x<160; x++){
   //        *d++ = *s++;
   //      }
   //      d+= 160;
   //    }*/
   //    if(SDL_MUSTLOCK(ScreenSurface)) SDL_UnlockSurface(ScreenSurface);
   //    SDL_Flip(ScreenSurface);
   //  }
}



void waitInMs()
{
    static float lasttime=0;
    float currenttime=0;
    static int frames;
    float oldwpf;
    static float wpf=0; // wait per frame
    
    currenttime = SDL_GetTicks () *0.001;
    frames++;

    oldwpf=wpf;
    // recalc the wait per frame every 10 frames
    if (currenttime-lasttime > 0.1 ) {
	// x frame per 0.2 seconds
	int efps= frames*10;

	// wir haben ? aber wollen 42
	wpf=(efps/42.0)*10;
//	printf("wait per frame: %f\n",wpf);
	
	lasttime=currenttime;
	frames=0;
    }
    if ( wpf > 0 ) oldwpf=wpf;
     SDL_Delay(oldwpf);
}


int synchroStart()
{
       sstart = SDL_GetTicks();
       return 0;
}

int synchroEnd(int ms)
{
       ssend = SDL_GetTicks();
       int t = (ms-(ssend-sstart));
       if ( t>0 )
       if ( t<=ms )
       {
            waitInMs((ms-(ssend-sstart)));
       }
       
    return 0;
}

int checkController()
{
    static int volume=80;
    SDL_Event event;
    while (SDL_PollEvent(&event)) 
    {
        switch (event.key.keysym.sym)
        {
        case SDLK_LEFT:
          keyLeft = 1;
          break;

        case SDLK_RIGHT:
          keyRight = 1;
          break;

        case SDLK_UP:
          keyUp = 1;
          break;

        case SDLK_DOWN:
          keyDown = 1;
          break;

         case SDLK_LALT:
          keyAction1 = 1;
          break;

         case SDLK_LCTRL:
          keyAction2 = 1;
          break;

         case SDLK_LSHIFT:
          keyAction3 = 1;
          break;

         case SDLK_SPACE:
          keyAction4 = 1;
          break;

         case SDLK_RETURN:
          keyActionPause = 1;
          keyAction4 = 1;
          break;

         case SDLK_ESCAPE:
          // If escape is pressed, return (and thus, quit)
          keyQuit = 1;
          return 0;
	  
	  default: break;
        }


      switch (event.type) 
      {

      case SDL_KEYDOWN:
        switch (event.key.keysym.sym)
        {
            case SDLK_f:
            fpsshow= (fpsshow+1) % 2;
	    printf("fpsshow: %d\n",fpsshow);
	    break;
	    
	    default: break;
	}
	break;
	
      case SDL_KEYUP:          
        switch (event.key.keysym.sym)
        {
        case SDLK_LEFT:
          keyLeft = 0;
          break;

        case SDLK_RIGHT:
          keyRight = 0;
          break;

        case SDLK_UP:
          keyUp = 0;
          break;

        case SDLK_DOWN:
          keyDown = 0;
          break;

         case SDLK_LALT:
          keyAction1 = 0;
          break;

         case SDLK_LCTRL:
          keyAction2 = 0;
          break;

         case SDLK_LSHIFT:
          keyAction3 = 0;
          break;

         case SDLK_SPACE:
          keyAction4 = 0;
          break;  

         case SDLK_RETURN:
          keyActionPause = 0;
          keyAction4 = 0;
          break;                  

#ifndef NO_SOUND
    	    case SDLK_KP_PLUS :
	    volume+=10;
	    Mix_Volume(-1,volume);
	    fprintf(stdout,"volume set to: %d\n",volume);
            break;

    	    case SDLK_KP_MINUS :
	    volume-=10;
	    Mix_Volume(-1,volume);
	    fprintf(stdout,"volume set to: %d\n",volume);
            break;
#endif

        case SDLK_ESCAPE:
          // If escape is pressed, return (and thus, quit)
          keyQuit = 1;
          return 0;
	  
	default: break;
        }
        break;

       case SDL_QUIT:
        keyQuit = 1;
        return(0);
	
#ifdef GP2X_VERSION
       case SDL_JOYBUTTONUP:
        switch (event.button.button) {

    	    case GP2X_BUTTON_LEFT: 
            keyLeft = 0;
            break;
	    
    	    case GP2X_BUTTON_RIGHT: 
            keyRight = 0;
            break;
	    
    	    case GP2X_BUTTON_UP: 
            keyUp = 0;
            break;
	    
    	    case GP2X_BUTTON_DOWN: 
            keyDown = 0;
            break;

    	    case GP2X_BUTTON_A: 
            keyAction1 = 0;
            break;

    	    case GP2X_BUTTON_B: 
            keyAction2 = 0;
            break;

    	    case GP2X_BUTTON_Y: 
            keyAction3 = 0;
            break;

    	    case GP2X_BUTTON_X: 
            keyAction4 = 0;
            break;

    	    case GP2X_BUTTON_START: 
            keyActionPause = 0;
            break;                  

    	    case GP2X_BUTTON_SELECT:
            keyQuit = 1;
            return 0;
            break;                  
	    default : break;
	}

	break;
#endif	    

#ifdef CAANOO
	case SDL_JOYBUTTONUP:
		switch(event.jbutton.button) {
		case 0: // (A)
			keyAction1 = 0;
		break;
		case 2: // (B)
			keyAction2 = 0;
		break;
		case 3: // (Y)
			keyAction3 = 0;
		break;
		case 1: // (X)
			keyAction4 = 0;
		break;
		case 8: // (I)
			keyActionPause = 0;
		break;
		}
	break;
	case SDL_JOYBUTTONDOWN:
		switch(event.jbutton.button) {
		case 0: // (A)
			keyAction1 = 1;
		break;
		case 2: // (B)
			keyAction2 = 1;
		break;
		case 4: // (L)
			fpsshow= (fpsshow+1) % 2;
		break;
		case 3: // (Y)
			keyAction3 = 1;
		break;
		case 1: // (X)
			keyAction4 = 1;
		break;
		case 8: // (I)
			keyActionPause = 1;
		break;
		case 9: // (II)
			keyQuit = 1;
		break;
		}
	break;
	case SDL_JOYAXISMOTION:
		// up side
		if ((( event.jaxis.value < -11585 ) && (event.jaxis.axis == 1))) {
            		keyUp = 1;
		}
		if ((( event.jaxis.value >  11585 ) && (event.jaxis.axis == 1))) {
            		keyDown = 1;
		}
		if ((( event.jaxis.value < -11585 ) && (event.jaxis.axis == 0))) {
            		keyLeft = 1;
		}
		if ((( event.jaxis.value >  11585 ) && (event.jaxis.axis == 0))) {
            		keyRight = 1;
		}
		// down side
		if ((( event.jaxis.value >= -11585 ) && (event.jaxis.axis == 1))) {
            		keyUp = 0;
		}
		if ((( event.jaxis.value <=  11585 ) && (event.jaxis.axis == 1))) {
            		keyDown = 0;
		}
		if ((( event.jaxis.value >= -11585 ) && (event.jaxis.axis == 0))) {
            		keyLeft = 0;
		}
		if ((( event.jaxis.value <=  11585 ) && (event.jaxis.axis == 0))) {
            		keyRight = 0;
		}
	break;
#endif

#ifdef GP2X_VERSION
       case SDL_JOYBUTTONDOWN:
        switch (event.button.button)
        {
    	    case GP2X_BUTTON_LEFT: 
            keyLeft = 1;
            break;
	    
    	    case GP2X_BUTTON_RIGHT: 
            keyRight = 1;
            break;
	    
    	    case GP2X_BUTTON_UP: 
            keyUp = 1;
            break;
	    
    	    case GP2X_BUTTON_DOWN: 
            keyDown = 1;
            break;

    	    case GP2X_BUTTON_A: 
            keyAction1 = 1;
            break;

    	    case GP2X_BUTTON_B: 
            keyAction2 = 1;
            break;

            case GP2X_BUTTON_L:
            fpsshow= (fpsshow+1) % 2;
            break;

    	    case GP2X_BUTTON_Y: 
            keyAction3 = 1;
            break;

    	    case GP2X_BUTTON_X: 
            keyAction4 = 1;
            break;

    	    case GP2X_BUTTON_START: 
            keyActionPause = 1;
            break;                  

    	    case GP2X_BUTTON_SELECT:
            keyQuit = 1;
            return 0;
            break;                  
	    
#ifndef NO_SOUND
    	    case GP2X_BUTTON_VOLUP:
	    volume+=10;
	    Mix_Volume(-1,volume);
	    fprintf(stdout,"volume set to: %d\n",volume);
            break;

    	    case GP2X_BUTTON_VOLDOWN:
	    volume-=10;
	    Mix_Volume(-1,volume);
	    fprintf(stdout,"volume set to: %d\n",volume);
            break;
#endif
	}

#endif
      }
    
    }
    return 1;
}

void initSpriteEngine()
{
	int i;
	for (i=0;i<MAX_SPRITE;i++)
	{
		sprite[i].utilise=0;
	}
}

// Recup?e une image n?n en x,y de taille h,l dans l'image BMP n?imageNo
//void getImage(int n,int x,int y,int l,int h,int imageNo)
void getImage(int n,int x,int y,int l,int h,int imageNo,int w,int he)
{

    Uint32 rmask, gmask, bmask, amask;

    /* SDL interprets each pixel as a 32-bit number, so our masks must depend
       on the endianness (byte order) of the machine */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif


	// Si l'image est d?a allou? on la lib?e.
	SDL_FreeSurface(imageBank[n].image);
    imageBank[n].image = SDL_CreateRGBSurface(SDL_SWSURFACE, l, h, 32,
                                   rmask, gmask, bmask, amask);

	// On copie une portion de la BMP dans la nouvelle surface
    SDL_Rect src;
    src.x = x;
    src.y = y;
    src.h = h;
    src.w = l;                
		SDL_BlitSurface(imagesBMP[imageNo],&src,imageBank[n].image,NULL);
		
	
	imageBank[n].imageh = h;
	imageBank[n].imagel = l;    
}

// Recup?e une image n?n en x,y de taille h,l dans l'image BMP n?imageNo
void getImageBankFromBMP(int n,int x,int y,int l,int h,int imageNo)
{

    Uint32 rmask, gmask, bmask, amask;

    /* SDL interprets each pixel as a 32-bit number, so our masks must depend
       on the endianness (byte order) of the machine */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif

	// Si l'image est d?a allou? on la lib?e.
	SDL_FreeSurface(imageBank[n].image);
    imageBank[n].image = SDL_CreateRGBSurface(SDL_SWSURFACE, l, h, 32,
                                   rmask, gmask, bmask, amask);

	// On copie une portion de la BMP dans la nouvelle surface
    SDL_Rect src;
    src.x = x;
    src.y = y;
    src.h = h;
    src.w = l;                
		SDL_BlitSurface(imagesBMP[imageNo],&src,imageBank[n].image,NULL);
		
	//SDL_BlitSurface(imagesBMP[imageNo],&src,imageBank[n].image,NULL);
	imageBank[n].imageh = h;
	imageBank[n].imagel = l;    
}


void initSprite(int n,int x,int y,int i)
{
	sprite[n].posx = x;
	sprite[n].posy = y;
	sprite[n].image = i;
	sprite[n].utilise = 1;
	sprite[n].current_animation = -1;
	sprite[n].intern1 = 0;
	sprite[n].current_animation_frame = -1;
}



int initFreeSprite(int x,int y,int i)
{
	int s=0;

	while ((sprite[s].utilise==1) && (s<MAX_SPRITE))
	{
		s++;
	}
	sprite[s].posx = x;
	sprite[s].posy = y;
	sprite[s].image = i;
	sprite[s].utilise = 1;
	sprite[s].current_animation = -1;
	sprite[s].intern1 = 0;
	sprite[s].current_animation_frame = -1;

	return s;
}

void releaseSprite(int n)
{
	sprite[n].utilise = 0;
}

void releaseAllSprite()
{
	int i;

	for (i=0;i<MAX_SPRITE;i++)
	{
		sprite[i].utilise=0;
	}
}


void animateSprite(int s,int a)
{
	sprite[s].current_animation = a;
	sprite[s].current_animation_frame = -1;
	sprite[s].intern1 = 0;
}

void setSpriteAnimation(int s,int a,int speed,int f0,int f1,int f2,int f3,int f4,int f5,int f6,int f7,int f8,int f9,int f10,int f11,int f12,int f13,int f14,int f15,int f16,int f17,int f18,int f19)
{
	sprite[s].animation_speed[a] = speed;
	sprite[s].animation[a][0] = f0;
	sprite[s].animation[a][1] = f1;
	sprite[s].animation[a][2] = f2;
	sprite[s].animation[a][3] = f3;
	sprite[s].animation[a][4] = f4;
	sprite[s].animation[a][5] = f5;
	sprite[s].animation[a][6] = f6;
	sprite[s].animation[a][7] = f7;
	sprite[s].animation[a][8] = f8;
	sprite[s].animation[a][9] = f9;
	sprite[s].animation[a][10] = f10;
	sprite[s].animation[a][11] = f11;
	sprite[s].animation[a][12] = f12;
	sprite[s].animation[a][13] = f13;
	sprite[s].animation[a][14] = f14;
	sprite[s].animation[a][15] = f15;
	sprite[s].animation[a][15] = f16;
	sprite[s].animation[a][17] = f17;
	sprite[s].animation[a][18] = f18;
	sprite[s].animation[a][19] = f19;
}

void moveSprite(int n,int x,int y)
{
	sprite[n].posx=x;
	sprite[n].posy=y;
}



void changeSpriteImage(int n,int i)
{
	sprite[n].image=i;
}


// DEPRECATED !
void pasteImage(int n,int x,int y)
{
		blitImageBankToScreen(sprite[n].image,x,y);    
}
// DEPRECATED !

void pasteSpriteImage(int n,int x,int y)
{
		blitImageBankToScreen(sprite[n].image,x,y);

}


void showSprite(int n)
{
	if (sprite[n].utilise== 1)
	{
		if (sprite[n].current_animation!=-1)
		{
			if (sprite[n].intern1<=0)
			{
				sprite[n].current_animation_frame ++;
				if (sprite[n].current_animation_frame>19) sprite[n].current_animation_frame=0;
				if (sprite[n].animation[sprite[n].current_animation][sprite[n].current_animation_frame] == -1) sprite[n].current_animation_frame=0;
				sprite[n].image = sprite[n].animation[sprite[n].current_animation][sprite[n].current_animation_frame];
				sprite[n].intern1 = sprite[n].animation_speed[sprite[n].current_animation];
			}
			sprite[n].intern1--;
		}
		else
		{
			sprite[n].intern1 = 0;
		}

        blitImageBankToScreen(sprite[n].image,sprite[n].posx, sprite[n].posy);
	}
}

void stopAnimateSprite(int s)
{
	sprite[s].current_animation = -1;
	sprite[s].current_animation_frame = -1;
	sprite[s].intern1 = sprite[s].animation_speed[sprite[s].current_animation];
}

void showLRSprite(int n)
{
	// Non impl?ent? !
}

void showAllSprite()
{
	int i;
	for (i=0;i<MAX_SPRITE;i++)
	{
		if (sprite[i].utilise==1)
		{
			showSprite(i);
		}
	}
}

int CollideTransparentPixelTest(SDL_Surface *surface , int u , int v)
{
	
	SDL_Color Color;
	
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p = (Uint8 *)surface->pixels + v * surface->pitch + u * bpp;

	Uint32 pixelcolor;

	switch(bpp)
	{
		case(1):
			pixelcolor = *p;
		break;

		case(2):
			pixelcolor = *(Uint16 *)p;
		break;

		case(3):
			if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
				pixelcolor = p[0] << 16 | p[1] << 8 | p[2];
			else
				pixelcolor = p[0] | p[1] << 8 | p[2] << 16;
		break;

		case(4):
			pixelcolor = *(Uint32 *)p;
		break;
		
		default: pixelcolor = *p;
	}

	SDL_GetRGB( pixelcolor, surface->format, &Color.r, &Color.g, &Color.b);
	

	 if ( ( (Color.r==255) && (Color.g==255) && (Color.b==255) ) || ( (Color.r==0) && (Color.g==0) && (Color.b==0) ) )return 0;
    	else return 1;
}


int isSpriteCollide(int sprite1,int sprite2)
{

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

	/*D?ection par bounding box
	Retourne 0 et sort de la fonction
	si les sprites ne poss?ent pas de zones superpos?s*/
	if(coorx_1 > coorx_2+imageBank[sprite[sprite2].image].imagel) return 0;
	if(coorx_1+imageBank[sprite[sprite1].image].imagel < coorx_2) return 0;
	if(coory_1 > coory_2+imageBank[sprite[sprite2].image].imageh) return 0;
	if(coory_1+imageBank[sprite[sprite1].image].imageh < coory_2) return 0;

	/*Le but des lignes suivantes est de d?inir un
	rectangle qui englobe la zone d'affichage
	commune aux deux sprites
	On traite les coordonn?s x du rectangle*/

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

	/*On traite les coordonn?s y du rectangle*/
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


	/*Il ne reste plus qu'?tester pour chaque

	pixel du rectangle pr??emment d?ini si ses pixels

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
                                                                                         
				return 1; 
			}
		}

	}

 	if (SDL_MUSTLOCK(imageBank[sprite[sprite1].image].image)) SDL_UnlockSurface(imageBank[sprite[sprite1].image].image);
	if (SDL_MUSTLOCK(imageBank[sprite[sprite2].image].image)) SDL_UnlockSurface(imageBank[sprite[sprite2].image].image);
	
	return 0;
}

