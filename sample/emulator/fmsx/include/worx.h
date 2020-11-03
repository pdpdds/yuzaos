/********************************************* 
 *   WORX Sound Blaster Developers Toolkit   * 
 *   Version 2.01(L) 05/27/93                * 
 *   (c) 1993 by Mystic Software             * 
 *   All rights reserved.                    * 
 *********************************************/

#include "../port.h"
#ifdef __cplusplus
extern "C" {
#endif

void StartWorx(void);
void CloseWorx(void);
int16_t ForceConfig(int16_t Port);
int16_t AdlibDetect(void);

void DSPClose(void);
int16_t DSPReset(void);
int16_t DSPPortSetting(void);
int16_t ResetMPU401(void);

int16_t SetMasterVolume(unsigned char left,unsigned char right);
int16_t SetVOCVolume(unsigned char left,unsigned char right);
int16_t SetFMVolume(unsigned char left,unsigned char right);

int16_t StartResource(char *f_name);
int32_t OpenElement(char *f_name);
char *ElementGets(char *data,unsigned char maxlen);
unsigned ElementRead(void far *buffer,uint16_t length);

void SetLoopMode(int16_t m);
void GoNote(unsigned char midi_channel,unsigned char note,unsigned char velo);
void StopNote(unsigned char midi_channel,unsigned char note);

char far *GetSequence(char *f_name);
char far *LoadOneShot(char *f_name);
int16_t LoadSBIFile(char *f_name,char pnum);
void ProgramChange(int16_t midi_channel,int16_t program);
int16_t LoadIBKFile(char *f_name);

int16_t PlayPWMBlock(char far *pwm);
int16_t PlayVOCBlock(char far *voc,int16_t volume);
void PlayCMFBlock(char far *seq);
void PlayMIDBlock(char far *seq);
int16_t PlayVOCFile(char *f_name,int16_t volume);
int16_t PlayWAVBlock(char far *wav);
int16_t PlayWAVFile(char *f_name);

void ResetRealTime(void);
int16_t SetRealTime(int16_t count);
int16_t TimerDone(void);

void ContinueSequence(void);
void StopSequence(void);
void StopVOC(void);
void StopPWM(void);
unsigned GetLastVOCMarker(void);
void SetVOCIndex(unsigned x);

int16_t SequencePlaying(void);
int16_t VOCPlaying(void);
int16_t PWMPlaying(void);
int16_t GetMIDIBeat(void);

void SetAudioMode(unsigned char Mode);
void SetMIDISpeaker(unsigned char Channel);

int16_t WorxPresent(void);
void CloseWorxDriver(void);

int16_t JoyStickY(void);
int16_t JoyStickX(void);
int16_t JoyStickButton(int16_t num);
void JoyStickUpdate(void);
void EnableExtenderMode(void);

/********************************************* 
 *   WORX+ Polyphonic Sample Player          * 
 *   Version 1.0(C)  05/05/93                * 
 *   (c) 1993 by Mystic Software             * 
 *   All rights reserved.                    * 
 *********************************************/

int16_t StartPoly(void);
void ClosePoly(void);
int16_t PlaySMPBlock(char *sample,int16_t cell,int16_t note);
int16_t PolyCellStatus(int16_t cell);

#ifdef __cplusplus
}
#endif

