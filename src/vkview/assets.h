#pragma once

#define TEXTURE_FILE(n) "textures/" #n ".tga"

typedef struct Image
{
	int Width;
	int Height;
	long DataSize;
	void* Data;
} Image;

C_API EResult TGALoadImage(const char* fName, Image** image);
