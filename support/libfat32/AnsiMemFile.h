#pragma once
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>

class CAnsiMemFile
{
protected:
	UINT m_nGrowBytes;
	DWORD m_nPosition;
	DWORD m_nBufferSize;
	DWORD m_nFileSize;
	BYTE* m_lpBuffer;
	BOOL m_bAutoDelete;

#pragma intrinsic(memcpy)
	BYTE* Memcpy(BYTE* lpMemTarget, const BYTE* lpMemSource,
		UINT nBytes)
	{
		if (0 == nBytes)
			return NULL;
		assert(lpMemTarget != NULL);
		assert(lpMemSource != NULL);

		//assert(AfxIsValidAddress(lpMemTarget, nBytes));
		//assert(AfxIsValidAddress(lpMemSource, nBytes, FALSE));

		return (BYTE*)memcpy(lpMemTarget, lpMemSource, nBytes);
	}
#pragma function(memcpy)

	void Free(BYTE* lpMem) { free(lpMem); }
	BYTE* Realloc(BYTE* lpMem, DWORD nBytes) {
		//			//Free(lpMem);
		//			if(nBytes<=m_nFileSize){ //Not support
		//				//throw exception("Not support");
		//				throw "Realloc Error!";
		//			}
		//			BYTE *pNewBlock=Alloc(nBytes);
		//			//copy to new block
		//			::memcpy(pNewBlock,lpMem,m_nFileSize);
		//			Free(lpMem);
		//			return pNewBlock;
		return (BYTE*)realloc(lpMem, (UINT)nBytes);
	}
	BYTE* Alloc(DWORD nBytes) {
		//BYTE *pNewBlock=new BYTE[nBytes];
		//assert(pNewBlock!=NULL);
		//return pNewBlock;
		return (BYTE*)malloc((UINT)nBytes);
	}

	void GrowFile(DWORD dwNewLen)
	{
		assert(this);

		if (dwNewLen > m_nBufferSize)
		{
			// grow the buffer
			DWORD dwNewBufferSize = (DWORD)m_nBufferSize;

			// watch out for buffers which cannot be grown!
			assert(m_nGrowBytes != 0);
			//if (m_nGrowBytes == 0)
			// AfxThrowMemoryException();

			// determine new buffer size
			while (dwNewBufferSize < dwNewLen)
				dwNewBufferSize += m_nGrowBytes;

			// allocate new buffer
			BYTE* lpNew;
			if (m_lpBuffer == NULL)
				lpNew = Alloc(dwNewBufferSize);
			else
				lpNew = Realloc(m_lpBuffer, dwNewBufferSize);

			if (lpNew == NULL) {
				
				assert("Encounter error when allocating memory!\n");
			}


			m_lpBuffer = lpNew;
			m_nBufferSize = dwNewBufferSize;
		}
		assert(this);
	}
public:
	enum SeekPosition { begin = 0x0, current = 0x1, end = 0x2 };

	
	CAnsiMemFile(UINT nGrowBytes = 1024)
	{
		assert(nGrowBytes <= UINT_MAX);

		m_nGrowBytes = nGrowBytes;
		m_nPosition = 0;
		m_nBufferSize = 0;
		m_nFileSize = 0;
		m_lpBuffer = NULL;
		m_bAutoDelete = TRUE;
	}
	
	CAnsiMemFile(const BYTE* lpBuffer, UINT nBufferSize, UINT nGrowBytes = 0)
	{
		assert(nGrowBytes <= UINT_MAX);

		m_nGrowBytes = nGrowBytes;
		m_nPosition = 0;
		m_nBufferSize = nBufferSize;
		m_nFileSize = nGrowBytes == 0 ? nBufferSize : 0;
		m_lpBuffer = Alloc(nBufferSize);
		memcpy(m_lpBuffer, lpBuffer, nBufferSize);
		//	m_lpBuffer = lpBuffer;
		m_bAutoDelete = TRUE;
	}

	~CAnsiMemFile()
	{
		// Close should have already been called, but we check anyway
		if (m_lpBuffer)
			Close();
		assert(m_lpBuffer == NULL);

		m_nGrowBytes = 0;
		m_nPosition = 0;
		m_nBufferSize = 0;
		m_nFileSize = 0;
	}


	DWORD GetPosition() const
	{
		assert(this);
		return m_nPosition;
	}
	
