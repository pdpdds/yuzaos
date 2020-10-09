/*////////////////////////////////////////////////////////////////////////////////////////

1.  먼저 실행파일, MAP 파일, COD 파일을 한 디렉토리에 모아 둔다.
2.  CODEMAP <모아둔 경로의 실행파일> [별도의 DBG 파일]
    -  DBG 파일명을 입력하면 별도의 파일에 디버깅 정보를 기록하고 
	   그렇지 않으면 실행파일의 뒷 부분에 덧붙인다.
3.  실행파일 이름으로부터 MAP 파일 이름을 구하여 MAP 파일을 먼저 처리한 후 
    MAP 파일에 등장하는 COD 파일을 처리한다.
4.  라이브러리의 경우 모아둔 경로 아래에 라이브러리 이름을 만든 후 그곳에 COD
    파일만 모아둔다.
5.  _TEXT SEGMENT로 전역함수와 지역함수를 구분했었는데 이는 잘못된 것이었다. (2003-06-20) 

  <외부 모듈의 함수 Import>
  함수의 크기가 0으로 구해지는 것은 COD 파일이 없는 경우.
  extern 함수의 library stub을 링크하게 되면 MAP 파일에는 심볼이 있으나 COD 파일이
  없으므로 함수의 크기가 0으로 나온다.  이러한 함수들은 해당 모듈에 포함하지 않는
  편이 낫다.
  Stub Library 영역을 역어셈블해 보면 JMP 명령 하나가 있을 뿐이다.

*/////////////////////////////////////////////////////////////////////////////////////////

#include "codemap.h"
#include "pefile.h"

static int init_codemap(CodeMap* pCM )
{
	memset( pCM, 0, sizeof(CodeMap) );

	// 디폴트 엔트리를 할당한다. (안해줘도 확장될 것이므로 상관없다.)
	pCM->d.pFileTbl = (MyCoffDbg2FileStt*)CM_MALLOC( sizeof( MyCoffDbg2FileStt ) * CM_DEFAULT_ENT );
	pCM->d.pFuncTbl = (MyCoffDbg2FuncStt*)CM_MALLOC( sizeof( MyCoffDbg2FuncStt ) * CM_DEFAULT_ENT );
	pCM->d.pLineTbl = (struct _MY_IMAGE_LINENUMBER*)CM_MALLOC( sizeof( struct _MY_IMAGE_LINENUMBER ) * CM_DEFAULT_ENT *10 );
	pCM->d.pStrTbl  = (char*)CM_MALLOC( 64 * CM_DEFAULT_ENT );
	pCM->pSymIndex		  = (int*)CM_MALLOC( sizeof( int ) * CM_DEFAULT_ENT );
	
	// 어차피 맵 파일을 처리한 다음에 소트하기 때문에 미리할당할 필요가 없다.
	//pCM->d.pFuncNameIndex;
	//pCM->d.pFuncAddrIndex;

	// 엔트리의 개수(크기)를 설정한다.
	pCM->nMaxFileEnt	= CM_DEFAULT_ENT;
	pCM->nMaxFuncEnt	= CM_DEFAULT_ENT;
	pCM->nMaxLineEnt	= CM_DEFAULT_ENT;
	pCM->nMaxStrTblSize = 64 * CM_DEFAULT_ENT;
	pCM->nMaxSymIndex   = CM_DEFAULT_ENT;

	// 하나라도 제대로 할당되지 않았으면 그냥 에러!
	if( !pCM->d.pFileTbl | !pCM->d.pFuncTbl | !pCM->d.pLineTbl | !pCM->d.pStrTbl  )
		return( -1 );	
	
	return( 0 );
}

static int close_codemap(CodeMap* pCM )
{
	int				nI;
	HashEntStt		*pE, *pNext;

	if( pCM->d.pFileTbl       ) CM_FREE( pCM->d.pFileTbl        );
	if( pCM->d.pFuncTbl	      )	CM_FREE( pCM->d.pFuncTbl        );
	if( pCM->d.pLineTbl	      )	CM_FREE( pCM->d.pLineTbl        );
	if( pCM->d.pLocalTbl      )	CM_FREE( pCM->d.pLocalTbl       );
	if( pCM->d.pStrTbl		  )	CM_FREE( pCM->d.pStrTbl         );
	if( pCM->d.pFuncNameIndex ) CM_FREE( pCM->d.pFuncNameIndex  );
	if( pCM->d.pFuncAddrIndex ) CM_FREE( pCM->d.pFuncAddrIndex  );
	if( pCM->pSymIndex		  ) CM_FREE( pCM->pSymIndex		   	);

	for( nI = 0; nI < MAX_HASH_ENT; nI++ )
	{
		if( pCM->hash_index[nI] != NULL )
		{
			for( pE = pCM->hash_index[nI]; pE != NULL; nI++ )
			{
				pNext = pE->pNext;
				CM_FREE( pE );
				pE = pNext;
			}	
		}
	}
	return( 0 );	
}

// 공백과 탭을 건너뛴다.
static char *skip_space( char *pS )
{
	for( ;; pS++)
	{
		if( *pS == ' ' || *pS == 9 )
			continue;
		break;
	}
	return( pS );
}


#pragma pack( push, 1 )

typedef struct {
	int		nType;
	char* pS;
} KWStt;

static KWStt	kw[] = {
	{ KWT_COLON			, ":"			},
	{ KWT_SEMI_COLON	, ";"			},
	{ KWT_QUESTION		, "?"			},
	{ KWT_F				, "f"			},
	{ KWT_I				, "i"			},
	{ KWT_ASSIGN		, "="			},
	{ KWT_TEXT			, "_TEXT"		},
	{ KWT_SEGMENT		, "SEGMENT"		},
	{ KWT_PROC			, "PROC"		},
	{ KWT_ENDP			, "ENDP"		},
	{ KWT_PARA			, "PARA"		},
	{ KWT_NEAR			, "NEAR"		},
	{ KWT_PREFERRED		, "Preferred"	},
	{ 0					, NULL			}
}; 
#pragma pack( pop )

// 주어진 단어의 타입을 구한다.
static int get_keyword_type( char *pS )
{
	int nI;
	
	// 숫자
	if( '0' <= pS[0] && pS[0] <= '9' )
	{
		return( KWT_NUMBER );
	}

	// 예약어인가?
	for( nI = 0; kw[nI].pS != NULL; nI++ )
	{
		if( strcmp( kw[nI].pS, pS ) == 0 )
			return( kw[nI].nType );
	}

	// 알수 없는 타입
	return( KWT_STR );
}

