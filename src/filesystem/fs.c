/* Tiny read-only virtual file layer: intentionally no disk dependency in Lite. */
struct vfile { const char *name; const char *data; };
static const struct vfile files[]={{"README.TXT","Welcome to xOS v1 Lite.\n"},{"ABOUT.TXT","A small operating system for old computers.\n"},{"LICENSE","MIT License\n"}};
int fs_count(void){return 3;} const char *fs_name(int i){return i>=0&&i<3?files[i].name:"";} const char *fs_read(const char *name){for(int i=0;i<3;i++){const char *a=files[i].name,*b=name;while(*a&&*a==*b){a++;b++;}if(!*a&&!*b)return files[i].data;}return 0;}
