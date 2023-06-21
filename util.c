#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>

size_t get_filesize(char *filename)
{
    size_t offset = 0;
    FILE *f = fopen(filename, "r");
    if (f)
    {
        fseek(f, 0, SEEK_END);
        offset = ftell(f);
        fclose(f);
    }
    return offset;
}

void dump_file(char *target_file)
{
    size_t offset = 0;
    size_t filesize = get_filesize(target_file);
    FILE *f = fopen(target_file, "r");
    if (f)
    {
        char buf[2001];

        while (offset < filesize)
        {
            size_t read = fread(buf, 1, 2000, f);
            buf[read] = '\0';
            fprintf(stderr, "%s", buf);
            offset += read;
        }
        fclose(f);
        fflush(stderr);
    }
}
