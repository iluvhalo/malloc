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
  //printf("pad: %d\n", pad);

  // if freeList is more than one element
  if (FL->flink != NULL) {
    set = NULL;
    // iterates through the Free list and finds the first element that is big enough to malloc
    for (set = free_list_begin(); set->flink != NULL; set = free_list_next(set)) {
      //printf("   set = %d\n", set->size);
      /*if (set->size <= pad) {
        printf("here\n");
        break;
      } else */if ((set->size < pad) && (set->flink == NULL )) {
        // reached the end of the list and none of them were big enough
        // need to call sbrk()
        //printf("need to call sbrk()\n");
//        printf("need to call sbrk on the end of the FL\n");
        set->flink = (Flist) sbrk(8192);
        set->flink->blink = set;
        set = set->flink;
        set->size = 8192;
        set->flink = NULL;
        break;
      } else if ((set->size >= pad) && (set->flink != NULL)) {
//        printf("found a big enough node in the middle of the list\n");
        break;
      }
    }
    if (set == NULL) printf("more than one element, but none of them big enough\n");
    if ((set->flink == NULL) && (set->size < pad)) {
//      printf("need to call sbrk()\n");
      set->flink = (Flist) sbrk(8192);
      set->flink->blink = set;
      set = set->flink;
      set->size = 8192;
      set->flink = NULL;
    }

//    printf("set: %d\n", set->size);

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
    // FL is one element and just big enough
    ret = FL;
    ret += 8;
    FL = NULL;

    return ret;
  } else {
    // FL is not big enough
    //printf("need to call sbrk() to make bigger heap\n");
    if (FL != NULL) {
      set = (Flist) sbrk(8192);
      FL->flink = set;
      set = FL->flink;
      set->size = 8192;
      set->blink = FL;
      beg = (void *) set;
      ret = beg;
      beg += pad;
      rem = (Flist) beg;
    }
  }
  //  printf("After if/else if\n");
//      printf("\nsetb: 0x%x\nset: 0x%x\nret: 0x%x\nbeg: 0x%x\nrem: 0x%x\n", setb, set, ret, beg, rem);
   //   printf("\nset: %d\nret: %d\nbeg: %d\nrem: %d\n", *set, (int *) *ret, (int *) *beg, *rem);

//  printf("set: %d\n", set->size);
  setb = NULL;
  t = set->size;
  set->size = pad;
  if (set->blink != NULL) {
    setb = set->blink;
    if (set->flink != NULL) {
      setb->flink = set->flink;
    } else {
      setb->flink = rem;
    }
  }
  set->flink = NULL;
  set->blink = NULL;
  if((beg - pad) == FL) {
    FL = beg;
  }
  rem->size = t - pad;
  rem->flink = NULL;
  //rem->flink = set->flink;
  rem->blink = setb;
  if (rem->size < 16) set->size += rem->size;
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

  if (FL == NULL) {
    FL = f;
    FL->flink = NULL;
    FL->blink = NULL;
    return;
  }

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
    } else if (tmp->flink == NULL) {
      // at end of FL
      tmp->flink = f;
      f->blink = tmp;
      f->flink = NULL;

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
    /* printf("P: 0x%x 0x%x %d\n", l, h, *ip); */
    if (nbytes == 0 || l < low) low = l;
    if (nbytes == 0 || h > high) high = h;
    nbytes += *ip;
  }

  nfl = 0;
  for (l = free_list_begin(); l != NULL; l = free_list_next(l)) {
    ip = (int *) l;
    h = l + *ip;
    /* printf("F: 0x%x 0x%x %d\n", l, h, *ip);   */
    if (nbytes == 0 || l < low) low = l;
    if (nbytes == 0 || h > high) high = h;
    nbytes += *ip;
    nfl++;
  }

  if (nbytes != 8192*2) {
    printf("Error: Total bytes allocated and on the free list = %d, not 16384\n", nbytes);
    exit(0);
  }

  if (high - low != 8192*2) {
    printf("Error: Highest address (0x%x) minus lowest (0x%x) does not equal 16384\n", (int) high, (int) low);
    exit(0);
  }

  if (nfl != fl_size && fl_size != -1) {
    printf("Error: %d nodes on the free list -- should be %d\n", nfl, fl_size);
    exit(0);
  }
}

main()
{
  int *ptrs[9];
  int *free_ptrs[9];
  int dc[9];
  int i;

  for (i = 0; i < 6; i++) {
    if (i > 0) my_free(free_ptrs[i-1]);
    ptrs[i] = my_malloc(1000+i*16+i%7+1);
    free_ptrs[i] = my_malloc(1000+i*16+i%7+1);
    dc[i] = 1000+i*16+16;
  }
  my_free(free_ptrs[5]);

  double_check_memory(ptrs, dc, 6, 8);
  printf("Correct\n");
}

