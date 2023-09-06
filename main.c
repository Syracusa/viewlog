#include "viewlog.h"

int main()
{
    signal(SIGINT, app_exit);
    AppContext *ctx = get_context();
    stdin_mode_immediate(ctx);
    viewlog_mainloop(ctx);

    /* Will not reach here */
    app_exit(SIGINT);
    return 0;
}