// 주어진 문자열 pS에서 한 단어를 추출한다.
static char *get_next_word( char *pWord, char *pS, int *pType )
{
	int nI;

	pWord[0] = 0;

	// 공백을 건너뛴다.
	pS = skip_space( pS );

	for( nI = 0; pS[nI] != 0;  nI++ )
	{	// CR, LF는 0으로 변경한다.
		if( pS[nI] == 10 || pS[nI] == 13 )
		{
			pS[nI] = 0;
			break;
		}
		// 탭은 공백으로 바꾸고 단어의 끝으로 처리한다.
		if( pS[nI] == ' ' || pS[nI] == 9 )
		{
			pS[nI] = ' ';
			break;
		}

		// 구분문자면 돌아간다.
		if( pS[nI] == ':' || pS[nI] == ';' )
		{
			if( nI == 0 )
			{
				pWord[0] = pS[0];
				nI++;
			}
			break;
		}

		pWord[nI] = pS[nI];
	}

	pWord[nI] = 0;

	// 단어의 타입을 계산한다. 
	pType[0] = get_keyword_type( pWord );

	return( &pS[nI] );	
}

void uppercase( char *pS )
{
	int nI;

	for( nI = 0; pS[nI] != 0; nI++ )
	{
		if( pS[nI] >= 'a' && pS[nI] <= 'z' )
			pS[nI] = pS[nI] - ( 'a' - 'A' );
	}
}

DWORD dwHexValue( char *pS )
{
	DWORD dwR = 0;
	int   nI;

	uppercase( pS );

	for( nI = 0; pS[nI] != 0; nI++ )
	{
		if( pS[nI] >= 'A' && pS[nI] <= 'Z' )
		{
			dwR = (DWORD)(dwR << 4);
			dwR += (DWORD)( ( pS[nI] - 'A' ) + 10 );
		}
		else if( pS[nI] >= '0' && pS[nI] <= '9' )
		{
			dwR = (DWORD)(dwR << 4);
			dwR += (DWORD)( pS[nI] - '0' );
		}
	}
	return( dwR );
}

// 파일 테이블을 확장한다.
static int expand_file_tbl(CodeMap* pCM )
{
	int					nSize;
	MyCoffDbg2FileStt	*pFTbl;

	nSize = sizeof( MyCoffDbg2FileStt ) * (pCM->nMaxFileEnt + CM_DEFAULT_ENT);
	pFTbl = (MyCoffDbg2FileStt*)CM_MALLOC( nSize );
	if( pFTbl == NULL )
		return( -1 );			// 메모리를 할당할 수 없으면 에러리턴!
	// 0으로 초기화한다.
	memset( pFTbl, 0, nSize );

	// 기존 데이터를 복사한다.
	memcpy( pFTbl, pCM->d.pFileTbl, pCM->d.nTotalFileEnt * sizeof( MyCoffDbg2FileStt ) );
	// 기존 버퍼를 해제한다.
	CM_FREE( pCM->d.pFileTbl );
	pCM->d.pFileTbl = pFTbl;

	// MAX 값을 증가시킨다.
	pCM->nMaxFileEnt += CM_DEFAULT_ENT;
	
	return( 0 );
}	

// 함수 테이블을 확장한다.
static int expand_func_tbl(CodeMap* pCM )
{
	int					nSize;
	MyCoffDbg2FuncStt	*pFTbl;

	nSize = sizeof( MyCoffDbg2FuncStt ) * (pCM->nMaxFuncEnt + CM_DEFAULT_ENT);
	pFTbl = (MyCoffDbg2FuncStt*)CM_MALLOC( nSize );
	if( pFTbl == NULL )
		return( -1 );			// 메모리를 할당할 수 없으면 에러리턴!
	// 0으로 초기화한다.
	memset( pFTbl, 0, nSize );

	// 기존 데이터를 복사한다.
	memcpy( pFTbl, pCM->d.pFuncTbl, pCM->d.nTotalFuncEnt * sizeof( MyCoffDbg2FuncStt ) );
	// 기존 버퍼를 해제한다.
	CM_FREE( pCM->d.pFuncTbl );
	pCM->d.pFuncTbl = pFTbl;

	// MAX 값을 증가시킨다.
	pCM->nMaxFuncEnt += CM_DEFAULT_ENT;
	
	return( 0 );
}	

// 지정된 nNameIndex와 동일한 인덱스를 사용하는 파일 엔트리가 있는지 찾는다.
static int find_file_ent( int nNameIndex, CodeMap* pCM )
{
	int					nI;
	MyCoffDbg2FileStt	*pFTbl;

	for( nI = 0; nI < pCM->d.nTotalFileEnt; nI++ )
	{
		pFTbl = &pCM->d.pFileTbl[nI];
		if( pFTbl->nNameIndex == nNameIndex )
			return( nI );
	}  

	// 찾을 수 없다.
	return( -1 );
}	

// 심볼 테이블을 확장한다.
static int expand_str_tbl(CodeMap* pCM )
{
	char	*pTbl;
	int		nSize;

	nSize = pCM->nMaxStrTblSize + (64 * CM_DEFAULT_ENT);
	pTbl = (char*)CM_MALLOC( nSize );
	if( pTbl == NULL )
		return( -1 );

	// 0으로 초기화한다.
	memset( pTbl, 0, nSize );
	memcpy( pTbl, pCM->d.pStrTbl, pCM->d.nStrTblSize );
	free( pCM->d.pStrTbl );
	pCM->d.pStrTbl = pTbl;

	pCM->nMaxStrTblSize = nSize;

	return( 0 );
}


// 심볼 인덱스 테이블을 확장한다.
static int expand_symindex_tbl(CodeMap* pCM )
{
	int *pTbl;
	int	nSize;

	nSize =  sizeof( int ) * (pCM->nMaxSymIndex + CM_DEFAULT_ENT);
	pTbl = (int*)CM_MALLOC( nSize );
	if( pTbl == NULL )
		return( -1 );

	// 0으로 초기화한다.
	memset( pTbl, 0, nSize );
	memcpy( pTbl, pCM->pSymIndex, pCM->nTotalSymIndex * sizeof( int ) );
	free( pCM->pSymIndex );
	pCM->pSymIndex = pTbl;

	pCM->nMaxSymIndex += CM_DEFAULT_ENT;

	return( 0 );
}

