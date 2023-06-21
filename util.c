#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>

size_t get_filesize(char *filename)
{
    FILE *f = fopen(filename, "r");
    fseek(f, 0, SEEK_END);
    size_t offset = ftell(f);
    fclose(f);
    return offset;
}