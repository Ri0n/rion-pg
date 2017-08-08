#include <stdio.h>

struct Pool;
struct MemItem {
    struct MemItem *next;
    struct MemItem *prev;
    struct Pool *pool; // Instead of keepint it here we can check
                       // memory boundries in up-level memory manager.
    // it's pretty good aligned for the following data.
};

struct Pool {
    size_t blockSz;
    struct MemItem *free_head;
    struct MemItem *busy_head;
};

int PoolInit(struct Pool *pool, char *buf, size_t bufSz, size_t blockSz)
{
    size_t leftSz = bufSz;
    struct MemItem *prev_item = NULL;

    if (buf == NULL || bufSz < blockSz || blockSz <= sizeof(struct MemItem)) {
        return -1; //shit happened
    }
    pool->blockSz = blockSz;
    pool->busy_head = NULL;

    // init free blocks
    while (leftSz >= blockSz) {
        struct MemItem *item = (struct MemItem *)buf;
        item->prev = prev_item;
        item->next = NULL;
        item->pool = pool;
        if (prev_item) {
            prev_item->next = item;
        }
        prev_item = item;
        buf += blockSz;
        leftSz -= blockSz;
    }
    pool->free_head = prev_item;
    return 0;
}

void* PoolAlloc(struct Pool *pool)
{
    // TODO lock heads here in case of multithreading
    struct MemItem *item = pool->free_head;
    if (item == NULL) {
        return NULL; // crap. no mem
    }
    if (item->prev) {
        item->prev->next = NULL;
        pool->free_head = item->prev;
    } else {
        pool->free_head = NULL; // the last one
    }
    item->prev = pool->busy_head;
    if (pool->busy_head) {
        pool->busy_head->next = item;
    }
    pool->busy_head = item;
    // TODO unlock heads
    return (char*)item + sizeof(struct MemItem);
}

void PoolFree(void *addr)
{
    // TODO lock
    struct MemItem *item = (struct MemItem *)((char*)addr - sizeof(struct MemItem));
    struct Pool *pool = item->pool;

    if (item == pool->busy_head) {
        pool->busy_head = item->prev;
    }

    // remove item from the lists
    if (item->prev) {
        item->prev->next = item->next;
    }
    if (item->next) {
        item->next->prev = item->prev;
    }

    // insert to free list
    item->next = NULL;
    item->prev = pool->free_head;
    if (item->prev) {
        item->prev->next = item;
    }
    pool->free_head = item;
    // TODO unlock.
}

#define myalloc() ({ void *p = PoolAlloc(&pool); if (!p) printf("Alloc failed\n"); else printf("Alloc success\n"); fflush(stdout); p; })

int main()
{
    struct Pool pool;
    char buf[1024];

    PoolInit(&pool, buf, sizeof(buf), 256);

    char *p1 = (char*)myalloc();
    char *p2 = (char*)myalloc();
    char *p3 = (char*)myalloc();
    char *p4 = (char*)myalloc();
    char *p5 = (char*)myalloc();
    printf("4 success and 1 fail? good! it works!\n");
    PoolFree(p2);
    p2 = (char*)myalloc();
    myalloc();
    printf("And now 1 success and 1 fail are expected!\n");
    PoolFree(p1);
    PoolFree(p4);
    PoolFree(p3);
    PoolFree(p2);
    printf("Now all are freed and we can allocate the 4 again. but 5th should fail\n");
    p1 = (char*)myalloc();
    p2 = (char*)myalloc();
    p3 = (char*)myalloc();
    p4 = (char*)myalloc();
    p5 = (char*)myalloc();
}
