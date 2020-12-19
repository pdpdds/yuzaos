#pragma once
#include <windef.h>
#include <stdbool.h>

class CBitSet  
{
	bool		m_bAutoDelete;				// Is will be deleted?
	char*		m_pBitSet;					// Bitset raw memory pointer
	DWORD		m_dwUsedBytes;				// Used bytes.
public:

	CBitSet();
	virtual ~CBitSet();

	inline static DWORD sCalcUsedBytes( DWORD dwBits, bool b32BitAlign = TRUE )
	{
		DWORD dwUsedBytes = dwBits>>3;
		if( dwBits%8 ) dwUsedBytes++;

		if( b32BitAlign )
		{
			//<TODO> if os is 64 bit , then maybe change code little.
			dwUsedBytes += (4 - (dwUsedBytes&3) );
			
		}
		return dwUsedBytes;
	}

	bool	Create( DWORD dwBits, bool bZeroInit, bool b32BitAlign = TRUE, char* pszSource = NULL, DWORD dwSourceLen = 0 );

	const DWORD	GetUsedBytes() const
	{
		return m_dwUsedBytes;
	}
	inline char Get( DWORD dwBit )
	{
		return m_pBitSet[dwBit>>3] & 1<<(dwBit&7);
	}
	inline void Set( DWORD dwBit, char bit )
	{
		bit?m_pBitSet[dwBit>>3]|=1<<(dwBit&7):
			m_pBitSet[dwBit>>3]&=~(1<<(dwBit&7));		
	}

	inline char operator [] (int nBit)
	{
		return Get( nBit );
	}
};
