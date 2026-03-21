#include <sys/mman.h>

char* heap_ptr = (char*)mmap(nullptr, 65536, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);