// 이진 탐색 루틴
/*
static int find_symbol( char *pS, CodeMapStt *pCM, int *pLastIndex )
{
	int nMid, nStart, nEnd, nR;

	nStart = 0;
	pLastIndex[0] = -1;
	nEnd   = pCM->nTotalSymIndex;
	
	for( ; nStart < nEnd; )
	{
		pLastIndex[0] = nMid = ( nStart + nEnd ) / 2;
		nR = strcmp( pS, &pCM->d.pStrTbl[ pCM->pSymIndex[ nMid ] ] );
		if( nR == 0 )
			return( nMid );
		if( nR > 0 )
		{
			if( nStart == nMid )
			{
				pLastIndex[0]++;
				break;
			}
			nStart = nMid;
		}
		else
		{
			if( nEnd == nMid )
				break;
			nEnd = nMid;
		}
	}

	return( -1 );
}

static int insert_sym_index( CodeMapStt *pCM, int nLastIndex, int nIndex )
{
	int nI;

	// nLastIndex 위치에 넣기 위해 가장 끝에서 부터 nLastIndex까지 하나씩 인덱스를 뒤로 민다.
	if( nLastIndex >= 0 )
	{
		for( nI = pCM->nTotalSymIndex; nI > nLastIndex; nI-- )
			pCM->pSymIndex[nI] = pCM->pSymIndex[nI-1];

		pCM->pSymIndex[nI] = nIndex;
	}
	else
		pCM->pSymIndex[0] = nIndex;	// 입력된 것이 하나도 없다고 보고 0번에 집어 넣는다.

	pCM->nTotalSymIndex++;

	return( 0 );
}
*/
/*
// (순차탐색) 심볼을 찾는다.
static int find_symbol( char *pS, CodeMapStt *pCM, int *pLastIndex )
{
	int nI;

	pLastIndex[0] = -1;

	for( nI = 0; nI < pCM->nTotalSymIndex; nI++ )
	{
		if( strcmp( pS, &pCM->d.pStrTbl[ pCM->pSymIndex[ nI ] ] ) == 0 )
			return( pCM->pSymIndex[ nI ] );
	}

	return( -1 );
}
static int insert_sym_index( CodeMapStt *pCM, int nLastIndex, int nIndex )
{
	pCM->pSymIndex[pCM->nTotalSymIndex] = nIndex;	// 입력된 것이 하나도 없다고 보고 0번에 집어 넣는다.

	pCM->nTotalSymIndex++;

	return( 0 );
}
*/
/////////////////////////////////////////////////////////////////////////
// 해시 키 값을 구한다.
static DWORD get_hash_key( char *pS )
{
	int					nI;
	unsigned short int	wX, wM;

	wX = 0;

	for( nI = 0; pS[nI] != 0; nI++ )
	{
		// ROL
		if( wX & (unsigned short int)0x8000 )
			wM = 1;
		else
			wM = 0;
		wX = (unsigned short int)(wX << 1) + wM;

		// XOR
		wX = (unsigned short int)( wX ^ (unsigned short int)pS[nI] );
	}	

	wM = wX / MAX_HASH_ENT;
	wX = wX % MAX_HASH_ENT;
	wX = (wX ^ wM);
	return( (DWORD)wX );
}

// 해시 테이블을 이용한 탐색
static int find_symbol( char *pS, CodeMap* pCM )
{
	int			nI;
	HashEntStt	*pE;

	nI = (int)get_hash_key( pS );

	pE = pCM->hash_index[nI];
	if( pE == NULL )
		return( -1 );	// 아직 등록되지 않은 심볼이다.

	for( ; pE != NULL; pE = pE->pNext )
	{	// Hash Entry들을 대상으로 일일이 비교를 해야 한다.
		if( strcmpi( &pCM->d.pStrTbl[ pCM->pSymIndex[ pE->nSymIndex ] ], pS ) == 0 )
			return( pCM->pSymIndex[ pE->nSymIndex ] );
	}
	return( -1 );
}
static int insert_sym_index(CodeMap* pCM, int nIndex )
{
	int			nI;
	char		*pS;
	HashEntStt	*pE, *pNew;

	pS = &pCM->d.pStrTbl[nIndex];
	nI = (int)get_hash_key( pS );
	pCM->pSymIndex[pCM->nTotalSymIndex] = nIndex;

	// 해시 테이블에 등록한다.
	pNew = (HashEntStt*)CM_MALLOC( sizeof( HashEntStt ) );
	if( pNew == NULL )
	{
		CM_PRINTF( "insert_sym_index() - memory allocation failed!\n" );
		return( -1 );
	}
	memset( pNew, 0, sizeof( HashEntStt ) );
	pNew->nSymIndex = pCM->nTotalSymIndex;
	nI = (int)get_hash_key( pS );
	pE = pCM->hash_index[nI];
	if( pE == NULL )
	{	// 제일 처음에 연결한다.
		pCM->hash_index[ nI ] = pNew;
	}
	else
	{	
		pCM->nTotalCollision++;
		// 가장 뒤에 연결한다.
		for( nI = 0; pE->pNext != NULL; pE = pE->pNext, nI++ )
			;
		if( pCM->nMaxCollision < nI )
			pCM->nMaxCollision = nI;

		pE->pNext = pNew;
	}	

	pCM->nTotalSymIndex++;

	return( 0 );
}
/////////////////////////////////////////////////////////////////////////
// 심볼 테이블에 새로운 심볼을 등록한다.
// 동일한 것이 있으면 기존 인덱스를 리턴한다.
static int register_symbol( char *pS, CodeMap* pCM )
{
	int nSize, nIndex, nR;

	// 동일한 심볼이 이미 등록되어 있는지 확인한다.
	nIndex = find_symbol( pS, pCM );
	if( nIndex >= 0 )
		return( nIndex );		// 찾은 인덱스를 리턴한다.

	// 공간을 확장해야 하는지 확인한다.
	nSize = strlen( pS );
	if( pCM->nMaxStrTblSize <= pCM->d.nStrTblSize + nSize )
	{	// 심볼 테이블을 확장한다.
		nR = expand_str_tbl( pCM );
		if( nR < 0 )
			return(-1 );
	}

	// 인덱스 공간을 확장해야 하는지 확인한다.
	if( pCM->nTotalSymIndex >= pCM->nMaxSymIndex )
	{
		nR = expand_symindex_tbl( pCM );
		if( nR < 0 )
			return( -1 );
	}

	// 심볼을 복사한다.
	nIndex = pCM->d.nStrTblSize;
	strcpy( &pCM->d.pStrTbl[ nIndex ], pS );
	pCM->d.nStrTblSize += nSize + 1;	// 끝에 붙은 0까지 계산한다.

	// 심볼의 인덱스를 추가한다.
	insert_sym_index( pCM, nIndex );
	
	return( nIndex );
}

// 파일이 존재하는지 확인한다.
static int file_exists( char *pPath )
{
	struct _finddata_t	fi;
	long				lR;

	lR = _findfirst( pPath, &fi );
	if( lR < 0 )
		return( -1 );	// 파일이 존재하지 않는다.

	_findclose( lR );
	
	// 파일을 찾았다.
	return( 0 );
}

