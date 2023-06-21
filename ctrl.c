#include "viewlog.h"
#include "ctrl.h"

static void viewmode_stop_handle_input(AppContext *ctx, int c)
{
    if (c == 'r' || c == 'R')
    {
        /* View mode to VIEW_MODE_REALTIME */
        ERASE_SCREEN();
        size_t filesize = get_filesize(TARGET_FILE);
        ctx->offset = filesize > 4000 ? filesize - 2000 : 0;
        ctx->view_mode = VIEW_MODE_REALTIME;
        ctx->input_mode = INPUT_MODE_COMMAND;
        draw_footer(ctx, BACK_COLOR_GRAY);
    }
}

static void command_mode_handle_input(AppContext *ctx, int c)
{
    if (c == 'r' || c == 'R')
    {
        /* View mode to VIEW_MODE_STOP */
        ERASE_SCREEN();
        ctx->view_mode = VIEW_MODE_STOP;
        ctx->input_mode = INPUT_MODE_STOP;
        dump_file(TARGET_FILE);
        fprintf(stderr, BACK_COLOR_BLUE "Press <R> to realtime log" COLOR_NONE);
    }
}

static void filesel_mode_handle_input(AppContext *ctx, int c)
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
    draw_footer(ctx, BACK_COLOR_GRAY);
}

static void viewmode_realtime_handle_input(AppContext *ctx, int c)
{
    if (ctx->input_mode == INPUT_MODE_COMMAND)
    {
        command_mode_handle_input(ctx, c);
    }
    else if (ctx->input_mode == INPUT_MODE_FILESEL)
    {
        filesel_mode_handle_input(ctx, c);
    }
}

static void handle_input(AppContext *ctx)
{
    int c = getchar();

    if (ctx->view_mode == VIEW_MODE_REALTIME)
    {
        if (c == '`')
        {
            /* Toggle input mode */
            if (ctx->input_mode == INPUT_MODE_COMMAND)
            {
                ctx->input_mode = INPUT_MODE_FILESEL;
            }
            else
            {
                ctx->input_mode = INPUT_MODE_COMMAND;
            }
        }
        else
        {
            viewmode_realtime_handle_input(ctx, c);
        }
    }
    else if (ctx->view_mode == VIEW_MODE_STOP)
    {
        viewmode_stop_handle_input(ctx, c);
    }
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
