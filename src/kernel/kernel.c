#include <stddef.h>
#include "../drivers/io.h"
extern void gui_init(unsigned,unsigned,int,int); extern void gui_input(char); extern char keyboard_poll(void); extern void mouse_init(void); extern void mouse_poll(int*,int*); extern void *kmalloc(size_t); extern size_t memory_used(void);
static void pit_init(void){outb(0x43,0x36);outb(0x40,0x9B);outb(0x40,0x2E);}
static unsigned long fb_addr,fb_pitch; static int fb_w=1024,fb_h=768;
/* Multiboot 1 information table. GRUB's framebuffer fields are used directly. */
void kmain(unsigned magic,unsigned mbi){
 if(magic!=0x2BADB002){for(;;)__asm__ volatile("hlt");}
 unsigned flags=*(unsigned*)mbi;
 if(flags&(1<<12)){fb_addr=*(unsigned*)(mbi+88);fb_pitch=*(unsigned*)(mbi+92);fb_w=*(unsigned*)(mbi+96);fb_h=*(unsigned*)(mbi+100);unsigned bpp=*(unsigned char*)(mbi+104);if(bpp!=32)fb_addr=0;}
 /* The requested GRUB mode is a safe fallback when a bootloader omits the tag. */
 if(!fb_addr)for(;;)__asm__ volatile("hlt");
 pit_init(); mouse_init(); gui_init(fb_addr,fb_pitch,fb_w,fb_h); (void)kmalloc(1);
 for(;;){char c=keyboard_poll();if(c)gui_input(c);int x,y;mouse_poll(&x,&y);__asm__ volatile("hlt");}
}
