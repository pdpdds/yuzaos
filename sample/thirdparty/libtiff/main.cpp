#include <stdio.h>
#include <tiffio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
	TIFF* image;
	uint32 width, height;
	int r1, c1, t1, imagesize;
	int nsamples;
	unsigned char* scanline = NULL;

	uint16 BitsPerSample;           // normally 8 for grayscale image
	uint16 SamplesPerPixel;         // normally 1 for grayscale image
	uint16 i;

	// Open the TIFF image
	if ((image = TIFFOpen(argv[1], "r")) == NULL) {
		fprintf(stderr, "Could not open incoming image\n");
		exit(42);
	}

	// Find the width and height of the image
	TIFFGetField(image, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(image, TIFFTAG_IMAGELENGTH, &height);
	TIFFGetField(image, TIFFTAG_BITSPERSAMPLE, &BitsPerSample);
	TIFFGetField(image, TIFFTAG_SAMPLESPERPIXEL, &SamplesPerPixel);
	imagesize = height * width + 1;	//get image size

	//allocate memory for reading tif image
	scanline = (unsigned char*)_TIFFmalloc(SamplesPerPixel * width);
	if (scanline == NULL) {
		fprintf(stderr, "Could not allocate memory!\n");
		exit(0);
	}

	fprintf(stderr, "W=%i H=%i BitsPerSample=%i SamplesPerPixel=%i\n", width, height, BitsPerSample, SamplesPerPixel);
	for (r1 = 0; r1 < height; r1++)
	{
		TIFFReadScanline(image, scanline, r1, 0);
		for (c1 = 0; c1 < width; c1++)
		{
			t1 = c1 * SamplesPerPixel;

			for (i = 0; i < SamplesPerPixel; i++)
				printf("%u \t", *(scanline + t1 + i));
			printf("\n");
		}
	}

	_TIFFfree(scanline); //free allocate memory

	TIFFClose(image);

}