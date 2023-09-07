/**
 * @file viewlog.h
 * @brief Application context and core functions
 */

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

#define DEF_TARGET "/tmp/viewlog/test1.log"

#define MAX_ROW 1000
#define MAX_CMDBUF 300

#define INPUT_MODE_COMMAND      0
#define INPUT_MODE_FILESEL      1
#define INPUT_MODE_STOP         2

#define VIEW_MODE_REALTIME      0
#define VIEW_MODE_STOP          1

typedef struct AppContext
{
    int win_row;
    int win_col;
    
    char target[MAX_CMDBUF];

    size_t offset;
    char cmdbuf[MAX_ROW];
    int cmdbuf_offset;

    char dir[MAX_CMDBUF];

    int view_mode;
    int input_mode;

    struct termios term;
    struct termios orig;
} AppContext;

/**
 * @brief Get the global singleton app context
 * If the context is not initialized, it will be initialized.
 * 
 * @return AppContext* Application context
 */
AppContext *get_context();

/**
 * @brief Main loop of viewlog
 * This function will not end.
 * @param ctx Application context
 */
void viewlog_mainloop(AppContext *ctx);

/**
 * @brief Change target log file
 * 
 * @param ctx Application context
 * @param filename New target log file
 */
void change_target(AppContext *ctx, const char *filename);

/**
 * @brief Draw footer
 * 
 * @param ctx Application context
 * @param color Background color
 */
void draw_footer(AppContext *ctx, const char *color);

/**
 * @brief Signal Handler - Exit application. 
 * 
 * @param sig Signal number
 */
void app_exit(int sig);

#endif