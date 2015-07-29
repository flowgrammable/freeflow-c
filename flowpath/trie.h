// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#ifndef FLOWPATH_TRIE_H
#define FLOWPATH_TRIE_H

/* This module defines a trie implementation for flow tables. 
   This maps packet keys to positions in the instruction memory
   using prefix matches.

   TODO: Determine the best branching level for the trie.

   TODO: This does not yet account for priorities. As with the
   the hash table, we might chain each node to an array of
   k/v pairs ordered on priority. */

#include "packet.h"

/* A node in the trie. */
struct fp_trie_node {
  int key;
  int value;
};

/* The trie */
struct fp_trie {
  struct fp_trie_node* root;
};

struct fp_trie* fp_trie_new();
struct fp_trie_node* fp_trie_find(struct fp_trie* t, struct fp_packet_subkey* k);
struct fp_trie_node* fp_trie_insert(struct fp_trie* t, struct fp_packet_subkey* k, int v);
void fp_trie_remove(struct fp_trie* t, struct fp_packet_subkey* k);

#endif
