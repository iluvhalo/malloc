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
      } else*/ if ((set->size < pad) && (set->flink == NULL )) {
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
  rem->size = t - pad;
  if (set->blink != NULL) {
    setb = set->blink;
    if (t == pad) {
      setb->flink = set->flink;
    }
    /*if (set->flink != NULL) {
      //setb->flink = set->flink;
    }*/ else {
      setb->flink = rem;
    }
  }
  if (set->flink != NULL) {
    rem->flink = set->flink;
    rem->flink->blink = rem;
  } else {
    rem->flink = NULL;
  }
  set->flink = NULL;
  set->blink = NULL;
  if((beg - pad) == FL) {
    FL = beg;
  }
  //rem->size = t - pad;
  //rem->flink = NULL;
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
