// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#ifndef FLOWPATH_UTIL_H
#define FLOWPATH_UTIL_H

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

/* Allocate a single object of type T. */
#define fp_allocate(T) (T*)malloc(sizeof(T))
#define fp_allocate_n(T, N) (T*)malloc(N * sizeof(T))
#define fp_deallocate(p) free(p)

/* FIXME: Deprecate these things. */
#define allocate(T) (T*)malloc(sizeof(T))
#define allocate_n(T, N) (T*)malloc(N * sizeof(T))


/* Circular ring (FIFO) of pointers.  To avoid confusion of empty vs. full
 *   (in both cases head == tail), we can not fill every entry as must leave
 *   1 unused.
 * - head is advanced by removing items from the ring
 * - tail is advanced by adding items to the ring
 * -- i.e.: the head is always trying to catch the tail...
 * - empty when: head == tail
 * - full when: tail is directly behind head (fp_ring_count() == num-1)
 * - fp_ring_count() is calculated: (tail - head + num) % num
 * -- where % num is replaced with a conditional (may or may not be better)
 * -- TODO: if enforced ring to be a power of 2, a simple mask would be all
 *    that is needed.
 * Assuming operating on pointers, thus void* is the handle.  Ideally, this
 *   would be a C++ Template...
 */
struct fp_ring
{
  int head;      // pop from head
  int tail;      // push to tail
  int num;       // number of elements in ring[]
  void* ring[];  // fexible array member (pointer array is apart of struct)
};

struct fp_ring* fp_ring_new(int);
void fp_ring_delete(struct fp_ring*);
void  fp_ring_init(struct fp_ring*);
int   fp_ring_push(struct fp_ring*, void*);
int   fp_ring_push_n(struct fp_ring*, void*[], int);
void* fp_ring_pop(struct fp_ring*);
int   fp_ring_pop_n(struct fp_ring*, void*[], int);
void* fp_ring_top(const struct fp_ring*);
int   fp_ring_top_n(struct fp_ring*, void*[], int);
int   fp_ring_remove_n(struct fp_ring*, int);
int   fp_ring_count(const struct fp_ring*);
int   fp_ring_free(const struct fp_ring*);

#endif
