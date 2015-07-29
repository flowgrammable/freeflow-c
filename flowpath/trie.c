// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#include "trie.h"
#include "util.h"


/* Allocate a new trie. */
struct fp_trie* 
fp_trie_new()
{
  struct fp_trie* t = fp_allocate(struct fp_trie);
  /* FIXME: Initialize the root node. */
  return t;
}

/* FIXME: This is returning the wrong thing. */
struct fp_trie_node* 
fp_trie_find(struct fp_trie* t, struct fp_packet_subkey* k)
{
  return NULL;
}

/* FIXME: This is returning the wrong thing. */
struct fp_trie_node* 
fp_trie_insert(struct fp_trie* t, struct fp_packet_subkey* k, int v)
{
  return NULL;
}

/* Remove the given key from the packet. */
void
fp_trie_remove(struct fp_trie* t, struct fp_packet_subkey* k)
{
}
