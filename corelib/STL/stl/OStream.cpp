//stdlib.cpp
#include "OStream.h"
#include <memory.h>
#include "string.h"
#include <sprintf.h>

extern "C" void printf(const char* str, ...);

namespace std
{
	ostream cout;		//OStream cout

	ostream::ostream()
	{
		memset(m_aDataBuffer, 0, MAX_MESSAGE_BUFFER);
		m_usDataSize = 0;
	}

	ostream& ostream::operator<<(char *cp)
	{
		int len = (int)strlen((const char*)cp);

		memcpy(&m_aDataBuffer[m_usDataSize], cp, len);
		m_usDataSize += (unsigned short)len;
		m_aDataBuffer[m_usDataSize] = 0;
		m_usDataSize++;

		return *this;
	}

	ostream& ostream::operator<<(const char *cp)
	{
		int len = (int)strlen((const char*)cp);

		memcpy(&m_aDataBuffer[m_usDataSize], cp, len);
		m_usDataSize += (unsigned short)len;
		//m_aDataBuffer[m_usDataSize] = 0;
		//m_usDataSize++;

		return *this;
	}

	ostream& ostream::operator<<(const char c)
	{
		memcpy(&m_aDataBuffer[m_usDataSize], &c, sizeof(const char));
		m_usDataSize += sizeof(const char);
		return *this;
	}

	ostream& ostream::operator<<(int value)
	{
		char buf[512];
		itoa(value, 10, buf);
		memcpy(&m_aDataBuffer[m_usDataSize], buf, sizeof(char) * strlen(buf));
		m_usDataSize += sizeof(char) * strlen(buf);
		return *this;
	}

	ostream& ostream::operator<<(float value)
	{
		char buf[512];
		ftoa_fixed(buf, value);
		memcpy(&m_aDataBuffer[m_usDataSize], buf, sizeof(char) * strlen(buf));
		m_usDataSize += sizeof(char) * strlen(buf);
		return *this;
	}	

	ostream& ostream::operator<<(double value)
	{
		char buf[32] = { 0, };
		ftoa_fixed(buf, value);
		memcpy(&m_aDataBuffer[m_usDataSize], buf, sizeof(char) * strlen(buf));
		m_usDataSize += sizeof(char) * strlen(buf);
		return *this;
	}

	ostream& ostream::operator<<(unsigned long value)
	{
		memcpy(&m_aDataBuffer[m_usDataSize], &value, sizeof(unsigned long));
		m_usDataSize += sizeof(unsigned long);
		return *this;
	}

	ostream& ostream::operator<<(unsigned char *cq)
	{
		int len = (int)strlen((const char*)cq);
	
		memcpy(&m_aDataBuffer[m_usDataSize], cq, len);
		m_usDataSize += (unsigned short)len;
		m_aDataBuffer[m_usDataSize] = 0;
		m_usDataSize++;

		return *this;
	}

	ostream& ostream::operator<<(unsigned char c1)
	{
		memcpy(&m_aDataBuffer[m_usDataSize], &c1, sizeof(unsigned char));
		m_usDataSize += sizeof(unsigned char);
		return *this;
	}
	

	/*ostream& ostream::operator<<(char *cp)
	{
		printf(cp);
		return *this;
	}

	ostream& ostream::operator<<(const char *cp)
	{
		printf("%s", cp);
		return *this;
	}

	ostream& ostream::operator<<(const char c)
	{
		printf("%c", c);
		return *this;
	}

	ostream& ostream::operator<<(int value)
	{
		printf("%d", value);
		return *this;
	}

	ostream& ostream::operator<<(unsigned long value)
	{
		printf("%d", value);
		return *this;
	}

	ostream& ostream::operator<<(unsigned char *cq)
	{
		printf((char*)cq);		
		return *this;
	}

	ostream& ostream::operator<<(unsigned char c1)
	{
		printf("%c", (char)c1);
		return *this;
	}*/
}

