#include <windows.h>
#include "codemap.h"

int main( int argc, char *argv[] )
{
	int		nR;
	ULONGLONG	qwTick;

	printf("YUZA OS Symbol Extractor (c) Copyright 2020 by Juhang Park\n" );

	if( argc <= 1 )
	{
		printf( "USAGE : SymbolExtractor <MAP_FILE> [DBG_FILE]\n" );
		return 0;
	}

	qwTick = GetTickCount64();
	
	// Map file, Dbg file
	nR = MakeCodeMap( argv[1], argv[2] );

	qwTick = GetTickCount64() - qwTick;

	printf( "Ok. [Elapsed Time: %lld.%lld sec]\n", qwTick / 1000, qwTick % 1000 );
    return 0;
}