// 풀패스에서 마지막 파일명을 제거한다.  끝에 \가 남는다.
static void make_pure_path( char *pS )
{
	int nI;

	for( nI = strlen( pS ); nI > 0; nI-- )
	{
		if( pS[nI] == '\\' )
		{
			pS[nI+1] = 0;
			return;
		}
	}
	strcpy( pS, ".\\" );
}

// 파일을 등록한다.
static int register_file( char *pMapFile, char *pS, DWORD dwAddr, CodeMap* pCM )
{
	char				szT[260];
	MyCoffDbg2FileStt	*pFile;
	int					nNameIndex, nFileIndex, nR;

	// 동일한 심볼이 이미 등록되어 있는지 확인한다.
	nNameIndex = find_symbol( pS, pCM );
	if( nNameIndex >= 0 )
	{	// 여기서 그냥 nNameIndex를 리턴해 버리는 버그가 있었음.  (2003-08-28)
		// pFunc->nFileIndex가 터무늬 없이 큰 값이 들어가 결국 
		// filename출력하는 부분에서 문제가 발생(GP Fault)!!!.
		nFileIndex = find_file_ent( nNameIndex, pCM );
		if( nFileIndex >= 0 )
			return( nFileIndex );		// 찾은 인덱스를 리턴한다.
		else
			return( -1 );
	}

	// 파일이 존재하는지 확인한다.
	strcpy( szT, pMapFile );
	make_pure_path( szT );
	nR = strlen( szT );		  // 마지막이 역슬래시가 아니면 역슬래시를 붙인다.
	if( nR > 0 && szT[nR-1] != '\\' )
		strcat( szT, "\\" );
	strcat( szT, pS );		  // path + cod_file_name
	nR = strlen( szT );
	strcpy( &szT[nR-3], "cod" );
	// 파일이 존재하는지 찾아 본다.
	nR = file_exists( szT );
	if( nR < 0 )
		return( -1 );		  // OCD 파일을 찾을 수 없으면 에러 리턴.

	// 파일명을 심볼에 등록한다.
	nNameIndex = register_symbol( pS, pCM );
	if( nNameIndex < 0 )
		return( -1 );	// 에러
	
	// 이미 등록되었는지 찾아본다.
	nFileIndex = find_file_ent( nNameIndex, pCM );
	if( nFileIndex >= 0 )
	{						   
		pFile = &pCM->d.pFileTbl[nFileIndex];
		if( pFile->dwAddr > dwAddr )
			pFile->dwAddr = dwAddr;
		return( nFileIndex );		// 이미 등록되어 있었다.
	}

	// 파일 테이블에 공간이 있는지 확인한다.
	if( pCM->nMaxFileEnt <= pCM->d.nTotalFileEnt )
	{	// 공간이 없으면 확장하여야 한다.
		nR = expand_file_tbl( pCM );
		if( nR < 0 )
			return( -1 );		// 확장할 수 없으면 에러 리턴!
	}		

	// 필드를 채우고 카운터를 증가시킨다.
	nFileIndex = pCM->d.nTotalFileEnt;
	pFile = &pCM->d.pFileTbl[nFileIndex];
	memset( pFile, 0, sizeof( MyCoffDbg2FileStt ) );
	pFile->dwAddr     = dwAddr;
	pFile->nNameIndex = nNameIndex;
	pCM->d.nTotalFileEnt++;

	return( nFileIndex );
}

// 함수를 등록한다.
static int register_func( char *pS, int nFileNameIndex, DWORD dwAddr, CodeMap* pCM )
{
	MyCoffDbg2FuncStt	*pFunc;
	int					nNameIndex, nFuncIndex, nR;

	// 함수명을 심볼에 등록한다.
	nNameIndex = register_symbol( pS, pCM );
	if( nNameIndex < 0 )
		return( -1 );
	// 이미 등록되어 있는지는 확인해 볼 필요가 없다.

	// 파일 테이블에 공간이 있는지 확인한다.
	if( pCM->nMaxFuncEnt <= pCM->d.nTotalFuncEnt )
	{	// 공간이 없으면 확장하여야 한다.
		nR = expand_func_tbl( pCM );
		if( nR < 0 )
			return( -1 );		// 확장할 수 없으면 에러리턴!
	}		

	// 필드를 채우고 카운터를 증가시킨다.
	nFuncIndex = pCM->d.nTotalFuncEnt;
	pFunc = &pCM->d.pFuncTbl[nFuncIndex];
	memset( pFunc, 0, sizeof( MyCoffDbg2FuncStt ) );
	pFunc->dwAddr     = dwAddr;
	pFunc->nNameIndex = nNameIndex;
	pFunc->nFileIndex = nFileNameIndex;
	pCM->d.nTotalFuncEnt++;

	return( 1 );
}

// MAP 파일의 한 라인을 처리한다.
// <리턴값> 0 - 그냥 스킵, 1 - 처리됨.
static int process_map_line( char *pMapFile, char *pS, CodeMap* pCM )
{
	DWORD				dwRVA;
	int					nI, nType, nFileIndex;
	char				*pNext, szT[512], szFuncName[128], szFileName[128];

	// 숫자로 시작하지 않으면 그냥 돌아간다.		0001
	pNext = get_next_word( szT, pS, &nType );
	if( nType != KWT_NUMBER )
	{	// 링커의 베이스 어드레스를 구한다.
		if( nType != KWT_PREFERRED )
			return( 0 );
		for( nI = 0; nI < 4; nI++ )
			pNext = get_next_word( szT, pNext, &nType );
		pCM->dwLinkerBaseAddr = dwHexValue( szT );				
		return( 0 );
	}

	// 다음에 콜론이 나오지 않으면 그냥 돌아간다.	:	
	pNext = get_next_word( szT, pNext, &nType );
	if( nType != KWT_COLON )
		return( 0 );

	// 다음에 숫자 나오지 않으면 그냥 돌아간다.		00001b94
	pNext = get_next_word( szT, pNext, &nType );
	if( nType != KWT_NUMBER )
		return( 0 );

	// 다음에 문자열이 아니면 그냥 돌아간다.		_kdbg_printf
	pNext = get_next_word( szT, pNext, &nType );
	if (nType != KWT_STR)
	{
		//20200925
		if (nType == KWT_QUESTION)
			nType = KWT_STR;
		else
		   return 0;
	}
	// 함수 이름을 구했다.
	strcpy( szFuncName, szT );

	// 다음에 숫자 나오지 않으면 그냥 돌아간다.		00401000
	pNext = get_next_word( szT, pNext, &nType );
	if( nType != KWT_NUMBER )
		return( 0 );
	// RVA를 구한다.
	dwRVA = dwHexValue( szT ) - pCM->dwLinkerBaseAddr;

	// 다음에 F가 나와야 한다.							f
	pNext = get_next_word( szT, pNext, &nType );
	if( nType != KWT_F )
		return( 0 );

	//파일이름이 나오기 전에 I가 나올수도 있다.
	// 숫자로 시작하는 파일도 있다.					dbg.obj
	pNext = get_next_word( szT, pNext, &nType );

	if (nType == KWT_I)
	{
		pNext = get_next_word(szT, pNext, &nType);

	}

	if (nType != KWT_STR && nType != KWT_NUMBER)
		return(0);

	strcpy( szFileName, szT );
	if( pNext[0] == ':' )
	{
		pNext = get_next_word( szT, &pNext[1], &nType );
		strcat( szFileName, "\\" );
		strcat( szFileName, szT );
	}

	// RVA, 함수 이름, 파일 이름을 구했다.
 	//CM_PRINTF( "0x%08X %-12s %s\n", dwRVA, szFileName, szFuncName );			

	// 파일을 등록한다.
	nFileIndex = register_file( pMapFile, szFileName, dwRVA, pCM );
	if( nFileIndex < 0 )
		return( 0 );	// 파일이 없으면 그냥 스킵한다.
	else if( nFileIndex >= pCM->d.nTotalFileEnt )
	{	// 심각한 에러.
		return( -1 );
	}

	// 함수를 등록한다.
	register_func( szFuncName, nFileIndex, dwRVA, pCM );

	return( 0 );
}

