#pragma once

class SFCompressZLib
{
public:
	SFCompressZLib(void);
	virtual ~SFCompressZLib(void);

	bool Compress(char* pDest, int& destLen, unsigned char* pSrc, int srcLen);
	bool Uncompress(unsigned char* pDest, int& destLen, unsigned char* pSrc, int srcLen);

protected:

private:
};
