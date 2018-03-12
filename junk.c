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
  int *ptrs[45];
  int *free_ptrs[18];
  int dc[45];

  ptrs[6] = my_malloc(129); dc[6] = 144;
  ptrs[17] = my_malloc(136); dc[17] = 144;
  free_ptrs[2] = my_malloc(150);
  free_ptrs[3] = my_malloc(131);
  free_ptrs[5] = my_malloc(128);
  free_ptrs[0] = my_malloc(125);
  free_ptrs[11] = my_malloc(117);
  free_ptrs[8] = my_malloc(125);
  free_ptrs[16] = my_malloc(114);
  ptrs[2] = my_malloc(117); dc[2] = 128;
  free_ptrs[13] = my_malloc(138);
  ptrs[4] = my_malloc(122); dc[4] = 136;
  free_ptrs[4] = my_malloc(134);
  free_ptrs[1] = my_malloc(116);
  ptrs[5] = my_malloc(136); dc[5] = 144;
  free_ptrs[10] = my_malloc(147);
  free_ptrs[7] = my_malloc(148);
  free_ptrs[6] = my_malloc(128);
  ptrs[0] = my_malloc(151); dc[0] = 160;
  ptrs[7] = my_malloc(116); dc[7] = 128;
  free_ptrs[14] = my_malloc(132);
  ptrs[1] = my_malloc(128); dc[1] = 136;
  ptrs[10] = my_malloc(121); dc[10] = 136;
  ptrs[14] = my_malloc(126); dc[14] = 136;
  free_ptrs[15] = my_malloc(141);
  ptrs[16] = my_malloc(129); dc[16] = 144;
  ptrs[8] = my_malloc(118); dc[8] = 128;
  ptrs[15] = my_malloc(142); dc[15] = 152;
  ptrs[11] = my_malloc(142); dc[11] = 152;
  ptrs[12] = my_malloc(139); dc[12] = 152;
  ptrs[13] = my_malloc(142); dc[13] = 152;
  ptrs[3] = my_malloc(148); dc[3] = 160;
  ptrs[9] = my_malloc(128); dc[9] = 136;
  free_ptrs[17] = my_malloc(113);
  free_ptrs[9] = my_malloc(124);
  free_ptrs[12] = my_malloc(144);

  my_free(free_ptrs[3]);
  my_free(free_ptrs[17]);
  my_free(free_ptrs[10]);
  my_free(free_ptrs[15]);
  my_free(free_ptrs[9]);
  my_free(free_ptrs[16]);
  my_free(free_ptrs[13]);
  my_free(free_ptrs[7]);
  my_free(free_ptrs[11]);
  my_free(free_ptrs[6]);
  my_free(free_ptrs[14]);
  my_free(free_ptrs[5]);
  my_free(free_ptrs[0]);
  my_free(free_ptrs[2]);
  my_free(free_ptrs[12]);
  my_free(free_ptrs[4]);
  my_free(free_ptrs[8]);
  my_free(free_ptrs[1]);

  ptrs[23] = my_malloc(81); dc[23] = 96;
  ptrs[19] = my_malloc(88); dc[19] = 96;
  ptrs[25] = my_malloc(83); dc[25] = 96;
  ptrs[31] = my_malloc(78); dc[31] = 88;
  ptrs[18] = my_malloc(84); dc[18] = 96;
  ptrs[30] = my_malloc(95); dc[30] = 104;
  ptrs[38] = my_malloc(73); dc[38] = 88;
  ptrs[24] = my_malloc(78); dc[24] = 88;
  ptrs[33] = my_malloc(90); dc[33] = 104;
  ptrs[39] = my_malloc(93); dc[39] = 104;
  ptrs[34] = my_malloc(90); dc[34] = 104;
  ptrs[22] = my_malloc(90); dc[22] = 104;
  ptrs[21] = my_malloc(79); dc[21] = 88;
  ptrs[26] = my_malloc(87); dc[26] = 96;
  ptrs[43] = my_malloc(95); dc[43] = 104;
  ptrs[44] = my_malloc(82); dc[44] = 96;
  ptrs[42] = my_malloc(85); dc[42] = 96;
  ptrs[37] = my_malloc(89); dc[37] = 104;
  ptrs[36] = my_malloc(79); dc[36] = 88;
  ptrs[41] = my_malloc(81); dc[41] = 96;
  ptrs[40] = my_malloc(75); dc[40] = 88;
  ptrs[27] = my_malloc(94); dc[27] = 104;
  ptrs[28] = my_malloc(73); dc[28] = 88;
  ptrs[29] = my_malloc(85); dc[29] = 96;
  ptrs[35] = my_malloc(84); dc[35] = 96;
  ptrs[20] = my_malloc(74); dc[20] = 88;
  ptrs[32] = my_malloc(77); dc[32] = 88;

  double_check_memory(ptrs, dc, 45, 19);
  printf("Correct\n");
}

