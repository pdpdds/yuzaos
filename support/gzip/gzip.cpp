/* zlib.h -- interface of the 'zlib' general purpose compression library
  version 1.1.4, March 11th, 2002

  Copyright (C) 1995-2002 Jean-loup Gailly and Mark Adler

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
	 claim that you wrote the original software. If you use this software
	 in a product, an acknowledgment in the product documentation would be
	 appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
	 misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  Jean-loup Gailly jloup@gzip.org
  Mark Adler madler@alumni.caltech.edu

*/

#include <stdlib.h>
#include <stdio.h>
#include "zlib.h"  
#include "gzip.h"
#include <sys/types.h>
#include <fcntl.h>
#include <list>
#include <utility>
#include <unistd.h>

#pragma warning(disable : 4996)	// vs2005에서 추가됨. 보안상의 이유로 더 이상 사용되지 않는 MFC나 ATL 함수를 사용하는 경우에도 C4996이 발생할 수 있습니다

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

namespace SDK {

	bool CZipper::Open(EArchiveMode eArchiveMode)
	{
		m_eArchiveMode = eArchiveMode;

		return true;
	}

#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)

	LPCWSTR CZipper::m_stsVersion = WIDEN(ZLIB_VERSION);

	CZipper::CZipper()
		: m_eCompressionMode(CompressionModeDefault),
		m_eArchiveMode(ArchiveModeClosed),
		m_eStrategy(StrategyDefault)
	{

	}

	CZipper::~CZipper()
	{
	}

	CGZip::~CGZip()
	{
		if (IsOpen())
			Close();
	}

	bool CGZip::Open(LPCSTR szFileName, EArchiveMode eArchiveMode)
	{

		if (IsOpen())
			return false;

		if (eArchiveMode == ArchiveModeWrite)
		{
			m_gzf = gzopen(szFileName, "wb");
			UpdateParams();
		}
		else if (eArchiveMode == ArchiveModeRead)
		{
			if (access(szFileName, 02))
				return false;

			struct stat info;
			int result = stat(szFileName, &info);

			if (result == 0)
				return false;

			 m_bufferSize = info.st_size;

			m_gzf = gzopen(szFileName, "rb");
		}

		if (m_gzf != 0)
			m_eArchiveMode = eArchiveMode;
		else
			m_eArchiveMode = ArchiveModeClosed;

		return m_gzf != 0;
	}

	bool CGZip::Close()
	{
		if (!IsOpen())
			return false;

		int result = gzclose(m_gzf);
		m_gzf = 0;

		return result == 0;
	};

	bool CGZip::WriteBuffer(void* pBuffer, size_t nBytes)
	{
		if (!IsOpen() || !IsWriting())
			return false;
		int written = gzwrite(m_gzf, pBuffer, nBytes);

		return written == (int)(nBytes);
	};

	bool CGZip::Flush(EFlushMode eFlush)
	{
		if (!IsOpen() || !IsWriting())
			return false;

		return gzflush(m_gzf, eFlush) == Z_OK;
	}

	bool CGZip::WriteString(LPCWSTR str)
	{
		return WriteBuffer((void*)str, (wcslen(str)) * sizeof(WCHAR));
	};

	int CGZip::ReadBuffer(voidp* ppBuffer, size_t& nBytes)
	{
		using namespace std;
		int read;

		nBytes = 0;

		if (!IsOpen() || !IsReading())
			return false;

		if (!m_bufferSize)
			return false;

		list< pair < char*, size_t > > lBuffers;
		char* pBuffer = NULL;

		read = 1;
		while (read > 0)
		{
			pBuffer = new char[m_bufferSize];
			read = gzread(m_gzf, pBuffer, m_bufferSize);
			if (read > 0)
			{
				lBuffers.push_back(pair<char*, size_t>(pBuffer, read));
				nBytes += read;
			}
			else
				delete pBuffer;
		};

		if (read == -1)
		{
			while (!lBuffers.empty())
			{
				delete[] lBuffers.front().first;
				lBuffers.pop_front();
			}
			return false;
		}

		// allocating memory and writing buffer	
		*ppBuffer = new char[nBytes];
		size_t offset = 0;
		while (!lBuffers.empty())
		{
			pBuffer = lBuffers.front().first;
			read = lBuffers.front().second;
			memcpy((char*)*ppBuffer + offset, pBuffer, read);
			offset += read;

			delete[] pBuffer;
			lBuffers.pop_front();
		}

		return nBytes != 0;
	};

	int CGZip::ReadString(LPWSTR* ppString)
	{
		using namespace std;
		int read;

		size_t nBytes = 0;

		if (!IsOpen() || !IsReading())
			return false;

		if (!m_bufferSize)
			return false;

		list< pair < char*, size_t > > lBuffers;
		char* pBuffer = NULL;

		read = 1;
		while (read > 0)
		{
			pBuffer = new char[m_bufferSize];
			read = gzread(m_gzf, pBuffer, m_bufferSize);
			if (read > 0)
			{
				lBuffers.push_back(pair<char*, size_t>(pBuffer, read));
				nBytes += read;
			}
			else
				delete pBuffer;
		};

		if (read == -1)
		{
			while (!lBuffers.empty())
			{
				delete[] lBuffers.front().first;
				lBuffers.pop_front();
			}
			return false;
		}

		// allocating memory and writing buffer	
		*ppString = new WCHAR[nBytes + 1];
		size_t offset = 0;
		while (!lBuffers.empty())
		{
			pBuffer = lBuffers.front().first;
			read = lBuffers.front().second;
			memcpy(((char*)*ppString) + offset, pBuffer, read);
			offset += read;

			delete[] pBuffer;
			lBuffers.pop_front();
		}
		(*ppString)[nBytes] = '\0';

		return nBytes != 0;
	}

	int CGZip::ReadBufferSize(voidp pBuffer, size_t nBytes)
	{
		if (!IsOpen() || !IsReading())
			return false;

		return gzread(m_gzf, pBuffer, nBytes);
	}

	void CGZip::UpdateParams()
	{
		if (!IsOpen() || !IsWriting())
			return;

		gzsetparams(m_gzf, m_eCompressionMode, m_eStrategy);
	};

	bool CGZip::IsEOF() const
	{
		if (!IsOpen())
			return true;

		return gzeof(m_gzf) == 1;
	}

};