// MAP 파일 정보를 분석하여 pCM에 저장한다.
static int map_info( char *pFileName, CodeMap* pCM )
{
	int		nR;
	FILE	*pF;
	char	szT[512], *pS;

	// MAP 파일을 오픈한다.
	pF = fopen( pFileName, "rt" );
	if( pF == NULL )
	{	// 파일을 오픈할 수 없다.
		CM_PRINTF( "Map file <%s> open error!\n", pFileName );
		return( -1 );
	}

	// 한 라인을 처리한다.
	for( ;; )
	{
		pS = fgets( szT, sizeof(szT) -1, pF );
		if( pS == NULL )
		{
			nR = 0;
			break;		// 더이상 읽어들일 수 없다.
		}

		// 읽어들인 라인을 처리한다.
		nR = process_map_line( pFileName, szT, pCM );
		if( nR < 0 )
			break;
	}

	// 파일을 닫고 돌아간다.
	fclose( pF );

	return( nR );
}

// Local Table을 확장한다.
static int expand_local_tbl(CodeMap* pCM )
{
	int					nSize;
	MyCoffDbg2LocalStt	*pLTbl;

	nSize = sizeof( MyCoffDbg2LocalStt ) * ( pCM->nMaxLocalEnt + CM_DEFAULT_ENT );
	pLTbl = (MyCoffDbg2LocalStt*)CM_MALLOC( nSize );
	if( pLTbl == NULL )
		return( -1 );			// 메모리를 할당할 수 없으면 에러리턴!

	// 0으로 초기화한다.
	memset( pLTbl, 0, nSize );

	// 기존 데이터를 복사한다.
	memcpy( pLTbl, pCM->d.pLocalTbl, pCM->d.nTotalLocalEnt * sizeof( MyCoffDbg2LocalStt ) );

	// 기존 버퍼를 해제한다.
	CM_FREE( pCM->d.pLocalTbl );
	pCM->d.pLocalTbl = pLTbl;

	// MAX 값을 증가시킨다.
	pCM->nMaxLocalEnt += CM_DEFAULT_ENT;
	
	return( 0 );
}	

// 파러메터와 지역변수를 설정한다.
static int register_param_and_local_var(CodeMap*pCM, char *pSymName, char *pValue )
{
	MyCoffDbg2LocalStt		*pL;
	int nValue,				nR, nNameIndex;

	nValue = atoi( pValue );
	
	// 로컬 테이블에 공간이 있는지 확인한다.
	if( pCM->nMaxLocalEnt <= pCM->d.nTotalLocalEnt )
	{	// 공간이 없으면 확장하여야 한다.
		nR = expand_local_tbl( pCM );
		if( nR < 0 )
			return( -1 );		// 확장할 수 없으면 에러리턴!
	}		

	// 심볼을 등록한다.
	if( pSymName[ strlen( pSymName ) -1 ] == '$' )
		pSymName[ strlen( pSymName ) -1 ] = 0;	  // 마지막의 '$'는 제거한다.

	nNameIndex = register_symbol( pSymName, pCM );
	if( nNameIndex < 0 )
		return( -1 );		// 심볼을 등록할 수 없다.

	pL = &pCM->d.pLocalTbl[ pCM->d.nTotalLocalEnt ];

	// 필드를 채우고 카운터를 증가시킨다.
	pL->nNameIndex = nNameIndex;
	pL->nEBPAdder  = nValue;
	pCM->d.nTotalLocalEnt++;

	return( 0 ); 
}

static MyCoffDbg2FuncStt *find_func_by_name(CodeMap* pCM, char *pFuncName )
{
	int nStart, nMid, nEnd, nR;

	nStart = 0;
	nEnd   = pCM->d.nTotalFuncEnt-1;
	

	for( ; nStart <= nEnd; )
	{
		nMid   = (nStart + nEnd) / 2;

		nR = strcmpi( pFuncName, &pCM->d.pStrTbl[ pCM->d.pFuncTbl[ pCM->d.pFuncNameIndex[ nMid] ].nNameIndex ] );
		if( nR == 0 )
			return( &pCM->d.pFuncTbl[ pCM->d.pFuncNameIndex[nMid] ] );
		else if( nR > 0 )
			nStart = nMid+1;
		else
			nEnd = nMid-1;
	}

	return( NULL );
}

// 라인 테이블을 확장한다.
static int expand_line_tbl(CodeMap* pCM )
{
	struct _MY_IMAGE_LINENUMBER *pLT;
	int							nSize;

	nSize = sizeof(struct _MY_IMAGE_LINENUMBER) * (pCM->nMaxLineEnt + CM_DEFAULT_ENT*4);
	pLT = (struct _MY_IMAGE_LINENUMBER*)CM_MALLOC( nSize );
	if( pLT == NULL )
		return( -1 );		// 메모리를 할당할 수 없다.
	memset( pLT, 0, nSize );

	// 기존 데이터를 복사한다.
	memcpy( pLT, pCM->d.pLineTbl, sizeof(struct _MY_IMAGE_LINENUMBER) * pCM->nMaxLineEnt );
	CM_FREE( pCM->d.pLineTbl );
	pCM->d.pLineTbl = pLT;

	pCM->nMaxLineEnt += CM_DEFAULT_ENT*4;

	return( 0 );
}

