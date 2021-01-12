#include <stdio.h>
#include <stdlib.h>
#include "Style.h"
#include <termios.h>
#include <string.h>
#include <stdbool.h>

static struct termios old, new;

// -------- Initiliaser les nouveaux parametres du terminal -------- // 

void initTermios(int echo) {
    tcgetattr(0, &old);  // Recuperer les parametres de l ancien terminal
    new = old;  // Definit les nouveaux parametres comme les anciens
    new.c_lflag &= ~ICANON;  // Desactive le buffer
    new.c_lflag &= echo ? ECHO : ~ECHO;  // Definit le mode
    tcsetattr(0, TCSANOW, &new); // Application des nouveaux parametres
}

// -------- Restauration des anciens parametres du terminal -------- //

void resetTermios(void) {
    tcsetattr(0, TCSANOW, &old);
}

// -------- Lecture d un caractere -------- //

char getch_(int echo) {
    char ch;
    initTermios(echo);
    ch = getchar();
    resetTermios();
    return ch;
}

// -------- Lecture d un caractere sans echo getch() -------- //

char getch(void) {
    return getch_(0);
}

// -------- Lecture d un caractere avec echo getche() -------- //

char getche(void) {
    return getch_(1);
}

color console_color(uint8_t r, uint8_t g, uint8_t b) {
    color c;
    c.r = r;
    c.g = g;
    c.b = b;
    return c;
}

void console_formatMode(char *content, uint8_t flags, ...) {
    va_list args;
    va_start(args, flags);
    char str[50] = "";
    bool firstAdded = false;

    if (flags & CONSOLE_FLAG_BOLD) {
        strcat(str, "1");
        firstAdded = true;
    }

    if (flags & CONSOLE_FLAG_UNDERLINE) {
        if (firstAdded)
            strcat(str, ";4");
        else {
            strcat(str, "4");
            firstAdded = true;
        }
    }

    if (flags & CONSOLE_FLAG_BLINK) {
        if (firstAdded)
            strcat(str, ";5");
        else {
            strcat(str, "5");
            firstAdded = true;
        }
    }

    if (flags & CONSOLE_FLAG_REVERSE_COLOR) {
        if (firstAdded)
            strcat(str, ";7");
        else
            strcat(str, "7");
    }

    printf("\x1b[%sm", str);
    vprintf(content, args);
    printf("\x1b[0m");
    va_end(args);
}

void console_formatRGBBackground(char *content, color background, ...) {
    va_list args;
    va_start(args, background);
    printf("\x1b[48;2;%d;%d;%dm", background.r, background.g, background.b);
    vprintf(content, args);
    printf("\x1b[0m");
    va_end(args);
}

void console_formatRGBBackgroundMode(char *content, color background, uint8_t flags, ...) {
    va_list args;
    va_start(args, flags);
    char str[50];
    snprintf(str, 50, "48;2;%d;%d;%d", background.r, background.g, background.b);
    if (flags & CONSOLE_FLAG_BOLD)
        strcat(str, ";1");
    if (flags & CONSOLE_FLAG_UNDERLINE)
        strcat(str, ";4");
    if (flags & CONSOLE_FLAG_BLINK)
        strcat(str, ";5");
    if (flags & CONSOLE_FLAG_REVERSE_COLOR)
        strcat(str, ";7");
    printf("\x1b[%sm", str);
    vprintf(content, args);
    printf("\x1b[0m");
    va_end(args);
}

void console_formatSystemBackground(char *content, int background, ...) {
    va_list args;
    va_start(args, background);
    printf("\x1b[%dm", background);
    vprintf(content, args);
    printf("\x1b[0m");
    va_end(args);
}

void console_formatSystemBackgroundMode(char *content, int background, uint8_t flags, ...) {
    va_list args;
    va_start(args, flags);
    char str[50];
    snprintf(str, 50, "%d", background + 10);
    if (flags & CONSOLE_FLAG_BOLD)
        strcat(str, ";1");
    if (flags & CONSOLE_FLAG_UNDERLINE)
        strcat(str, ";4");
    if (flags & CONSOLE_FLAG_BLINK)
        strcat(str, ";5");
    if (flags & CONSOLE_FLAG_REVERSE_COLOR)
        strcat(str, ";7");
    printf("\x1b[%sm", str);
    vprintf(content, args);
    printf("\x1b[0m");
    va_end(args);
}

void console_formatRGBForeground(char *content, color foreground, ...) {
    va_list args;
    va_start(args, foreground);
    printf("\x1b[38;2;%d;%d;%dm", foreground.r, foreground.g, foreground.b);
    vprintf(content, args);
    printf("\x1b[0m");
    va_end(args);
}

