#pragma once
#include <minwindef.h>


#define MAXOUTPUTLENGTH     60

typedef struct BufferManagerStruct
{
	char vcInputBuffer[20];
	int iInputBufferLength;

	char vcOutputBufferForProcessing[3];
	char vcOutputBufferForComplete[3];

	char vcOutputBuffer[MAXOUTPUTLENGTH];
	int iOutputBufferLength;
} BUFFERMANAGER;

void ConvertJaumMoumToLowerCharactor(BYTE* pbInput);