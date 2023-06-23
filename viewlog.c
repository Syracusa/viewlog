
#include "viewlog.h"
#include "ctrl.h"

struct termios term, orig;

void update_dir(AppContext *ctx, const char *filename)
{
    int offset = strlen(filename);
    memcpy(ctx->dir, filename, strlen(filename));
    for (int i = offset; i >= 0; i--)
    {
        if (filename[i] == '/')
        {
            ctx->dir[i + 1] = '\0';
            break;
        }
    }
}

void change_target(AppContext *ctx, const char *filename)
{
    ERASE_SCREEN();
    if (filename[strlen(filename) - 1] == '/')
    {
        update_dir(ctx, filename);
        return;
    }

    if (filename[0] == '/')
    {
        update_dir(ctx, filename);
        sprintf(ctx->target, "%s", filename);
    }
    else
    {
        sprintf(ctx->target, "%s%s", ctx->dir, filename);
    }
    size_t filesize = get_filesize(ctx->target);
    ctx->offset = filesize > 4000 ? filesize - 2000 : 0;
}

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

void draw_header(AppContext *ctx, const char *color)
{
    CURSOR_HOME();
    fprintf(stderr, "%s", color);
    for (int i = 0; i < ctx->win_col; i++)
        putc(' ', stderr);

    CURSOR_LEFT(MAX_COL);
    fprintf(stderr, "%s (Size : %luKB)" COLOR_NONE,
            ctx->target, ctx->offset / 1024);
}

void draw_footer(AppContext *ctx, const char *color)
{
    if (ctx->input_mode == INPUT_MODE_STOP)
        return;

    CURSOR_DOWN(MAX_ROW);
    CURSOR_LEFT(MAX_COL);
    fprintf(stderr, "%s", color);
    for (int i = 0; i < ctx->win_col; i++)
        putc(' ', stderr);
    CURSOR_LEFT(MAX_COL);
    if (ctx->input_mode == INPUT_MODE_COMMAND)
    {
        fprintf(stderr, "<`> File Select <R> Realtime Toggle" COLOR_NONE);
    }
    else if (ctx->input_mode == INPUT_MODE_FILESEL)
    {
        fprintf(stderr, "Open file : %s >>> %s          (Press <`> to cmd mode)" COLOR_NONE,
                ctx->dir, ctx->cmdbuf);
    }
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

    if (diff.tv_nsec > 10 * 1000 * 1000)
    {
        oldtime = currtime;
        return 1;
    }
    return 0;
}

void update_log(AppContext *ctx)
{
    char buf[2001];
    FILE *f = fopen(ctx->target, "r");
    size_t read = 0;
    if (f)
    {
        fseek(f, ctx->offset, SEEK_SET);
        read = fread(buf, 1, 2000, f);
        ctx->offset += read;
        fclose(f);
    }

    if (read > 0)
    {
        ERASE_LINE();
        CURSOR_DOWN(MAX_ROW);
        CURSOR_UP(1);
        putc('\n', stderr);
        buf[read] = '\0';
        fprintf(stderr, "%s", buf);
    }
}

void poll_log(AppContext *ctx)
{
    size_t filesize = get_filesize(ctx->target);
    if (filesize <= ctx->offset)
    {
        ctx->offset = filesize;
        return;
    }
    update_log(ctx);
}

void update_screen(AppContext *ctx)
{
    if (poll_interval_check())
    {
        poll_log(ctx);
        draw_header(ctx, BACK_COLOR_BLUE);
        draw_footer(ctx, BACK_COLOR_GRAY);
    }
}

void mainloop(AppContext *ctx)
{
    size_t filesize = get_filesize(ctx->target);
    ctx->offset = filesize > 4000 ? filesize - 2000 : 0;

    ERASE_SCREEN();

    while (1)
    {
        poll_input(ctx);

        if (ctx->view_mode == VIEW_MODE_REALTIME)
            update_screen(ctx);

        usleep(5000);
    }
}

AppContext *create_context()
{
    AppContext *ctx = (AppContext *)malloc(sizeof(AppContext));
    memset(ctx, 0x00, sizeof(AppContext));

    change_target(ctx, DEF_TARGET);
    get_screen_size(&ctx->win_row, &ctx->win_col);
    ctx->view_mode = VIEW_MODE_REALTIME;
    ctx->input_mode = INPUT_MODE_COMMAND;

    update_screen(ctx);
    return ctx;
}

void stdin_mode_immediate(void)
{
    if (tcgetattr(0, &term))
    {
        printf("tcgetattr failed\n");
        exit(-1);
    }

    orig = term;

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

void app_exit(int sig)
{
    if (tcsetattr(0, TCSANOW, &orig))
    {
        printf("tcsetattr failed\n");
        exit(-1);
    }
    exit(2);
}

int main()
{
    signal(SIGINT, app_exit);
    stdin_mode_immediate();
    AppContext *ctx = create_context();

    mainloop(ctx);

    return 0;
}