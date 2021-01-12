#ifndef DEF_CONSOLE_COLORS_H
#define DEF_CONSOLE_COLORS_H
#include <stdint.h>
#include <stdarg.h>

#define CONSOLE_COLOR_BLACK 30
#define CONSOLE_COLOR_RED 31
#define CONSOLE_COLOR_GREEN 32
#define CONSOLE_COLOR_YELLOW 33
#define CONSOLE_COLOR_BLUE 34
#define CONSOLE_COLOR_MAGENTA 35
#define CONSOLE_COLOR_CYAN 36
#define CONSOLE_COLOR_GREY 90
#define CONSOLE_COLOR_GRAY CONSOLE_COLOR_GREY
#define CONSOLE_COLOR_BRIGHT_GREY 37
#define CONSOLE_COLOR_BRIGHT_GRAY CONSOLE_COLOR_BRIGHT_GREY
#define CONSOLE_COLOR_BRIGHT_RED 91
#define CONSOLE_COLOR_BRIGHT_GREEN 92
#define CONSOLE_COLOR_BRIGHT_YELLOW 93
#define CONSOLE_COLOR_BRIGHT_BLUE 94
#define CONSOLE_COLOR_BRIGHT_MAGENTA 95
#define CONSOLE_COLOR_BRIGHT_CYAN 96
#define CONSOLE_COLOR_WHITE 97

#define CONSOLE_FLAG_BOLD 0b1 << 0
#define CONSOLE_FLAG_UNDERLINE 0b1 << 1
#define CONSOLE_FLAG_BLINK 0b1 << 2
#define CONSOLE_FLAG_REVERSE_COLOR 0b1 << 3

// -------- Definition des differents types Style (a été repris d'un forum afin de styliser notre projet) -------- //

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} color;

color console_color(uint8_t red, uint8_t green, uint8_t blue);

void console_formatMode(char *content, uint8_t flags, ...);

void console_formatRGBBackground(char *content, color background, ...);

void console_formatRGBBackgroundMode(char *content, color background, uint8_t flags, ...);

void console_formatSystemBackground(char *content, int background, ...);

void console_formatSystemBackgroundMode(char *content, int background, uint8_t flags, ...);

void console_formatRGBForeground(char *content, color foreground, ...);

void console_formatRGBForegroundMode(char *content, color foreground, uint8_t flags, ...);

void console_formatSystemForeground(char *content, int foreground, ...);

void console_formatSystemForegroundMode(char *, int foreground, uint8_t flags, ...);

void console_formatRGBColor(char *content, color foreground, color background, ...);

void console_formatRGBColorMode(char *content, color foreground, color background, uint8_t flags, ...);

void console_formatSystemColor(char *content, int foreground, int background, ...);

void console_formatSystemColorMode(char *content, int foreground, int background, uint8_t flags, ...);

void console_saveCursorPosition();

void console_restoreCursorPosition();

void console_eraseEndOfLine();

void console_setCursorPosition(int x, int y);

#define CONSOLE_KEY_OTHER 0
#define CONSOLE_KEY_UP 1
#define CONSOLE_KEY_DOWN 2
#define CONSOLE_KEY_LEFT 3
#define CONSOLE_KEY_RIGHT 4
#define CONSOLE_KEY_RETURN 5

int console_getArrowPressed();

void console_clearScreen();

char getch();

char getche();

#endif