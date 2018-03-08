#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mymalloc.h"

typedef struct flist {
   int size;
   struct flist *flink;
} *Flist;

Flist FL;

void *my_malloc(size_t size) {
   Flist tmp, set;
   void *mallocBegin;
   void *ret;
   int pad, r, t;

   if (FL == NULL) {
      FL = (Flist) sbrk(8192);
      FL->size = 8192;
      FL->flink = NULL;
   }


   r = size % 8;
   if (r == 0) {
      pad = size + 8;
   } else {
      pad = size + 8 + (8 - r);
   }

   mallocBegin = NULL;
   for (tmp = free_list_begin(); tmp != NULL; tmp = free_list_next(tmp)) {
      if (tmp->size >= pad) {
         mallocBegin = (void *) tmp;
         break;
      }
   }

   if (mallocBegin == NULL) {
      printf("Need to call sbrk() and append it to the end of the currently allocated memory\n");
   }

   set = tmp;
   ret = mallocBegin;
   mallocBegin += pad;

   t = set->size;
   set->size = pad;
   set->flink = NULL;
   if((mallocBegin - pad) == FL) FL = mallocBegin;
   set = mallocBegin;
   tmp = mallocBegin;
   set->size = t - pad;
   set->flink = NULL;
   ret += 8;

   return ret;
}

void my_free(void *ptr) {
   Flist f, tmp;

   ptr -= 8;

   f = ptr;

   printf("f->size: %d\n", f->size);
   
   for (tmp = (Flist) free_list_begin(); tmp != NULL; tmp = (Flist) free_list_next(tmp)) {
      if (tmp->flink == NULL) {
         printf("Reached end of list\n");
         f->flink = tmp;
         //f->size += tmp->size;
         //tmp->size = 0;
         FL = f;
         return;
      }
      printf("block address: 0x%x\n", tmp);
      printf("   block size: %d\n", tmp->size);
      printf("   block flink: 0x%x\n", tmp->flink);
   }


}

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
   Flist thing;

   nbytes = 0;

   for (i = 0; i < nptrs; i++) {
      l = (void *) ptrs[i];
      thing = (Flist) ptrs[i];
      printf("size of alloc'ed  node: %d\n", thing->size);
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
      thing = (Flist) l;
      printf("size of free list node: %d\n", thing->size);
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
   ptrs[1] = my_malloc(101);
   my_free(ptrs[1]);
   ptrs[1] = my_malloc(108);
   dc[1] = 120;

   double_check_memory(ptrs, dc, 2, 2);
   printf("Correct\n");
}

