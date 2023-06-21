#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
    system("mkdir /tmp/viewlog");
    FILE *f = fopen("/tmp/viewlog/test1.log", "w+");
    int i = 0;
    while (1)
    {
        fprintf(f, "Test %d\n", i++);
        fflush(f);
        usleep(500 * 1000);
    }
    fclose(f);
    return 0;
}