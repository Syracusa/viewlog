/**
 * ANSI Escape Character Helper
 */
#ifndef ANSI_EC_H
#define ANSI_EC_H

/** Format string and print to stderr */
#define EPRT(...) fprintf(stderr, __VA_ARGS__);

/** Move cursor to 0, 0 */
#define CURSOR_HOME()   \
    {                   \
        EPRT("\x1b[H"); \
    }

/** Move cursor to row, col */
#define CURSOR_TO(row, col)            \
    {                                  \
        EPRT("\x1b[%d;%df", row, col); \
    }

/** Move cursor upward N times */
#define CURSOR_UP(n)         \
    {                        \
        EPRT("\x1b[%dA", n); \
    }

/** Move cursor downward N times */
#define CURSOR_DOWN(n)       \
    {                        \
        EPRT("\x1b[%dB", n); \
    }

/** Move cursor right N times */
#define CURSOR_RIGHT(n)      \
    {                        \
        EPRT("\x1b[%dC", n); \
    }

/** Move cursor left N times */
#define CURSOR_LEFT(n)       \
    {                        \
        EPRT("\x1b[%dD", n); \
    }

/** Clear entire screen */
#define ERASE_SCREEN()   \
    {                    \
        EPRT("\x1b[2J"); \
    }

/** Clear line that cursor is on */
#define ERASE_LINE()     \
    {                    \
        EPRT("\x1b[2K"); \
    }

/** Colors */
#define COLOR_NONE "\e[0m"
#define BACK_COLOR_RED "\e[0;41m"
#define BACK_COLOR_YELLOW "\e[1;43m"
#define BACK_COLOR_BLUE "\e[0;44m"
#define BACK_COLOR_CYAN "\e[0;46m"
#define BACK_COLOR_GREEN "\e[0;42m"
#define BACK_COLOR_GRAY "\e[0;47m"

#endif