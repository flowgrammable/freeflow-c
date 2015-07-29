// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#include "hash.h"
#include "util.h"


/* List of pirme numbers whose values are at least twice
   as large as the preceeding value. */
static size_t const primes[] = {
        53,          97,       193,       389,       769,
      1543,        3079,      6151,     12289,     24593, 
     49157,       98317,    196613,    393241,    786433, 
   1572869,     3145739,   6291469,  12582917,  25165843,
  50331653,   100663319, 201326611, 402653189, 805306457,
  1610612741,
};
static size_t const nprimes = sizeof(primes) / sizeof(size_t);


/* Return the next prime number larger that the given
   value. If there is no next larger prime, returns 0.

  TODO: This is an upper bound problem. Use a binary
  search! */
static size_t 
next_prime(size_t n)
{
  for (int i = 0; i < nprimes; ++i)
    if (primes[i] > n)
      return primes[i];
  return 0;
}


/* Compute the hash of a multi-byte string using the 
   one-at-a-time algorithm shown in the web page below:

     http://www.burtleburtle.net/bob/hash/doobs.html 

   FIXME: This needs to return a size_t.

   TODO: This currently uses a simple hash algorithm. We should
   probably replace this with CityHash or something more
   effective. Note that CityHash relies on CRC32 functions
   present in SSE4.2, so that could be very fast. CityHahsh
   is published here:

     https://code.google.com/p/cityhash/ 

   Alternatively, we could use 

   FIXME: The one-at-a-time hash generates a 32-bit key,
   meaning we're not effectively hashing the data.  */
