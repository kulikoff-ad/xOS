#include <stddef.h>
static unsigned char heap[64*1024]; static size_t used;
void *kmalloc(size_t n){ if(!n || used+n>sizeof heap)return 0; void *p=heap+used; used=(used+n+7)&~7; return p; }
size_t memory_used(void){return used;}
