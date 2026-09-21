#include <stddef.h>
extern void text(int,int,const char*,unsigned); extern void rect(int,int,int,int,unsigned); extern void reboot(void); extern void shutdown(void); extern int fs_count(void); extern const char *fs_name(int); extern const char *fs_read(const char*);
static char line[80];static int n; static int row=0; static unsigned green=0x00B8E090;
static int eq(const char*a,const char*b){while(*a&&*a==*b){a++;b++;}return !*a&&!*b;}
static void out(const char*s){text(310,195+row*18,s,green);if(++row>10)row=0;}
void terminal_draw(void){text(310,195+row*18,"$ ",green);text(334,195+row*18,line,green);}
void terminal_key(char c){if(c=='\n'){line[n]=0;if(eq(line,"help"))out("help clear about version sysinfo files reboot shutdown");else if(eq(line,"clear")){rect(300,130,570,290,0x001F2834);row=0;}else if(eq(line,"about"))out("xOS v1 Lite - small and fast");else if(eq(line,"version"))out("xOS v1 Lite 0.1.0");else if(eq(line,"sysinfo")){out("Architecture: x86  Display: 1024x768");out("RAM: boot memory available");}else if(eq(line,"files")){for(int i=0;i<fs_count();i++)out(fs_name(i));}else if(eq(line,"reboot"))reboot();else if(eq(line,"shutdown"))shutdown();else if(n)out("Unknown command. Type help.");n=0;line[0]=0;}else if(c=='\b'){if(n)n--;}else if(c>=32&&c<127&&n<78)line[n++]=c;terminal_draw();}
