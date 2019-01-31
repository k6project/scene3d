#pragma once

#include <cstddef>
#include <cstdint>

#include <memory.hpp>

template <typename T>
struct TLinkedListNode
{
    T* Next = nullptr;
};

template <typename T>
void DeleteAll(T*& listHead)
{
	T* begin = listHead;
	for (T* i = begin; i != nullptr;)
	{
		T* ptr = i;
		i = ptr->Next;
		delete ptr;
	}
	listHead = nullptr;
}

template <typename T>
struct TScopedPtr
{
    T* Pointer = nullptr;
    void operator=(void* ptr) { Pointer = static_cast<T*>(ptr); }
    operator T*() const { return Pointer; }
    ~TScopedPtr() { delete[] Pointer; }
};

class IOBuffer
{
public:
	void LoadFromFile(const char* name, const MemAlloc& mem = *MemAllocBase::Default());
	operator void*() const { return Data; }
	size_t Size() const { return Bytes; }
private:
	void* Data;
	size_t Bytes;
};

void* LoadFileIntoMemory(const char* name, size_t* size);

float MakeColorRGB(uint32_t r, uint32_t g, uint32_t b);
