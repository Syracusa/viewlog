#ifndef VIEWLOG_H
#define VIEWLOG_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <termios.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/select.h>

#include "ansi_ec.h"
#include "util.h"

#define TARGET_FILE "/tmp/viewlog/test1.log"

#define MAX_COL 300
#define MAX_ROW 100

#define INPUT_MODE_COMMAND      0
#define INPUT_MODE_FILESEL      1
#define INPUT_MODE_STOP         2

#define VIEW_MODE_REALTIME      0
#define VIEW_MODE_STOP          1

typedef struct AppContext
{
    int win_row;
    int win_col;
    
    size_t offset;
    char cmdbuf[MAX_ROW];
    int cmdbuf_offset;

    int view_mode;
    int input_mode;
} AppContext;

void draw_footer(AppContext *ctx, const char *color);

#endif