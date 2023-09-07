
#include "viewlog.h"
#include "ctrl.h"

AppContext *g_ctx = NULL; /* Global singleton context */

/**
 * @brief Update directory path from filename
 *
 * @param ctx Application context
 * @param filename Selected filename
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

/**
 * @brief Get the screen size
 *
 * @param row Pointer to save row size
 * @param col Pointer to save column size
 */
static void get_screen_size(int *row, int *col)
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    *row = w.ws_row;
    *col = w.ws_col;
}

/**
 * @brief Draw header
 *
 * @param ctx Application context
 * @param color ANSI color code
 */
static void draw_header(AppContext *ctx, const char *color)
{
    CURSOR_HOME();
    fprintf(stderr, "%s", color);
    for (int i = 0; i < ctx->win_col; i++)
        putc(' ', stderr);

    CURSOR_LEFT(ctx->win_col);
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

/**
 * @brief Check poll interval
 *
 * @param poll_interval_ms Poll interval in milliseconds
 * @return int 1 if poll interval is reached, 0 otherwise
 */
static int poll_interval_check(unsigned long poll_interval_ms)
{
    static struct timespec oldtime = {0, 0};
    if (oldtime.tv_sec == 0)
        clock_gettime(0, &oldtime);

    struct timespec currtime;
    clock_gettime(0, &currtime);

    struct timespec diff;
    timespec_sub(&currtime, &oldtime, &diff);

    if (diff.tv_nsec > poll_interval_ms * 1000 * 1000)
    {
        oldtime = currtime;
        return 1;
    }
    return 0;
}

/**
 * @brief Read updated log and write to screen
 *
 * @param ctx Application context
 */
static void update_log(AppContext *ctx)
{
    char buf[2001];
    FILE *f = fopen(ctx->target, "r");
    size_t read = 0;
    if (f)
    {
        /* Read updated log */
        fseek(f, ctx->offset, SEEK_SET);
        read = fread(buf, 1, 2000, f);
        ctx->offset += read;
        fclose(f);
    }

    if (read > 0)
    {
        /* Make new line */
        ERASE_LINE();
        CURSOR_DOWN(ctx->win_row);
        CURSOR_UP(1);
        putc('\n', stderr);

        /* Write updated log */
        buf[read] = '\0';
        fprintf(stderr, "%s", buf);
    }
}

/**
 * @brief Check file size and update log
 *
 * @param ctx Application context
 */
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

/**
 * @brief Poll log and update screen
 * If poll interval is not reached, this function will do nothing.
 *
 * @param ctx Application context
 */
static void update_screen(AppContext *ctx)
{
    /* Poll log every 10ms */
    if (!poll_interval_check(10))
        return;

    poll_log(ctx);
    draw_header(ctx, BACK_COLOR_BLUE);
    draw_footer(ctx, BACK_COLOR_GRAY);
}

void change_target(AppContext *ctx, const char *filename)
{
    ERASE_SCREEN();
    if (filename[strlen(filename) - 1] == '/') /* Not file, but directory */
    {
        update_dir(ctx, filename);
        return;
    }

    if (filename[0] == '/') /* Absolute path */
    {
        update_dir(ctx, filename);
        sprintf(ctx->target, "%s", filename);
    }
    else /* Relative path */
    {
        sprintf(ctx->target, "%s%s", ctx->dir, filename);
    }

    /* Reset offset */
    size_t filesize = get_filesize(ctx->target);
    ctx->offset = filesize > 2000 ? filesize - 2000 : 0;
}

void draw_footer(AppContext *ctx, const char *color)
{
    if (ctx->input_mode == INPUT_MODE_STOP)
        return;

    /* Set bottom line background color */
    CURSOR_DOWN(ctx->win_row);
    CURSOR_LEFT(ctx->win_col);
    fprintf(stderr, "%s", color);
    for (int i = 0; i < ctx->win_col; i++)
        putc(' ', stderr);

    /* Cursor to bottomleft most */
    CURSOR_LEFT(ctx->win_col);

    /* Draw footer */
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
    /* Check https://man7.org/linux/man-pages/man3/termios.3.html */
    if (tcgetattr(0, &ctx->term))
    {
        printf("tcgetattr failed\n");
        exit(-1);
    }

    ctx->orig = ctx->term;

    /**
     * In noncanonical mode input is available immediately (without the
     * user having to type a line-delimiter character), no input
     * processing is performed, and line editing is disabled.  The read
     * buffer will only accept 4095 chars; this provides the necessary
     * space for a newline char if the input mode is switched to
     * canonical.  The settings of MIN (c_cc[VMIN]) and TIME
     * (c_cc[VTIME]) determine the circumstances in which a read(2)
     * completes; there are four distinct cases:
     */
    ctx->term.c_lflag &= ~ICANON;

    /* No echo */
    ctx->term.c_lflag &= ~ECHO;

    /**
     * MIN == 0, TIME == 0 (polling read)
     * If data is available, read(2) returns immediately, with
     * the lesser of the number of bytes available, or the number
     * of bytes requested.  If no data is available, read(2)
     * returns 0.
     */
    ctx->term.c_cc[VMIN] = 0;
    ctx->term.c_cc[VTIME] = 0;

    if (tcsetattr(0, TCSANOW, &ctx->term))
    {
        printf("tcsetattr failed\n");
        exit(-1);
    }
}

/**
 * @brief Check if screen size changed
 * 
 * @param ctx Application context
 */
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
    ctx->offset = filesize > 2000 ? filesize - 2000 : 0;

    unsigned long tick = 0;

    ERASE_SCREEN();

    while (1)
    {
        tick++;
        poll_input(ctx);

        if (ctx->view_mode == VIEW_MODE_REALTIME) {
            update_screen(ctx);

            /* Update screen size every 100 ticks */
            if (tick % 100 == 0)
                check_screen_size_change(ctx);
        }

        /* Loop interval = 5ms */
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