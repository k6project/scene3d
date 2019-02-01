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

class IOBuffer
{
public:
	void LoadFromFile(const char* name, const MemAlloc& mem = *MemAllocBase::Default());
	operator void*() const { return Data; }
	size_t Size() const { return Bytes; }
	~IOBuffer() { Memory->Free(Data); Bytes = 0; }
private:
	const MemAlloc* Memory = nullptr;
	void* Data = nullptr;
	size_t Bytes = 0;
};

float MakeColorRGB(uint32_t r, uint32_t g, uint32_t b);

struct ScenePrimitive;

struct Scene
{
	virtual void CommitParameters(void* buffer, size_t max) const = 0;
	virtual const ScenePrimitive* GetPrimitives() const = 0;
};
