#include "io.h"
static const char map[128]="\0\0331234567890-_=\b\tqwertyuiop[]\n\0asdfghjkl;'`\0\\zxcvbnm,./\0*\0 \0";
char keyboard_poll(void){if(!(inb(0x64)&1))return 0; unsigned char s=inb(0x60); if(s&0x80||s>=128)return 0; return map[s];}
