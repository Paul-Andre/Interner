#pragma once
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>

const long get_page_size() {
  long page_size = sysconf(_SC_PAGESIZE);
  return page_size;
}

void *allocate_mmap(size_t size) {
  void *p = mmap(NULL, size, PROT_READ | PROT_WRITE,
      MAP_SHARED | MAP_ANONYMOUS, 0, 0);
  if (p == MAP_FAILED) {
    perror("Allocating memory page failed");
    return NULL;
  }
  return p;
}

// TODO: create a portable fallback version that uses malloc