// 라인번호를 추가한다.
static int register_linenum(CodeMap* pCM, int nLine, DWORD dwAddr )
{
	int							nR;
	struct _MY_IMAGE_LINENUMBER	*pL;

	if( pCM->nMaxLineEnt <= pCM->d.nTotalLineEnt )
	{
		nR = expand_line_tbl( pCM );
		if( nR < 0 )
			return( -1 );	// 라인 테이블을 확장할 수 없다.
	}

	pL = &pCM->d.pLineTbl[ pCM->d.nTotalLineEnt ];
	pL->Linenumber = (unsigned short int)nLine;
	pL->Type.VirtualAddress = dwAddr;		  

	pCM->d.nTotalLineEnt++;

	return( pCM->d.nTotalLineEnt-1 );
}


// COD 파일 내의 함수를 처리한다.
static char szWord[512], szWord2[512], szWord3[512];
static int function_in_cod_file(CodeMap* pCM, FILE *pF, MyCoffDbg2FileStt *pFile )
{
	MyCoffDbg2FuncStt	*pFunc;
	char				szT[512], szFuncName[64], *pS, *pNext;
	DWORD				dwOffset, dwFuncStartOffset, dwFuncLastOffset;
	int					nR, nType, nType2, nType3, nLine, nLineIndex, nTotalFuncLocal, nFuncLocalIndex;

	nFuncLocalIndex = -1;
	nTotalFuncLocal = 0;

	// 함수 처리하는 부분
	// MyCoffDbg2FuncStt의 nLineIndex와 dwSize만 설정하면 된다.
	// MyCoffDbg2FileStt의 nTotalLine을 설정해야 한다.
	for( ;; )
	{	// 한 라인을 읽어들인다.
		pNext = pS = fgets( szT, sizeof( szT )-1, pF );
		if( pS == NULL )
			goto QUIT;

		// 3개의 키워드를 읽는다.
		pNext = get_next_word( szWord,  pNext, &nType  );
		pNext = get_next_word( szWord2, pNext, &nType2 );
		pNext = get_next_word( szWord3, pNext, &nType3 );
		
		// 함수의 시작인가?
		//if( nType2 == KWT_PROC && nType3 == KWT_NEAR )
		if (nType2 == KWT_PROC)
			goto GET_LINE;

		// 지역변수와 파러메터를 나타내는 패턴인가?
		if( nType2 != KWT_ASSIGN )
			continue;	

		// 첫 번째 키워드가 $로 끝나는가?
		nR = strlen( szWord );
		if( nR <= 1 || szWord[nR-1] != '$' )
			continue;

		// parameter와 local variable을 등록한다.
		nR = register_param_and_local_var( pCM, szWord, szWord3 );
		if( nR == 0 )
		{
			if( nFuncLocalIndex < 0 )
			{	// 해당 함수의 시작 Local Index
				nFuncLocalIndex = pCM->d.nTotalLocalEnt-1;
			}
			// 해당 함수의 Local Entry(Local Var, Param) 개수
			nTotalFuncLocal++;
		}
		continue;
			
GET_LINE:		
		dwFuncStartOffset = dwFuncLastOffset = 0xFFFFFFFF;
		
		// szWord에 함수 이름이 저장되어 있다.
		strcpy( szFuncName, szWord );
		//CM_PRINTF( "FUNCTION : %s\n", szFuncName );

		// MAP 파일에서 해당 함수를 찾는다.
		pFunc = find_func_by_name( pCM, szFuncName );
		if( pFunc == NULL )
		{	// COD 파일에는 있지만 MAP 파일에는 등장하지 않은(LINK되지 않은) 함수
			//CM_PRINTF( "Function <%s> is not exist in MAP file.\n", szFuncName );
			pCM->nTotalUselessFunction++;
			// ENDP까지 스킵한다.
			for( ;; )
			{	// 한 라인을 읽는다.
				pNext = pS = fgets( szT, sizeof( szT )-1, pF );
				if( pS == NULL )
					goto QUIT;
				// 단어 2개를 읽어들인다.
				pNext = get_next_word( szWord, pNext, &nType );
				pNext = get_next_word( szWord2, pNext, &nType2 );
				if( nType2 == KWT_ENDP )
					break;
			}
			continue;
		}

		// 처리된 파러메터나 지역변수가 있으면 설정한다.
		pFunc->nLocalIndex = nFuncLocalIndex;
		pFunc->nTotalLocal = nTotalFuncLocal;
		nTotalFuncLocal = 0;
		nFuncLocalIndex = -1;

		// 라인번호를 처리한다.
		for( nLine = 0;; )
		{	// 한 라인을 읽는다.
			pNext = pS = fgets( szT, sizeof( szT )-1, pF );
			if( pS == NULL )
				goto QUIT;

			// 단어 2개를 읽어들인다.
			pNext = get_next_word( szWord, pNext, &nType );
			pNext = get_next_word( szWord2, pNext, &nType2 );
			if( nType == KWT_SEMI_COLON )
			{	// 세미콜론으로 시작하면 라인 번호.
				nLine = atoi( szWord2 );
				continue;
			}
			else if( nType == KWT_NUMBER )
			{	// 세미콜론이 아니면 파일 내에서의 오프셋
				if( nLine != 0 )  
				{	// 라인에 해당하는 오프셋을 설정한다.
					dwOffset = dwHexValue( szWord );
					if( dwFuncStartOffset == 0xFFFFFFFF )
						dwFuncStartOffset = dwOffset;

					// .. 라인번호와 오프셋을 추가한다.
					//CM_PRINTF( "LINENUM : %d, Offset : %05X\n", nLine, dwOffset );
								 
					// 라인 번호를 추가한다.
					nLineIndex = register_linenum( pCM, nLine, dwOffset );
					
					// 함수의 시작 라인번호 인덱스를 설정한다.
					if( pFunc != NULL && pFunc->nLineIndex == 0 )
						pFunc->nLineIndex = nLineIndex;
					
					// 파일의 라인번호 개수를 증가시킨다.
					pFile->nTotalLine++;
					nLine = 0;
					dwFuncLastOffset = dwOffset;
					continue;	// 다음 라인번호를 찾는다.
				}
				else
				{	// 함수의 마지막 오프셋을 구해야 한다.
					dwFuncLastOffset = dwHexValue( szWord );
					continue;
				}	
			}
			else if( nType2 == KWT_ENDP )
			{	// 함수의 크기와 파일의 크기를 설정한다. 				
				pFunc->dwSize = dwFuncLastOffset - dwFuncStartOffset;
				pFile->dwSize = dwFuncLastOffset;
				//CM_PRINTF( "END : %s\n\n", szFuncName );
				
				// 하나의 함수에 대한 처리가 끝났다.
				break;		
			}
			
		}// 라인번호 구하는 for 문
	}// 함수 처리하는 for 문

QUIT:
	// 더이상 처리할 것이 없다.
	return( -1 );
}


