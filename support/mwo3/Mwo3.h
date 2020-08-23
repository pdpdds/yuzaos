//	By kid
//	2008.10.2

#pragma once

#include <minwindef.h>

inline int getnum(const UCHAR*& src, int n = 0);
void uncompress(const UCHAR* src, const UCHAR* const srcend, UCHAR* dst, const UCHAR* const dstend);
void compress(const UCHAR* src, ULONG srcSize, UCHAR* dst, ULONG& dstsize, int level);
