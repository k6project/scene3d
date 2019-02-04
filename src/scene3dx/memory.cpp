#include "memory.hpp"

#include <stdlib.h>

#define PAGE_ALIGN 4096

#define ALIGN(size, align) ( (size > 0) ? ((((size-1)/align)+1)*align) : align )

struct DefaultMemAlloc : public MemAlloc
{
	virtual void* Alloc(size_t size, size_t align) const override
	{
		return malloc(ALIGN(size, align));
	}
	virtual void Free(void* ptr) const override
	{
		free(ptr);
	}
};

const MemAlloc* MemAllocBase::Default()
{
	static const DefaultMemAlloc defMAlloc;
	return &defMAlloc;
}

void MemAllocBase::Init(size_t capacity, const MemAlloc* parent)
{
	Capacity = 0u;
	Parent = nullptr;
	if (parent)
	{
		BasePtr = static_cast<uint8_t*>(parent->Alloc(capacity, PAGE_ALIGN));
		if (BasePtr != nullptr)
		{
			Capacity = capacity;
			Parent = parent;
		}
	}
}

void MemAllocBase::Destroy()
{
	if (Parent && BasePtr)
	{
		Parent->Free(BasePtr);
	}
	BasePtr = nullptr;
	Parent = nullptr;
	Capacity = 0;
}

void* MemAllocLinear::Alloc(size_t size, size_t align) const
{
	size_t alignedOffset = ALIGN(Offset, align);
	void* retval = BasePtr + alignedOffset;
	Offset = alignedOffset + ALIGN(size, align);
	return retval;
}

void MemAllocLinear::Free(void*) const
{
}

void MemAllocStack::PushFrame() const
{
	size_t offset = Offset;
	size_t* Prev = AllocT<size_t>();
	*Prev = Frame;
	Frame = offset;
}

void MemAllocStack::PopFrame() const
{
	size_t* Prev = reinterpret_cast<size_t*>(BasePtr + Frame);
	Offset = Frame;
	Frame = *Prev;
}

MemAllocStackFrame::MemAllocStackFrame(const MemAllocStack& stack)
	: Stack(stack)
{
	Stack.PushFrame();
}

MemAllocStackFrame::~MemAllocStackFrame()
{
	Stack.PopFrame();
}