#include <stdio.h>
#include <stdlib.h>
#include "mymalloc.h"

typedef struct flist {
   int size;
   struct flist *flink;
} *Flist;

Flist freeList;

void *my_malloc(size_t size) {
   Flist mallocBegin;
   char *mallocEnd;
   int pad, r;

   if (freeList->flink == NULL) {
      mallocBegin = (Flist) sbrk(8192);
      freeList->size = 0;
      freeList->flink = mallocBegin;
      mallocBegin->size = 8192;
      mallocBegin->flink = freeList;
   }



   r = size % 8;
   if (r == 0) {
      pad = size + 8;
   } else {
      pad = size + 8 + (8 - r);
   }
   printf("memory to allocate: %d\n", pad);

   printf("mallocBegin: 0x%x\n", mallocBegin);
   printf("mallocEnd: 0x%x\n", mallocEnd);

   return (void *) freeList;
}

void my_free(void *ptr) {

}

void *free_list_begin() {
   return (void *) freeList->flink;
}

void *free_list_next(void *node) {
   Flist n;
   n = (Flist) node;
   n = n->flink;
   return (void *) n;
}

void coalesce_free_list() {

}
