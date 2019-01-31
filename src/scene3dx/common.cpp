#include "common.hpp"

#include <windows.h>

void* LoadFileIntoMemory(const char* name, size_t* size)
{
    HANDLE file = CreateFile(name, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER fSize;
        if (GetFileSizeEx(file, &fSize)) 
        {
            void* buffer = new BYTE[fSize.QuadPart];
            DWORD bytesRead = static_cast<DWORD>(fSize.QuadPart);
            if (ReadFile(file, buffer, bytesRead, &bytesRead, nullptr))
            {
                *size = bytesRead;
                return buffer;
            }
            else
            {
                delete[] buffer;
            }
        }
        CloseHandle(file);
    }
    DWORD err = GetLastError();
    return nullptr;
}

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
			void* buffer = mem.Alloc(fSize.QuadPart);
			DWORD bytesRead = static_cast<DWORD>(fSize.QuadPart);
			if (ReadFile(file, buffer, bytesRead, &bytesRead, nullptr))
			{
				Bytes = bytesRead;
			}
			else
			{
				mem.Free(buffer);
			}
		}
		CloseHandle(file);
	}
}