static uint32_t
one_at_a_time_hash(unsigned char const* key, size_t len)
{
    size_t hash = 0, i = 0;
    for( ; i < len; ++i)
    {
        hash += key[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}


/* Compute the hash value of a pointer. */
size_t
fp_pointer_hash(uintptr_t p)
{
  return (size_t)p;
}


/* Compute the hash value of an unsigned integer. */
size_t
fp_uint_hash(uintptr_t n)
{
  return (size_t)n;
}


/* Compute the hash value of a string. */
size_t
fp_string_hash(uintptr_t x)
{
  char const* s = (char const*)x;
  return one_at_a_time_hash((unsigned char*)s, strlen(s));
}


/* Compare pointers for equality. */
bool
fp_pointer_eq(uintptr_t a, uintptr_t b)
{
  return a == b;
}


/* Compare unsinged integers for equality. */
bool
fp_uint_eq(uintptr_t a, uintptr_t b)
{
  return a == b;
}


/* Compare two strings for equality. */
bool
fp_string_eq(uintptr_t a, uintptr_t b)
{
  return !strcmp((char const*)a, (char const*)b);
}


/* Compute the hash index for the given key.

   TODO: This should be a macro. I presume that it's well
   defined for any hash table. */
static inline size_t
hash_index(struct fp_chained_hash_table const* t, uintptr_t k)
{
  return t->hash(k) % t->buckets;
}


/* Allocate a new separately chained hash table with thes
   specified number buckets (which should be a prime number),
   and the given hash and equality comparison functions. */
struct fp_chained_hash_table*
fp_chained_hash_table_new(size_t buckets, fp_hash_fn hash, fp_compare_fn comp)
{
  struct fp_chained_hash_table* t = fp_allocate(struct fp_chained_hash_table);
  t->size = 0;
  t->buckets = buckets;
  t->hash = hash;
  t->comp = comp;
  t->data = fp_allocate_n(struct fp_chained_hash_entry*, buckets);
  memset(t->data, 0, sizeof(struct fp_chained_hash_entry*) * buckets);
  return t;
}


/* Delete a separately chained hash table. This function is
   not responsible for deleting the contents of the table. */
void
fp_chained_hash_table_delete(struct fp_chained_hash_table* t)
{
  if (!t)
    return;
  fp_deallocate(t->data);
  fp_deallocate(t);
}


/* Returns the load factor for the table. */
double
fp_chained_hash_table_load(struct fp_chained_hash_table const* t)
{
  return (double)t->buckets / (double)t->size;
}


/* Search for the given key within the hash table. This returns
   a pointer to the mapped value if the key exists in the
   table, or NULL if it does not. */
struct fp_chained_hash_entry*
fp_chained_hash_table_find(struct fp_chained_hash_table const* t, uintptr_t k)
{
  int n = hash_index(t, k);
  struct fp_chained_hash_entry* p = t->data[n];
  if (!p)
    return p;

  /* TODO: This is where we could use a sorted sequence
     instead of a chained list. */
  while (p && !t->comp(p->key, k))
    p = p->next;

  return p;
}


/* Allocate a new hash entry. */
static struct fp_chained_hash_entry*
fp_chained_hash_entry_new(uintptr_t k, uintptr_t v)
{
  struct fp_chained_hash_entry* ent = fp_allocate(struct fp_chained_hash_entry);
  ent->key = k;
  ent->value = v;
  ent->next = NULL;
  return ent;
}


/* Deallocate a chained hash entry. This does not release
   memory associated with the key or value.  */
static void
fp_chained_hash_entry_delete(struct fp_chained_hash_entry* ent)
{
  fp_deallocate(ent);
}


/* Insert the hash entry into a bucket by prepending it
   to the entry chain. */
static struct fp_chained_hash_entry*
fp_chained_hash_entry_insert(struct fp_chained_hash_entry** head, 
                             uintptr_t k, 
                             uintptr_t v)
{
  struct fp_chained_hash_entry* ent = fp_chained_hash_entry_new(k, v);
  ent->next = *head;
  *head = ent;
  return *head;
}


/* Resize the hash table. If the hash table is maximally
   sized, no action is taken. */
void
fp_chained_hash_table_resize(struct fp_chained_hash_table* t1)
{
  size_t n = next_prime(t1->buckets);
  if (n == 0)
    return;

  /* Build a new hash table and all kv pairs of t into
     the new table. */
  struct fp_chained_hash_table* t2 = fp_chained_hash_table_new(n, t1->hash, t1->comp);
  for (int i = 0; i < t1->buckets; ++i) {
    struct fp_chained_hash_entry* p = t1->data[i];
    while (p) {
      fp_chained_hash_table_insert(t2, p->key, p->value);
      p = p->next;
    }
  }

  /* Move the essential information from t2 into t1 and get 
     rid of the temporary hash table. */
  fp_deallocate(t1->data);
  t1->buckets = n;
  t1->data = t2->data;
  fp_deallocate(t2);
}


/* Insert the the given key/value pair into the hash table,
   returning a pointer to the inserted value structure.

   TODO: Implement dynamic resize, rehash. */
struct fp_chained_hash_entry*
fp_chained_hash_table_insert(struct fp_chained_hash_table* t, 
                             uintptr_t k, 
                             uintptr_t v)
{
  /* Resize if we hit the high-water mark. */
  if (fp_chained_hash_table_load(t) >= .75)
    fp_chained_hash_table_resize(t);

  int n = hash_index(t, k);
  ++t->size;
  if (!t->data[n])
    return t->data[n] = fp_chained_hash_entry_new(k, v);
  else
    return fp_chained_hash_entry_insert(&t->data[n], k, v);
}


/* Remove the key from the table.  */
void 
fp_chained_hash_table_remove(struct fp_chained_hash_table* t, uintptr_t k)
{
  int n = hash_index(t, k);
  struct fp_chained_hash_entry* p = t->data[n];

  /* Find the first instance of the key to remove. */
  struct fp_chained_hash_entry* q = NULL;
  while (p && !t->comp(p->key, k)) {
    q = p;
    p = p->next;
  }

  /* The key does not exist. Do nothing. */
  if (!p)
    return;

  /* Stitch around the node being removed, possibly
     making a new head for the bucket. */
  if (q)
    q->next = p->next;
  else
    t->data[n] = p->next;
  --t->size;

  /* Reclaim memory. */
  fp_chained_hash_entry_delete(p);
}
