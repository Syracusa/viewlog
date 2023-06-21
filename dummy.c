#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    system("mkdir /tmp/viewlog");
    char filename[100];
    sprintf(filename, "/tmp/viewlog/test1.log");

    FILE *f;
    if (argc > 1)
        sprintf(filename, "/tmp/viewlog/%s", argv[1]);

    f = fopen(filename, "w+");
    int i = 0;
    while (1)
    {
        fprintf(f, "%s : Test %d\n", filename, i++);
        fflush(f);
        usleep(500 * 1000);
    }
    fclose(f);
    return 0;
}