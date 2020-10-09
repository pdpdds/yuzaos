#include <windows.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include "pefile.h"
#include "mydbg2.h"

int is_coff_dbg2( char *pB )
{		
	if( memcmp( pB, MY_COFF_DBG2_MAGIC_STR, strlen( MY_COFF_DBG2_MAGIC_STR ) ) == 0 )
		return( 1 );	// ok.

	return( 0 );		// no coffdbg2
}


MyCoffDbg2Stt *get_coff_dbg2( char *pB )
{
	int				nI;
	char			*pX;
	MyCoffDbg2Stt	*pMy;

	pMy = (MyCoffDbg2Stt*)pB;
	pX  = pB;

	// check magic string
	if( is_coff_dbg2( pB ) == 0 )
		return( 0 );
	
	nI = sizeof( MyCoffDbg2Stt);

	pMy->pFileTbl = (MyCoffDbg2FileStt*)&pX[ nI ];
	nI += sizeof( MyCoffDbg2FileStt ) * pMy->nTotalFileEnt;
	
	pMy->pFuncTbl = (MyCoffDbg2FuncStt*)&pX[ nI ];
	nI += sizeof( MyCoffDbg2FuncStt ) * pMy->nTotalFuncEnt;

	pMy->pFuncNameIndex = (int*)&pX[ nI ];
	nI += sizeof( int ) * pMy->nTotalFuncEnt;

	pMy->pFuncAddrIndex = (int*)&pX[ nI ];
	nI += sizeof( int ) * pMy->nTotalFuncEnt;

	pMy->pLineTbl = (struct _MY_IMAGE_LINENUMBER*)&pX[ nI ];
	nI += sizeof( struct _MY_IMAGE_LINENUMBER ) * pMy->nTotalLineEnt;
	
	pMy->pLocalTbl = (MyCoffDbg2LocalStt*)&pX[ nI ];
	nI += sizeof( MyCoffDbg2LocalStt ) * pMy->nTotalLocalEnt;

	pMy->pStrTbl = (char*)&pX[ nI ];

	return( pMy );
}

// load debug info dynamically
// load debug info dynamically
MyCoffDbg2Stt* load_mydbg2_info(char* pFileName)
{
	int				nR;
	long			lSize;
	char* pBuff;
	int				nHandle;
	MyCoffDbg2Stt* pMyDbg2;

	// open file
	nHandle = open(pFileName, O_RDONLY);

	if (nHandle == -1)
		return 0;

	// get image size
	lSize = lseek(nHandle, 0, SEEK_END);
	if (lSize < 0)
	{
		close(nHandle);
		return 0;
	}

	lseek(nHandle, 0, SEEK_SET);

	// allocate memory
	pBuff = (char*)malloc(lSize);
	if (pBuff == NULL)
	{
		printf("load_mydbg2_info() - insufficient memory!\n");
		close(nHandle);
		return 0;
	}

	// read image
	nR = read(nHandle, pBuff, lSize);
	// close file
	close(nHandle);

	// get debug information
	pMyDbg2 = get_coff_dbg2(pBuff);
	if (pMyDbg2 == NULL)
	{
		free(pBuff);
		printf("load_mydbg2_info() - get_coff_dbg2 returned NULL\n");
		return 0;
	}

	printf("debug information (%d) - ok\n", pMyDbg2->dwSize);

	return pMyDbg2;
}

// get function entry by virtual address
MyCoffDbg2FuncStt *get_func_ent_by_addr( MyCoffDbg2Stt *pMy, DWORD dwAddr )
{
	MyCoffDbg2FuncStt	*pFuncTbl, *pFX, *pFY;
	int					nX, nY, nMin, *pAddrIndex;

	if( pMy == NULL )
		return( NULL );

	nX = 0;
	nY = pMy->nTotalFuncEnt-1;
	pAddrIndex = pMy->pFuncAddrIndex;
	pFuncTbl   = pMy->pFuncTbl;

	for( ;; )									  
	{	// binary search
		pFX = &pFuncTbl[ pAddrIndex[nX] ];
		pFY = &pFuncTbl[ pAddrIndex[nY] ];

		if( pFX->dwAddr == dwAddr )
			return( pFX );
		if( pFY->dwAddr == dwAddr )
			return( pFY );

		nMin = (nX+nY) / 2;

		if( nMin == nX || nMin == nY )
			break;

		if( dwAddr < pFuncTbl[ pAddrIndex[nMin] ].dwAddr )
			nY = nMin;
		else
			nX = nMin;
	}	

	return( NULL );	// not found
}

