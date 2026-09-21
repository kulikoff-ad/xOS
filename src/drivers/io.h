#ifndef XOS_IO_H
#define XOS_IO_H
static inline unsigned char inb(unsigned short p){unsigned char v; __asm__ volatile("inb %1,%0":"=a"(v):"Nd"(p));return v;}
static inline void outb(unsigned short p,unsigned char v){__asm__ volatile("outb %0,%1"::"a"(v),"Nd"(p));}
static inline void io_wait(void){__asm__ volatile("outb %%al,$0x80"::"a"(0));}
#endif
