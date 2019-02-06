#pragma once

#include <stdint.h>

#define ALIGN(size, align) ( (size > 0) ? ((((size-1)/align)+1)*align) : align )

struct MemRange
{
	uint32_t Offset = 0;
	uint32_t Size = 0;
};

struct MemAlloc
{
	static const size_t DEFAULT_ALIGN = 1u;
	template <typename T> T* TAlloc(size_t count = 1, size_t align = 1) const
	{
		size_t itemSize = ALIGN(sizeof(T), align);
		return static_cast<T*>(Alloc(itemSize * count));
	}
	virtual void Init(size_t capacity, const MemAlloc* parent = Default()) = 0;
	virtual void* Alloc(size_t size, size_t align = DEFAULT_ALIGN) const = 0;
	virtual void Free(void* ptr) const = 0;
	virtual void Destroy() = 0;
	static const MemAlloc* Default();
};

class MemAllocBase : public MemAlloc
{
public:
	virtual void Init(size_t capacity, const MemAlloc* parent = Default()) override;
    size_t GetCapacity() const { return Capacity; }
	virtual void Destroy() override;
protected:
	uint8_t* BasePtr = nullptr;
	const MemAlloc* Parent;
	size_t Capacity = 0u;
};

class MemAllocLinear : public MemAllocBase
{
public:
	virtual void* Alloc(size_t size, size_t align = DEFAULT_ALIGN) const override;
	virtual void Free(void* ptr) const override;
    void CopyTo(void* buffer, size_t max) const;
    void Reset() const { Offset = 0u; }
	size_t GetBytesUsed() const;
protected:
	mutable size_t Offset = 0u;
};

class MemAllocStack : public MemAllocLinear
{
public:
	void PushFrame() const;
	void PopFrame() const;
private:
	mutable size_t Frame = 0u;
};

class MemAllocStackFrame
{
public:
	explicit MemAllocStackFrame(const MemAllocStack& stack);
	~MemAllocStackFrame();
private:
	const MemAllocStack& Stack;
};

template <typename T, typename Allocator = MemAllocLinear>
class TMemAlloc
{
public:
	explicit TMemAlloc(size_t align) : Stride(ALIGN(sizeof(T), align)) {}
	void Init(size_t capacity, const MemAlloc* parent = Default())
	{ 
		capacity = ALIGN(capacity, Stride);
		MAlloc.Init(capacity, parent); 
	}
	T* Alloc(size_t size = 1)
	{
		T* retval = static_cast<T*>(MAlloc.Alloc(size * Stride));
		if (retval != nullptr)
		{
			++Count;
		}
		return retval;
	}
	void Free(void* ptr)
	{
		MAlloc.Free(ptr);
		--Count;
	}
	void Destroy()
	{
		MAlloc.Destroy();
		Count = 0;
	}
private:
	size_t Stride = sizeof(T);
	size_t Count = 0;
	Allocator MAlloc;
};
