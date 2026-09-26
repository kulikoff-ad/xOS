/*
 * xOS v1 Lite kernel
 *
 * This is deliberately small and freestanding: it starts in 32-bit protected
 * mode, owns VGA text memory, and talks directly to the PS/2 keyboard. There
 * is no libc, firmware runtime, or hidden monitor underneath the shell.
 */

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
typedef signed int     s32;

enum {
    VGA_WIDTH = 80,
    VGA_HEIGHT = 25,
    INPUT_MAX = 70
};

enum color {
    BLACK = 0,
    BLUE = 1,
    GREEN = 2,
    CYAN = 3,
    RED = 4,
    MAGENTA = 5,
    BROWN = 6,
    LIGHT_GREY = 7,
    DARK_GREY = 8,
    LIGHT_BLUE = 9,
    LIGHT_GREEN = 10,
    LIGHT_CYAN = 11,
    LIGHT_RED = 12,
    LIGHT_MAGENTA = 13,
    YELLOW = 14,
    WHITE = 15
};

static volatile u16 *const vga = (volatile u16 *)0xb8000;
static u8 term_color = (u8)((BLUE << 4) | WHITE);
static u8 term_row;
static u8 term_col;

static inline void outb(u16 port, u8 value)
{
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline u8 inb(u16 port)
{
    u8 value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void io_wait(void)
{
    __asm__ volatile ("outb %%al, $0x80" : : "a"((u8)0));
}

static u8 make_color(enum color foreground, enum color background)
{
    return (u8)(((u8)background << 4) | (u8)foreground);
}

static void cursor_update(void)
{
    u16 position = (u16)term_row * VGA_WIDTH + term_col;
    outb(0x3d4, 0x0f);
    outb(0x3d5, (u8)(position & 0xff));
    outb(0x3d4, 0x0e);
    outb(0x3d5, (u8)(position >> 8));
}

static void terminal_scroll(void)
{
    u32 row;
    u32 col;

    for (row = 1; row < VGA_HEIGHT; ++row) {
        for (col = 0; col < VGA_WIDTH; ++col) {
            vga[(row - 1) * VGA_WIDTH + col] = vga[row * VGA_WIDTH + col];
        }
    }
    for (col = 0; col < VGA_WIDTH; ++col) {
        vga[(VGA_HEIGHT - 1) * VGA_WIDTH + col] =
            ((u16)term_color << 8) | (u16)' ';
    }
    term_row = VGA_HEIGHT - 1;
}

static void terminal_clear(enum color foreground, enum color background)
{
    u32 i;
    term_color = make_color(foreground, background);
    for (i = 0; i < VGA_WIDTH * VGA_HEIGHT; ++i) {
        vga[i] = ((u16)term_color << 8) | (u16)' ';
    }
    term_row = 0;
    term_col = 0;
    cursor_update();
}

static void terminal_putc(char character)
{
    if (character == '\n') {
        term_col = 0;
        ++term_row;
    } else if (character == '\r') {
        term_col = 0;
    } else if (character == '\b') {
        if (term_col != 0) {
            --term_col;
            vga[(u32)term_row * VGA_WIDTH + term_col] =
                ((u16)term_color << 8) | (u16)' ';
        }
    } else if (character == '\t') {
        terminal_putc(' ');
        terminal_putc(' ');
        terminal_putc(' ');
        terminal_putc(' ');
    } else {
        vga[(u32)term_row * VGA_WIDTH + term_col] =
            ((u16)term_color << 8) | (u8)character;
        ++term_col;
        if (term_col >= VGA_WIDTH) {
            term_col = 0;
            ++term_row;
        }
    }

    if (term_row >= VGA_HEIGHT) {
        terminal_scroll();
    }
    cursor_update();
}

static void terminal_write(const char *text)
{
    while (*text != '\0') {
        terminal_putc(*text++);
    }
}

static void terminal_write_color(const char *text, enum color foreground)
{
    u8 old_color = term_color;
    term_color = make_color(foreground, BLUE);
    terminal_write(text);
    term_color = old_color;
}

static void terminal_write_u32(u32 value)
{
    char digits[10];
    u32 length = 0;

    if (value == 0) {
        terminal_putc('0');
        return;
    }
    while (value != 0 && length < sizeof(digits)) {
        digits[length++] = (char)('0' + (value % 10));
        value /= 10;
    }
    while (length != 0) {
        terminal_putc(digits[--length]);
    }
}

static void terminal_write_hex(u32 value)
{
    static const char hex[] = "0123456789abcdef";
    s32 shift;

    terminal_write("0x");
    for (shift = 28; shift >= 0; shift -= 4) {
        terminal_putc(hex[(value >> (u32)shift) & 0x0f]);
    }
}

static u32 text_length(const char *text)
{
    u32 length = 0;
    while (text[length] != '\0') {
        ++length;
    }
    return length;
}

static int text_equal(const char *left, const char *right)
{
    u32 i = 0;
    while (left[i] != '\0' && right[i] != '\0') {
        if (left[i] != right[i]) {
            return 0;
        }
        ++i;
    }
    return left[i] == right[i];
}

static int text_starts_with(const char *text, const char *prefix)
{
    u32 i = 0;
    while (prefix[i] != '\0') {
        if (text[i] != prefix[i]) {
            return 0;
        }
        ++i;
    }
    return 1;
}

static char ascii_lower(char character)
{
    if (character >= 'A' && character <= 'Z') {
        return (char)(character + ('a' - 'A'));
    }
    return character;
}

static void write_line(const char *text)
{
    terminal_write(text);
    terminal_putc('\n');
}

static void print_prompt(void)
{
    if (term_row >= VGA_HEIGHT - 1) {
        terminal_scroll();
    }
    terminal_write_color("xOS> ", YELLOW);
    term_color = make_color(LIGHT_GREY, BLUE);
    cursor_update();
}

static void command_help(void)
{
    write_line("Commands:");
    write_line("  help       show this list");
    write_line("  about      what is xOS");
    write_line("  info       CPU, memory and boot information");
    write_line("  files/ls   list the built-in files");
    write_line("  cat FILE   read a built-in file");
    write_line("  settings   show the current machine settings");
    write_line("  time       read the RTC clock");
    write_line("  echo TEXT  print text");
    write_line("  clear      clear the screen");
    write_line("  reboot     restart the virtual machine");
    write_line("  halt       stop the CPU");
    write_line("Tip: Ctrl+L also clears the screen.");
}

static void command_about(void)
{
    write_line("xOS v1 Lite");
    write_line("A small, bootable operating system for old PCs.");
    write_line("The kernel is freestanding 32-bit C and x86 assembly.");
    write_line("It boots directly from a BIOS disk and has no host OS below it.");
}

static void command_info(void)
{
    write_line("System information");
    terminal_write("  architecture: ");
    terminal_write_color("i386 protected mode\n", LIGHT_CYAN);
    terminal_write("  video:         ");
    terminal_write_color("VGA text 80x25\n", LIGHT_CYAN);
    terminal_write("  keyboard:      ");
    terminal_write_color("PS/2 polling\n", LIGHT_CYAN);
    terminal_write("  kernel entry:  ");
    terminal_write_hex(0x1000);
    terminal_putc('\n');
    terminal_write("  stack top:     ");
    terminal_write_hex(0x90000);
    terminal_putc('\n');
    terminal_write("  conventional memory visible: ");
    terminal_write_u32(640);
    terminal_write(" KiB\n");
}

static void command_files(void)
{
    write_line("Built-in files (read-only ROM files):");
    write_line("  readme.txt");
    write_line("  welcome.txt");
    write_line("  license.txt");
}

static void command_cat(const char *name)
{
    if (text_equal(name, "readme.txt")) {
        write_line("xOS is a tiny operating system that boots on a legacy BIOS PC.");
        write_line("Try help, info, files, settings, time, or echo hello.");
    } else if (text_equal(name, "welcome.txt")) {
        write_line("Welcome to xOS!");
        write_line("This shell is running directly in ring 0, not in an emulator console.");
    } else if (text_equal(name, "license.txt")) {
        write_line("xOS source is provided by its project repository.");
        write_line("See the repository license for redistribution terms.");
    } else {
        terminal_write("cat: file not found: ");
        write_line(name);
    }
}

static void command_settings(void)
{
    write_line("xOS machine settings");
    write_line("  boot:       BIOS / INT 13h extensions");
    write_line("  mode:       32-bit protected mode");
    write_line("  disk:       first BIOS drive, raw image");
    write_line("  display:    VGA-compatible 80x25 text");
    write_line("  input:      PS/2 keyboard, US layout");
    write_line("  UTM:        x86_64 emulation, Legacy BIOS, IDE disk");
}

static u8 cmos_read(u8 register_number)
{
    outb(0x70, register_number);
    io_wait();
    return inb(0x71);
}

static u8 from_bcd(u8 value)
{
    return (u8)((value & 0x0f) + ((value >> 4) * 10));
}

static void write_two_digits(u8 value)
{
    terminal_putc((char)('0' + (value / 10)));
    terminal_putc((char)('0' + (value % 10)));
}

static void command_time(void)
{
    u8 second = from_bcd(cmos_read(0x00));
    u8 minute = from_bcd(cmos_read(0x02));
    u8 hour = from_bcd(cmos_read(0x04));
    u8 day = from_bcd(cmos_read(0x07));
    u8 month = from_bcd(cmos_read(0x08));
    u8 year = from_bcd(cmos_read(0x09));

    terminal_write("RTC: 20");
    write_two_digits(year);
    terminal_putc('-');
    write_two_digits(month);
    terminal_putc('-');
    write_two_digits(day);
    terminal_putc(' ');
    write_two_digits(hour);
    terminal_putc(':');
    write_two_digits(minute);
    terminal_putc(':');
    write_two_digits(second);
    terminal_putc('\n');
}

static void command_reboot(void)
{
    u32 timeout = 1000000;
    write_line("Rebooting...");
    while ((inb(0x64) & 0x02) != 0 && timeout != 0) {
        --timeout;
    }
    outb(0x64, 0xfe);
    for (;;) {
        __asm__ volatile ("cli; hlt");
    }
}

static void command_halt(void)
{
    write_line("CPU halted. You can close the UTM window.");
    for (;;) {
        __asm__ volatile ("cli; hlt");
    }
}

static void command_desktop(void)
{
    terminal_clear(LIGHT_GREY, BLUE);
    terminal_write_color(" xOS v1 Lite    desktop\n", LIGHT_CYAN);
    write_line("+------------------------------------------------------------------------------+");
    write_line("|  [ terminal ]    [ files ]    [ settings ]                                  |");
    write_line("|                                                                            |");
    write_line("|  A tiny, responsive operating system for low-end hardware.                  |");
    write_line("|  Open a command with the keyboard: type help and press Enter.               |");
    write_line("|                                                                            |");
    write_line("+------------------------------------------------------------------------------+");
    terminal_putc('\n');
}

static void execute_command(char *command)
{
    u32 length = text_length(command);
    u32 i;
    char *argument;

    while (length != 0 && command[length - 1] == ' ') {
        command[--length] = '\0';
    }
    if (length == 0) {
        return;
    }

    if (text_equal(command, "help")) {
        command_help();
    } else if (text_equal(command, "about")) {
        command_about();
    } else if (text_equal(command, "info")) {
        command_info();
    } else if (text_equal(command, "files") || text_equal(command, "ls")) {
        command_files();
    } else if (text_equal(command, "settings")) {
        command_settings();
    } else if (text_equal(command, "time")) {
        command_time();
    } else if (text_equal(command, "clear") || text_equal(command, "cls")) {
        terminal_clear(LIGHT_GREY, BLUE);
    } else if (text_equal(command, "desktop")) {
        command_desktop();
    } else if (text_equal(command, "reboot")) {
        command_reboot();
    } else if (text_equal(command, "halt") || text_equal(command, "shutdown")) {
        command_halt();
    } else if (text_starts_with(command, "echo ")) {
        write_line(command + 5);
    } else if (text_starts_with(command, "cat ")) {
        argument = command + 4;
        for (i = 0; argument[i] != '\0'; ++i) {
            argument[i] = ascii_lower(argument[i]);
        }
        command_cat(argument);
    } else {
        terminal_write("Unknown command: ");
        terminal_write(command);
        write_line(" (type help)");
    }
}

static const char keymap[128] = {
    [0x02] = '1', [0x03] = '2', [0x04] = '3', [0x05] = '4',
    [0x06] = '5', [0x07] = '6', [0x08] = '7', [0x09] = '8',
    [0x0a] = '9', [0x0b] = '0', [0x0c] = '-', [0x0d] = '=',
    [0x0e] = '\b', [0x0f] = '\t',
    [0x10] = 'q', [0x11] = 'w', [0x12] = 'e', [0x13] = 'r',
    [0x14] = 't', [0x15] = 'y', [0x16] = 'u', [0x17] = 'i',
    [0x18] = 'o', [0x19] = 'p', [0x1a] = '[', [0x1b] = ']',
    [0x1c] = '\n', [0x1e] = 'a', [0x1f] = 's', [0x20] = 'd',
    [0x21] = 'f', [0x22] = 'g', [0x23] = 'h', [0x24] = 'j',
    [0x25] = 'k', [0x26] = 'l', [0x27] = ';', [0x28] = '\'',
    [0x29] = '`', [0x2b] = '\\', [0x2c] = 'z', [0x2d] = 'x',
    [0x2e] = 'c', [0x2f] = 'v', [0x30] = 'b', [0x31] = 'n',
    [0x32] = 'm', [0x33] = ',', [0x34] = '.', [0x35] = '/',
    [0x37] = '*', [0x39] = ' '
};

static char shifted_punctuation(char character)
{
    switch (character) {
    case '1': return '!';
    case '2': return '@';
    case '3': return '#';
    case '4': return '$';
    case '5': return '%';
    case '6': return '^';
    case '7': return '&';
    case '8': return '*';
    case '9': return '(';
    case '0': return ')';
    case '-': return '_';
    case '=': return '+';
    case '[': return '{';
    case ']': return '}';
    case '\\': return '|';
    case ';': return ':';
    case '\'': return '"';
    case '`': return '~';
    case ',': return '<';
    case '.': return '>';
    case '/': return '?';
    default: return character;
    }
}

static char keyboard_get_char(void)
{
    static int shift;
    static int caps_lock;
    static int control;
    static int extended;
    u8 scan_code;
    char character;

    while ((inb(0x64) & 0x01) == 0) {
        __asm__ volatile ("pause");
    }
    scan_code = inb(0x60);

    if (scan_code == 0xe0) {
        extended = 1;
        return 0;
    }
    if (extended != 0) {
        extended = 0;
        return 0;
    }
    if (scan_code == 0x2a || scan_code == 0x36) {
        shift = 1;
        return 0;
    }
    if (scan_code == 0xaa || scan_code == 0xb6) {
        shift = 0;
        return 0;
    }
    if (scan_code == 0x1d) {
        control = 1;
        return 0;
    }
    if (scan_code == 0x9d) {
        control = 0;
        return 0;
    }
    if (scan_code == 0x3a) {
        caps_lock = !caps_lock;
        return 0;
    }
    if ((scan_code & 0x80) != 0) {
        return 0;
    }

    character = keymap[scan_code & 0x7f];
    if (character == 0) {
        return 0;
    }
    if (control != 0 && (character == 'l' || character == 'L')) {
        return '\f';
    }
    if (shift != 0) {
        character = shifted_punctuation(character);
    }
    if (character >= 'a' && character <= 'z' && ((shift != 0) ^ (caps_lock != 0))) {
        character = (char)(character - ('a' - 'A'));
    }
    return character;
}

static void shell(void)
{
    static char input[INPUT_MAX + 1];
    u32 input_length = 0;

    print_prompt();
    for (;;) {
        char character = keyboard_get_char();

        if (character == 0) {
            continue;
        }
        if (character == '\f') {
            terminal_clear(LIGHT_GREY, BLUE);
            input_length = 0;
            print_prompt();
            continue;
        }
        if (character == '\n') {
            input[input_length] = '\0';
            terminal_putc('\n');
            execute_command(input);
            input_length = 0;
            print_prompt();
            continue;
        }
        if (character == '\b') {
            if (input_length != 0) {
                --input_length;
                input[input_length] = '\0';
                terminal_putc('\b');
            }
            continue;
        }
        if (character >= ' ' && character <= '~' && input_length < INPUT_MAX) {
            input[input_length++] = character;
            terminal_putc(character);
        }
    }
}

void kmain(void)
{
    terminal_clear(LIGHT_GREY, BLUE);
    terminal_write_color(" xOS v1 Lite", LIGHT_CYAN);
    terminal_write(" - fast text desktop\n");
    terminal_write(" ----------------------------------------\n");
    terminal_write(" Kernel online at ");
    terminal_write_hex(0x1000);
    terminal_write(" | keyboard online\n\n");
    terminal_write_color(" Welcome. Type help for commands.\n\n", WHITE);
    shell();
}
