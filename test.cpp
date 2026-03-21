#include <sys/mman.h>

char test_heap_mem[10000];
char *heap_ptr = test_heap_mem;

typedef struct GC_ID
{   
    bool is_free = 0;
    int size;
};


char* m_allocate(int size)
{
    GC_ID *id = (GC_ID*)heap_ptr;   
    id->is_free = false;
    id->size = size;

    char* payload_ptr = heap_ptr + sizeof(GC_ID);

    heap_ptr = heap_ptr + sizeof(GC_ID) + size;

    return payload_ptr;

}


void m_free(char* user_ptr)
{
    GC_ID *id = (GC_ID*)(user_ptr - sizeof(GC_ID));
    id->is_free = 1;
}



int main()
{
    return 0;
}