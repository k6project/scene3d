#include "common.hpp"

#include <windows.h>

#define NK_IMPLEMENTATION
#include <nuklear.h>

float MakeColorRGB(uint32_t r, uint32_t g, uint32_t b)
{
	uint32_t tmp = (((r) << 24) | ((g) << 16) | ((b) << 8) | 0xff);
	return *(reinterpret_cast<float*>(&tmp));
}

void IOBuffer::LoadFromFile(const char* name, const MemAlloc& mem)
{
	HANDLE file = CreateFile(name, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER fSize;
		if (GetFileSizeEx(file, &fSize))
		{
			Memory = &mem;
			void* buffer = Memory->Alloc(fSize.QuadPart);
			DWORD bytesRead = static_cast<DWORD>(fSize.QuadPart);
			if (ReadFile(file, buffer, bytesRead, &bytesRead, nullptr))
			{
				Bytes = bytesRead;
				Data = buffer;
			}
			else
			{
				Memory->Free(buffer);
				Memory = nullptr;
			}
		}
		CloseHandle(file);
	}
}

float PackColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	uint32_t tmp = (r << 24) | (g << 16) | (b << 8) | a;
	return *(reinterpret_cast<float*>(&tmp));
}
