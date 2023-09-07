
#include "viewlog.h"
#include "ctrl.h"

AppContext *g_ctx = NULL; /* Global singleton context */

/**
 * @brief
 *
 * @param ctx
 * @param filename
 */
static void update_dir(AppContext *ctx, const char *filename)
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

static void get_screen_size(int *row, int *col)
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    *row = w.ws_row;
    *col = w.ws_col;
}

static void draw_header(AppContext *ctx, const char *color)
{
    CURSOR_HOME();
    fprintf(stderr, "%s", color);
    for (int i = 0; i < ctx->win_col; i++)
        putc(' ', stderr);

    CURSOR_LEFT(MAX_COL);
    fprintf(stderr, "%s (Size : %luKB)" COLOR_NONE,
            ctx->target, ctx->offset / 1024);
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

static int poll_interval_check()
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

static void update_log(AppContext *ctx)
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

static void poll_log(AppContext *ctx)
{
    size_t filesize = get_filesize(ctx->target);
    if (filesize <= ctx->offset)
    {
        ctx->offset = filesize;
        return;
    }
    update_log(ctx);
}

static void update_screen(AppContext *ctx)
{
    if (poll_interval_check())
    {
        poll_log(ctx);
        draw_header(ctx, BACK_COLOR_BLUE);
        draw_footer(ctx, BACK_COLOR_GRAY);
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

static void stdin_mode_immediate(AppContext *ctx)
{
    if (tcgetattr(0, &ctx->term))
    {
        printf("tcgetattr failed\n");
        exit(-1);
    }

    ctx->orig = ctx->term;

    ctx->term.c_lflag &= ~ICANON;
    ctx->term.c_lflag &= ~ECHO;
    ctx->term.c_cc[VMIN] = 0;
    ctx->term.c_cc[VTIME] = 0;

    if (tcsetattr(0, TCSANOW, &ctx->term))
    {
        printf("tcsetattr failed\n");
        exit(-1);
    }
}

static void check_screen_size_change(AppContext *ctx)
{
    int row, col;
    get_screen_size(&row, &col);
    if (row != ctx->win_row || col != ctx->win_col)
    {
        ERASE_SCREEN();
        ctx->win_row = row;
        ctx->win_col = col;
        update_screen(ctx);
    }
}

AppContext *get_context()
{
    if (g_ctx == NULL)
    {
        AppContext *ctx = (AppContext *)malloc(sizeof(AppContext));
        memset(ctx, 0x00, sizeof(AppContext));

        change_target(ctx, DEF_TARGET);
        get_screen_size(&ctx->win_row, &ctx->win_col);
        ctx->view_mode = VIEW_MODE_REALTIME;
        ctx->input_mode = INPUT_MODE_COMMAND;

        update_screen(ctx);
        stdin_mode_immediate(ctx);
        g_ctx = ctx;
    }
    return g_ctx;
}

void viewlog_mainloop(AppContext *ctx)
{
    /* Set read offset to last 2000 bytes */
    size_t filesize = get_filesize(ctx->target);
    ctx->offset = filesize > 4000 ? filesize - 2000 : 0;

    unsigned long tick = 0;

    ERASE_SCREEN();

    while (1)
    {
        tick++;
        poll_input(ctx);

        if (ctx->view_mode == VIEW_MODE_REALTIME)
            update_screen(ctx);

        /* Update screen size every 100 ticks */
        if (tick % 100 == 0)
            check_screen_size_change(ctx);
        
        /* Update interval = 5ms */
        usleep(5000);
    }
}

void app_exit(int sig)
{
    /* Restore terminal setting */
    if (tcsetattr(0, TCSANOW, &g_ctx->orig))
    {
        printf("tcsetattr failed\n");
        exit(-1);
    }
    exit(2);
}