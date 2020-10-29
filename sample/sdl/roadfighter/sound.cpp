#include "SDL.h"
#include "SDL_mixer.h"
#include "sound.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "assert.h"

#include "filehandling.h"
#include <systemcall_impl.h>

#ifdef KITSCHY_DEBUG_MEMORY 
#include "debug_memorymanager.h"
#endif

#define AUDIO_BUFFER	2048


bool sound_enabled=false;
Mix_Music *music_sound=0;
int n_channels=-1;


bool Sound_initialization(void)
{
	if (-1==Sound_initialization(0,0)) return false;
	return true;
} /* Sound_initialization */ 


int Sound_initialization(int nc,int nrc)
{
    //char SoundcardName[256];
	int audio_rate = 44100;
	int audio_channels = 2;
	int audio_bufsize = AUDIO_BUFFER;
	Uint16 audio_format = AUDIO_S16;
	SDL_version compile_version;
	n_channels=8;

	sound_enabled=false;
#ifdef __DEBUG_MESSAGES
	output_debug_message("Initializing SDL_mixer.\n");
#endif
	if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_bufsize))  {
	  sound_enabled=false;
#ifdef __DEBUG_MESSAGES
  	  output_debug_message("Unable to open audio: %s\n", Mix_GetError());
  	  output_debug_message("Running the game without audio.\n");
#endif
	  return -1;	
	} /* if */ 

	//SDL_AudioDriverName (SoundcardName, sizeof (SoundcardName));
	Mix_QuerySpec (&audio_rate, &audio_format, &audio_channels);
#ifdef __DEBUG_MESSAGES
	output_debug_message("    opened %s at %d Hz %d bit %s, %d bytes audio buffer\n",
						 SoundcardName, audio_rate, audio_format & 0xFF,
						 audio_channels > 1 ? "stereo" : "mono", audio_bufsize);
#endif

	MIX_VERSION (&compile_version);
#ifdef __DEBUG_MESSAGES
	output_debug_message("    compiled with SDL_mixer version: %d.%d.%d\n",
						 compile_version.major,
						 compile_version.minor,
						 compile_version.patch);
	output_debug_message("    running with SDL_mixer version: %d.%d.%d\n",
						 Mix_Linked_Version()->major,
						 Mix_Linked_Version()->minor,
						 Mix_Linked_Version()->patch);
#endif

	if (nc>0) n_channels=Mix_AllocateChannels(nc);
	if (nrc>0) Mix_ReserveChannels(nrc);

	return n_channels;
} /* Sound_init */ 

void Sound_release(void)
{
	Sound_release_music();
	if (sound_enabled) {
//		Sound_Quit();
		Mix_CloseAudio();
	} /* if */ 
	sound_enabled=false;
} /* Sound_release */ 


void Stop_playback(void)
{
	if (sound_enabled) {
		Sound_pause_music();
//		Mix_HookMusic(0, 0);
		Mix_CloseAudio();
		sound_enabled=false;
	} /* if */ 
} /* Stop_playback */ 

void Resume_playback(void)
{
	Resume_playback(0,0);
} /* Resume_playback */ 


int Resume_playback(int nc,int nrc)
{
    //char SoundcardName[256];
	int audio_rate = 44100;
	int audio_channels = 2;
	int audio_bufsize = AUDIO_BUFFER;
	Uint16 audio_format = AUDIO_S16;
	SDL_version compile_version;
	n_channels=8;

	sound_enabled=true;
#ifdef __DEBUG_MESSAGES
	output_debug_message("Initializing SDL_mixer.\n");
#endif
	if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_bufsize))  {
	  sound_enabled=false;
#ifdef __DEBUG_MESSAGES
  	  output_debug_message("Unable to open audio: %s\n", Mix_GetError());
  	  output_debug_message("Running the game without audio.\n");
#endif
	  return -1;	
	} /* if */ 

	//SDL_AudioDriverName (SoundcardName, sizeof (SoundcardName));
	Mix_QuerySpec (&audio_rate, &audio_format, &audio_channels);
#ifdef __DEBUG_MESSAGES
	output_debug_message("    opened %s at %d Hz %d bit %s, %d bytes audio buffer\n",
						 SoundcardName, audio_rate, audio_format & 0xFF,
						 audio_channels > 1 ? "stereo" : "mono", audio_bufsize);
#endif
	
	MIX_VERSION (&compile_version);
#ifdef __DEBUG_MESSAGES
	output_debug_message("    compiled with SDL_mixer version: %d.%d.%d\n",
						 compile_version.major,
						 compile_version.minor,
						 compile_version.patch);
	output_debug_message("    running with SDL_mixer version: %d.%d.%d\n",
						 Mix_Linked_Version()->major,
						 Mix_Linked_Version()->minor,
						 Mix_Linked_Version()->patch);
#endif

	if (nc>0) n_channels=Mix_AllocateChannels(nc);
	if (nrc>0) Mix_ReserveChannels(nrc);

	Sound_unpause_music();

	return n_channels;
} /* Resume_playback */ 


