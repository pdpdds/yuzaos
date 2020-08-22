#ifndef WAV_H
#define WAV_H

#include "windef.h"
#include "./audio/ac97_intel.h"

#ifdef  __cplusplus
extern "C" {
#endif

#pragma pack(push, 1)
// cf. http://soundfile.sapp.org/doc/WaveFormat/
typedef struct
{
    uint8_t     marker[4]; // fmt: "fmt " 0x666d7420,  data: "data" 0x64617461
    uint32_t    length;    // fmt: 16 for PCM,         data: numSamples * numChannels * bitsPerSample/8
} subChunkHeader_t; // sub-chunks: fmt, data

typedef struct
{
    uint8_t     riffMarker[4];      // RIFF 0x52494646 (Resource Interchange File Format) // Files with big-endian data show RIFX instead of RIFF
    uint32_t    fileLength;         // file size - 8
    uint8_t     fileTypeHeader[4];  // WAVE 0x57415645

    subChunkHeader_t fmtHeader;

    // 16 byte format chunk
    uint16_t    audioFormat;    // 1=PCM (Pulse-Code-Modulation)
    uint16_t    numChannels;    // Mono = 1, Stereo = 2
    uint32_t    sampleRate;     // samples per second: 8000, 44100, etc.
    uint32_t    bytesPerSecond; // sampleRate * numChannels * bitsPerSample/8
    uint16_t    bytesPerSample; // numChannels * bitsPerSample/8
    uint16_t    bitsPerSample;  // 8 bits = 8, 16 bits = 16

    // search for "fact", "data", ...
    uint8_t     byte[100];
} wavHeaderPCM_t;
#pragma pack(pop)

bool wav_parseWAV(ac97Intel_t* ac97_currentDevice, void* wav);
void wav_playStartWav(ac97Intel_t* ac97_currentDevice);

#ifdef  __cplusplus
}
#endif
#endif
