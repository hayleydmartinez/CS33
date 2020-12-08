/*
 * mm-explicit.c - Explicit List Function. First fit.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

team_t team = {
    /* Your UID */
    "104926567",
    /* Your first name */
    "Hayley",
    /* Your last name */
    "Martinez",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};


/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

/* Basic constants and macros */
#define WSIZE     4        /* Word and header/footer size (bytes) */
#define DSIZE     8        /* Double word size (bytes) */
#define MINIMUM   16       /* Minimum blocksize */
#define ALIGNMENT 8        /* Single word (4) or double word (8) alignment */
#define CHUNKSIZE (1<<12)  /* Extend heap by this amount (bytes) */

/* Returns the maximum of x and y */
#define MAX(x, y) ((x) > (y)? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p)      (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)  (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

/* Given free block ptr bp, compute address of next and previous free blocks */
/* NOTE: must add *(void **) before, otherwise it causes an lvalue error :( */
#define NEXT_FREE(bp) (*(void **)((char *)(bp) + WSIZE))
#define PREV_FREE(bp) (*(void **)((char *)(bp)))

/* Global variables */
static void *heap_listp;
static void *free_listp;

///////////////////////////////////////////////////////////////////////////////
//////////////////////////////// MALLOC DESIGN ////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*
 *
 * ALLOCATED
 * 31                                                       3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      BLOCKSIZE                          | | |A|   HEADER
 * -----------------------------------------------------------------   <-- BP
 * |                           PAYLOAD                             |
 * -----------------------------------------------------------------
 * |                           PADDING                             |
 * -----------------------------------------------------------------
 * |                      BLOCKSIZE                          | | |A|   FOOTER
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *
 *
 * FREE
 * 31                                                       3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      BLOCKSIZE                          | | |A|   HEADER
 * -----------------------------------------------------------------   <-- BP
 * |                         PREV POINTER                          |
 * -----------------------------------------------------------------   <-- BP + WSIZE
 * |                         NEXT POINTER                          |
 * -----------------------------------------------------------------   <-- BP + DSIZE
 * |                           PAYLOAD                             |
 * -----------------------------------------------------------------
 * |                           PADDING                             |
 * -----------------------------------------------------------------
 * |                      BLOCKSIZE                          | | |A|   FOOTER
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 */

/* Helper function declaration */
static void *coalesce(void *bp);
static void *extend_heap(size_t words);
static void *find_fit(size_t asize);
static void place(void *bp, size_t asize);
static void put_block(void *bp, size_t size, int alloc);
static void add_block(void *bp);
static void del_block(void *bp);


/*
 * mm_init - Initialize the malloc package.
 */
int mm_init(void)
{
    /* Create the initial empty heap */
    if ((heap_listp = mem_sbrk(8 * WSIZE)) == (void *)-1)
        return -1;
    
    PUT(heap_listp, 0);                            /* Alignment padding */
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1)); /* Prologue header */
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1)); /* Prologue footer */
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));     /* Epilogue header */
    
    /* Initialize empty free list */
    free_listp = heap_listp + DSIZE;
    
    /* Extend the empty heap with minimum number of bytes */
    if (extend_heap(WSIZE) == NULL)
        return -1;
    return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize;      /* Adjusted block size */
    size_t extendsize; /* Amount to extend heap if no fit */
    char *bp;
    
    /* Ignore spurious requests */
    if (size <= 0)
        return NULL;
    
    /* Adjust block size to include overhead and alignment reqs. */
    asize = MAX(ALIGN(size) + DSIZE, 2 * DSIZE);
    
    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }
    
    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize,CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
        return NULL;
    place(bp, asize);
    return bp;
}

/*
 * mm_free - Changes alloc tag to 0 and coalesces.
 */
void mm_free(void *bp)
{
    /* If you're freeing nothing */
    if (bp == NULL)
        return;
    
    /* Gets size and coalesces */
    size_t size = GET_SIZE(HDRP(bp));
    put_block(bp, size, 0);
    coalesce(bp);
}

/*
 * mm_realloc - Reallocates data.
 *  - First checks to see if we need to reallocate
 *  - If we can just use the next free block, we use that
 *  - Otherwise we malloc a new free space
 */