/* a check to see if file is readable and greater than zero */
int file_check(char *fname)
{
	FILE *fp;

	if ((fp=f1open(fname, "r", GAMEDATA))!=NULL) {
		if (fseek(fp,0L, SEEK_END)==0 && ftell(fp)>0) {
  			fclose(fp);
			return true;
		} /* if */ 
		/* either the file could not be read (==-1) or size was zero (==0) */ 
#ifdef __DEBUG_MESSAGES
		output_debug_message("ERROR in file_check(): the file %s is corrupted.\n", fname);
#endif		
		fclose(fp);
		exit(1);
	} /* if */ 
	return false;
} /* file_check */ 

SOUNDT Sound_create_sound(char* file)
{
	int n_ext = 6;
	char* ext[6] = { ".WAV",".OGG",".MP3",".wav",".ogg",".mp3" };
	char name[256];
	int i;

	if (sound_enabled) 
	{
		for (i = 0; i < n_ext; i++) {
			strcpy(name, file);
			strcat(name, ext[i]);
			if (file_check(name))
				return Mix_LoadWAV(name);
		} /* for */

#ifdef __DEBUG_MESSAGES
		output_debug_message("ERROR in Sound_create_sound(): Could not load sound file: %s.(wav|ogg|mp3)\n", file);
#endif

		exit(1);
	}
	else {
		return 0;
	} /* if */
} /* Sound_create_sound */


void Sound_delete_sound(SOUNDT s)
{
	if (sound_enabled) Mix_FreeChunk(s);
} /* Sound_delete_sound */ 


int Sound_play(SOUNDT s)
{
	if (sound_enabled) return Mix_PlayChannel(-1,s,0);
	return -1;
} /* Sound_play */ 


int Sound_play(SOUNDT s,int volume)
{
	if (sound_enabled) {
		int channel=Mix_PlayChannel(-1,s,0);
//#ifdef __DEBUG_MESSAGES
//		output_debug_message("SFX player on channel %i (volume = %i)\n",channel,volume);
//#endif	
		Mix_Volume(channel,volume);
		return channel;
	} /* if */ 
	return -1;
} /* Sound_play */ 


int Sound_play_continuous(SOUNDT s)
{
	if (sound_enabled) return Mix_PlayChannel(-1,s,-1);
	return -1;
} /* Sound_play */ 


int Sound_play_continuous(SOUNDT s,int volume)
{
	if (sound_enabled) {
		int channel=Mix_PlayChannel(-1,s,-1);
		Mix_Volume(channel,volume);
		return channel;
	} /* if */ 
	return -1;
} /* Sound_play */ 


void Sound_play_ch(SOUNDT s,int ch)
{
	if (sound_enabled && ch<n_channels) Mix_PlayChannel(ch,s,0);
} /* Sound_play_ch */ 


void Sound_play_ch(SOUNDT s,int ch,int volume)
{
//#ifdef __DEBUG_MESSAGES
//	output_debug_message("Playing %p in %i at %i...",(void *)s,ch,volume);
//#endif
	if (sound_enabled && ch<n_channels) {
		int channel=Mix_PlayChannel(ch,s,0);
		Mix_Volume(channel,volume);
	} /* if */ 
//#ifdef __DEBUG_MESSAGES
//	output_debug_message("ok\n");
//#endif
} /* Sound_play_ch */ 


Mix_Music *Sound_create_stream(char *file)
{
	int n_ext=6;
	char *ext[6]={".WAV",".OGG",".MP3",".wav",".ogg",".mp3"};
	char name[256];
	int i;

	if (sound_enabled) {
		for(i=0;i<n_ext;i++) {
			strcpy(name,file);
			strcat(name,ext[i]);
			if (file_check(name)) return Mix_LoadMUS(name);
		} /* for */ 
		
#ifdef __DEBUG_MESSAGES
		output_debug_message("ERROR in Sound_create_stream(): Could not load sound file: %s.(wav|ogg|mp3)\n", file);
#endif	
		exit(1);
	} else {
		return 0;
	} /* if */ 
} /* Sound_create_stream */ 


void Sound_create_music(char *f1,int loops)
{
	if (sound_enabled) {
		if (f1!=0) {
			music_sound=Sound_create_stream(f1);
			Mix_PlayMusic(music_sound,loops);
		} else {
			music_sound=0;
		} /* if */ 

//		playing_music=true;
	} /* if */ 
} /* Sound_create_music */ 


bool Sound_file_test(char *f1)
{
	int n_ext=6;
	char *ext[6]={".WAV",".OGG",".MP3",".wav",".ogg",".mp3"};
	char name[256];
	int i;

	if (sound_enabled) {
		for(i=0;i<n_ext;i++) {
			strcpy(name,f1);
			strcat(name,ext[i]);
			if (file_check(name)) return true;
		} /* for */ 
		
		return false;
	} else {
		return false;
	} /* if */ 
} /* Sound_file_test */ 


void Sound_release_music(void)
{
	if (sound_enabled) {
//		playing_music=false;
		Mix_HaltMusic();
		if (music_sound!=0) Mix_FreeMusic(music_sound);
		music_sound=0;
	} /* if */ 
} /* Sound_release_music */ 



void Sound_pause_music(void)
{
	Mix_PauseMusic();
} /* Sound_pause_music */ 


void Sound_unpause_music(void)
{
	Mix_ResumeMusic();
} /* Sound_unpause_music */ 


void Sound_music_volume(int volume)
{
	if (volume<0) volume=0;
	if (volume>127) volume=127;
	Mix_VolumeMusic(volume);
} /* Sound_music_volume */ 