// 하나의 COD 파일을 처리한다.
static int cod_file( char *pFileName, CodeMap* pCM, MyCoffDbg2FileStt *pFile )
{
	int		nR;
	FILE	*pF;

	// MAP 파일을 오픈한다.
	pF = fopen( pFileName, "rt" );
	if( pF == NULL )
	{	// 파일을 오픈할 수 없다.
		CM_PRINTF( "SKIP : %s (Open failed.)\n", pFileName );
		return( -1 );
	}

	//CM_PRINTF( "COD_FILE : %s\n", pFileName );

	// COD 파일 내에 포함되어 있는 함수들을 처리한다.
	for( ;; )
	{
		nR = function_in_cod_file( pCM, pF, pFile ); 
		if( nR < 0 )
			break;
	}

	// 파일을 닫고 돌아간다.
	fclose( pF );
	// 처리된 파일의 개수를 증가시킨다.
	pCM->nTotalProcessedFiles++;
	
	//CM_PRINTF( "Total %d linenumbers.\n", pCM->d.nTotalLineEnt );

	return( nR );
}

// Working 패스로부터 pCM속에 포함된 COD 파일을 찾아 처리한다.
static int cod_info( char *pWorkPath, CodeMap* pCM )
{
	MyCoffDbg2FileStt	*pFile;
	char				szT[260];
	int					nI, nX, nR;
	
	for( nI = 0; nI < pCM->d.nTotalFileEnt; nI++ )
	{
		pFile = &pCM->d.pFileTbl[nI];
		// COD 파일 이름을 조합한다.
		sprintf( szT, "%s%s", pWorkPath, &pCM->d.pStrTbl[pFile->nNameIndex] );
		nX = strlen( szT );
		strcpy( &szT[nX-3], "COD" );

		// COD 파일을 처리한다.
		nR = cod_file( szT, pCM, pFile );
	}

	return( 0 );
}

// 풀 패스에 등록된 마지막 파일명을 잘라낸다.
static int cut_last_filename( char *pS )
{
	int nI;

	for( nI = strlen( pS )-1; nI > 0; nI-- )
	{
		if( pS[nI] == '\\' )
		{
			pS[nI+1] = 0;
			return( 0 );
		}
	}					

	return( -1 );
}

// 함수 테이블로부터 (주소, 함수명)에 의한 인덱스를 생성한다.
static int make_func_index(CodeMap* pCM )
{
	int					nI, nJ, nMinAddr, nMinName;

	// 두 개의 인덱스를 할당한다.
	pCM->d.pFuncNameIndex = (int*)CM_MALLOC( sizeof(int) * pCM->d.nTotalFuncEnt );
	pCM->d.pFuncAddrIndex = (int*)CM_MALLOC( sizeof(int) * pCM->d.nTotalFuncEnt );
	if( !pCM->d.pFuncAddrIndex || !pCM->d.pFuncNameIndex )
	{
		CM_PRINTF( "make_func_index() - memory allocation error!\n" );
		return( -1 );
	}

	for( nI = 0; nI < pCM->d.nTotalFuncEnt; nI++ )
	{
		pCM->d.pFuncAddrIndex[nI] = nI;
		pCM->d.pFuncNameIndex[nI] = nI;
	}

	for( nI = 0; nI < pCM->d.nTotalFuncEnt; nI++ )
	{
		for( nMinAddr = nMinName = nJ = nI; nJ < pCM->d.nTotalFuncEnt; nJ++ )
		{
			if( pCM->d.pFuncTbl[pCM->d.pFuncAddrIndex[nJ]].dwAddr < pCM->d.pFuncTbl[pCM->d.pFuncAddrIndex[nMinAddr]].dwAddr )
				nMinAddr = nJ;
			if( strcmpi( &pCM->d.pStrTbl[pCM->d.pFuncTbl[pCM->d.pFuncNameIndex[nJ]].nNameIndex], 
				&pCM->d.pStrTbl[pCM->d.pFuncTbl[pCM->d.pFuncNameIndex[nMinName]].nNameIndex] ) < 0 )
				nMinName = nJ;
		}	

		nJ = pCM->d.pFuncNameIndex[nMinName];
		pCM->d.pFuncNameIndex[nMinName] = pCM->d.pFuncNameIndex[nI]; 
		pCM->d.pFuncNameIndex[nI] = nJ;

		nJ = pCM->d.pFuncAddrIndex[nMinAddr];
		pCM->d.pFuncAddrIndex[nMinAddr] = pCM->d.pFuncAddrIndex[nI];
		pCM->d.pFuncAddrIndex[nI] = nJ;
	}	

	// 이름에 의해 정렬된 함수명을 출력한다.
//	{
//		MyCoffDbg2FuncStt	*pFunc;
//		for( nI = 0; nI < pCM->d.nTotalFuncEnt; nI++ )
//		{
//			// 이름에 따라 출력
//			//pFunc = &pCM->d.pFuncTbl[ pCM->d.pFuncNameIndex[nI] ];
//			// 주소에 따라 출력
//			pFunc = &pCM->d.pFuncTbl[ pCM->d.pFuncAddrIndex[nI] ];
//			CM_PRINTF( "[%3d] 0x%08X  %s\n", nI, pFunc->dwAddr, &pCM->d.pStrTbl[ pFunc->nNameIndex ] );
//		}
//	}

	return( 0 );
}