// get nearest function entry by virtual address
MyCoffDbg2FuncStt *get_nearest_func_ent_by_addr( MyCoffDbg2Stt *pMy, DWORD dwAddr )
{
	MyCoffDbg2FuncStt	*pFuncTbl, *pFStart, *pFEnd;
	int					nStart, nEnd, nMin, *pAddrIndex;

	nStart     = 0;
	nEnd       = pMy->nTotalFuncEnt-1;
	pAddrIndex = pMy->pFuncAddrIndex;
	pFuncTbl   = pMy->pFuncTbl;

	// binary search
	for( ;; )
	{	
		pFStart = &pFuncTbl[ pAddrIndex[ nStart ] ];
		pFEnd   = &pFuncTbl[ pAddrIndex[  nEnd  ] ];

		if( pFStart->dwAddr <= dwAddr && dwAddr < pFStart->dwAddr + pFStart->dwSize )
			return( pFStart );
		if( pFEnd->dwAddr <= dwAddr && dwAddr < pFEnd->dwAddr + pFEnd->dwSize )
			return( pFEnd );

		nMin = ( nStart + nEnd ) / 2;

		if( nMin == nStart || nMin == nStart )
			break;
		
		if( dwAddr < pFuncTbl[ pAddrIndex[nMin] ].dwAddr )
			nEnd = nMin;
		else
			nStart = nMin;
	}	
	return( NULL );	// not found
}

extern void uppercase(char* pS);

// compare filename
static int chk_func_name( MyCoffDbg2Stt *pMy, MyCoffDbg2FuncStt *pFunc, char *pS )
{
	int		nI;
	char	*pFuncName;
	char	szS1[64], szS2[64];
	pFuncName = &pMy->pStrTbl[ pFunc->nNameIndex ];

	if( pFuncName[0] == '_' )
		pFuncName++;

	strcpy( szS1, pFuncName );
	strcpy( szS2, pS );

	uppercase( szS1 );
	uppercase( szS2 );

	for( nI = 0; szS2[nI] != 0; nI++ )
	{
		if( szS1[nI] < szS2[nI] )
			return( -1 );
		else if( szS1[nI] > szS2[nI] )
			return( 1 );
	}			 
	
	return( 0 );	// string is matched.
}	

// get function entry by name
MyCoffDbg2FuncStt *get_func_ent_by_name( MyCoffDbg2Stt *pMy, char *pS )
{
	int					nX, nY, nMin, *pNameIndex;
	MyCoffDbg2FuncStt	*pFuncTbl, *pFMin, *pFX, *pFY;

	if( pMy == NULL )
		return( NULL );

	nX = 0;
	nY = pMy->nTotalFuncEnt-1;
	pNameIndex = pMy->pFuncNameIndex;
	pFuncTbl   = pMy->pFuncTbl;

	for( ;; )
	{	// binary search
		pFX = &pFuncTbl[ pNameIndex[nX] ];
		pFY = &pFuncTbl[ pNameIndex[nY] ];

		if( chk_func_name( pMy, pFX, pS ) == 0 )
			return( pFX );				   
		if( chk_func_name( pMy, pFY, pS ) == 0 )
			return( pFX );				   

		nMin = (nX+nY) / 2;

		if( nMin == nX || nMin == nY )
			break;
		
		pFMin = &pFuncTbl[ pNameIndex[nMin] ];
		if( chk_func_name( pMy, pFMin, pS ) > 0 )
			nY = nMin;
		else
			nX = nMin;
	}	
	return( NULL );	// not found
}

static void disp_mydbg2( MyCoffDbg2Stt *pMy )
{
	printf( "MyCoffDbg2Stt: 0x%X\n", (DWORD)pMy );
	printf( " dwSize			= %d\n", (DWORD)pMy->dwSize			  );
	printf( " pFileTbl         = 0x%X\n", (DWORD)pMy->pFileTbl          );
	printf( " nTotalFileEnt    = %d\n",   (DWORD)pMy->nTotalFileEnt     );
	printf( " pFuncTbl         = 0x%X\n", (DWORD)pMy->pFuncTbl          );
	printf( " nTotalFuncEnt    = %d\n",   (DWORD)pMy->nTotalFuncEnt     );
	printf( " pFuncNameIndex   = 0x%X\n", (DWORD)pMy->pFuncNameIndex    );
	printf( " pFuncAddrIndex   = 0x%X\n", (DWORD)pMy->pFuncAddrIndex    );
	printf( " pLineTbl         = 0x%X\n", (DWORD)pMy->pLineTbl 		  );
	printf( " nTotalLineEnt    = %d\n",   (DWORD)pMy->nTotalLineEnt     );
	printf( " pLocalTbl        = 0x%X\n", (DWORD)pMy->pLocalTbl         );
	printf( " nTotalLocalEnt   = %d\n",   (DWORD)pMy->nTotalLocalEnt    );
	printf( " pStrTbl          = 0x%X\n", (DWORD)pMy->pStrTbl           );
	printf( " nStrTblSize      = %d\n",   (DWORD)pMy->nStrTblSize       );
}

