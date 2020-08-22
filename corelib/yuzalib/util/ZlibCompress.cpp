#include "ZlibCompress.h"
#include "memory.h"
#include "zlib.h"

extern "C" void printf(const char* str, ...);

extern "C" __declspec(dllexport) I_Compress* GetZlibCompress()
{
	return new ZlibCompress();
}

ZlibCompress::ZlibCompress()
{
}


ZlibCompress::~ZlibCompress()
{
}

int ZlibCompress::Compress(unsigned char* pDest, long* pnDestLen, unsigned char* pSrc, long nSrcLen)
{
	if (pDest == nullptr)
		return 0;

	return compress(pDest, (uLongf*)pnDestLen, pSrc, nSrcLen);		
}

int ZlibCompress::Decompress(unsigned char* pDest, long* pnDestLen, unsigned char* pSrc, long nSrcLen)
{
	if (pDest == nullptr)
		return 0;
	
	return uncompress(pDest, (uLongf*)pnDestLen, pSrc, nSrcLen);
}

