/*
	Group Members :-
	Name : Pratik Ramesh Barhate
	ID : 1001649826
	Name : Darshak Deepan Rairakhia
	ID : 1001637657
*/

#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>

#define ALIGN4(s)         (((((s) - 1) >> 2) << 2) + 4)
#define BLOCK_DATA(b)      ((b) + 1)
#define BLOCK_HEADER(ptr)   ((struct block *)(ptr) - 1)


static int atexit_registered = 0;
static int num_mallocs       = 0;
static int num_frees         = 0;
static int num_reuses        = 0;
static int num_grows         = 0;
static int num_splits        = 0;
static int num_coalesces     = 0;
static int num_blocks        = 0;
static int num_requested     = 0;
static int max_heap          = 0;

/*
 *  \brief printStatistics
 *
 *  \param none
 *
 *  Prints the heap statistics upon process exit.  Registered
 *  via atexit()
 *
 *  \return none
 */
void printStatistics( void )
{
  printf("\nheap management statistics\n");
  printf("mallocs:\t%d\n", num_mallocs );
  printf("frees:\t\t%d\n", num_frees );
  printf("reuses:\t\t%d\n", num_reuses );
  printf("grows:\t\t%d\n", num_grows );
  printf("splits:\t\t%d\n", num_splits );
  printf("coalesces:\t%d\n", num_coalesces );
  printf("blocks:\t\t%d\n", num_blocks );
  printf("requested:\t%d\n", num_requested );
  printf("max heap:\t%d\n", max_heap );
}

//struct block *NEXT = NULL;//NEXT to track the next free block

struct block
{
   size_t      size;  /* Size of the allocated block of memory in bytes */
   struct block *next;  /* Pointer to the next block of allcated memory   */
   bool        free;  /* Is this block free?                     */
};


struct block *FreeList = NULL; /* Free list to track the blocks available */
// struct block *nextpointer = NULL;

/*
 * \brief findFreeBlock
 *
 * \param last pointer to the linked list of free blocks
 * \param size size of the block needed in bytes
 *
 * \return a block that fits the request or NULL if no free block matches
 *
 * \TODO Implement Next Fit
 * \TODO Implement Best Fit
 * \TODO Implement Worst Fit
 */
struct block *findFreeBlock(struct block **last, size_t size)
{
   struct block *curr = FreeList;

#if defined FIT && FIT == 0
   /* First fit */
   while (curr && !(curr->free && curr->size >= size))
   {
      *last = curr;
      curr  = curr->next;
   }
#endif

#if defined BEST && BEST == 0
    struct block *best = NULL;
    // Find first block whose size is big and is free.
    while (curr)
        {
        //If the free block is bigger than the size of the process and has the smallest
        //unallocated memory left then store that
        //as curr
        if (curr->free && curr->size >= size)
        {
            best = curr;
            *last = curr;
            curr  = curr->next;
            break;
        }

        *last = curr;
        curr  = curr->next;
       }
    //If the block is the best block or the biggest block
    //then go into this loop
    if (best)
       {
        while (curr)
        {

            if (curr->free && curr->size >= size)
            {
                //to find if the free unallocated memory is
                //smallest or not
                if (curr->size < best->size)
                    best = curr;//update the pointer
            }

            *last = curr;
            curr  = curr->next;
        }
     }

      /* if (best)
       {
         while (curr && !(curr->free && curr->size >= size))
         {
           if (curr->size < best->size)
             {
               best = curr;
             }
           *last = curr;
           curr = curr->next;
         }
       } */
		curr = best;//update curr so that we can find the next big block in the list
#endif

#if defined WORST && WORST == 0
struct block *best = NULL;
    // Find first block whose size is big enough and is free.
    while (curr)
        {
        //If the free block is bigger than the size of the process and has the biggest
        //unallocated memory left then store that
        //as curr
        if (curr->free && curr->size >= size)
        {
            best = curr;
            *last = curr;
            curr  = curr->next;
            break;
        }

        *last = curr;
        curr  = curr->next;
       }
    if (best)
        {
        while (curr)
        {
            if (curr->free && curr->size >= size)
            {
                //to find if the free unallocated memory is
                //the biggest or not
                if (curr->size > best->size)
                    best = curr;//update the pointer
            }

            *last = curr;
            curr  = curr->next;
        }
       }

