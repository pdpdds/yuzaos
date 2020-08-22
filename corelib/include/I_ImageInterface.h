#pragma once

#define IAMGE_BUFFER_LINEAR 0
#define IAMGE_BUFFER_2D_ARRAY 1

typedef struct tag_ImageBuffer
{
	int _width;
	int _height;
	int _bpp;
	int _colorType;
	int _bufferType;
	char** _pBuffer;

}ImageBuffer;

class I_ImageInterface
{
public:
	virtual bool Initialize() { return false; }
	virtual ImageBuffer* GetPixelDataFromFile(char* szFileName) = 0;
	virtual ImageBuffer* GetPixelDataFromBuffer(char* pBuffer, int size) = 0;
	virtual bool SavePixelData(char* szFileName, char* pBuffer, int size) = 0;
};