#include "io.h"
static int mx=512,my=384; static unsigned char cycle;
void mouse_init(void){outb(0x64,0xA8); outb(0x64,0x20); while(!(inb(0x64)&1)); unsigned char s=inb(0x60); outb(0x64,0x60); while(!(inb(0x64)&2)); outb(0x60,s|2); outb(0x64,0xD4); while(!(inb(0x64)&2)); outb(0x60,0xF4); while(!(inb(0x64)&1)); inb(0x60); }
void mouse_poll(int *x,int *y){ if((inb(0x64)&1)&&(inb(0x64)&0x20)){unsigned char d=inb(0x60); if(cycle==0&&(d&8))cycle=1; else if(cycle==1)cycle=2; else if(cycle==2){int dx=(signed char)d; (void)dx; cycle=0;} } *x=mx;*y=my; }
