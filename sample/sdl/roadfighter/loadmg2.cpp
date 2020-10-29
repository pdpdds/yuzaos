#include "string.h"

#include "SDL.h"
#include "SDL_ttf.h"
#include "SDL_image.h"
#include "SDL_mixer.h"

#include "sound.h"

#include "CTile.h"
#include "CObject.h"
#include "CGame.h"

#include "auxiliar.h"
#include "filehandling.h"

bool CGame::load_map(char *mapname)
{
	FILE *fp;
	int i,j,k,n;
	int i_tmp;
	int current_object;
	int semaphore_object;
	int semaphore_tiles[5][2];
	List<TILE_SOURCE> l;
	TILE_SOURCE *source;
	char tmp[256];

	fp=f1open(mapname, "r", GAMEDATA);
	if (fp==0) return false;

	/* Load map: */ 

	/* Tile sources: */ 
	if (2!=fscanf(fp,"%s %i",tmp,&n)) return false;
	for(i=0;i<n;i++) {
		source=new TILE_SOURCE();
		source->load(fp);
		tile_sources.Add(source);
	} /* for */ 

	/* Tiles: */ 
	for(j=0;j<256;j++) {
		if (2!=fscanf(fp,"%s %i",tmp,&n)) return false;
		tiles[j].Delete();
		for(i=0;i<n;i++) {
			bool found=false;
			int i1,i2,i3,i4,mask_type,collision_mask_type;
			CTile *t;

			if (1!=fscanf(fp,"%s",tmp)) return false;
			if (6!=fscanf(fp,"%i %i %i %i %i %i",&i1,&i2,&i3,&i4,&mask_type,&collision_mask_type)) return false;

			t=0;
			l.Instance(tile_sources);
			l.Rewind();
			while(!found && l.Iterate(source)) {
				if (source->cmp(tmp)) {
					found=true;
					t=new CTile(i1,i2,i3,i4,source->sfc,(collision_mask_type==2 ? true:false));
				} /* if */ 
			} /* while */ 

			if (t!=0) tiles[j].Add(t);	
		} /* for */ 
	} /* for */ 

	/* Objects: */ 
	if (2!=fscanf(fp,"%s %i",tmp,&n)) return false;
	current_object=0;
	semaphore_object=0;
	for(i=0;i<n;i++) {
		int tile_bank,tile_num;
		int nbitmaps,nlinks,nparts;
		/* "tmp" contains the name of the object */ 
		if (2!=fscanf(fp,"%s %s",tmp,tmp)) return false;
		if (strcmp(tmp,"\"semaphore\"")==0) semaphore_object=current_object;

		if (1!=fscanf(fp,"%i",&nbitmaps)) return false;
		for(j=0;j<nbitmaps;j++) {
			if (3!=fscanf(fp,"%i %i %i",&tile_bank,&tile_num,&nlinks)) return false;
			if (strcmp(tmp,"\"semaphore\"")==0) {
				semaphore_tiles[j][0]=tile_bank;
				semaphore_tiles[j][1]=tile_num;
			} /* if */ 
			for(k=0;k<nlinks;k++) if (2!=fscanf(fp,"%i %i",&i_tmp,&i_tmp)) return false;
		} /* for */ 
		if (1!=fscanf(fp,"%i",&nparts)) return false;
		for(j=0;j<nparts;j++) {
			if (5!=fscanf(fp,"%i %i %i %i %i",&i_tmp,&i_tmp,&i_tmp,&i_tmp,&i_tmp)) return false;
		} /* for */ 

		if (1!=fscanf(fp,"%i",&i_tmp)) return false;
		if (1!=fscanf(fp,"%i",&i_tmp)) return false;
		if (2!=fscanf(fp,"%i %i",&i_tmp,&i_tmp)) return false;

		for(i=0;i<23;i++) {
			for(j=0;j<nparts;j++) 		
				if (1!=fscanf(fp,"%s",&tmp)) return false;
		} /* for */ 

		if (5!=fscanf(fp,"%i %i %i %i %i",&i_tmp,&i_tmp,&i_tmp,&i_tmp,&i_tmp)) return false;
		if (4!=fscanf(fp,"%i %i %i %i",&i_tmp,&i_tmp,&i_tmp,&i_tmp)) return false;
		if (1!=fscanf(fp,"%i",&i_tmp)) return false;
		if (i_tmp!=0) return false; /* No states should be defined!!! */ 
		if (1!=fscanf(fp,"%i",&i_tmp)) return false;
		if (i_tmp!=0) return false; /* No condition states should be defined!!! */ 

		if (4!=fscanf(fp,"%i %i %i %i",&i_tmp,&i_tmp,&i_tmp,&i_tmp)) return false;
		if (1!=fscanf(fp,"%s ",tmp)) return false;
		if (strcmp(tmp,"DAMAGE")==0) {
			fgets(tmp,80,fp);
		} /* if */ 
		if (2!=fscanf(fp,"%s %i",tmp,&i_tmp)) return false;
		if (i_tmp!=0) return false; /* No variables should be defined!!! */ 
		if (10!=fscanf(fp,"%i %i %i %i %i %i %i %i %i %i",&i_tmp,&i_tmp,&i_tmp,&i_tmp,&i_tmp,
														  &i_tmp,&i_tmp,&i_tmp,&i_tmp,&i_tmp)) return false;
		
		current_object++;
	} /* for */ 

	if (2!=fscanf(fp,"%s %i",tmp,&i_tmp)) return false;
	if (i_tmp!=0) return false; /* No game variables should be defined!!! */ 

	if (2!=fscanf(fp,"%s %i",tmp,&i_tmp)) return false;
	if (i_tmp!=1) return false; /* Only one map should be defined!!! */ 

	if (2!=fscanf(fp,"%s %s",tmp,tmp)) return false;
	if (2!=fscanf(fp,"%s %i",tmp,&i_tmp)) return false;
	if (i_tmp!=0) return false; /* No map variables should be defined!!! */ 

	if (2!=fscanf(fp,"%s %i",tmp,&i_tmp)) return false;
	if (i_tmp!=1) return false; /* Only one room should be defined!!! */ 
	if (2!=fscanf(fp,"%s %i",tmp,&i_tmp)) return false;
	if (i_tmp!=0) return false; /* No room variables should be defined!!! */ 

	if (3!=fscanf(fp,"%s %i %i",tmp,&i_tmp,&i_tmp)) return false;

	/* Map: */ 
	{
		int x,y,tile_bank,tile_num;

		if (3!=fscanf(fp,"%s %i %i",tmp,&dx,&dy)) return false;
		dx*=16;
		dy*=16;

		if (1!=fscanf(fp,"%i",&i_tmp)) return false;
		
		/* Background tiles: */ 
		if (1!=fscanf(fp,"%i",&n)) return false;
		for(i=0;i<n;i++) {
			if (4!=fscanf(fp,"%i %i %i %i",&x,&y,&tile_bank,&tile_num)) return false;
			background.Add(new CObject(x,y,tiles[tile_bank][tile_num],CONSTITUTION_NONE,this));
		} /* for */ 

		/* Background objects: */ 
		if (1!=fscanf(fp,"%i",&n)) return false;
		for(i=0;i<n;i++) {
			if (3!=fscanf(fp,"%i %i %i",&x,&y,&i_tmp)) return false;
			if (1!=fscanf(fp,"%i",&i_tmp)) return false;
			if (i_tmp!=0) return false; /* No object variables should be defined!!! */ 

			if (i_tmp==semaphore_object) {
				/* a semaphore: */ 
				CObject *o;
				o=new CSemaphoreObject(x,y,tiles[semaphore_tiles[0][0]][semaphore_tiles[0][1]],
										   tiles[semaphore_tiles[1][0]][semaphore_tiles[1][1]],
										   tiles[semaphore_tiles[2][0]][semaphore_tiles[2][1]],
										   tiles[semaphore_tiles[3][0]][semaphore_tiles[3][1]],
										   tiles[semaphore_tiles[4][0]][semaphore_tiles[4][1]],this);
				objects.Add(o);
				start_delay=SEMAPHORE_TIME*7;
			} /* if */ 
		} /* for */ 

		/* Middleground tiles: */ 
		if (1!=fscanf(fp,"%i",&n)) return false;
		for(i=0;i<n;i++) {
			if (4!=fscanf(fp,"%i %i %i %i",&x,&y,&tile_bank,&tile_num)) return false;
			middleground.Add(new CObject(x,y,tiles[tile_bank][tile_num],CONSTITUTION_NONE,this));
		} /* for */ 

		/* Middleground objects: */ 
		if (1!=fscanf(fp,"%i",&n)) return false;
		for(i=0;i<n;i++) {
			if (3!=fscanf(fp,"%i %i %i",&x,&y,&i_tmp)) return false;
			if (1!=fscanf(fp,"%i",&i_tmp)) return false;
			if (i_tmp!=0) return false; /* No object variables should be defined!!! */ 

			if (i_tmp==semaphore_object) {
				/* a semaphore: */ 
				CObject *o;
				o=new CSemaphoreObject(x,y,tiles[semaphore_tiles[0][0]][semaphore_tiles[0][1]],
										   tiles[semaphore_tiles[1][0]][semaphore_tiles[1][1]],
										   tiles[semaphore_tiles[2][0]][semaphore_tiles[2][1]],
										   tiles[semaphore_tiles[3][0]][semaphore_tiles[3][1]],
										   tiles[semaphore_tiles[4][0]][semaphore_tiles[4][1]],this);
				objects.Add(o);
				start_delay=SEMAPHORE_TIME*7;
			} /* if */ 
		} /* for */ 

		/* Foreground tiles: */ 
		if (1!=fscanf(fp,"%i",&n)) return false;
		for(i=0;i<n;i++) {
			if (4!=fscanf(fp,"%i %i %i %i",&x,&y,&tile_bank,&tile_num)) return false;
			foreground.Add(new CObject(x,y,tiles[tile_bank][tile_num],CONSTITUTION_NONE,this));
		} /* for */ 

		/* Foreground objects: */ 
		if (1!=fscanf(fp,"%i",&n)) return false;
		for(i=0;i<n;i++) {
			if (3!=fscanf(fp,"%i %i %i",&x,&y,&i_tmp)) return false;
			if (1!=fscanf(fp,"%i",&i_tmp)) return false;
			if (i_tmp!=0) return false; /* No object variables should be defined!!! */ 

			if (i_tmp==semaphore_object) {
				/* a semaphore: */ 
				CObject *o;
				o=new CSemaphoreObject(x,y,tiles[semaphore_tiles[0][0]][semaphore_tiles[0][1]],
										   tiles[semaphore_tiles[1][0]][semaphore_tiles[1][1]],
										   tiles[semaphore_tiles[2][0]][semaphore_tiles[2][1]],
										   tiles[semaphore_tiles[3][0]][semaphore_tiles[3][1]],
										   tiles[semaphore_tiles[4][0]][semaphore_tiles[4][1]],this);
				objects.Add(o);
				start_delay=SEMAPHORE_TIME*7;
			} /* if */ 
		} /* for */ 
	}



	fclose(fp);
	return true;
} /* CGame::load_map */ 
