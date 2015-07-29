
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#include "util.h"

int 
main(int argc, char** argv)
{
  const int MAX_INTS = 128;
  int* a[MAX_INTS];
  int* b[MAX_INTS];
  int fail = 0;

  srand(time(NULL));
  for (int i = 0; i < MAX_INTS; i++) {
    a[i] = (int*) rand();
  }

  struct np_ring* r3 = np_ring_new(3);
  struct np_ring* r5 = np_ring_new(5);
  struct np_ring* r8 = np_ring_new(8);

  if (np_ring_push(r3, a[10]) != 1) {fail += 1;
    printf("%d Expected to push a[10] into r3\n", __LINE__);}
  if (np_ring_push(r3, a[11]) != 1) {fail+=1;
    printf("%d Expected to push a[11] into r3\n", __LINE__);}
  if (np_ring_push(r3, a[12]) != 0) {fail+=1;
    printf("%d Expected to fail pushing a[12] into r3\n",__LINE__);}
  if (np_ring_pop(r3) != a[10]) {fail+=1;
    printf("%d Expected to pop a[10] from r3\n", __LINE__);}
  if (np_ring_push(r3, a[13]) != 1) {fail+=1;
    printf("%d Expected to push a[13] into r3\n", __LINE__);}
  if (np_ring_pop(r3) != a[11]) {fail+=1;
    printf("%d Expected to pop a[11] from r3\n", __LINE__);}
  if (np_ring_pop(r3) != a[13]) {fail+=1;
    printf("%d Expected to pop a[13] from r3\n", __LINE__);}
  if (np_ring_pop(r3) != NULL) {fail+=1;
    printf("%d Expected to pop to return NULL from r3\n", __LINE__);}


  if (np_ring_push(r5, a[20]) != 1) {fail += 1;
    printf("%d Expected to push a[20] into r5\n", __LINE__);}
  if (np_ring_push(r5, a[21]) != 1) {fail+=1;
    printf("%d Expected to push a[21] into r5\n", __LINE__);}
  if (np_ring_push(r5, a[22]) != 1) {fail+=1;
    printf("%d Expected to push a[22] into r5\n", __LINE__);}
  if (np_ring_top(r5) != a[20]) {fail+=1;
    printf("%d Expected to top to return a[20] from r5\n", __LINE__);}
  if (np_ring_push(r5, a[23]) != 1) {fail+=1;
    printf("%d Expected to push a[23] into r5\n", __LINE__);}
  if (np_ring_push(r5, a[24]) != 0) {fail+=1;
    printf("%d Expected to fail pushing a[24] into r5\n",__LINE__);}
  if (np_ring_pop(r5) != a[20]) {fail+=1;
    printf("%d Expected to pop a[20] from r5\n", __LINE__);}
  if (np_ring_push(r5, a[25]) != 1) {fail+=1;
    printf("%d Expected to push a[25] into r5\n", __LINE__);}
  if (np_ring_pop(r5) != a[21]) {fail+=1;
    printf("%d Expected to pop a[21] from r5\n", __LINE__);}
  if (np_ring_pop_n(r5, (void**)b, 1) != 1 || b[0]!=a[22]) {fail+=1;
    printf("%d Expected to pop a[22] from r5\n", __LINE__);}
  if (np_ring_pop_n(r5, (void**)b, 3) != 2 || b[0]!=a[23] || b[1]!=a[25]) {
    fail+=1; printf("%d Expected to pop a[23] and a[25] r5\n", __LINE__);}

  if (np_ring_push_n(r8, (void**)&a[30], 10) != 7) {fail += 1;
    printf("%d Expected to push a[30] -> a[36] into r8\n", __LINE__);}
  if (np_ring_top_n(r8, (void**)b, 10) != 7) {fail += 1;
    printf("%d Expected top to return a[30] -> a[36] from r8\n", __LINE__);}
  for (int i = 0; i < 7; i++) {
    if (a[30+i] != b[i]) {fail += 1;
      printf("%d Expected a[%d] == b[%d] from r8\n", __LINE__, 30+i, i);}}
  if (np_ring_remove_n(r8, 7) != 7) {fail += 1;
    printf("%d Expected to remove a[30] -> a[26] from r8\n", __LINE__);}

  if (np_ring_push_n(r8, (void**)&a[40], 6) != 6) {fail += 1;
    printf("%d Expected to push a[40] -> a[45] into r8\n", __LINE__);}
  if (np_ring_pop_n(r8, (void**)b, 14) != 6) {fail += 1;
    printf("%d Expected to pop a[40] -> a[45] from r8\n", __LINE__);}
  for (int i = 0; i < 6; i++) {
    if (a[40+i] != b[i]) {fail += 1;
        printf("%d Expected a[%d] == b[%d] from r8\n", __LINE__, 40+i, i);}}
  if (np_ring_top_n(r8, (void**)b, 10) != 0) {fail += 1;
    printf("%d Expected top to return nothing from r8\n", __LINE__);}
  for (int i = 0; i < 6; i++) {
    if (a[40+i] != b[i]) {fail += 1;
      printf("%d Expected a[%d] == b[%d] from r8\n", __LINE__, 40+i, i);}}
  if (np_ring_push_n(r8, (void**)&a[50], 1) != 1) {fail += 1;
    printf("%d Expected to push a[50] into r8\n", __LINE__);}
  if (np_ring_top_n(r8, (void**)b, 10) != 1) {fail += 1;
    printf("%d Expected top to return nothing from r8\n", __LINE__);}
  if (a[50] != b[0]) {fail += 1;
    printf("%d Expected a[%d] == b[%d] from r8\n", __LINE__, 50, 1);}
  for (int i = 1; i < 6; i++) {
    if (a[40+i] != b[i]) {fail += 1;
    printf("%d Expected a[%d] == b[%d] from r8\n", __LINE__, 40+i, i);}}
  if (a[36] != b[6]) {fail += 1;
    printf("%d Expected a[%d] == b[%d] from r8\n", __LINE__, 36, 7);}

  if (np_ring_push_n(r8, (void**)&a[51], 7) != 6) {fail += 1;
    printf("%d Expected to push a[51] -> a[55] into r8\n", __LINE__);}
  if (np_ring_pop_n(r8, (void**)b, 1) != 1) {fail += 1;
    printf("%d Expected to pop a[50] from r8\n", __LINE__);}
  if (np_ring_pop_n(r8, (void**)b, 12) != 6) {fail += 1;
    printf("%d Expected to pop a[51] -> a[55] from r8\n", __LINE__);}

  np_ring_delete(r3);
  np_ring_delete(r5);
  np_ring_delete(r8);

  printf("%d tests failed!\n", fail);
  return (fail == 0) ? 0 : -1;
}
