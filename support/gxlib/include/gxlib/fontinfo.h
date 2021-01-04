#ifndef __FONT_INFO_H__
#define __FONT_INFO_H__


#define NFONTCACHE  1000
#define ___FONT_INFO_S___

#define TRUE        1
#define FALSE       0

#define GX_FONT_INDEX_MAX 256

typedef struct __bdfFont
{                   
	unsigned char   data[1024];
    char            datalen;
    char            dwidth;
	unsigned int    code;
	unsigned int    unicode;
    char            bbw;
    char            bbh;
    unsigned int    bbxoff;
    unsigned int    bbyoff;
    char            vvectorx;
    char            vvectory;
    int             nUsage;
} bdfFont;

typedef bdfFont *pbdfFont;

typedef struct __bdfFontCache
{
	bdfFont*	pFontItem[NFONTCACHE];
	int			nIndex;
	int			nMaxIndex;
	int			iSortIndex[NFONTCACHE];
} bdfFontCache;

typedef struct __bdfFontIndex
{
	unsigned int	nCode;
	int nPos;
} bdfFontIndex;

extern void FontCacheInit( bdfFontCache* pCache );
extern void FontCacheDeInit( bdfFontCache* pCache );
extern int ReadFileLine( FILE* file ,char* buf ,int nbufMax );
extern int ReadFontOfCode( bdfFontCache* pCache, FILE* file, bdfFontIndex *fndex, unsigned int nCode  ,pbdfFont pRetFont );
extern unsigned int TellMeNextNearestFont( bdfFontCache* pCache, FILE* file , unsigned int *pnPos );
extern int  ReadFont( bdfFontCache* pCache, FILE* file , unsigned int nPos ,unsigned int* nRet , pbdfFont pRetFont );
extern void FontChanged( FILE *file, bdfFontCache* pCache, bdfFontIndex *fndex);

#endif
