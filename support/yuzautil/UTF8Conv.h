/**
* Copyright (C) 2008 by Kyung-jin Kim
* Homepage		: http://www.kilojuliet.com
* e-mail		: kilojuliet@paran.com
*
*
* Description	: UTF-8 String Conversion Macros
* Created		: Jun-13,2008
* Version		: UTF8Conv V1.1
*/

/**
* The names of the UTF-8 string conversion macros take the form.
* SourceType2DestinationType[EX] ex) W2UTF8, UTF82A_EX
*
* SourceType and DestinationType are described below.
* A (ANSI character string.)
* W (Unicode character string.)
* T (Generic character string.)
* UTF8 (UTF-8 encoded character string.)
*
* When using an UTF-8 conversion macro, specify the UTF8_CONVERSION[EX] macro 
* at the beginning of your function in order to avoid compiler errors.
*
* Sample code
* void SampleFunc(LPCTSTR lpsz)
* {
*	UTF8_CONVERSION;
*	LPSTR utf8 = T2UTF8(lpsz);
*
*	// Do something with utf8
*	...
* }
*/

#pragma once
#include <malloc.h>
#include <vector>

template <typename Ty>
class _SafeAllocator
{
public:
	_SafeAllocator() {}
	~_SafeAllocator() 
	{ 
		int nCount = static_cast<int>(_parray.size());
		for (int i=0;i<nCount;i++)
		{
			if (_parray[i] != NULL)
				delete [] _parray[i];
		}
	}

	int Alloc(int nSize)
	{		
		Ty* ptr;
		ptr = new(nothrow) Ty[nSize];
		if (!ptr)
			return -1;

		_parray.push_back(ptr);
		return static_cast<int>(_parray.size() - 1);
	}

	void Free(int nIndex)
	{
		delete _parray[nIndex];
		_parray[nIndex] = NULL;
	}

	Ty* GetPtr(int nIndex) const 
	{			
		return _parray[nIndex]; 
	}

private:	
	std::vector<Ty*> _parray;
};

inline LPSTR WINAPI W2UTF8Helper(LPSTR lpu, LPCWSTR lpw, int nChars)
{
	ASSERT(lpu != NULL);
	ASSERT(lpw != NULL);
	if (lpu == NULL || lpw == NULL)
		return NULL;

	lpu[0] = '\0';
	int ret = WideCharToMultiByte(CP_UTF8, 0, lpw, -1, lpu, nChars, 0, 0);
	if (ret == 0)
	{
		ASSERT(FALSE);
		return NULL;
	}
	return lpu;
}

inline LPWSTR WINAPI UTF82WHelper(LPWSTR lpw, LPCSTR lpu, int nChars)
{
	ASSERT(lpw != NULL);
	ASSERT(lpu != NULL);
	if (lpw == NULL || lpu == NULL)
		return NULL;

	lpw[0] = '\0';
	int ret = MultiByteToWideChar(CP_UTF8, 0, lpu, -1, lpw, nChars);
	if (ret == 0)
	{
		ASSERT(FALSE);
		return NULL;
	}
	return lpw;
}

inline LPWSTR WINAPI A2WHelper(LPWSTR lpw, LPCSTR lpa, int nChars)
{
	ASSERT(lpw != NULL);
	ASSERT(lpa != NULL);
	if (lpw == NULL || lpa == NULL)
		return NULL;

	lpw[0] = '\0';
	int ret = MultiByteToWideChar(CP_ACP, 0, lpa, -1, lpw, nChars);
	if (ret == 0)
	{
		ASSERT(FALSE);
		return NULL;
	}
	return lpw;
}

inline LPSTR WINAPI W2AHelper(LPSTR lpa, LPCWSTR lpw, int nChars)
{
	ASSERT(lpa != NULL);
	ASSERT(lpw != NULL);
	if (lpa == NULL || lpw == NULL)
		return NULL;

	lpa[0] = '\0';
	int ret = WideCharToMultiByte(CP_ACP, 0, lpw, -1, lpa, nChars, 0, 0);
	if (ret == 0)
	{
		ASSERT(FALSE);
		return NULL;
	}
	return lpa;
}

#define UTF8_CONVERSION	int _conv_uc; (_conv_uc); LPCWSTR _lpw_uc; (_lpw_uc); LPCSTR _lpa_uc; (_lpa_uc); LPCSTR _lpu_uc; (_lpu_uc);

#define A2UTF8(lpa) (\
					((_lpa_uc = lpa) == NULL) ? NULL : (\
					_conv_uc = MultiByteToWideChar(CP_ACP, 0, _lpa_uc, -1, NULL, 0) * sizeof(WCHAR),\
					_lpw_uc = A2WHelper((LPWSTR)_alloca(_conv_uc), _lpa_uc, _conv_uc),\
					_conv_uc = WideCharToMultiByte(CP_UTF8, 0, _lpw_uc, -1, NULL, 0, 0, 0) * sizeof(CHAR),\
					W2UTF8Helper((LPSTR)_alloca(_conv_uc), _lpw_uc, _conv_uc)))

