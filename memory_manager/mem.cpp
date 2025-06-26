#include <iostream>
#include <unistd.h>
using namespace std;

struct mem_chunk
{
    mem_chunk *prev, *next;
    int index;
    char *allocated;
};

mem_chunk *freehead = NULL, *freetail = NULL, *usedhead = NULL, *usedtail = NULL;
char *pool;

void add_ll(mem_chunk *&head, mem_chunk *&tail, int index)
{
    mem_chunk *a = &((mem_chunk *)pool)[index];
    if (a == NULL)
        return;
    if (head == NULL)
    {
        head = a;
        tail = a;
        a->next = a->prev = NULL;
        return;
    }
    tail->next = a;
    a->prev = tail;
    a->next = NULL;
    tail = a;
}

void remove_ll(mem_chunk *&head, mem_chunk *&tail, int index)
{
    mem_chunk *a = &((mem_chunk *)pool)[index];
    if (a == NULL || head == NULL || tail == NULL)
    {
        cout << "Illegal remove called\n";
        return;
    }

    if (head == a)
    {
        if (head->next == NULL)
        {
            head = tail = NULL;
            return;
        }
        head = head->next;
        head->prev = NULL;
    }
    else if (a == tail)
    {
        if (tail->prev == NULL)
        {
            head = tail = NULL;
            return;
        }
        tail = tail->prev;
        tail->next = NULL;
    }
    else
    {
        a->prev->next = a->next;
        a->next->prev = a->prev;
    }
    a->next = a->prev = NULL;
}

void _init()
{
    pool = (char *)malloc(1024 * 1024);
    mem_chunk *tmp;

    // for (int i = 0; i < 2; i++)
    // {
    //     tmp = (mem_chunk *)pool;
    //     tmp->is_free = 0;
    //     tmp->index = i;
    //     tmp->allocated = &pool[4 * 1024 * i];
    //     add_ll(usedhead, usedtail, tmp);
    // }

    cout << "size = " << usedtail << " - " << usedhead << endl;

    for (int i = 2; i < 256; i++)
    {
        tmp = &((mem_chunk *)pool)[i];
        tmp->is_free = 1;
        tmp->index = i;
        tmp->allocated = &pool[4 * 1024 * i];
        add_ll(freehead, freetail, tmp->index);
    }
    cout << "memory allocated" << endl;
}

void *getmem()
{
    if (freehead == NULL || freetail == NULL)
    {
        cout << "Error: memory full\n";
        return NULL;
    }

    mem_chunk *tmp = freetail;
    remove_ll(freehead, freetail, tmp->index);
    tmp->is_free = 0;
    add_ll(usedhead, usedtail, tmp->index);
    return (void *)tmp->allocated;
}

void freemem(void *memory)
{
    int index = (((char *)memory - &pool[0]) >> 12);

    mem_chunk *tmp = &((mem_chunk *)pool)[((char *)memory - &pool[0]) >> 12];

    remove_ll(usedhead, usedtail, tmp->index);
    tmp->is_free = 1;
    add_ll(freehead, freetail, tmp->index);
}

int main()
{
    cout << sizeof(mem_chunk) << endl;
    _init();
    char *ptr[256];
    for (int i = 2; i < 256; i++)
    {
        ptr[i] = (char *)getmem();
        cout << "size = " << usedtail - usedhead << endl;
    }

    for (int i = 2; i < 256; i++)
    {
        freemem(ptr[i]);
        cout << "size = " << usedtail - usedhead << endl;
    }
}