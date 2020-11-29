#pragma once
#include "I_ImageInterface.h"
#include "memory.h"
#include "libbmp.h"

class BMPImageModule : public I_ImageInterface
{
public:
	BMPImageModule();
	~BMPImageModule();

	virtual bool Initialize() override;
	virtual ImageBuffer* GetPixelDataFromFile(char* szFileName)  override;
	virtual ImageBuffer* GetPixelDataFromBuffer(char* pBuffer, int size)  override;
	virtual bool SavePixelData(char* szFileName, char* pBuffer, int size)  override;

private:
	ImageBuffer m_imageBuffer;
	bmp_img m_img;
};

