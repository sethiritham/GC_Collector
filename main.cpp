#include <sys/mman.h>
#include <list>
#include<cstdint>
#include<pthread.h>
#include<iostream>

char* heap_ptr = (char*)mmap(nullptr, 65536, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

typedef struct memID
{
    size_t size;
    bool is_free = 0;
    bool is_marked = 0;
} memID;

memID* free_list_head = (memID*)heap_ptr;
void* heap_start = (void*)heap_ptr;
void* heap_end = (void*)(heap_start + 65536);
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
  memID *next_id = (memID*)(ptr + ptr_id->size + sizeof(memID));

  ptr_id->is_free = 1;

  if(next_id->is_free)
  {
    ptr_id->size = ptr_id->size + next_id->size + sizeof(memID);
    return;
  }

}

void get_stack_bounds(void **stack_top)
{
  pthread_t self =  pthread_self();
  pthread_attr_t attr;
  void* addr;
  size_t size;

  pthread_getattr_np(self, &attr);
  pthread_attr_getstack(&attr, &addr, &size);
  pthread_attr_destroy(&attr);

  *stack_top = (void*)((char*)addr + size);

}

void* get_current_sp()
{
  return (__builtin_frame_address(0));
}

bool looks_like_pointer(void* pointer)
{
  if(pointer <= heap_end && pointer >= heap_start) return true;

  return false;
}

void traverse_mark(memID* ptr_id)
{

  if(ptr_id->is_free || ptr_id->is_marked) return;

  void* ptr_payload = (void*)((char*)ptr_id + sizeof(memID));
  
  ptr_id->is_marked = 1;

  void** payload = (void**)(ptr_payload);
  size_t count = (ptr_id->size/sizeof(void*));

  for(size_t i = 0; i < count; i++)
  {
    memID* candidate_id = (memID*)((char*)payload[i] - sizeof(memID));
    if(looks_like_pointer(payload[i]))
    {
      if(!candidate_id->is_free) traverse_mark(candidate_id);
    }
  }
}

void mark_algo()
{
  asm volatile("" ::: "memory"); //flushing registers 

  void* current_sp = get_current_sp();
  void* stack_top;
  get_stack_bounds(&stack_top);

  for(void** sp = (void**)current_sp; sp < (void**)stack_top; ++sp)
  {
    if(looks_like_pointer(*sp))
    {
      memID* ptr_id = (memID*)((char*)*sp - sizeof(memID));
      if(ptr_id->is_free || ptr_id->is_marked) continue;

      traverse_mark(ptr_id);      
    }
  }
}

void sweep_algo()
{
  memID* prev_id = nullptr;
  memID* current_id = (memID*)heap_start;
  
  while((void*)current_id < heap_end)
  {
    memID* next_id = (memID*)((char*)current_id + sizeof(memID) + current_id->size);

    if(current_id->is_free || current_id->is_marked) 
    {
      current_id->is_marked = 0;
      current_id = next_id;
      continue;
    }

    current_id->is_marked = 0;
    current_id->is_free = 1;

    if(next_id->is_free) current_id->size += next_id->size;
    if(prev_id == nullptr) free_list_head = current_id; continue;

    char* prev_payload = (char*)prev_id + sizeof(memID);

    prev_payload = &current_id;

    prev_id = current_id;
    current_id = (memID*)((char*)current_id + sizeof(memID) + current_id->size);
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
