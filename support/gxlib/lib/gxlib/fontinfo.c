#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fontinfo.h"

const unsigned int con[]={4,1,2,3,};

static int readfontofcode( bdfFontCache* pCache, FILE* file ,unsigned int nCode , unsigned int lowPos , unsigned int highPos , pbdfFont pRetFont );

void FontCacheInit( bdfFontCache* pCache )
{
	int i = 0;

	pCache->nIndex = 0;
	pCache->nMaxIndex = 0;
	for( i =0 ; i < NFONTCACHE ; ++i )
	{
		pCache->pFontItem[i] = NULL;
		pCache->iSortIndex[i] = i;
	}
}

void FontCacheDeInit( bdfFontCache* pCache )
{
	int i; 

	for( i =0 ; i < pCache->nMaxIndex ; ++i )
	{       
		free( pCache->pFontItem[i] );
	}
}

int ReadFileLine( FILE* file ,char* buf ,int nbufMax )
{
	int i = 0;
	int ch;
	ch = fgetc( file );

	for( i=0 ; (i < nbufMax ) && ch != '\n'; i++ )
	{
      	  buf[i] = (char)ch;
	  ch = fgetc( file );
	  if( ch == EOF )
	  {
		  if( i < 1 ) return 0;
	  }
	}


	if( buf[i-1] == '\r' && ch ==  '\n' )
	{
		buf[i-1] = 0x00;
		return i;
	}
	if( ch == '\n' )
	{
		buf[i] = 0x00;
		return i+1;
	}

	buf[i] = 0x00;
	return i;
}

void FontChanged( FILE *file, bdfFontCache* pCache, bdfFontIndex *fndex)
{
	int n, i;
	int nIncrease;

	fseek(file, 0 ,SEEK_END );
	n = ftell(file);
	nIncrease = n/GX_FONT_INDEX_MAX;

	fndex[0].nCode = 0;
	fndex[0].nPos = 0;

	for( i = 1 ; i < GX_FONT_INDEX_MAX-1 ; ++ i )
	{
		unsigned int nPos = i*nIncrease;

		fndex[i].nCode = TellMeNextNearestFont( pCache , file, &nPos );
		fndex[i].nPos = nPos;
	}

	fndex[GX_FONT_INDEX_MAX-1].nCode = 0xFFFF;
	fndex[GX_FONT_INDEX_MAX-1].nPos = n;
}                

int ReadFontOfCode( bdfFontCache* pCache, FILE* file, bdfFontIndex *fndex, unsigned int nCode ,pbdfFont pRetFont )
{
	int i;

	for( i = 0 ; i < pCache->nMaxIndex ; ++i )
	{
		if( pCache->pFontItem[i]->code == nCode )
		{
			int m = 0;
			int nIndexForSwap = i;
			pbdfFont pFont = pCache->pFontItem[i];
			pFont->nUsage++;
			memcpy( pRetFont, pFont , sizeof(bdfFont));

			for( m = i; m >= 0 ; --m )
			{
				if( pCache->pFontItem[m]->nUsage <= pFont->nUsage )
				{
					nIndexForSwap = m;
				}
			}

			if( nIndexForSwap < i  )
			{
				int k = 0;
				pbdfFont qFont;
				qFont = pCache->pFontItem[ i ];
				for( k = i -1 ; k >= nIndexForSwap ; --k )
				{
					pCache->pFontItem[ k+1 ] = pCache->pFontItem[k] ;
				}
				pCache->pFontItem[ nIndexForSwap ] = qFont;
			}

			return TRUE;
		}
	}

	for ( i = 0 ; i < GX_FONT_INDEX_MAX-1 ; ++i )
	{
		if( fndex[i].nCode <= nCode && fndex[i+1].nCode >= nCode )
			break;
	}


	return readfontofcode( pCache, file, nCode, fndex[i].nPos , fndex[i+1].nPos ,pRetFont );
}

static int readfontofcode( bdfFontCache* pCache, FILE* file ,unsigned int nCode , unsigned int lowPos , unsigned int highPos , pbdfFont pRetFont )
{
	unsigned int midPos;
	unsigned int midSave = midPos = ( lowPos + highPos )/2;
	unsigned int nFound;

	nFound = TellMeNextNearestFont( pCache, file,  &midPos );
	if( nFound != 0 )
	{
		if( midPos == midSave ||  midPos == highPos ) {
			for( ;; )
			{
				lowPos += 3;
				nFound = TellMeNextNearestFont( pCache, file,  &lowPos );
				if( nFound == nCode )
				{
					unsigned int nRet;
					return ReadFont( pCache, file, lowPos , &nRet , pRetFont );
				}
			}
		}

		if( nFound == nCode )
		{
			unsigned int nRet;
			return ReadFont( pCache, file, midPos , &nRet , pRetFont );
		}
		else if( nFound < nCode )
			return readfontofcode( pCache, file, nCode, midPos , highPos ,pRetFont );
		else
			return readfontofcode( pCache, file, nCode, lowPos , midPos ,pRetFont );
	}

	return FALSE;
}

