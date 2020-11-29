#include "BMPImageModule.h"

BMPImageModule::BMPImageModule()
{
}


BMPImageModule::~BMPImageModule()
{
	bmp_img_free(&m_img);
}

bool BMPImageModule::Initialize()
{
	return true;
}

ImageBuffer* BMPImageModule::GetPixelDataFromFile(char* szFileName)
{

	bmp_error error = bmp_img_read(&m_img, szFileName);

	if (BMP_OK != error)
		return nullptr;

	m_imageBuffer._bufferType = IAMGE_BUFFER_2D_ARRAY;
	m_imageBuffer._pBuffer = (char**)m_img.img_pixels;
	m_imageBuffer._height = m_img.img_header.biHeight;
	m_imageBuffer._width = m_img.img_header.biWidth;
	m_imageBuffer._bpp = m_img.img_header.biBitCount;
	m_imageBuffer._colorType = 0;

	return &m_imageBuffer;
}

ImageBuffer* BMPImageModule::GetPixelDataFromBuffer(char* pBuffer, int size)
{
	//not implemented
	return nullptr;
}

bool BMPImageModule::SavePixelData(char* szFileName, char* pBuffer, int size)
{
	bmp_img_write(&m_img, szFileName);

	return true;
}

