#pragma once

typedef struct Image
{
	int Width;
	int Height;
	long DataSize;
	void* Data;
} Image;

C_API EResult TGALoadImage(const char* fName, Image** image);