unsigned int TellMeNextNearestFont( bdfFontCache* pCache, FILE* file , unsigned int *pnPos )
{
	int n;
	char buf[256];
	int nCount=0;
	int h;

	pCache = pCache;

	fseek(file, *pnPos, SEEK_SET );
	while( 0 != ( h = ReadFileLine(file , buf, 256 )))
	{
		if( strncmp( "STARTCHAR" , buf ,9 ) == 0 )
		{
			*pnPos += nCount;
			break;
		}
		nCount+= h;
	}

	while( 0 != ( h = ReadFileLine( file, buf, 256 ) ) )
	{
		char temp[256];

		if( strncmp("ENCODING" ,buf,8 ) == 0 )
		{
			sscanf( buf , "%s%d" , temp ,&n );
			return n;
		}


	}

	return 0;

}
int  ReadFont( bdfFontCache* pCache, FILE* file , unsigned int nPos ,unsigned int* nRet , pbdfFont pRetFont )
{
	char     buf[256];
	pbdfFont pFont;
	int      iIndexFont;
	int      nCount=0;
	int      h = 0;
	int      i;
	char     mm;
	int      t_cnt;


	fseek(file, nPos, SEEK_SET );
	while( 0 != ( h = ReadFileLine(file ,buf ,256)))
	{
		nCount+= h;
		if( strncmp("STARTCHAR" , buf,9 ) == 0 )
			break;
	}
	if( pCache->nMaxIndex < NFONTCACHE )
	{
		iIndexFont = pCache->nMaxIndex;
		pFont = pCache->pFontItem[pCache->nMaxIndex] = (pbdfFont) malloc( sizeof(bdfFont) );
		++(pCache->nMaxIndex);
		memset( pFont, 0x00, sizeof(bdfFont));
	}
	else
	{
		int k = iIndexFont = NFONTCACHE-1;
		pFont = pCache->pFontItem[k];
		pFont->datalen = 0;
		memset( pFont, 0x00, sizeof(bdfFont));
	}

	while( 0 != (h = ReadFileLine(file ,buf ,256 )) )
	{
		char temp[256];

		nCount+= h;
		if( strncmp("ENCODING" ,buf,8 ) == 0 )
		{
			int a0;
			sscanf( buf , "%s%d" , temp ,&(a0) );
			pFont->code = a0;

			if( pFont->code == 106 )
				pFont->code = 106;
		}
		if( strncmp( "DWIDTH" , buf, 6 ) == 0 )
		{
			int a0;
			sscanf( buf , "%s %d" , temp ,&(a0) );
			pFont->dwidth = a0;
		}
		if( strncmp( "BBX" ,buf, 3 ) == 0 )
		{
			int a0,a1,a2,a3;

			sscanf( buf , "%s %d %d %d %d" , temp ,&(a0),&(a1),&(a2),&(a3) );
			pFont->bbw = a0;
			pFont->bbh = a1;
			pFont->bbxoff = a2;
			pFont->bbyoff = a3;
			pFont->datalen = (pFont->bbw + 7 )/8 * pFont->bbh;
		}

		if( strncmp( "VVECTOR" , buf ,7 ) == 0 )
		{
			int a0, a1;
			sscanf( buf , "%s %d %d" , temp ,&(a0),&(a1));
			pFont->vvectorx = a0;
			pFont->vvectory = a1;
		}




		if( strncmp( "BITMAP" , buf ,6 ) == 0 )
		{
			int k =0;
			while( 0 != (h = ReadFileLine(file, buf ,256 )) )
			{
				char buf3[256] = "0x";
				int oct,oct1;
				int nbyte = ( pFont->bbw + 7 )/8;


				t_cnt = nbyte %4;
				t_cnt = con[t_cnt];

				nCount += h;
				if( strncmp("ENDCHAR" , buf, 7 ) == 0 )
					break;

				strcpy( &buf3[2] ,buf );


				for(i=0; i<20; i++){
				}


				sscanf( &buf3[2], "%8x %8x", &oct,&oct1);

				if(nbyte>4){
					for(i=0; i<	4; i++){
						mm = ( oct >> ( ( 3*8)-( i*8))) & 0xFF;
						pFont->data[k] = mm;
						k++;
					}
					for(i=0; i<	t_cnt; i++){

						mm = ( oct1 >> (((t_cnt-1)*8)-(i*8))) & 0xFF;
						pFont->data[k] = mm;
						k++;
					}

				}else{

					for(i=0; i<	nbyte; i++){

						mm = ( oct >> ( ((nbyte-1)*8)-(i*8))) & 0xFF;
						pFont->data[k] = mm;
						k++;

					}
				}
			}
		}

		if( strncmp( "ENDCHAR" , buf ,7 ) ==0 )
		{
			int m = 0;
			int nIndexForSwap = pCache->nMaxIndex;

			pFont->nUsage++;

			for( m = iIndexFont; m >= 0 ; --m )
			{
				if( pCache->pFontItem[m]->nUsage <= pFont->nUsage )
				{
					nIndexForSwap = m;
				}
			}


			if( nIndexForSwap < iIndexFont  )
			{
				int k = 0;
				pbdfFont qFont;
				qFont = pCache->pFontItem[ iIndexFont ];
				for( k = iIndexFont -1 ; k >= nIndexForSwap ; --k )
				{
					pCache->pFontItem[ k+1 ] = pCache->pFontItem[k] ;
				}
				pCache->pFontItem[nIndexForSwap ] = qFont;
			}

			memcpy( pRetFont, pFont , sizeof(bdfFont));

			*nRet = nPos + nCount;
			return TRUE;
		}

	}

	*nRet = nPos;
	return FALSE;

}