// 디버그 정보를 파일에 기록한다.
static int write_debug_info(CodeMap* pCM, char *pDbgFile, int nCreateNew )
{
	MY_IMAGE_OPTIONAL_HEADER	iohd;
	MY_IMAGE_DOS_HEADER			doshd;
	int							nHandle;
	long						lDbgLoc, lOffs;

	lDbgLoc = 0;

	nCreateNew = 1;
	if( nCreateNew == 0 )
	{	// 기존 파일을 연다.
		nHandle = open( pDbgFile, _O_BINARY | _O_RDWR );
		if( nHandle < 0 )
			goto CN;		// 기존 파일이 없으면 새로 생성한다.
		// 파일 포인터를 뒤로 옮긴다.
		lDbgLoc = lseek( nHandle, 0, SEEK_END );
	}
	else
	{	// 파일을 지우고 새로 생성한다.
		remove( pDbgFile );
CN:
		
		nHandle = open( pDbgFile, _O_BINARY | _O_RDWR | _O_CREAT, _S_IREAD | _S_IWRITE );
		if( nHandle == -1 )
			return( -1 );
	}

	write( nHandle, &pCM->d, sizeof( MyCoffDbg2Stt ) );
	write( nHandle, pCM->d.pFileTbl, sizeof( MyCoffDbg2FileStt ) * pCM->d.nTotalFileEnt );
	write( nHandle, pCM->d.pFuncTbl, sizeof( MyCoffDbg2FuncStt ) * pCM->d.nTotalFuncEnt );
	write( nHandle, pCM->d.pFuncNameIndex, sizeof( int ) * pCM->d.nTotalFuncEnt );
	write( nHandle, pCM->d.pFuncAddrIndex, sizeof( int ) * pCM->d.nTotalFuncEnt );
	write( nHandle, pCM->d.pLineTbl,  sizeof(MY_IMAGE_LINENUMBER ) * pCM->d.nTotalLineEnt );
	write( nHandle, pCM->d.pLocalTbl, sizeof( MyCoffDbg2LocalStt ) * pCM->d.nTotalLocalEnt );
	write( nHandle, pCM->d.pStrTbl, pCM->d.nStrTblSize );

	// 파일을 새로 생성한 경우에는 파일을 닫고 돌아간다.
	if( nCreateNew != 0 )
	{	
		close( nHandle );
		return( 0 );
	}

	// 2002-05-12
	// Image Optional Header의 dd_Debug_dwVAddr, dd_Debug_dwSize를 갱신해야 한다.
	lseek( nHandle, 0, SEEK_SET );
	// 도스 헤더를 읽는다.
	read( nHandle, &doshd, sizeof( doshd ) );
	
	// Image Optional Header를 읽는다.
	lOffs = doshd.e_lfanew + sizeof( MY_IMAGE_FILE_HEADER );
	lseek( nHandle, lOffs, SEEK_SET );
	read( nHandle, &iohd, sizeof( iohd ) );

	// Virtual Address가 아니고 단순히 파일 선두 부터의 오프셋이다.
	iohd.dd_Debug_dwVAddr = lDbgLoc;
	iohd.dd_Debug_dwSize  = pCM->d.dwSize;

	// Image Optional Header를 기록한다.
	lseek( nHandle, lOffs, SEEK_SET );
	write( nHandle, &iohd, sizeof( iohd ) );

	// 파일을 닫고 돌아간다.
	close( nHandle );

	printf( "dd_Debug_dwVAddr/dwSize = (%d/%d)\n", lDbgLoc, pCM->d.dwSize );

	return( 0 );
}
 
static CodeMap	cm;
int MakeCodeMap( char *pMapFile, char *pDbgFile )
{
	char		szT[260];
	int			nR, nCreateNew;

	// CodeMap을 초기화한다.
	nR = init_codemap( &cm );
	if( nR < 0 )
	{
		CM_PRINTF( "memory allocation error!\n" );
		return( -1 );
	}
	
	// MAP 파일을 처리한다.
	nR = map_info( pMapFile, &cm );
	if( nR < 0 )
		return( -1 );

	// 함수의 이름과 주소에 따라 정렬을 한다.
	make_func_index( &cm );

	// COD 파일을 처리한다.
	strcpy( szT, pMapFile );
	make_pure_path( szT );
	nR = cod_info( szT, &cm );
	if( nR , 0 )
		return( -1 );

	// 디버그 파일의 크기를 계산한다.
	cm.d.dwSize  = sizeof( MyCoffDbg2Stt );
	cm.d.dwSize += sizeof( MyCoffDbg2FileStt  ) * cm.d.nTotalFileEnt;
	cm.d.dwSize += sizeof( MyCoffDbg2FuncStt  ) * cm.d.nTotalFuncEnt;
	cm.d.dwSize += sizeof( MyCoffDbg2LocalStt ) * cm.d.nTotalLocalEnt;
	cm.d.dwSize += sizeof( int ) * cm.d.nTotalFuncEnt * 2;
	cm.d.dwSize += sizeof( struct _MY_IMAGE_LINENUMBER ) * cm.d.nTotalLineEnt;
	cm.d.dwSize += cm.d.nStrTblSize;
	strcpy( cm.d.szMagicStr, MY_COFF_DBG2_MAGIC_STR );

	// 디버그 파일을 생성한다.
	if( pDbgFile == NULL )
	{
		strcpy( szT, pMapFile );
		nR = strlen( szT );
		strcpy( &szT[nR-3], "DBG" );
		pDbgFile = szT;
		nCreateNew = 1;
	}	
	else
		nCreateNew = 0;
	write_debug_info( &cm, pDbgFile, nCreateNew );
	   
	// 처리 결과를 출격한다.
	CM_PRINTF( "HASH  TABLE  : %d Collisions, %d Max Collisions\n", cm.nTotalCollision, cm.nMaxCollision );
	CM_PRINTF( "FILE  TABLE  : %d Processed, Total %d COD files.\n", cm.nTotalProcessedFiles, cm.d.nTotalFileEnt );
	CM_PRINTF( "FUNC  TABLE  : %d functions, %d unlinked functtions.\n", cm.d.nTotalFuncEnt, cm.nTotalUselessFunction );
	CM_PRINTF( "LOCAL TABLE  : %d local entries.\n", cm.d.nTotalLocalEnt );
	CM_PRINTF( "LINE  TABLE  : %d line numbers.\n", cm.d.nTotalLineEnt );
	CM_PRINTF( "SYMB  TABLE  : %d symbols, ( Size = %d )\n", cm.nTotalSymIndex, cm.d.nStrTblSize );
	CM_PRINTF( "DEBUG INFO  : %d bytes.\n", cm.d.dwSize );
	CM_PRINTF( "LINKER BASE : 0x%08X\n",  cm.dwLinkerBaseAddr );

	// 프로그램 디버깅 용
	/*
	{
		int nI;
		MyCoffDbg2FileStt	*pFile;
		MyCoffDbg2FuncStt	*pFunc;
		CodeMap			*pCM;
		pCM = &cm;		
		for( nI = 0; nI < pCM->d.nTotalFileEnt; nI++ )
		{
			pFile = &pCM->d.pFileTbl[nI];
			CM_PRINTF( "[%2d] 0x%08X (%d bytes, %d lines ) %s\n", nI, pFile->dwAddr, pFile->dwSize, pFile->nTotalLine, &pCM->d.pStrTbl[ pFile->nNameIndex ] );
		}
		CM_PRINTF( "\n" );
		for( nI = 0; nI < pCM->d.nTotalFuncEnt; nI++ )
		{
			pFunc = &pCM->d.pFuncTbl[ pCM->d.pFuncAddrIndex[nI]];
			pFile = &pCM->d.pFileTbl[ pFunc->nFileIndex ];
			CM_PRINTF( "[%2d] 0x%08X (%d bytes) %s/%s\n", nI, pFunc->dwAddr, pFunc->dwSize, &pCM->d.pStrTbl[ pFile->nNameIndex ], &pCM->d.pStrTbl[ pFunc->nNameIndex ] );
		}
	}*/
	

  return( 0 );
}

