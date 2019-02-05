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
	static const size_t DEFAULT_ALIGN = 16u;
	virtual void* Alloc(size_t size, size_t align = DEFAULT_ALIGN) const = 0;
	virtual void Free(void* ptr) const = 0;
};

class MemAllocBase : public MemAlloc
{
public:
	static const MemAlloc* Default();
	template <typename T> T* TAlloc(size_t count = 1, size_t align = sizeof(T)) const
	{ 
		return static_cast<T*>(Alloc(sizeof(T)*count, align)); 
	}
	void Init(size_t capacity, const MemAlloc* parent = Default());
    size_t GetCapacity() const { return Capacity; }
	void Destroy();
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
