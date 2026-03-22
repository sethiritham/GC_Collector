#include <sys/mman.h>
#include <list>
#include<cstdint>

char* heap_ptr = (char*)mmap(nullptr, 65536, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
memID* free_list_head = (memID*)heap_ptr;

typedef struct memID
{
    size_t size;
    bool is_free = 0;
};

char* m_allocate(size_t size)
{
    
    size_t free_size = free_list_head->size;

    memID* id = (memID*)free_list_head;
    id->is_free = 0;
    id->size = size;

    free_list_head = (memID*)((char*)free_list_head + sizeof(memID) + size);

    free_list_head->is_free = 1;
    free_list_head->size = free_size - size - sizeof(memID);


    return (char*)id + sizeof(memID);
}

int main()
{
    free_list_head->is_free = 1;
    free_list_head->size = 65536 - sizeof(memID);
    
    heap_ptr = heap_ptr + sizeof(memID);


    return 0;
}