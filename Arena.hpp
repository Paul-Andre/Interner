#include "allocate.h"

struct Arena {

  long capacity;

  void *data;
  void *ptr;

  int count; // Purely for info/debugging

  void *_align(void *start, size_t alignment) {
    assert(alignment-(alignment&-alignment) == 0 && "alignment needs to be a power of two");
    // Check alignment
    if (((size_t)start&(alignment-1)) != 0) {
      // If unaligned:
      // Zero out the lower bits.
      start = (void*)((size_t)start &~(alignment-1));
      // Bump up the alignment bit.
      start += alignment;
    }
    return start;
  }

  // TODO: create linked list for deallocation?
  void *alloc(size_t length, size_t alignment) {
    if (data == NULL) {
      capacity = get_page_size();
      data = allocate_mmap(capacity);
      ptr = data;
      count = 1;
    }

    if (_align(ptr, alignment) + length <= data+capacity) {
      void *ret = _align(ptr, alignment);
      ptr = _align(ptr, alignment) + length;
      return ret;
      /*
    } else if (length > capacity/16) {
      // Don't waste too much space...
      // In such a case just offload to the default allocator...
      // TODO: does this make sense?
      return calloc(1, length);
      */
    } else {
      capacity = capacity*2;
      data = allocate_mmap(capacity);
      ptr = data + length;
      count += 1;
      printf("Page full, creating a new one %d\n", count);
      return data;
    }
  }
};
