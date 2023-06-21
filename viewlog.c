#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <termios.h>
#include <string.h>
#include <ctype.h>

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/select.h>

#include "ansi_ec.h"

#define TARGET_FILE "/tmp/viewlog/test1.log"

#define MAX_COL 300
#define MAX_ROW 100

typedef struct AppContext
{
    int win_row;
    int win_col;
    int realtime;
    size_t offset;
    char cmdbuf[MAX_ROW];
    int cmdbuf_offset;
} AppContext;

void get_cursor_pos(int *xp, int *yp)
{
    fprintf(stderr, "\033[6n");
    char buf[100];
    size_t offset = 0;
    size_t read = 0;
    while (read < 100)
    {
        read += fread(buf + offset, 1, 100, stdin);
        offset += read;
    }
    buf[offset] = '\0';
    sscanf(buf, "\033[%d;%dR", yp, xp);
}

void get_screen_size(int *row, int *col)
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    *row = w.ws_row;
    *col = w.ws_col;
}

size_t get_filesize(char *filename)
{
    FILE *f = fopen(filename, "r");
    fseek(f, 0, SEEK_END);
    size_t offset = ftell(f);
    fclose(f);
    return offset;
}

void draw_header(AppContext *ctx, const char *color)
{
    CURSOR_HOME();
    fprintf(stderr, "%s", color);
    for (int i = 0; i < ctx->win_col; i++)
        putc(' ', stderr);

    CURSOR_LEFT(MAX_COL);
    fprintf(stderr, "%s (Size : %'luKB)" COLOR_NONE,
            TARGET_FILE, ctx->offset / 1024);
}

void draw_footer(AppContext *ctx, const char *color)
{
    CURSOR_DOWN(MAX_ROW);
    CURSOR_LEFT(MAX_COL);
    fprintf(stderr, "%s", color);
    for (int i = 0; i < ctx->win_col; i++)
        putc(' ', stderr);
    CURSOR_LEFT(MAX_COL);
    fprintf(stderr, "Command : %s" COLOR_NONE, ctx->cmdbuf);
}

#define timespec_sub(after, before, result)                       \
    do                                                            \
    {                                                             \
        (result)->tv_sec = (after)->tv_sec - (before)->tv_sec;    \
        (result)->tv_nsec = (after)->tv_nsec - (before)->tv_nsec; \
        if ((result)->tv_nsec < 0)                                \
        {                                                         \
            --(result)->tv_sec;                                   \
            (result)->tv_nsec += 1000000000;                      \
        }                                                         \
    } while (0)

int poll_interval_check()
{
    static struct timespec oldtime = {0, 0};
    if (oldtime.tv_sec == 0)
        clock_gettime(0, &oldtime);

    struct timespec currtime;
    clock_gettime(0, &currtime);

    struct timespec diff;
    timespec_sub(&currtime, &oldtime, &diff);

    if (diff.tv_nsec > 50 * 1000 * 1000)
    {
        oldtime = currtime;
        return 1;
    }
    return 0;
}

void update_screen(AppContext *ctx)
{
    if (!poll_interval_check())
        return;

    size_t filesize = get_filesize(TARGET_FILE);
    if (filesize <= ctx->offset)
    {
        ctx->offset = filesize;
        return;
    }
    char buf[2001];
    FILE *f = fopen(TARGET_FILE, "r");
    fseek(f, ctx->offset, SEEK_SET);
    size_t read = fread(buf, 1, 2000, f);
    ctx->offset += read;
    fclose(f);

    if (read > 0)
    {
        ERASE_LINE();
        CURSOR_DOWN(MAX_ROW);
        CURSOR_UP(1);
        putc('\n', stderr);
        buf[read] = '\0';
        fprintf(stderr, "%s", buf);
    }
    draw_header(ctx, BACK_COLOR_BLUE);
    draw_footer(ctx, BACK_COLOR_GRAY);
}

void handle_input(AppContext *ctx)
{
    int c = getchar();
    if (!iscntrl(c))
    {
        ctx->cmdbuf[ctx->cmdbuf_offset] = c;
        ctx->cmdbuf_offset++;
        ctx->cmdbuf[ctx->cmdbuf_offset] = '\0';
    }
    else
    {
        if (c == '\n')
        {
            ctx->cmdbuf[0] = '\0';
            ctx->cmdbuf_offset = 0;
        }
    }
    draw_footer(ctx, BACK_COLOR_GRAY);
}

void poll_input(AppContext *ctx)
{
    fd_set fds;
    FD_ZERO(&fds);

    FD_SET(STDIN_FILENO, &fds);
    struct timeval tv = {0, 0};
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
    if (FD_ISSET(STDIN_FILENO, &fds))
    {
        handle_input(ctx);
    }
}

void mainloop(AppContext *ctx)
{
    size_t filesize = get_filesize(TARGET_FILE);
    ctx->offset = filesize > 4000 ? filesize - 2000 : 0;

    ERASE_SCREEN();

    while (1)
    {
        poll_input(ctx);
        if (ctx->realtime)
            update_screen(ctx);
    }
}

AppContext *create_context()
{
    AppContext *ctx = (AppContext *)malloc(sizeof(AppContext));
    memset(ctx, 0x00, sizeof(AppContext));

    get_screen_size(&ctx->win_row, &ctx->win_col);
    ctx->realtime = 1;

    update_screen(ctx);
    return ctx;
}

void stdin_mode_immediate(void)
{
    struct termios term;

    if (tcgetattr(0, &term))
    {
        printf("tcgetattr failed\n");
        exit(-1);
    }

    term.c_lflag &= ~ICANON;
    term.c_lflag &= ~ECHO;
    term.c_cc[VMIN] = 0;
    term.c_cc[VTIME] = 0;

    if (tcsetattr(0, TCSANOW, &term))
    {
        printf("tcsetattr failed\n");
        exit(-1);
    }
}

int main()
{
    stdin_mode_immediate();
    AppContext *ctx = create_context();

    mainloop(ctx);

    return 0;
}