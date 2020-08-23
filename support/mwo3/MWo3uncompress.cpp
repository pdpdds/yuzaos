/*
	SRWZ Compresser

	Author:		kid, K_I_D@126.com
	Function:	MWo3 uncompress 

	History:
		2008.10.2	first version
*/

#include "MWo3.h"

inline int getnum(const UCHAR*& src, int n)
{
	int num = n;
	do {
		num = (num << 7) | *src++;
	}while ((num & 1) == 0);

	return num >> 1;
}

void uncompress(const UCHAR* src, const UCHAR* const srcend, UCHAR* dst, const UCHAR* const dstend)
{
	const UCHAR* const dstbegin = dst;
	const UCHAR* const srcbegin = src;
	ULONG maxpos = 0;
	int maxnum = 0;

	while (src < srcend && dst < dstend)
	{
		UCHAR flag1 = *src++;
		int f1 = flag1 & 0xf;		//direct num 
		int f2 = flag1 >> 4;		//flag num
		if (!f1) f1 = getnum(src);
		if (!f2) f2 = getnum(src);

		//printf("d %d w %d\n", f1, f2);
		for (int i = 0; i < f1; i++)
			*dst++ = *src++;

		if (dst >= dstend) break;
		
		while (f2--)
		{
			flag1 = *src++;
			int pos = flag1 & 0xf;		//window pos
			if ((pos & 1) == 0) {pos = getnum(src, pos);}
			else pos >>= 1;

			int num = flag1 >> 4;		//window num
			if (!num) {num = getnum(src, num);}

			UCHAR* p = dst - pos - 1;
			
			//{printf("[%x %x]\n", /*src - srcbegin, dst - dstbegin, */pos + 1, num + 1);}
			//if (pos + 1 > maxpos) {maxpos = pos + 1;maxnum = num + 1;}
			/*if (src - srcbegin == 0x1529) 
				WriteToFile("temp", (UCHAR*)dstbegin, dst - dstbegin, 0, "wb+");*/
			for (int i = 0; i < num + 1; i++)
				*dst++ = *p++;
		}
	}
	//printf("maxpos=%x %x\n",maxpos, maxnum);
}