#include <iostream>
#include <sstream>
#include <string>
#include <string.h>
#include <gif_lib.h>


bool gif_write(const char* fileName)
{
    int error;
    GifFileType* gifFile = EGifOpenFileName(fileName, false, &error);
    if (!gifFile) {
        std::cout << "EGifOpenFileName() failed - " << error << std::endl;
        return false;
    }

    GifColorType colors[64];
    GifColorType* c = colors;
    int level[4] = { 0, 85, 170, 255 };
    for (int r = 0; r < 4; ++r) {
        for (int g = 0; g < 4; ++g) {
            for (int b = 0; b < 4; ++b, ++c) {
                c->Red = level[r];
                c->Green = level[g];
                c->Blue = level[b];
            }
        }
    }
    GifByteType pix[16] = {
        0,  1,  2,  3,  // B
        0,  4,  8, 12,  // G
        0, 16, 32, 48,  // R
        0, 21, 42, 63,  // BK
    };

    gifFile->SWidth = 4;
    gifFile->SHeight = 4;
    gifFile->SColorResolution = 8;
    gifFile->SBackGroundColor = 0;
    gifFile->SColorMap = GifMakeMapObject(64, colors);

    SavedImage gifImage;
    gifImage.ImageDesc.Left = 0;
    gifImage.ImageDesc.Top = 0;
    gifImage.ImageDesc.Width = 4;
    gifImage.ImageDesc.Height = 4;
    gifImage.ImageDesc.Interlace = false;
    gifImage.ImageDesc.ColorMap = nullptr;
    gifImage.RasterBits = (GifByteType*)malloc(16);
    gifImage.ExtensionBlockCount = 0;
    gifImage.ExtensionBlocks = nullptr;
    memcpy(gifImage.RasterBits, pix, 16);

    GifMakeSavedImage(gifFile, &gifImage);

    if (EGifSpew(gifFile) == GIF_ERROR) {
        std::cout << "EGifSpew() failed - " << gifFile->Error << std::endl;
        EGifCloseFile(gifFile, &error);
        return false;
    }

    EGifCloseFile(gifFile, &error);
    return true;
}



bool gif_read(const char* fileName)
{
    int error;
    GifFileType* gifFile = DGifOpenFileName(fileName, &error);
    if (!gifFile) {
        std::cout << "DGifOpenFileName() failed - " << error << std::endl;
        return false;
    }
    if (DGifSlurp(gifFile) == GIF_ERROR) {
        std::cout << "DGifSlurp() failed - " << gifFile->Error << std::endl;
        DGifCloseFile(gifFile, &error);
        return false;
    }

    ColorMapObject* commonMap = gifFile->SColorMap;
    std::cout << fileName << ": " << gifFile->SWidth << "x" << gifFile->SHeight << std::endl;

    for (int i = 0; i < gifFile->ImageCount; ++i) {
        const SavedImage& saved = gifFile->SavedImages[i];
        const GifImageDesc& desc = saved.ImageDesc;
        const ColorMapObject* colorMap = desc.ColorMap ? desc.ColorMap : commonMap;
        std::cout << "[" << i << "] "
            << desc.Width << "x" << desc.Height << "+" << desc.Left << "," << desc.Top
            << ", has local colorMap: " << (desc.ColorMap ? "Yes" : "No") << std::endl;

        std::stringstream ss;
        for (int v = 0; v < desc.Height; ++v) {
            for (int u = 0; u < desc.Width; ++u) {
                int c = saved.RasterBits[v * desc.Width + u];
                printf(" %02X", c);
                if (colorMap) {
                    GifColorType rgb = colorMap->Colors[c];
                    ss << " [" << (int)rgb.Red << "," << (int)rgb.Green << "," << (int)rgb.Blue << "]";
                }
            }
            std::cout << ":" << ss.str().c_str() << std::endl;
        }
    }

    DGifCloseFile(gifFile, &error);
    return true;
}


int main(int argc, char** argv)
{
    return (gif_write("test.gif") && gif_read("test.gif")) ? 0 : 1;
}