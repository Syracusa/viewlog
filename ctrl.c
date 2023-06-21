#include "viewlog.h"
#include "ctrl.h"

static void command_mode_handle_input(AppContext *ctx)
{

}

static void realtime_mode_handle_input()
{

}

static void stop_mode_handle_input()
{
    
}

static void dump_file(AppContext *ctx)
{
    ctx->offset = 0;
    size_t filesize = get_filesize(TARGET_FILE);
    FILE *f = fopen(TARGET_FILE, "r");
    char buf[2001];

    while (ctx->offset < filesize)
    {
        size_t read = fread(buf, 1, 2000, f);
        buf[read] = '\0';
        fprintf(stderr, "%s", buf);
        ctx->offset += read;
    }
    fclose(f);
    fflush(stderr);
}

static void handle_input(AppContext *ctx)
{
    int c = getchar();

    if (ctx->mode == MODE_COMMAND)
    {
    
        if (c == 'r' || c == 'R')
        {
            ERASE_SCREEN();
            if (ctx->realtime == 1)
            {
                ctx->realtime = 0;
                dump_file(ctx);
                fprintf(stderr, BACK_COLOR_BLUE "Press <R> to realtime log" COLOR_NONE);
            }
            else
            {
                size_t filesize = get_filesize(TARGET_FILE);
                ctx->offset = filesize > 4000 ? filesize - 2000 : 0;
                ctx->realtime = 1;
            }
            return;
        }
    }
    else if (ctx->mode == MODE_FILESEL)
    {
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