void *mm_realloc(void *bp, size_t size)
{
    /* Checks for errors */
    if (size <= 0) {
        mm_free(bp);
        return NULL;
    }
    
    /* If there's nothing, just malloc */
    if (bp == NULL)
        return mm_malloc(size);
    
    /* New variables */
    void *newptr;
    size_t oldsize = GET_SIZE(HDRP(bp));
    size_t newsize = ALIGN(size) + DSIZE;
    
    /* We already have room! */
    if (newsize <= oldsize)
        return bp;
    
    /* Check to see if next block is free */
    size_t realloc_size = oldsize + GET_SIZE(HDRP(NEXT_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    
    /* If it is free and we have enough room to reallocate */
    if (!next_alloc && (realloc_size >= newsize)) {
        del_block(NEXT_BLKP(bp));
        put_block(bp, realloc_size, 1);
        return bp;
    }
    
    /* Otherwise we need to reallocate a new space and free our old bp */
    else {
        newptr = mm_malloc(newsize);
        place(newptr, newsize);
        memcpy(newptr, bp, newsize);
        mm_free(bp);
        return newptr;
    }
}


/* Helper function implementation */

/*
 * coalesce - Coalesces adjacent memory blocks and adds to free list.
 *  - Case 1: blocked on both sides by memory
 *  - Case 2: open next block
 *  - Case 3: open prev block
 *  - Case 4: open block on both sides
 */
static void *coalesce(void *bp)
{
    
    /* Added in second part of prev_alloc b/c program broke on first block */
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp))) || PREV_BLKP(bp) == bp;
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
    
    /* Case 2 */
    if (prev_alloc && !next_alloc) {
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        del_block(NEXT_BLKP(bp));
        put_block(bp, size, 0);
    }
    
    /* Case 3 */
    else if (!prev_alloc && next_alloc) {
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        bp = PREV_BLKP(bp);
        del_block(bp);
        put_block(bp, size, 0);
    }
    
    /* Case 4 */
    else if (!prev_alloc && !next_alloc) {
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
        del_block(PREV_BLKP(bp));
        del_block(NEXT_BLKP(bp));
        bp = PREV_BLKP(bp);
        put_block(bp, size, 0);
    }
    
    /* Case 1 */
    add_block(bp);
    return bp;
}

/*
 * extend_heap - Extend heap by a certain amount of words,
 *     then return the block pointer.
 */
static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;
    
    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    size = size < MINIMUM ? MINIMUM : size;
    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL;
    
    /* Initialize free block header/footer and the epilogue header */
    put_block(bp, size, 0);
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* New epilogue header */
    
    /* Coalesce if the previous block was free */
    return coalesce(bp);
}

/*
 * find_fit - Search through free list for first space large
 *      enough to fit a block of size asize.
 */
static void *find_fit(size_t asize)
{
    void *bp;
    
    /* First-fit search */
    for (bp = free_listp; GET_ALLOC(HDRP(bp)) == 0; bp = NEXT_FREE(bp)) {
        if (asize <= GET_SIZE(HDRP(bp))) {
            return bp;
        }
    }
    
    return NULL; /* No fit */
}

/*
 * place - Modeled off the book implementation.
 *      Added calls to del_block for compatibility with
 *      explicit free list.
 */
static void place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp));
    
    /* If there is room for another block in free block*/
    if ((csize - asize) >= 2 * DSIZE) {
        put_block(bp, asize, 1);
        del_block(bp);
        bp = NEXT_BLKP(bp);
        put_block(bp, csize - asize, 0);
        coalesce(bp);
    }
    
    else {
        put_block(bp, csize, 1);
        del_block(bp);
    }
}

/*
 * put_block - Cleans up repetitive code.
 */
static void put_block(void *bp, size_t size, int alloc)
{
    PUT(HDRP(bp), PACK(size, alloc));
    PUT(FTRP(bp), PACK(size, alloc));
}

/*
 * add_block - LIFO system for adding free blocks to block list.
 */
static void add_block(void *bp)
{
    /* Inserts free block at front of the list */
    NEXT_FREE(bp) = free_listp;
    PREV_FREE(bp) = NULL;
    PREV_FREE(free_listp) = bp;
    free_listp = bp;
    return;
}

/*
 * del_block - Delete a block from the free list.
 *  - Checks if the block is at the beginning of the list
 *  - If it isn't, assigns the previous's next ptr to the next
 *  - If it is, moves the free_listp to the next
 *  - Fixes next ptr
 */
static void del_block(void *bp)
{
    if (!PREV_FREE(bp))
        free_listp = NEXT_FREE(bp);
    else
        NEXT_FREE(PREV_FREE(bp)) = NEXT_FREE(bp);
    PREV_FREE(NEXT_FREE(bp)) = PREV_FREE(bp);
}