void console_formatRGBForegroundMode(char *content, color foreground, uint8_t flags, ...) {
    va_list args;
    va_start(args, flags);
    char str[50];
    snprintf(str, 50, "38;2;%d;%d;%d", foreground.r, foreground.g, foreground.b);
    if (flags & CONSOLE_FLAG_BOLD)
        strcat(str, ";1");
    if (flags & CONSOLE_FLAG_UNDERLINE)
        strcat(str, ";4");
    if (flags & CONSOLE_FLAG_BLINK)
        strcat(str, ";5");
    if (flags & CONSOLE_FLAG_REVERSE_COLOR)
        strcat(str, ";7");
    printf("\x1b[%sm", str);
    vprintf(content, args);
    printf("\x1b[0m");
    va_end(args);
}

void console_formatSystemForeground(char *content, int foreground, ...) {
    va_list args;
    va_start(args, foreground);
    printf("\x1b[%dm", foreground);
    vprintf(content, args);
    printf("\x1b[0m");
    va_end(args);
}

void console_formatSystemForegroundMode(char *content, int foreground, uint8_t flags, ...) {
    va_list args;
    va_start(args, flags);
    char str[50];
    snprintf(str, 50, "%d", foreground);
    if (flags & CONSOLE_FLAG_BOLD)
        strcat(str, ";1");
    if (flags & CONSOLE_FLAG_UNDERLINE)
        strcat(str, ";4");
    if (flags & CONSOLE_FLAG_BLINK)
        strcat(str, ";5");
    if (flags & CONSOLE_FLAG_REVERSE_COLOR)
        strcat(str, ";7");
    printf("\x1b[%sm", str);
    vprintf(content, args);
    printf("\x1b[0m");
    va_end(args);
}

void console_formatRGBColor(char *content, color foreground, color background, ...) {
    va_list args;
    va_start(args, background);
    printf("\x1b[38;2;%d;%d;%d;48;2;%d;%d;%dm", foreground.r, foreground.g, foreground.b, background.r, background.g, background.b);
    vprintf(content, args);
    printf("\x1b[0m");
    va_end(args);
}

void console_formatRGBColorMode(char *content, color foreground, color background, uint8_t flags, ...) {
    va_list args;
    va_start(args, flags);
    char str[50];
    snprintf(str, 50, "38;2;%d;%d;%d;48;2;%d;%d;%d", foreground.r, foreground.g, foreground.b, background.r, background.g, background.b);
    if (flags & CONSOLE_FLAG_BOLD)
        strcat(str, ";1");
    if (flags & CONSOLE_FLAG_UNDERLINE)
        strcat(str, ";4");
    if (flags & CONSOLE_FLAG_BLINK)
        strcat(str, ";5");
    if (flags & CONSOLE_FLAG_REVERSE_COLOR)
        strcat(str, ";7");
    printf("\x1b[%sm", str);
    vprintf(content, args);
    printf("\x1b[0m");
    va_end(args);
}

void console_formatSystemColor(char *content, int foreground, int background, ...) {
    va_list args;
    va_start(args, background);
    printf("\x1b[%d;%dm", foreground, background + 10);
    vprintf(content, args);
    printf("\x1b[0m");
    va_end(args);
}

void console_formatSystemColorMode(char *content, int foreground, int background, uint8_t flags, ...) {
    va_list args;
    va_start(args, flags);
    char str[50];
    snprintf(str, 50, "%d;%d", foreground, background + 10);
    if (flags & CONSOLE_FLAG_BOLD)
        strcat(str, ";1");
    if (flags & CONSOLE_FLAG_UNDERLINE)
        strcat(str, ";4");
    if (flags & CONSOLE_FLAG_BLINK)
        strcat(str, ";5");
    if (flags & CONSOLE_FLAG_REVERSE_COLOR)
        strcat(str, ";7");
    printf("\x1b[%sm", str);
    vprintf(content, args);
    printf("\x1b[0m");
    va_end(args);
}

void console_saveCursorPosition() {
    printf("\x1b[s");
}

void console_restoreCursorPosition() {
    printf("\x1b[u");
}

void console_eraseEndOfLine() {
    printf("\x1b[K");
}

void console_setCursorPosition(int x, int y) {
    printf("\x1b[%d;%dH", y, x);
}

void console_clearScreen() {
    printf("\x1b[2J\x1b[1;1H");
}

int console_getArrowPressed() {
    int var = getch();
    if (var == 27) {
        getch();
        switch (getch()) {
            case 65:
                return CONSOLE_KEY_UP;
            case 66:
                return CONSOLE_KEY_DOWN;
            case 68:
                return CONSOLE_KEY_LEFT;
            case 67:
                return CONSOLE_KEY_RIGHT;
            default:
                return CONSOLE_KEY_OTHER;
        }
    }
    else if (var == 10)
        return CONSOLE_KEY_RETURN;
    else
        return CONSOLE_KEY_OTHER;
}
