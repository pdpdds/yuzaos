#include "kheap.h"
#include <InterruptDisabler.h>
#include "platformapi.h"
#include <kassert.h>
#include <systemcall_impl.h>

// end is defined in the linker script.
//extern u32int end;
//u32int placement_address = (u32int)&end;

heap_t kheap;
DWORD g_usedHeapSize = 0;

u32int kmalloc_int(u32int sz, int align, u32int* phys)
{
	void* addr = memory_alloc(sz, (u8int)align, &kheap);

	if (phys != 0)
	{
		*phys = Syscall_GetPAFromVM(addr);
	}

	return (u32int)addr;
}

void kfree(void* p)
{
	if (p == 0)
		return;

	InterruptDisabler disabler;

	// Get the header and footer associated with this pointer.
	header_t* header = (header_t*)((u32int)p - sizeof(header_t));
	footer_t* footer = (footer_t*)((u32int)header + header->size - sizeof(footer_t));

	// Sanity checks.
	SKY_ASSERT(header->magic == HEAP_MAGIC, "header->magic == HEAP_MAGIC");
	SKY_ASSERT(footer->magic == HEAP_MAGIC, "footer->magic == HEAP_MAGIC");
	header->is_hole = 1;

	g_usedHeapSize -= header->size;

	// Do we want to add this header into the 'free holes' index?
	char do_add = 1;

	if (((u32int)header - sizeof(footer_t) >= kheap.start_address))
	{
		// Unify left
	// If the thing immediately to the left of us is a footer...
		footer_t* test_footer = (footer_t*)((u32int)header - sizeof(footer_t));
		if (test_footer->magic == HEAP_MAGIC &&
			test_footer->header->is_hole == 1)
		{
			u32int cache_size = header->size; // Cache our current size.
			header = test_footer->header;     // Rewrite our header with the new one.
			footer->header = header;          // Rewrite our footer to point to the new header.
			header->size += cache_size;       // Change the size.
			do_add = 0;                       // Since this header is already in the index, we don't want to add it again.		
		}
	}

	if ((header_t*)((u32int)footer + sizeof(footer_t) < kheap.end_address))
	{
		// Unify right
	// If the thing immediately to the right of us is a header...
		header_t* test_header = (header_t*)((u32int)footer + sizeof(footer_t));
		if (test_header->magic == HEAP_MAGIC &&
			test_header->is_hole)
		{
			header->size += test_header->size; // Increase our size.

			footer_t* test_footer = (footer_t*)((u32int)test_header + test_header->size - sizeof(footer_t));

			footer = test_footer;
			footer->header = header;

			// Find and remove this header from the index.
			u32int iterator = 0;
			while ((iterator < kheap.index.size) &&
				(lookup_ordered_array(iterator, &kheap.index) != (void*)test_header))
				iterator++;

			// Make sure we actually found the item.
			SKY_ASSERT(iterator < kheap.index.size, "iterator < heap->index.size");
			// Remove it.
			remove_ordered_array(iterator, &kheap.index);


		}
	}

	// If required, add us to the index.
	if (do_add == 1)
	{
		ordered_array_t* hole_array = &kheap.index;
		insert_ordered_array((void*)header, hole_array);
	}
}



u32int kmalloc_p(u32int sz, u32int* phys)
{
	return kmalloc_int(sz, 0, phys);
}

u32int kmalloc_ap(u32int sz, u32int* phys)
{
	return kmalloc_int(sz, 1, phys);
}

void* kcalloc(size_t count, size_t size)
{
	void* ptr = (void*)kmalloc((u32int)count * size);
	memset(ptr, 0, (u32int)count * size);
	return ptr;
}

void* kmalloc(size_t size)
{
	return (void*)kmalloc_int(size, 0, 0);
}

size_t kmalloc_size(void* ptr)
{
	InterruptDisabler disabler;
	header_t* header = (header_t*)((char*)ptr - sizeof(header_t));
	size_t ptrSize = header->size - sizeof(header_t) - sizeof(footer_t);

	SKY_ASSERT(ptrSize > 0, "malloc_size must return more 0!!");

	return ptrSize;
}

//메모리 크기를 재조정한다.
void* krealloc(void* ptr, size_t size) //메모리 크기를 재조정한다.
{
	if (size == 0)
		return 0;

	InterruptDisabler disabler;
	void* _new;
	if (!ptr) { //기존 포인터가 널이면 size 크기의 새 버퍼를 생성하고 리턴한다.
		_new = (void*)kmalloc(size);
		if (!_new) { goto error; }
	}
	else
	{
		size_t real_size = kmalloc_size(ptr);
		if (real_size > size)
			real_size = size;

		_new = (void*)kmalloc(size);
		if (!_new)
		{
			goto error;
		}
		memcpy(_new, ptr, real_size);
		kfree(ptr);
	}

	return _new;
error:
	SKY_ASSERT(0, "krealloc null!!");

	return NULL;
}

HEAP_API void* kmalloc_aligned(size_t size, size_t alignment)
{
	return (void*)kmalloc_int(size, 1, 0);
}

HEAP_API void  kfree_aligned(void* ptr)
{
	kfree(ptr);
}

static s32int find_smallest_hole(u32int size, u8int page_align, heap_t* heap)
{
	// Find the smallest hole that will fit.
	u32int iterator = 0;
	while (iterator < heap->index.size)
	{
		header_t* header = (header_t*)lookup_ordered_array(iterator, &heap->index);
		// If the user has requested the memory be page-aligned
		if (page_align > 0)
		{
			// Page-align the starting point of this header.
			u32int location = (u32int)header;
			s32int offset = 0;
			if (((location + sizeof(header_t)) & 0xFFFFF000) != 0)
				offset = 0x1000 /* page size */ - (location + sizeof(header_t)) % 0x1000;
			s32int hole_size = (s32int)header->size - offset;
			// Can we fit now?
			if (hole_size >= (s32int)size)
				break;
		}
		else if (header->size >= size)
			break;
		iterator++;
	}
	// Why did the loop exit?
	if (iterator == heap->index.size)
		return -1; // We got to the end and didn't find anything.
	else
		return iterator;
}

