#include <stdio.h>
#include <stdlib.h>
#include <string.h> // REMOVE THIS
#include "mymalloc.h"

typedef struct flist {
  int size;
  struct flist *flink;
  struct flist *blink;
} *Flist;

Flist FL;

void *my_malloc(size_t size) {
  Flist setb, set, rem;
  void *beg, *ret;
  int pad, t;

  if (FL == NULL) {
    FL = (Flist) sbrk(8192);
    FL->size = 8192;
    FL->flink = NULL;
    FL->blink = NULL;
  }

  if ((size % 8) == 0) {
    pad = size + 8;
  } else {
    pad = size + 8 + (8 - (size % 8));
  }

  // if freeList is more than one element
  if (FL->flink != NULL) {
    set = NULL;
    for (set = free_list_begin(); set->size < pad; set = free_list_next(set)) {}
    if (set->size < pad) printf("more than one element, but none of them big enough\n");

    beg = (void *) set;
    ret = beg;
    beg += pad;
    rem = (Flist) beg;
    //setb->flink = rem;
  } else if (FL->size > pad) {
    // FL is one element and is big enough
    set = FL;
    beg = (void *) set;
    ret = beg;
    beg += pad;
    rem = (Flist) beg;

    //    printf("\nsetb: 0x%x\nset: 0x%x\nret: 0x%x\nbeg: 0x%x\nrem: 0x%x\n", setb, set, ret, beg, rem);

    //    setb = NULL;
  } else if (FL->size == pad) {
    printf("just big enough\n"); 
  } else {
    // FL is not big enough
    printf("need to call sbrk() to make bigger heap\n");
  }
  //  printf("After if/else if\n");

  setb = NULL;
  t = set->size;
  set->size = pad;
  if (set->blink != NULL) {
    setb = set->blink;
    set->blink->flink = set->flink;
  }
  set->flink = NULL;
  set->blink = NULL;
  if((beg - pad) == FL) {
    FL = beg;
  }
  rem->size = t - pad;
  rem->flink = NULL;
  rem->blink = setb;
  if (setb != NULL) setb->flink = rem;
  //if (setb != NULL) {
  //setb->flink = rem;
  // }
  ret += 8;

  return ret;
} // end of my_malloc()

void my_free(void *ptr) {
  Flist f, tmp;

  ptr -= 8;

  f = ptr;

  //  printf("f->size: %d\n", f->size);

  for (tmp = (Flist) free_list_begin(); tmp != NULL; tmp = (Flist) free_list_next(tmp)) {
    if (tmp > f) {
      // found the next node after ptr in the FL
      f->flink = tmp;
      if (tmp->blink != NULL) tmp->blink->flink = f;
      f->blink = tmp->blink;
      tmp->blink = f;

      if (FL == tmp) FL = f;
      
      return;
    }
  }
} // end of my_free()

void *free_list_begin() {
  return (void *) FL;
}

void *free_list_next(void *node) {
  Flist n;
  n = (Flist) node;
  return (void *) n->flink;
}

void coalesce_free_list() {

}

void double_check_memory(int **ptrs, int *dc, int nptrs, int fl_size)
{
  void *low, *l;
  void *high, *h;
  int *ip, i;
  int nbytes;
  int nfl;

  nbytes = 0;

  for (i = 0; i < nptrs; i++) {
    l = (void *) ptrs[i];
    l -= 8;
    ip = (int *) l;

    if (*ip != dc[i]) {
      printf("Error: pointer number %d the wrong size (%d instead of %d)\n", i, *ip, dc[i]);
      exit(1);
    }
    h = l + *ip;
    if (nbytes == 0 || l < low) low = l;
    if (nbytes == 0 || h > high) high = h;
    nbytes += *ip;
  }

  nfl = 0;
  for (l = free_list_begin(); l != NULL; l = free_list_next(l)) {
    ip = (int *) l;
    h = l + *ip;
    if (nbytes == 0 || l < low) low = l;
    if (nbytes == 0 || h > high) high = h;
    nbytes += *ip;
    nfl++;
  }

  if (nbytes != 8192) {
    printf("Error: Total bytes allocated and on the free list = %d, not 8192\n", nbytes);
    exit(0);
  }

  if (high - low != 8192) {
    printf("Error: Highest address (0x%x) minus lowest (0x%x) does not equal 8192\n", (int) high, (int) low);
    exit(0);
  }

  if (nfl != fl_size && fl_size != -1) {
    printf("Error: %d nodes on the free list -- should be %d\n", nfl, fl_size);
    exit(0);
  }
}


main()
{
  int *ptrs[2];
  int dc[2];

  ptrs[0] = my_malloc(23);
  dc[0] = 32;
  ptrs[1] = my_malloc(8145);
  dc[1] = 8160;

  double_check_memory(ptrs, dc, 2, 0);
  printf("Correct\n");
}

