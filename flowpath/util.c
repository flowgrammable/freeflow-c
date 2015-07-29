// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#include "util.h"


/* Allocate and initialize a new ring. */
struct fp_ring*
fp_ring_new(int num)
{
  // TODO: fp_allocate_n() doesn't support flexible array members
  struct fp_ring* r = malloc(sizeof(struct fp_ring) + num * sizeof(void*));
  r->num = num;
  fp_ring_init(r);
  return r;
}


/* Destroy a ring. */
void
fp_ring_delete(struct fp_ring* r)
{
  free(r);
}


/* Reset the ring so that it is empty. */
void
fp_ring_init(struct fp_ring* r)
{
  r->head = r->tail = 0;
}


/* Push a single item into the FIFO. */
int
fp_ring_push(struct fp_ring* r, void* x)
{
//  printf("%s head=%d tail=%d\n", __func__, r->head, r->tail);
  int count = fp_ring_count(r);
  if (count < (r->num - 1)) {  // always leave 1 slot free
    r->ring[r->tail++] = x;
    if (r->tail == r->num) {
      r->tail = 0;
    }
    return 1;
  }
  return 0;
}


/* Push multiple items into the FIFO. */
int
fp_ring_push_n(struct fp_ring* r, void* x[], int n)
{
  int free = fp_ring_free(r) - 1;  // always leave one slot free
  int items = (n <= free) ? n : free;  // min(n, free);
  for (int i = 0; i < items; i++) {
    r->ring[r->tail++] = x[i];
    if (r->tail == r->num) {
      r->tail = 0;
    }
  }
  return items;
}


/* Pop the front element of the FIFO, returning a pointer.
 * If empty, return NULL. */
void*
fp_ring_pop(struct fp_ring* r)
{
//  printf("%s head=%d tail=%d\n", __func__, r->head, r->tail);
  if (r->head != r->tail) {
    void* ret = r->ring[r->head++];
    if (r->head == r->num) {
      r->head = 0;
    }
    return ret;
  }
  return NULL;
}


/* Pop multiple items from the FIFO. */
int
fp_ring_pop_n(struct fp_ring* r, void* x[], int n)
{
  int count = fp_ring_count(r);
  int items = (n <= count) ? n : count;  // min(n, free);
  for (int i = 0; i < items; i++) {
    x[i] = r->ring[r->head++];
    if (r->head == r->num) {
      r->head = 0;
    }
  }
  return items;
}


/* Get the front element of the FIFO, returning a pointer.
 * If empty, return NULL. */
void*
fp_ring_top(const struct fp_ring* r)
{
  int count = fp_ring_count(r);
  return (count > 0) ? r->ring[r->head] : NULL;
}


/* Get multiple items from the FIFO without popping.
 * Returns count with items into array
 */
int
fp_ring_top_n(struct fp_ring* r, void* x[], int n)
{
  int count = fp_ring_count(r);
  int items = (n <= count) ? n : count;  // min(n, free);
  int tmp_head = r->head;
  for (int i = 0; i < items; i++) {
    x[i] = r->ring[tmp_head++];
    if (tmp_head == r->num) {
      tmp_head = 0;
    }
  }
  return items;
}


/* Remove multiple items from the FIFO without returning.
 * - Only makes sense to follow a fp_ring_top_n(). */
int
fp_ring_remove_n(struct fp_ring* r, int n)
{
  int count = fp_ring_count(r);
  int items = (n <= count) ? n : count;  // min(n, free);
  r->head += items;
  if (r->head >= r->num) {
    r->head -= r->num;
  }
  return items;
}


/* Return the number of free elements in the FIFO. */
inline int
fp_ring_count(const struct fp_ring* r)
{
  int count = r->tail - r->head;
  if (count < 0) {
    count += r->num;
  }
//  printf("count=%d\n", count);
  return count;
}


/* Return the number of elements in the FIFO. */
inline int
fp_ring_free(const struct fp_ring* r)
{
  int count = fp_ring_count(r);
  int free = r->num - count;
//  printf("free=%d\n", free);
  return free;
}

