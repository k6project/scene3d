#include "coredefs.h"

#include "assets.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

// TGA internal image type
typedef enum TGAImgType
{
	TGA_UNCOMPRESSED_CM = 1,   // uncompressed color-mapped
	TGA_UNCOMPRESSED_RGB = 2,  // uncompressed RGB(A)
	TGA_UNCOMPRESSED_GS = 3,   // uncompressed gray scale
	TGA_RLE_CM = 9,            // RLE-compressed color-mapped
	TGA_RLE_RGB = 10,          // RLE-compressed RGB(A)
	TGA_COMPRESSED_GS = 11,    // compressed gray scale
	TGA_COMPRESSED_CM = 32,    // compressed color-mapped
	TGA_COMPRESSED_CM_4P = 33, // compressed color-mapped, 4-pass(?)
	TGA_INVALID_IMAGE = 0      // invalid, no image data
} TGAImgType;

typedef enum TGAStorage
{
	TGA_NON_INTERLEAVED,
	TGA_INTERLEAVED_2WAY,
	TGA_INTERLEAVED_4WAY
} TGAStorage;

typedef enum TGAOrigin
{
	TGA_LOWER_LEFT,
	TGA_UPPER_LEFT
} TGAOrigin;

#define TGA_HEADER_SIZE 18u

typedef struct TGAHeader
{
	int ImgIdSize               : 8;
	int ColorMapType            : 8;
	TGAImgType ImageType        : 8;
	unsigned int CMOriginLo     : 8;
	unsigned int CMOriginHi     : 8;
	unsigned int CMLengthLo     : 8;
	unsigned int CMLengthHi     : 8;
	unsigned int CMEntrySize    : 8;
	int XOrigin                 : 16;
	int YOrigin                 : 16;
	int Width                   : 16;
	int Height                  : 16;
	int BitsPerPixel            : 8;
	unsigned int ImgDescriptor  : 8;
} TGAHeader;

#define TGA_STORAGE(h) ((TGAStorage)(h.ImgDescriptor & 7u))
#define TGA_ORIGIN(h) ((TGAOrigin)((h.ImgDescriptor >> 5) & 1u))

static EResult TGAReadUncompressed(FILE* fp, Image* img, const TGAHeader* tgaHeader)
{
	if (tgaHeader->ImageType != TGA_UNCOMPRESSED_RGB)
	{
		return RES_NOT_SUPPORTED;
	}
	else
	{
		TGAOrigin origin = TGA_ORIGIN((*tgaHeader));
		int rowStep = (origin == TGA_LOWER_LEFT) ? -1 : 1;
		const int rowSize = tgaHeader->Width * (tgaHeader->BitsPerPixel >> 3);
		int row = (origin == TGA_LOWER_LEFT) ? (tgaHeader->Height - 1) : 0;
		for (int r = 0; r < tgaHeader->Height; r++, row += rowStep)
		{
			void* rowMem = ((char*)img->Data) + row * rowSize;
			if (fread(rowMem, rowSize, 1, fp) != 1)
			{
				return RES_PLATFORM_ERROR;
			}
		}
		return RES_NO_ERROR;
	}
	return RES_UNKNOWN_ERROR;
}

C_API EResult TGALoadImage(const char* fName, Image** image)
{
	FILE* fp = NULL;
	bool extImage = (*image != NULL);
	if (fopen_s(&fp, fName, "rb") != 0)
	{
		return RES_UNKNOWN_ERROR;
	}
	TGAHeader tgaHeader;
	fread(&tgaHeader, TGA_HEADER_SIZE, 1, fp);
	fseek(fp, tgaHeader.ImgIdSize, SEEK_CUR); //skip image id section
	Image* img = (extImage) ? *image : (Image*)malloc(sizeof(Image));
	img->DataSize = (tgaHeader.Width * tgaHeader.Height) * (tgaHeader.BitsPerPixel >> 3);
	img->Data = malloc(img->DataSize);
	img->Width = tgaHeader.Width;
	img->Height = tgaHeader.Height;
	EResult res = RES_NO_ERROR;
	switch (tgaHeader.ImageType)
	{
	case TGA_UNCOMPRESSED_RGB:
		res = TGAReadUncompressed(fp, img, &tgaHeader);
		break;
	default:
		res = RES_NOT_SUPPORTED;
		break;
	}
	fclose(fp);
	if (res != RES_NO_ERROR)
	{
		free(img->Data);
		if (!extImage)
		{
			free(img);
			img = NULL;
		}
	}
	*image = img;
	return res;
}