static s8int header_t_less_than(void* a, void* b)
{
	return (((header_t*)a)->size < ((header_t*)b)->size) ? 1 : 0;
}

void  kmalloc_init(size_t base, size_t heap_size)
{
	size_t end_addr = base + heap_size;
	// All our assumptions are made on startAddress and endAddress being page-aligned.
	SKY_ASSERT(base % 0x1000 == 0, "start % 0x1000 == 0");
	SKY_ASSERT((base + heap_size) % 0x1000 == 0, "end_addr % 0x1000 == 0");

	// Initialize the index.
	kheap.index = place_ordered_array((void*)base, HEAP_INDEX_SIZE, &header_t_less_than);

	// Shift the start address forward to resemble where we can start putting data.
	base += sizeof(type_t) * HEAP_INDEX_SIZE;

	// Make sure the start address is page-aligned.
	if ((base & 0x00000FFF) != 0)
	{
		base &= 0xFFFFF000;
		base += 0x1000;

	}
	// Write the start, end and max addresses into the heap structure.
	kheap.start_address = base;
	kheap.end_address = end_addr;
	kheap.max_address = end_addr;
	kheap.supervisor = 1;
	kheap.readonly = 0;

	// We start off with one large hole in the index.
	header_t* hole = (header_t*)base;
	hole->size = end_addr - base;
	hole->magic = HEAP_MAGIC;
	hole->is_hole = 1;

	footer_t* footer = (footer_t*)((uint32)base + hole->size - sizeof(footer_t));
	footer->magic = HEAP_MAGIC;
	footer->header = hole;

	insert_ordered_array((void*)hole, &kheap.index);

}

void* memory_alloc(u32int size, u8int page_align, heap_t* heap)
{
	InterruptDisabler disabler;

	// Make sure we take the size of header/footer into account.
	u32int new_size = size + sizeof(header_t) + sizeof(footer_t);

	// Find the smallest hole that will fit.
	s32int iterator = find_smallest_hole(new_size, page_align, heap);

	if (iterator == -1)
		SKY_ASSERT(0, "memory_alloc fail!!");

	header_t* orig_hole_header = (header_t*)lookup_ordered_array(iterator, &heap->index);

	SKY_ASSERT(orig_hole_header != 0, "ADVVVVV");

	u32int orig_hole_pos = (u32int)orig_hole_header;
	u32int orig_hole_size = orig_hole_header->size;
	// Here we work out if we should split the hole we found into two parts.
	// Is the original hole size - requested hole size less than the overhead for adding a new hole?
	if (orig_hole_size - new_size <= sizeof(header_t) + sizeof(footer_t))
	{
		// Then just increase the requested size to the size of the hole we found.
		//size += orig_hole_size - new_size;
		new_size = orig_hole_size;
	}

	// If we need to page-align the data, do it now and make a new hole in front of our block.
	if (page_align && (orig_hole_pos & 0xFFFFF000))
	{
		//SKY_ASSERT(0, "SDFDSSFD");
		u32int new_location = orig_hole_pos + 0x1000 /* page size */ - (orig_hole_pos & 0xFFF) - sizeof(header_t);
		header_t* hole_header = (header_t*)orig_hole_pos;
		hole_header->size = 0x1000 /* page size */ - (orig_hole_pos & 0xFFF) - sizeof(header_t);
		hole_header->magic = HEAP_MAGIC;
		hole_header->is_hole = 1;
		footer_t* hole_footer = (footer_t*)((u32int)new_location - sizeof(footer_t));
		hole_footer->magic = HEAP_MAGIC;
		hole_footer->header = hole_header;
		orig_hole_pos = new_location;
		orig_hole_size = orig_hole_size - hole_header->size;
	}
	else
	{
		// Else we don't need this hole any more, delete it from the index.
		remove_ordered_array(iterator, &heap->index);
	}

	// Overwrite the original header...
	header_t* block_header = (header_t*)orig_hole_pos;
	block_header->magic = HEAP_MAGIC;
	block_header->is_hole = 0;
	block_header->size = new_size;
	// ...And the footer
	footer_t* block_footer = (footer_t*)((uint32)orig_hole_pos - sizeof(footer_t) + block_header->size);
	block_footer->magic = HEAP_MAGIC;
	block_footer->header = block_header;

	// We may need to write a new hole after the allocated block.
	// We do this only if the new hole would have positive size...
	if (orig_hole_size - new_size  > sizeof(header_t) + sizeof(footer_t))
	{
		header_t* hole_header = (header_t*)((uint32)orig_hole_pos + new_size);
		hole_header->magic = HEAP_MAGIC;
		hole_header->is_hole = 1;
		hole_header->size = orig_hole_size - new_size;
		footer_t* hole_footer = (footer_t*)((u32int)hole_header + hole_header->size - sizeof(footer_t));
		if ((u32int)hole_footer < heap->end_address)
		{
			hole_footer->magic = HEAP_MAGIC;
			hole_footer->header = hole_header;
		}
		// Put the new hole in the index;
		insert_ordered_array((void*)hole_header, &heap->index);
	}

	if (heap == &kheap)
		g_usedHeapSize += new_size;

	// ...And we're done!
	return (void*)((u32int)block_header + sizeof(header_t));
}

