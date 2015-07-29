// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#ifndef FREEFLOW_HASH_H
#define FREEFLOW_HASH_H

/* This module provides facilities for defining hash tables. 
   The hash tables defined by this module map from intptrs 
   to intptrs. This provides facilities for defining a wide 
   range of data structures such as:

   - integer ids to object pointers
   - strings to object pointers
   - byte-strings to integers
   - etc. 

   Currently, this module contains hashing features that
   support the following types of hash tables:

   - chained hash table (fp_chained_hash_table)
   - flat hash table (fp_flat_hash_table)

   TODO: Support sepraate chaining with an ordered list
   for the bucket. This would allow us to perform a
   binary search instead of a linear scan.

   TODO: Implement the flat hash table and various probing
   algorithms (linear, quadratic, double hashing). 

   TOOD: Build an iteration interface? */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>


/* A hash function maps a key to an unsigned 
   integer value. */
typedef size_t (*fp_hash_fn)(uintptr_t);


/* An equality comparison function compares two keys
   for equality. */
typedef bool (*fp_compare_fn)(uintptr_t, uintptr_t);


/* A key-value pair stored in the hash table. Each entry
   is chained to subsequent entries stored in the same
   bucket. */
struct fp_chained_hash_entry
{
  uintptr_t key;
  uintptr_t value;
  struct fp_chained_hash_entry* next;
};


/* A hash table using separate chaining. */
struct fp_chained_hash_table {
  size_t        size;    /* Number of elements. */
  size_t        buckets; /* Number of buckets. */
  fp_hash_fn    hash;    /* The hash function. */
  fp_compare_fn comp;    /* Equality comparison. */
  struct fp_chained_hash_entry** data;  /* The array of elements. */
};


struct fp_chained_hash_table* fp_chained_hash_table_new (size_t, fp_hash_fn, fp_compare_fn);
void                          fp_chained_hash_table_delete(struct fp_chained_hash_table*);
double                        fp_chained_hash_table_load(struct fp_chained_hash_table const*);
struct fp_chained_hash_entry* fp_chained_hash_table_find(struct fp_chained_hash_table const*, uintptr_t);
struct fp_chained_hash_entry* fp_chained_hash_table_insert(struct fp_chained_hash_table*, uintptr_t, uintptr_t);
void                          fp_chained_hash_table_remove(struct fp_chained_hash_table*, uintptr_t);

/* Hashing functions. */
size_t fp_pointer_hash(uintptr_t);
size_t fp_uint_hash(uintptr_t);
size_t fp_string_hash(uintptr_t);

/* Equality comparison functions. */
bool fp_pointer_eq(uintptr_t, uintptr_t);
bool fp_uint_eq(uintptr_t, uintptr_t);
bool fp_string_eq(uintptr_t, uintptr_t);

#endif
