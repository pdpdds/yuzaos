
#include "wav.h"
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <intrinsic.h>
#include <systemcall_impl.h>
#include <kmalloc.h>

bool wav_parseWAV(ac97Intel_t* ac97_currentDevice, void* wav)
{
    wavHeaderPCM_t* wavHeader = (wavHeaderPCM_t*)wav;

    if (wavHeader->riffMarker[0] == 'R' && wavHeader->riffMarker[1] == 'I' && wavHeader->riffMarker[2] == 'F' && wavHeader->riffMarker[3] == 'F')
    {
        //textColor(SUCCESS);
        printf("\nRIFF: ok\n");
       // textColor(TEXT);
    }

    printf("\tfileLength: %u", wavHeader->fileLength);

    if (wavHeader->fileTypeHeader[0] == 'W' && wavHeader->fileTypeHeader[1] == 'A' && wavHeader->fileTypeHeader[2] == 'V' && wavHeader->fileTypeHeader[3] == 'E')
    {
       // textColor(SUCCESS);
        printf("WAVE: ok\n");
      //  textColor(TEXT);
    }

    if (wavHeader->fmtHeader.marker[0] == 'f' && wavHeader->fmtHeader.marker[1] == 'm' && wavHeader->fmtHeader.marker[2] == 't' && wavHeader->fmtHeader.marker[3] == ' ')
    {
      //  textColor(SUCCESS);
        printf("fmt: ok\n");
      //  textColor(TEXT);
    }

    printf("\nfmtLength: %u",       wavHeader->fmtHeader.length);
    printf("\naudioFormat: %u",     wavHeader->audioFormat);
    printf("\nnumChannels: %u",     wavHeader->numChannels);

    printf("\nsampleRate: %u",      wavHeader->sampleRate);
    OutPortWord(ac97_currentDevice->nambar + PORT_NAM_FRONT_DAC_RATE, wavHeader->sampleRate); // General sample rate
	OutPortWord(ac97_currentDevice->nambar + PORT_NAM_LR_ADC_RATE,    wavHeader->sampleRate); // Stereo  sample rate

    printf("\nbytesPerSecond: %u",  wavHeader->bytesPerSecond);
    printf("\nbytesPerSample: %u",  wavHeader->bytesPerSample);
    printf("\nbitsPerSample: %u\n", wavHeader->bitsPerSample);

    uint8_t i=0;
    while (wavHeader->byte[i + 0] != 'd' || wavHeader->byte[i + 1] != 'a' || wavHeader->byte[i + 2] != 't' || wavHeader->byte[i + 3] != 'a')
    {
        i++;
        if (i>49)
        {
            printf("\ndata chunk not found.");
            return false;
        }
    }

    subChunkHeader_t* dataChunk = (subChunkHeader_t*)&wavHeader->byte[i];
    printf("\ndataLength: %u\n", dataChunk->length);

    //Syscall_GetChar();

	if(ac97_currentDevice->buffer)
		free(ac97_currentDevice->buffer);
    ac97_currentDevice->NUM = dataChunk->length/wavHeader->bytesPerSample;
    ac97_currentDevice->buffer = (uint16_t*)malloc(dataChunk->length);
	//ac97_currentDevice->buffer = (uint16_t*)kmalloc_ap(dataChunk->length, &pa);


	/*if (pa != Syscall_GetPAFromVM(ac97_currentDevice->buffer))
	{
		printf("error!! %x %x\n", pa, Syscall_GetPAFromVM(ac97_currentDevice->buffer));
		for (;;);
	}*/

    void* dataStart = dataChunk + 1;
    memcpy(ac97_currentDevice->buffer, dataStart, dataChunk->length);

	if(ac97_currentDevice->descs)
		free(ac97_currentDevice->descs);
    
	ac97_currentDevice->numDesc = 2;
    ac97_currentDevice->descs = (ac97Intel_bufDesc_t*)malloc(sizeof(ac97Intel_bufDesc_t) * ac97_currentDevice->numDesc);

    ac97Intel_start(ac97_currentDevice);
    //textColor(IMPORTANT);
    printf("\nPress key to stop sound...");
   // textColor(TEXT);
	//Syscall_GetChar();

	for (;;)
	{
		Syscall_Sleep(1000);
		//printf("\nPress key to stop sound... %x", ac97_currentDevice->NUM);
	}
    ac97Intel_stop(ac97_currentDevice);

    return true;
}

void wav_playStartWav(ac97Intel_t* ac97_currentDevice)
{
    FILE* file = fopen("START.wav", "r");

    if (file)
    {
		fseek(file, 0, SEEK_END);
		int fileSize = ftell(file);
		fseek(file, 0, SEEK_SET);

        void* wav_start = (void*)malloc(fileSize);
        fread(wav_start, fileSize, 1, file);
        fclose(file);

        wav_parseWAV(ac97_currentDevice, wav_start);
    }
    else
    {
        printf("\nSTART.WAV could not be opened.");
    }
}