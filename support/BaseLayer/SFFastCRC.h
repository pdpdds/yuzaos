#pragma once

class SFFastCRC
{
public:
	SFFastCRC(void);
	virtual ~SFFastCRC(void);

	bool Initialize();
	bool GetCRC(const unsigned char* pBuffer, unsigned int BufferSize, unsigned int& CRC);
	bool GetSimpleCRC(const char* pBuffer, unsigned int BufferSize, unsigned int& CRC);	
	bool GetZLibCRC(const unsigned char* pBuffer, unsigned int BufferSize, unsigned int& CRC);
	bool GetadlerCRC(const unsigned char* pBuffer, unsigned int BufferSize, unsigned int& CRC);
};



