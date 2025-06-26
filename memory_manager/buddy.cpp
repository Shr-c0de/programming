#include <iostream>
#include <unistd.h>
#include <cstdlib>
using namespace std;

#define MAX_ORDER 4
#define BLOCK_SIZE 1024                  // base block size (1KB)
#define POOL_SIZE (1 << (MAX_ORDER + 8)) // 256 blocks of 1KB = 256KB

struct mem_chunk
{
    mem_chunk *next;
    short size, stat;
    int index;
    mem_chunk() : next(NULL), size(0), index(0) {}
    mem_chunk(int s, int i) : next(NULL), size(s), index(i) {}
};

mem_chunk *buddy[MAX_ORDER + 1] = {NULL}; // head for each order
mem_chunk *usedhead = NULL;               // used list head
mem_chunk *mem_map;
char *pool;

void add_ll(mem_chunk *&head, mem_chunk *node)
{
    node->next = NULL;
    if (!head)
        head = node;
    else
    {
        mem_chunk *curr = head;
        while (curr->next)
            curr = curr->next;
        curr->next = node;
    }
}

void remove_ll(mem_chunk *&head, int index)
{
    if (!head)
        return;

    mem_chunk *curr = head, *prev = NULL;
    while (curr)
    {
        if (curr->index == index)
        {
            if (prev)
                prev->next = curr->next;
            else
                head = curr->next;

            curr->next = NULL;
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

void _init()
{
    pool = (char *)malloc(POOL_SIZE);
    mem_map = (mem_chunk *)pool;

    for (int i = 0; i < 256; i++)
    {
        mem_map[i].index = i;
        mem_map[i].size = 0;
        mem_map[i].next = NULL;
    }

    // Initially, one large chunk of order 4 (16KB) at index 0
    mem_map[0].index = 0;
    mem_map[0].size = MAX_ORDER;
    mem_map[0].next = NULL;
    buddy[MAX_ORDER] = &mem_map[0];

    cout << "Memory initialized.\n";
}

void *getmem(int req_size)
{
    int order = 0;
    while ((1 << order) * BLOCK_SIZE < req_size && order <= MAX_ORDER)
        order++;

    if (order > MAX_ORDER)
        return NULL;

    int i = order;
    while (i <= MAX_ORDER && buddy[i] == NULL)
        i++;

    if (i > MAX_ORDER)
        return NULL;

    mem_chunk *curr = buddy[i];
    buddy[i] = curr->next;

    while (i > order)
    {
        i--;
        int buddy_index = curr->index ^ (1 << i);
        mem_chunk *b = &mem_map[buddy_index];
        b->index = buddy_index;
        b->size = i;
        b->next = NULL;
        add_ll(buddy[i], b);
    }

    curr->size = order;
    curr->next = NULL;
    add_ll(usedhead, curr);
    curr->stat = 1;
    return (void *)(pool + curr->index * BLOCK_SIZE);
}

void freemem(void *memory)
{
    int offset = (char *)memory - pool;
    int index = offset / BLOCK_SIZE;
    mem_chunk *curr = &mem_map[index];
    remove_ll(usedhead, index);

    int buddy_index = curr->index ^ (1 << curr->size);
    if (mem_map[buddy_index].stat == 0 && curr->size == mem_map[buddy_index].size)
    {
        // buddy exists and is on same level
        mem_chunk *buddy_chunk = &mem_map[buddy_index];
        mem_chunk *tmp = buddy[curr->size], *prev = NULL;
        while (tmp)
        {
            if (tmp->index == buddy_index)
            {
                break;
            }
            prev = tmp;
            tmp = tmp->next;
        }
        prev->next = tmp->next;
        if (buddy_index < curr->index)
            curr = buddy_chunk;

        curr->size++;
    }

    add_ll(buddy[curr->size], curr);
}

int main()
{
    _init();
    char *ptr[2];

    for (int i = 0; i < 2; i++)
    {
        ptr[i] = (char *)getmem(1500); // ~1.5 KB
        cout << "ptr[" << i << "] = " << (void *)ptr[i] << endl;
    }

    for (int i = 0; i < 2; i++)
    {
        freemem(ptr[i]);
        cout << "Freed ptr[" << i << "] = " << (void *)ptr[i] << endl;
    }

    return 0;
}
