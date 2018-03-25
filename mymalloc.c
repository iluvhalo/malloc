// Matt Matto
// Lab 7 - mymalloc.c
//
// COMPILATION - no compilation
// 
// USAGE - no usage
//
// DESCRIPTION - implements malloc and free according the the way Dr. Plank wants us to
//               Also can coalecse free memory blocks into larger single blocks

#include "mymalloc.h"

// basic doubly linked list node
typedef struct flist {
  int size;
  struct flist *flink;
  struct flist *blink;
} *Flist;

// global free list pointer that points to the beginning of my free list
Flist FL;

// given a size returns a pointer to a spot in memory if that size
// if there is no size big enough, calls sbrk() to get more memory from the OS
void *my_malloc(size_t size) {
  Flist set;      // the Flist pointer for creating the memory block to return
  Flist setb;     // the Flist pointer for setting the blink pointer in free list nodes
  Flist rem;      // the Flist pointer for the remaining memory block after breaking off the return memory block
  void *beg;      // a void * pointer that is equivalent to rem
  void *ret;      // the return pointer
  int pad, t;     // pad is the amount of memory to pad size to, and t is just a temp value for setting the free list node's size

  // finds how much to pad the size to
  if ((size % 8) == 0) {
    pad = size + 8;
  } else {
    pad = size + 8 + (8 - (size % 8));
  }

  // if ther is no free list, the sbrk() a new list
  if (FL == NULL) {
    FL = (Flist) sbrk(find_sbrk(pad));
    FL->size = find_sbrk(pad);
    FL->flink = NULL;
    FL->blink = NULL;
  }

  // if freeList is more than one element
  if (FL->flink != NULL) {
    set = NULL;
    
    // iterates through the Free list and finds the first element that is big enough to malloc
    for (set = free_list_begin(); set->flink != NULL; set = free_list_next(set)) {
      if ((set->size < pad) && (set->flink == NULL )) {
        // reached the end of the list and none of them were big enough
        set->flink = (Flist) sbrk(find_sbrk(pad));
        set->flink->blink = set;
        set = set->flink;
        set->size = find_sbrk(pad);
        set->flink = NULL;
        break;
      } else if ((set->size >= pad) && (set->flink != NULL)) {
        // found a node in the middle of the list that is big enough
        break;
      }
    }
    
    // if set is the last element in the free list and it is not big enough
    // then we need to sbrk() a section
    if ((set->flink == NULL) && (set->size < pad)) {
      
      // check if the section we sbrk() off is going to be added to the free list
      if (find_sbrk(pad) == pad) {
        // sbrk() a section just big enough, but it doesn't have to be added to the free list
        set = (Flist) sbrk(find_sbrk(pad));
        set->size = find_sbrk(pad);
        set->flink = NULL;
        set->blink = NULL;
        ret  =(void *) set;
        ret += 8;
        return ret;
      } else {
        // sbrk() a new block and stitch it onto the back of the free list
        set->flink = (Flist) sbrk(8192);
        set->flink->blink = set;
        set = set->flink;
        set->size = find_sbrk(pad);
        set->flink = NULL;
      }
    }

    beg = (void *) set;
    ret = beg;
    beg += pad;
    rem = (Flist) beg;
  } else if (FL->size > pad) {
    // FL is one element and is big enough
    set = FL;
    beg = (void *) set;
    ret = beg;
    beg += pad;
    rem = (Flist) beg;
  } else if (FL->size == pad) {
    // FL is one element and just big enough
    // then we just have to return that last node and set FL to NULL
    ret = FL;
    ret += 8;
    FL = NULL;

    return ret;
  } else {
    // FL is not big enough
    if (FL != NULL) {
      set = (Flist) sbrk(find_sbrk(pad));
      if (find_sbrk(pad) == pad) {
        // the new node created is just big enough, so it doesn't need to be added to FL
        set->size = find_sbrk(pad);
        set->flink = NULL;
        set->blink = NULL;
        ret = (void *) set;
        ret += 8;
        return ret;
      }
      if (pad != find_sbrk(pad)) FL->flink = set;
      set->size = find_sbrk(pad);
      set->blink = FL;
      beg = (void *) set;
      ret = beg;
      beg += pad;
      rem = (Flist) beg;
    }
  }

  // now this chunk of code is the code that actually sets the return block and the free list block that we carved off of
  // Not all cases reach here or look like this, but in general memory looks like :
  //
  //        |           |
  // (Flist)|           |(void *)
  //        |-----------|
  //  set-->|   size    |<--ret
  //        |   flink   |
  //        |   blink   |       block carved off from the node in the free list
  //        |           |
  //        |           |
  //        |-----------|
  //  rem-->|   size    |<--beg
  //        |   flink   |
  //        |   blink   |       remainder of node from free list
  //        |           |
  //        |           |
  //
  // set's size, flink, and blink are not yet set, so they are still the values of the free list node we are carving off of
  // first thing I do is save the size of the free list node from set.size into t then i put pad into set.size
  // Then, if set has a blink, then we save that with setb and set setb's flink to skip over the block we are carving off
  // Then, if set has a flink, then we set the FL node set->flink pointed to to be the node rem point to and gets pointed to
  // if set and FL point to the same then, the we move FL to point to rem
  // after setting rem->blink to point to setb and setb->flink to point to rem, increment the return pointer 8 and return
  //
  setb = NULL;
  t = set->size;
  set->size = pad;
  rem->size = t - pad;
  if (set->blink != NULL) {
    setb = set->blink;
    if (t == pad) {
      setb->flink = set->flink;
    } else {
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
  rem->blink = setb;
  if (rem->size < 16) set->size += rem->size;
  if (setb != NULL) setb->flink = rem;
  ret += 8;

  return ret;
} // end of my_malloc()

// given a pointer, it creates a node for the free list
// then it searches the free list for the correct spot to add it
// the correct spot is the one that maintains sorted order
// I keep it sorted so coalescing is super easy
// I understand in the long run it is now as efficient, but I find it easier
void my_free(void *ptr) {
  Flist f, tmp;     // Flist node pointers for building the new node and iterating the free list

  ptr -= 8;

  f = ptr;

  // if ther is no free list. then our node is the new free list
  if (FL == NULL) {
    FL = f;
    FL->flink = NULL;
    FL->blink = NULL;
    return;
  }

  // iterate through the free list
  for (tmp = (Flist) free_list_begin(); tmp != NULL; tmp = (Flist) free_list_next(tmp)) {
    
    // if we found the node we need to insert in front of
    // stitch our new node into the free list
    if (tmp > f) {
      f->flink = tmp;
      if (tmp->blink != NULL) tmp->blink->flink = f;
      f->blink = tmp->blink;
      tmp->blink = f;

      if (FL == tmp) FL = f;

      return;
    } else if (tmp->flink == NULL) {
      // if we are at the end of the free list
      // then we just put our new node at the end of the list
      tmp->flink = f;
      f->blink = tmp;
      f->flink = NULL;

      return;
    }
  }
} // end of my_free()

// returns the beginning of the free list
void *free_list_begin() {
  return (void *) FL;
}

// returns the next node in the free list
void *free_list_next(void *node) {
  Flist n;
  n = (Flist) node;
  return (void *) n->flink;
}

// coalesces adjacent nodes in the free list so we are not wasting space on the heap
void coalesce_free_list() {
  Flist currentNode, nextNode;  // node pointers
  void *node;                   // used for pointer arithmetic
  
  // iterate through the Free list up to the second to last node
  for (currentNode = free_list_begin(); currentNode->flink != NULL; currentNode = free_list_next(currentNode)) {
    nextNode = currentNode->flink;
    node = (void *) currentNode;

    // while the next node is touching the current node
    // coalesce the two nodes together and remove the second node from the free list
    while ((node + currentNode->size) == nextNode) {
      currentNode->size += nextNode->size;
      currentNode->flink = nextNode->flink;
      if (currentNode->flink != NULL) currentNode->flink->blink = currentNode;
      nextNode = currentNode->flink;
      node = (void *) currentNode;
    }
    if (nextNode == NULL) break;
  }
}

// returns either size or 8192, whichever is bigger
int find_sbrk(int size) {
  if (size > 8192) {
    return size;
  } else {
    return 8192;
  }
}