static void disp_mydbg2_func( MyCoffDbg2FuncStt *pFunc )
{
	printf( "MyCoffDbg2FuncStt: 0x%X\n", (DWORD)pFunc      );
	printf( " nNameIndex  = %d\n",    pFunc->nNameIndex    );
	printf( " nFileIndex  = %d\n",    pFunc->nFileIndex    );
	printf( " nLineIndex  = %d\n",    pFunc->nLineIndex    );
	printf( " nLocalIndex = %d\n",    pFunc->nLocalIndex   );
	printf( " nTotalLocal = %d\n",    pFunc->nTotalLocal   );
	printf( " dwAddr      = 0x%X\n",  pFunc->dwAddr        );
	printf( " dwSize      = %d\n",    pFunc->dwSize        );
}	

// get source filename and linenumber by a virtual address
int get_file_func_lineno( MyCoffDbg2Stt *pMy, char *pFileName, char *pFuncName, DWORD dwAddr )
{
	int							nI;
	MyCoffDbg2FileStt			*pFile;
	MyCoffDbg2FuncStt			*pFunc;
	
	int							nLinenumber;
	struct _MY_IMAGE_LINENUMBER	*pLine, *pNextLine;

	pFileName[0] = 0;
	pFuncName[0] = 0;

	if( pMy == NULL )
		return( -1 );		// error

	// find nearest function entry by address
	pFunc = get_nearest_func_ent_by_addr( pMy, dwAddr );
	if( pFunc == NULL )
		return( -1 );		// error

	// 디버깅 용
	//disp_mydbg2( pMy );
	//disp_mydbg2_func( pFunc );

	// get file entry
	pFile = &pMy->pFileTbl[ pFunc->nFileIndex ];
	// get line number entry
	pLine = NULL;

	for( nI = pFunc->nLineIndex; nI < pFunc->nLineIndex + pFile->nTotalLine - 1; nI++ )
	{

		pLine = &pMy->pLineTbl[nI];
		pNextLine = &pMy->pLineTbl[ nI+1 ];

		//아래의 경우는 새로운 함수의 경계를 의미한다.
		if (pLine->Type.VirtualAddress > pNextLine->Type.VirtualAddress && pNextLine->Type.VirtualAddress == 0)
		{
			break;
		}

		// 디버깅을 위한것.
		/*printf( "VirtualAddress(%x - %x) Addr = %X\n", 
			pLine->Type.VirtualAddress+ dwFileBaseAddr, 
			pNextLine->Type.VirtualAddress+ dwFileBaseAddr, 
			dwAddr );*/
								
		DWORD dwFuncBaseOAddress = pFunc->dwAddr;
		//라인의 VirtualAddress는 함수시작 주소로부터 오프셋을 의미
		if( dwAddr < pLine->Type.VirtualAddress + dwFuncBaseOAddress)
		{
			pLine = NULL;
			break;
		}
		else if( pLine->Type.VirtualAddress == dwAddr )	// do not remove
			break;
		else if( pLine->Type.VirtualAddress + dwFuncBaseOAddress <= dwAddr &&
			     dwAddr < pNextLine->Type.VirtualAddress + dwFuncBaseOAddress)
			break;
	}
	
	if(pLine == 0 || nI == pMy->nTotalLineEnt)
		nLinenumber = 0;	// 라인번호를 찾을 수 없다.
	else
		nLinenumber =  (int)pLine->Linenumber;

	// 소스명과 함수명을 복사한다.
	strcpy( pFileName, &pMy->pStrTbl[ pFile->nNameIndex ] );
	strcpy( pFuncName, &pMy->pStrTbl[ pFunc->nNameIndex ] );

	return( nLinenumber );
}
















