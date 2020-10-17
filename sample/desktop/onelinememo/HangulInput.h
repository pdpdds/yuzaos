#pragma once

#include <windef.h>
#include <skyoswindow.h>

#define TOLOWER( x ) ( ( ( 'A' <= ( x ) ) && ( ( x ) <= 'Z' ) ) ? \
    ( ( x ) - 'A' + 'a' ) : ( x ) )

typedef struct HangulInputTableItemStruct
{
    char* pcHangul;
	char* pcInput;
} HANGULINPUTITEM;

typedef struct HangulIndexTableItemStruct
{
    char cStartCharactor;
	DWORD dwIndex;
} HANGULINDEXITEM;


////////////////////////////////////////////////////////////////////////////////
//
// 함수
//
////////////////////////////////////////////////////////////////////////////////
bool IsJaum( char cInput );
bool IsMoum( char cInput );
bool FindLongestHangulInTable( const char* pcInputBuffer, int iBufferCount,
                        int* piMatchIndex, int* piMatchLength );
bool ComposeHangul( char* pcInputBuffer, int* piInputBufferLength,
    char* pcOutputBufferForProcessing, char* pcOutputBufferForComplete );
