#include <sys/mman.h>
#include <list>
#include<cstdint>
#include<iostream>

char* heap_ptr = (char*)mmap(nullptr, 65536, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

typedef struct memID
{
    size_t size;
    bool is_free = 0;
} memID;

memID* free_list_head = (memID*)heap_ptr;
static int called = 0;

char* m_allocate(size_t size)
{

  if (size < 8) size = 8;

  char* payload;
  memID* prev;
  memID* current;
  memID* next_free;


  if (!called) {

    prev = nullptr;
    current = free_list_head;
    
    payload = (char*)free_list_head + sizeof(memID);

    free_list_head->is_free = 1;
    free_list_head->size = 65536 - sizeof(memID);
    size_t free_size = free_list_head->size;

    current->is_free = 0;
    current->size = size;

    free_list_head = (memID*)((char*)free_list_head + sizeof(memID) + size);

    free_list_head->is_free = 1;
    free_list_head->size = free_size - size - sizeof(memID);

    next_free = nullptr; 
    *(memID**)payload = next_free;

    called = 1;
    return (char*)current + sizeof(memID);
  }

  current = free_list_head;
  prev = nullptr;


  while(current!= nullptr && current->size < size)
  {
    prev = current;
    current = next_free;
    payload = (char*)current + sizeof(memID);

    next_free = *(memID**)payload;  
  }

  if(current == nullptr) 
  {
    std::cout<<"NOT ENOUGH MEMORY!"<<std::endl; 
    return nullptr;
  }

  int prev_size = current->size;
    
  current->size = size;
  current->is_free = 0;
 
  char* current_payload = (char*)current + sizeof(memID);
    

  memID* new_block =  (memID*)((char*)current + size + sizeof(memID));

  new_block->size = prev_size - size - sizeof(memID);
  new_block->is_free = 1;

  if(prev == nullptr) 
  {
    free_list_head = new_block;
    char* free_list_head_payload = (char*)free_list_head + sizeof(memID);
    *(memID**)free_list_head_payload = next_free;
  }

  else
  {
    char* prev_payload = (char*)prev + sizeof(memID);
    *(memID**)prev_payload = new_block;

    char* new_block_payload = (char*)new_block + sizeof(memID);
    *(memID**)new_block_payload = next_free;
  }

  return (char*)current + sizeof(memID);
}

void m_free(char* ptr)
{
  memID *ptr_id = (memID*)(ptr - sizeof(memID));
  memID *next_id = (memID*)(ptr + sizeof(memID));

  ptr_id->is_free = 1;

  if(next_id->is_free)
  {
    ptr_id->size = ptr_id->size + next_id->size + sizeof(memID);
    return;
  }

}


int main()
{
  char* ptr = m_allocate(425);
  char* ptr2 = m_allocate(300);

  char x = 'A';
  ptr = &x;

  char y = 'B';
  ptr2 = &y;

  m_free(ptr);

  std::cout<<"SUCESSFULLY ALLOCATED MEMORY "<<"\nVALUE OF VARIABLE IS "<<*ptr<<" AND y = "<<*ptr2<<std::endl;
  return 0; 
}
