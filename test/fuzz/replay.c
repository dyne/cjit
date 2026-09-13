#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "muntar.h"
#include "tinf.h"
#include "adapters/cli/route_parser.h"
#include "adapters/platform/library_resolver_posix.h"

int tinf_gzip_uncompress(void *, unsigned int *, const void *, unsigned int);
void _err(const char *format, ...) { (void)format; }
char *pstrcpy(char *destination, size_t size, const char *source)
{ if (size) snprintf(destination, size, "%s", source); return destination; }

int main(int argc, char **argv)
{
    FILE *file;
    unsigned char *data;
    long size;
    if (argc != 3 || !(file = fopen(argv[2], "rb"))) return 2;
    fseek(file, 0, SEEK_END); size = ftell(file); rewind(file);
    data = calloc((size_t)size + 1, 1);
    if (!data || fread(data, 1, (size_t)size, file) != (size_t)size) return 2;
    fclose(file);
    if (strncmp(argv[1], "tar/", 4) == 0) { mtar_t tar; (void)mtar_load(&tar, "fuzz", data, (size_t)size); }
    else if (strncmp(argv[1], "gzip/", 5) == 0) { unsigned char out[65536]; unsigned int len = sizeof(out); (void)tinf_gzip_uncompress(out, &len, data, (unsigned int)size); }
    else if (strncmp(argv[1], "cli/", 4) == 0) { (void)cli_parse_define_value((char *)data); }
    else if (strncmp(argv[1], "ldscript/", 9) == 0) { (void)posix_ldscript_parse_buffer(data, (size_t)size); }
    else { free(data); return 2; }
    free(data); return 0;
}
