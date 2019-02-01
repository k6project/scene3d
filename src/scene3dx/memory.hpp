#pragma once

#include <stdint.h>

struct MemRange
{
	uint32_t offset;
	uint32_t size;
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
	template <typename T> T* AllocT(size_t count = 1, size_t align = sizeof(T)) const
	{ 
		return static_cast<T*>(Alloc(sizeof(T)*count, align)); 
	}
	void Init(size_t capacity, const MemAlloc* parent = Default());
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