	DWORD GetLength() const
	{
		//		DWORD dwLen, dwCur;
		//		// Seek is a non const operation
		//		CAnsiMemFile* pFile = (CAnsiMemFile*)this;
		//		dwCur = pFile->Seek(0L, current);
		//		dwLen = pFile->SeekToEnd();
		//		pFile->Seek(dwCur, begin);
		//		
		//		return dwLen;
		return m_nFileSize;
	}
	
	const BYTE* GetPtr()
	{
		return m_lpBuffer;
	}

	
	
	void Attach(BYTE* lpBuffer, UINT nBufferSize, UINT nGrowBytes)
	{
		assert(m_lpBuffer == NULL);

		m_nGrowBytes = nGrowBytes;
		m_nPosition = 0;
		m_nBufferSize = nBufferSize;
		m_nFileSize = nGrowBytes == 0 ? nBufferSize : 0;
		m_lpBuffer = lpBuffer;
		m_bAutoDelete = FALSE;
	}
	
	BYTE* Detach()
	{
		BYTE* lpBuffer = m_lpBuffer;
		m_lpBuffer = NULL;
		m_nFileSize = 0;
		m_nBufferSize = 0;
		m_nPosition = 0;

		return lpBuffer;
	}
	
	DWORD SeekToEnd()
	{
		return Seek(0, CAnsiMemFile::end);
	}
	
	void SeekToBegin()
	{
		Seek(0, CAnsiMemFile::begin);
	}
	
	void SetLength(DWORD dwNewLen)
	{
		assert(this);

		if (dwNewLen > m_nBufferSize)
			GrowFile(dwNewLen);

		if (dwNewLen < m_nPosition)
			m_nPosition = dwNewLen;

		m_nFileSize = dwNewLen;
		assert(this);
	}
	
	UINT Read(void* lpBuf, UINT nCount)
	{
		assert(this);

		if (nCount == 0)
			return 0;

		assert(lpBuf != NULL);
		//assert(AfxIsValidAddress(lpBuf, nCount));

		if (m_nPosition > m_nFileSize)
			return 0;

		UINT nRead;
		if (m_nPosition + nCount > m_nFileSize)
			nRead = (UINT)(m_nFileSize - m_nPosition);
		else
			nRead = nCount;

		Memcpy((BYTE*)lpBuf, (BYTE*)m_lpBuffer + m_nPosition, nRead);
		m_nPosition += nRead;

		assert(this);

		return nRead;
	}
	
	void Write(const void* lpBuf, UINT nCount)
	{
		assert(this);

		if (nCount == 0)
			return;

		assert(lpBuf != NULL);
		//assert(AfxIsValidAddress(lpBuf, nCount, FALSE));

		if (m_nPosition + nCount > m_nBufferSize)
			GrowFile(m_nPosition + nCount);

		assert(m_nPosition + nCount <= m_nBufferSize);

		Memcpy((BYTE*)m_lpBuffer + m_nPosition, (BYTE*)lpBuf, nCount);

		m_nPosition += nCount;

		if (m_nPosition > m_nFileSize)
			m_nFileSize = m_nPosition;

		assert(this);
	}
	
	//enum SeekPosition { begin = 0x0, current = 0x1, end = 0x2 };
	LONG Seek(LONG lOff, UINT nFrom)
	{
		assert(this);
		assert(nFrom == begin || nFrom == end || nFrom == current);

		LONG lNewPos = m_nPosition;

		if (nFrom == begin)
			lNewPos = lOff;
		else if (nFrom == current)
			lNewPos += lOff;
		else if (nFrom == end)
			lNewPos = m_nFileSize + lOff;
		else
			return -1;

		if (lNewPos < 0) {
			
			assert("Encounter error when seek file\n");
		}

		m_nPosition = lNewPos;

		assert(this);
		return m_nPosition;
	}
	
	void Close()
	{
		assert((m_lpBuffer == NULL && m_nBufferSize == 0) ||
			!m_bAutoDelete || TRUE);
		assert(m_nFileSize <= m_nBufferSize);

		m_nGrowBytes = 0;
		m_nPosition = 0;
		m_nBufferSize = 0;
		m_nFileSize = 0;
		if (m_lpBuffer && m_bAutoDelete)
			Free(m_lpBuffer);
		m_lpBuffer = NULL;
	}
};