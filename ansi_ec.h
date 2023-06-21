#ifndef ANSI_EC_H
#define ANSI_EC_H
/**
 * ANSI Escape Charactors
 */

#define EPRT(...) fprintf(stderr, __VA_ARGS__);

#define CURSOR_HOME() { EPRT("\x1b[H"); }
#define CURSOR_TO(row, col) { EPRT("\x1b[%d;%df", row, col); }
#define CURSOR_UP(n) { EPRT("\x1b[%dA", n); }
#define CURSOR_DOWN(n) { EPRT("\x1b[%dB", n); }
#define CURSOR_RIGHT(n) { EPRT("\x1b[%dC", n); }
#define CURSOR_LEFT(n) { EPRT("\x1b[%dD", n); }

#define ERASE_SCREEN() { EPRT("\x1b[2J"); }
#define ERASE_LINE()  { EPRT("\x1b[2K"); }

#define COLOR_NONE                 "\e[0m"
#define BACK_COLOR_RED                  "\e[0;41m"
#define BACK_COLOR_YELLOW               "\e[1;43m"
#define BACK_COLOR_BLUE                 "\e[0;44m"
#define BACK_COLOR_CYAN                 "\e[0;46m"
#define BACK_COLOR_GREEN                "\e[0;42m"
#define BACK_COLOR_GRAY                 "\e[0;47m"

// #define CURSOR_HOME "\x1b[H"

// #define CURSOR_TO(row, col) "\x1b[" #row ";" #col "f"

// #define CURSOR_UP "\x1b[1A"
// #define CURSOR_DOWN "\x1b[1B"
// #define CURSOR_RIGHT "\x1b[1C"
// #define CURSOR_LEFT "\x1b[1D"

// #define CURSOR_UP_N(n) "\x1b[" #n "A"
// #define CURSOR_DOWN_N(n) "\x1b[" #n "B"
// #define CURSOR_RIGHT_N(n) "\x1b[" #n "C"
// #define CURSOR_LEFT_N(n) "\x1b[" #n "D"

#endif