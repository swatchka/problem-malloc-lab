/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 *
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "1team",
    /* First member's full name */
    "YangYoonsung",
    /* First member's email address */
    "yoonsung2007@gmail.com",
    /* Second member's full name (leave blank if none) */
    "KimTaehwan",
    /* Second member's email address (leave blank if none) */
    "qrxoghks1209@gmail.com"
};
/* | Header | Payload ... | Footer | */
/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7) // 정렬용

#define SIZE_T_SIZE (ALIGN(sizeof(size_t))) // 요청 크기 계산시 보조용
#define WSIZE 4 // header/footer 1워드 크기
#define DSIZE 8 // 최소 블록 크기 (더블워드)
#define CHUNKSIZE (1<<12) // 힙을 확장하는 기본 단위

#define MAX(x, y) ((x) > (y)? (x) : (y)) // x와 y중 더 큰 값을 반환(둘중 더 큰값 선택)
/* extend_heap()에서 heap 확장 크기 결정 */

#define PACK(size, alloc) ((size) | (alloc)) // header/footer 값 구성
/* 크기 + 할당여부 결합*/

#define GET(p) (*(size_t *)(p)) // 메모리 접근 
#define PUT(p, val) (*(size_t *)(p) = (val)) // 메모리 접근
/* header/footer 값 읽고 쓰기*/

#define GET_SIZE(p) (GET(p) & ~0x7) // header/footer에서 크기 추출
#define GET_ALLOC(p) (GET(p) & 0x1) // header/footer에서 할당 상태 추출

#define HDRP(bp) ((char *)(bp) - WSIZE) 
// bp = 0x1008 → HDRP(bp) = 0x1008 - 4 = 0x1004
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)
/* payload -> header/footer 주소 계산 */

#define NEXT_BLKP(bp) ((char*)(bp) + GET_SIZE(((char *)(bp) - WSIZE))) // 다음 블록으로 이동
#define PREV_BLKP(bp) ((char*)(bp) - GET_SIZE(((char *)(bp) - DSIZE))) // 이전 블록으로 이동
/*
 * mm_init - initialize the malloc package.
 */
static char *heap_listp = 0;
static void *coalesce(void *bp)
{
    // 1. 오른쪽이 프리인경우
    // 2. 왼쪽이 프리인경우
    // 3. 양쪽이 프리인경우
    // 3. 양쪽 다 alloc=1인 경우 (return bp;)
    return bp;
}
static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;

    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;

    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL;

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));

    return coalesce(bp);
}


int mm_init(void)
{
    /* 최소 힙 공간 확보 */
    if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *)-1)
        return -1;

    /* paddinf + prologue header/footer + epilogue header 초기화 */
    PUT(heap_listp, 0);                               // 패딩
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1));    // 프롤로그 헤더
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1));    // 프롤로그 푸터
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));        // 에필로그 헤더
    heap_listp += (2 * WSIZE);                        // payload 부분을 가리킴 

    /* 첫 번째 free block 생성 */
    if (extend_heap(CHUNKSIZE / WSIZE) == NULL) {
        return -1;
    }

    return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    int newsize = ALIGN(size + SIZE_T_SIZE);
    void *p = mem_sbrk(newsize);
    if (p == (void *)-1)
        return NULL;
    else
    {
        *(size_t *)p = size;
        return (void *)((char *)p + SIZE_T_SIZE);
    }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));
    /* 전체블록 크기 size에 저장 (header + payload + footer)*/

    PUT(HDRP(ptr), PACK(size, 0)); // 헤더에 블록 전체 사이즈와 alloc=0으로
    PUT(FTRP(ptr), PACK(size, 0)); // 푸터에 블록 전체 사이즈와 alloc=0으로

    coalesce(ptr);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;

    newptr = mm_malloc(size);
    if (newptr == NULL)
        return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
        copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}