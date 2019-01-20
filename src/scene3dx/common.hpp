#pragma once

#include <cstddef>

template <typename T>
struct LinkedListNode
{
    T* Next = nullptr;
};

template <typename T>
struct ScopedPtr
{
    T* Pointer = nullptr;
    void operator=(void* ptr) { Pointer = static_cast<T*>(ptr); }
    operator T*() const { return Pointer; }
    ~ScopedPtr() { delete Pointer; }
};

void* LoadFileIntoMemory(const char* name, size_t* size);