    curr = best;//update the pointer
#endif

#if defined NEXT && NEXT == 0
	struct block *nextpointer = NULL;

	while(curr)
	{
		if (curr->free && curr->size >= size)
			{
				nextpointer = curr;
			}

		*last = curr;
		curr = curr->next;
	}
	curr = nextpointer;


	/*
	if (nextpointer)
	{
	curr = nextpointer;
	while(curr && !(curr->free && curr->size >= size))
    {
    	*last = curr;
        curr  = curr->next;
    }
    	*last = curr;
        curr  = curr->next;
    }
    if (curr == NULL)
    {
      curr = FreeList;
      while (curr && !(curr->free && curr->size >= size))
      {
        if (curr == NULL)
          {
          	*last = curr;
            curr  = curr->next;
          }
        *last = curr;
        curr  = curr->next;
      }
      nextpointer = curr;
      }
      curr = nextpointer;
      */

#endif

   num_reuses++;
   return curr;
}

/*
 * \brief growheap
 *
 * Given a requested size of memory, use sbrk() to dynamically
 * increase the data segment of the calling process.  Updates
 * the free list with the newly allocated memory.
 *
 * \param last tail of the free block list
 * \param size size in bytes to request from the OS
 *
 * \return returns the newly allocated block of NULL if failed
 */
struct block *growHeap(struct block *last, size_t size)
{
   /* Request more space from OS */
   struct block *curr = (struct block *)sbrk(0);
   struct block *prev = (struct block *)sbrk(sizeof(struct block) + size);

   assert(curr == prev);

   /* OS allocation failed */
   if (curr == (struct block *)-1)
   {
      return NULL;
   }

   /* Update FreeList if not set */
   if (FreeList == NULL)
   {
      FreeList = curr;
   }

   /* Attach new block to prev block */
   if (last)
   {
      last->next = curr;
   }

   /* Update block metadata */
   curr->size = size;
   curr->next = NULL;
   curr->free = false;

   num_grows++;
   num_blocks++;
   max_heap += size;
   return curr;
}

/*
 * \brief malloc
 *
 * finds a free block of heap memory for the calling process.
 * if there is no free block that satisfies the request then grows the
 * heap and returns a new block
 *
 * \param size size of the requested memory in bytes
 *
 * \return returns the requested memory allocation to the calling process
 * or NULL if failed
 */
void *malloc(size_t size)
{

   if( atexit_registered == 0 )
   {
      atexit_registered = 1;
      atexit( printStatistics );
   }

   /* Align to multiple of 4 */
   size = ALIGN4(size);

   /* Handle 0 size */
   if (size == 0)
   {
      return NULL;
   }

   /* Look for free block */
   struct block *last = FreeList;
   struct block *next = findFreeBlock(&last, size);

   /* TODO: Split free block if possible */

   /* Could not find free block, so grow heap */
   if (next == NULL)
   {
      next = growHeap(last, size);
      num_reuses--;
   }

   /* Could not find free block or grow heap, so just return NULL */
   if (next == NULL)
   {
      return NULL;
   }

   /* Mark block as in use */
   next->free = false;

   num_mallocs++;
   num_requested += size;

   /* Return data address associated with block */
   return BLOCK_DATA(next);
}

/*
 * \brief free
 *
 * frees the memory block pointed to by pointer. if the block is adjacent
 * to another block then coalesces (combines) them
 *
 * \param ptr the heap memory to free
 *
 * \return none
 */
void free(void *ptr)
{
   if (ptr == NULL)
   {
      return;
   }

   /* Make block as free */
   struct block *curr = BLOCK_HEADER(ptr);
   assert(curr->free == 0);
   curr->free = true;
   num_frees++;

   /* TODO: Coalesce free blocks if needed */
   struct block *next = curr->next;

    if (next)
    {
      if (curr->free == true && next->free == true)
      {
        // New size of the allocated block of memory in bytes
        size_t new_size;
        new_size = curr->size + next->size + sizeof(struct block);
        curr->size = new_size;
        curr->next = next->next;
        num_coalesces++;
        num_blocks--;
        }
    }

}

/* vim: set expandtab sts=3 sw=3 ts=6 ft=cpp: --------------------------------*/