#define W2UTF8(lpw)	(\
					((_lpw_uc = lpw) == NULL) ? NULL : (\
					_conv_uc = WideCharToMultiByte(CP_UTF8, 0, _lpw_uc, -1, NULL, 0, 0, 0) * sizeof(CHAR),\
					W2UTF8Helper((LPSTR)_alloca(_conv_uc), _lpw_uc, _conv_uc)))

#define UTF82A(lpu)	(\
					((_lpu_uc = lpu) == NULL) ? NULL : (\
					_conv_uc = MultiByteToWideChar(CP_UTF8, 0, _lpu_uc, -1, NULL, 0) * sizeof(WCHAR),\
					_lpw_uc = UTF82WHelper((LPWSTR)_alloca(_conv_uc), _lpu_uc, _conv_uc),\
					_conv_uc = WideCharToMultiByte(CP_ACP, 0, _lpw_uc, -1, NULL, 0, 0, 0)  * sizeof(CHAR),\
					W2AHelper((LPSTR)_alloca(_conv_uc), _lpw_uc, _conv_uc)))

#define UTF82W(lpu)	(\
					((_lpu_uc = lpu) == NULL) ? NULL : (\
					_conv_uc = MultiByteToWideChar(CP_UTF8, 0, _lpu_uc, -1, NULL, 0) * sizeof(WCHAR),\
					UTF82WHelper((LPWSTR)_alloca(_conv_uc), _lpu_uc, _conv_uc)))

#ifdef _UNICODE
	#define T2UTF8	W2UTF8
	#define UTF82T	UTF82W
#else
	#define T2UTF8	A2UTF8
	#define UTF82T	UTF82A
#endif

#define UTF8_CONVERSION_EX	int _conv_uc_ex, _idx1, _idx2; (_conv_uc_ex); (_idx1); (_idx2);\
							_SafeAllocator<WCHAR> _saw; _SafeAllocator<CHAR> _saa; _SafeAllocator<CHAR> _sau

#define A2UTF8_EX(lpa)	(\
						((LPCSTR)lpa == NULL) ? NULL : (\
						_conv_uc_ex = MultiByteToWideChar(CP_ACP, 0, lpa, -1, NULL, 0),\
						((_idx1 = _saw.Alloc(_conv_uc_ex)) == -1) ? NULL : (\
						A2WHelper(_saw.GetPtr(_idx1), lpa, _conv_uc_ex),\
						_conv_uc_ex = WideCharToMultiByte(CP_UTF8, 0, _saw.GetPtr(_idx1), -1, NULL, 0, 0, 0),\
						((_idx2 = _sau.Alloc(_conv_uc_ex)) == -1) ? _saw.Free(_idx1), NULL : (\
						W2UTF8Helper(_sau.GetPtr(_idx2), _saw.GetPtr(_idx1), _conv_uc_ex),\
						_saw.Free(_idx1), _sau.GetPtr(_idx2)))))

#define W2UTF8_EX(lpw)	(\
						((LPCWSTR)lpw == NULL) ? NULL : (\
						_conv_uc_ex = WideCharToMultiByte(CP_UTF8, 0, lpw, -1, NULL, 0, 0, 0),\
						((_idx1 = _sau.Alloc(_conv_uc_ex)) == -1) ? NULL : (\
						W2UTF8Helper(_sau.GetPtr(_idx1), lpw, _conv_uc_ex))))

#define UTF82A_EX(lpu)	(\
						((LPCSTR)lpu == NULL) ? NULL : (\
						_conv_uc_ex = MultiByteToWideChar(CP_UTF8, 0, lpu, -1, NULL, 0),\
						((_idx1 = _saw.Alloc(_conv_uc_ex)) == -1) ? NULL : (\
						UTF82WHelper(_saw.GetPtr(_idx1), lpu, _conv_uc_ex),\
						_conv_uc_ex = WideCharToMultiByte(CP_ACP, 0, _saw.GetPtr(_idx1), -1, NULL, 0, 0, 0),\
						((_idx2 = _saa.Alloc(_conv_uc_ex)) == -1) ? _saw.Free(_idx1), NULL : (\
						W2AHelper(_saa.GetPtr(_idx2), _saw.GetPtr(_idx1), _conv_uc_ex),\
						_saw.Free(_idx1), _saa.GetPtr(_idx2)))))

#define UTF82W_EX(lpu)	(\
						((LPCSTR)lpu == NULL) ? NULL : (\
						_conv_uc_ex = MultiByteToWideChar(CP_UTF8, 0, lpu, -1, NULL, 0),\
						((_idx1 = _saw.Alloc(_conv_uc_ex)) == -1) ? NULL : (\
						UTF82WHelper(_saw.GetPtr(_idx1), lpu, _conv_uc_ex))))

#ifdef _UNICODE
	#define T2UTF8_EX	W2UTF8_EX
	#define UTF82T_EX	UTF82W_EX
#else
	#define T2UTF8_EX	A2UTF8_EX
	#define UTF82T_EX	UTF82A_EX
#endif