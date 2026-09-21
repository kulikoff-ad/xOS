#include "../drivers/io.h"
void reboot(void){outb(0x64,0xFE); for(;;)__asm__ volatile("hlt");}
void shutdown(void){ /* QEMU/Bochs ACPI shutdown */ outb(0xB004,0x00); for(;;)__asm__ volatile("hlt");}
unsigned long cpu_hz(void){unsigned int a,b,c,d; __asm__ volatile("cpuid":"=a"(a),"=b"(b),"=c"(c),"=d"(d):"a"(0)); return a;}
