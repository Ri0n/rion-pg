#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#define POOLSZ128 16
#define POOLSZ256 8
#define POOLSZ512 4
#define POOLSZ1024 2

struct Pool;
struct MemItem {
    struct MemItem *next;
};

struct Pool {
    size_t poolSz;
    size_t blockSz;
    char *startAddr;
    struct MemItem *freeHead;
};

struct PoolManager {
    char *allMem;
    struct Pool pool128;
    struct Pool pool256;
    struct Pool pool512;
    struct Pool pool1024;
};

struct PoolManager poolManager;

int PoolInit(struct Pool *pool, char *buf, size_t bufSz, size_t blockSz)
{
    size_t leftSz = bufSz;

    if (buf == NULL || bufSz < blockSz || blockSz <= sizeof(struct MemItem)) {
        return -1; //shit happened
    }
    pool->poolSz = bufSz;
    pool->blockSz = blockSz;
    pool->startAddr = buf;
    pool->freeHead = NULL;

    // init free blocks. we can instead keep uninited blocks and slightly rewrite alloc method.
    while (leftSz >= blockSz) {
        struct MemItem *item = (struct MemItem *)buf;
        item->next = pool->freeHead;
        pool->freeHead = item;
        buf += blockSz;
        leftSz -= blockSz;
    }
    return 0;
}

void* PoolAlloc(struct Pool *pool)
{
    if (pool->freeHead) {
        struct MemItem *item = pool->freeHead;
        pool->freeHead = item->next;
        return item;
    }
    return NULL;
}

void PoolFree(struct Pool *pool, void *addr)
{
    // TODO check alignment
    struct MemItem *item = (struct MemItem *)addr;
    item->next = pool->freeHead;
    pool->freeHead = item;
}

void PoolManagerInit(struct PoolManager *pm)
{
    pm->allMem = (char*)malloc(POOLSZ128 * 128 +
                        POOLSZ256 * 256 +
                        POOLSZ512 * 512 +
                        POOLSZ1024 * 1024);
    if (pm->allMem  == NULL) {
        printf("No mem O_o\n");
        exit(EXIT_FAILURE);
    }
    PoolInit(&pm->pool128, pm->allMem, POOLSZ128 * 128, 128);
    PoolInit(&pm->pool256, pm->pool128.startAddr + pm->pool128.poolSz, POOLSZ256 * 256, 256);
    PoolInit(&pm->pool512, pm->pool256.startAddr + pm->pool256.poolSz, POOLSZ512 * 512, 512);
    PoolInit(&pm->pool1024, pm->pool512.startAddr + pm->pool512.poolSz, POOLSZ1024 * 1024, 1024);
}

void *PoolManagerAlloc(struct PoolManager *pm, size_t sz)
{
    if (sz <= 128) {
        return PoolAlloc(&pm->pool128); // TODO we can have more memory in other pools if this one is ran out
    }
    if (sz <= 256) {
        return PoolAlloc(&pm->pool256);
    }
    if (sz <= 512) {
        return PoolAlloc(&pm->pool512);
    }
    if (sz <= 1024) {
        return PoolAlloc(&pm->pool1024);
    }
    return NULL;
}

void PoolManagerFree(struct PoolManager *pm, void *addr)
{
    char *c = (char*)addr;
    if ((c < pm->pool128.startAddr) || (c >= pm->pool1024.startAddr + pm->pool1024.blockSz)) {
        exit(EXIT_FAILURE);
    }
    if (c < pm->pool256.startAddr) {
        PoolFree(&pm->pool128, addr);
    } else
    if (c < pm->pool512.startAddr) {
        PoolFree(&pm->pool256, addr);
    } else
    if (c < pm->pool1024.startAddr) {
        PoolFree(&pm->pool512, addr);
    } else
    PoolFree(&pm->pool1024, addr);
}


struct PoolManager *_defAllocator;

#define myalloc(sz) ({ void *p = PoolManagerAlloc(_defAllocator, sz); \
if (!p) printf("Alloc failed\n"); \
else printf("Alloc success\n"); \
fflush(stdout); \
p; })

#define myfree(p) PoolManagerFree(_defAllocator, p)



int main()
{
    struct PoolManager pm;
    _defAllocator = &pm;
    PoolManagerInit(_defAllocator);

    char *p1 = (char*)myalloc(400);
    char *p2 = (char*)myalloc(400);
    char *p3 = (char*)myalloc(400);
    char *p4 = (char*)myalloc(400);
    char *p5 = (char*)myalloc(400);
    printf("4 success and 1 fail? good! it works!\n");
    myfree(p2);
    p2 = (char*)myalloc(400);
    myalloc(400);
    printf("And now 1 success and 1 fail are expected!\n");
    myfree(p1);
    myfree(p4);
    myfree(p3);
    myfree(p2);
    printf("Now all are freed and we can allocate the 4 again. but 5th should fail\n");
    p1 = (char*)myalloc(400);
    p2 = (char*)myalloc(400);
    p3 = (char*)myalloc(400);
    p4 = (char*)myalloc(400);
    p5 = (char*)myalloc(400);
    printf("512 pool is empty now, but lets alloc from 256 pool\n");
    p5 = (char*)myalloc(200);
}
