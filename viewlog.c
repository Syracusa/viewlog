#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <termios.h>
#include <string.h>
#include <ctype.h>

#include <sys/ioctl.h>
#include <sys/stat.h>

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

void update_screen(AppContext *ctx)
{
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
        CURSOR_DOWN(100);
        CURSOR_UP(1);
        putc('\n', stderr);
        buf[read] = '\0';
        fprintf(stderr, "%s", buf);
    }
    draw_header(ctx, BACK_COLOR_BLUE);
    draw_footer(ctx, BACK_COLOR_GRAY);
}

void handle_input()
{
    int c = getchar();
    if (isalnum(c)){

    }
}

void mainloop(AppContext *ctx)
{
    size_t filesize = get_filesize(TARGET_FILE);
    ctx->offset = filesize > 4000 ? filesize - 2000 : 0;

    ERASE_SCREEN();
    while (1)
    {
        handle_input(ctx);
        if (ctx->realtime)
            update_screen(ctx);
        usleep(100000);
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

int main()
{
    AppContext *ctx = create_context();

    mainloop(ctx);

    return 0;
}