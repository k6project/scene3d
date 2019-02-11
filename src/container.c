#include "container.h"

// Intrusive linked list contained in a contiguous memory block

struct LinkedArrayList
{
	uint32_t prev, next
};

// Swap two array elements, adjust internal link indices

void LinkedArrayList_Swap(uint32_t one, uint32_t two, void* list, uint32_t offs)
{
	// A{-1, 1} B{0,2} C{1,3} D{2,-1} : regular order
	// A{-1, 1} B{0,3} D{3,-1} C{1,2} : irregular order
	// remove element 1 (B) -> swap with 3 (C)
	// 1. capture state of 1 and 3: [1,0,3,3], [3,1,2,1] (old index, prev, next, new index)
	// 2. swap memory blocks													A{-1, 1} C{1,2} D{3,-1} B{0,3}
	// 3. for first saved state [1,0,3,3]:
	//    3.1 get elementPtr at prev (if prev == new index, get from old index) -> 0
	//	  3.2 elementPtr->next = new index										{-1, 3} {1,2} {3,-1} {0,3}
	//    3.3 get elementPtr at next (if next == new index, get from old index) -> 1
	//	  3.4 elementPtr->prev = new index										{-1, 3} {3,2} {3,-1} {0,3}
	// 4. for second saved state [3,1,2,1]:
	//    4.1 get elementPtr at prev (if prev == new index, get from old index) -> 3
	//	  4.2 elementPtr->next = new index										{-1, 3} {3,2} {3,-1} {0,1}
	//    4.3 get elementPtr at next (if next == new index, get from old index) -> 2
	//    4.4 elementPtr->prev = new index										{-1, 3} {3,2} {1,-1} {0,1}
}
