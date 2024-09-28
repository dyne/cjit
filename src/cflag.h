/*
 * cflag.h
 * Copyright (C) 2020 Adrian Perez de Castro <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef CFLAG_H
#define CFLAG_H

#include <stdio.h>

enum cflag_type {
    CFLAG_TYPE_BOOL = 0,
    CFLAG_TYPE_INT,
    CFLAG_TYPE_STRING,
    CFLAG_TYPE_CUSTOM,
    CFLAG_TYPE_HELP,
};

enum cflag_status {
    CFLAG_OK = 0,
    CFLAG_SHOW_HELP,
    CFLAG_UNDEFINED,
    CFLAG_BAD_FORMAT,
    CFLAG_NEEDS_ARG,
};

struct cflag;

typedef enum cflag_status (*cflag_func) (const struct cflag*, const char *arg);

struct cflag {
    cflag_func  func;
    const char *name;
    int         letter;
    void       *data;
    const char *help;
};


#define CFLAG(_t, _name, _letter, _data, _help) \
    ((struct cflag) {                           \
        .func = cflag_ ## _t,                   \
        .name = (_name),                        \
        .letter = (_letter),                    \
        .data = (_data),                        \
        .help = (_help),                        \
    })
#define CFLAG_HELP \
    CFLAG(help, "help", 'h', NULL, "Prints command line usage help.")
#define CFLAG_END \
    { .name = NULL, .letter = '\0' }

enum cflag_status cflag_bool   (const struct cflag*, const char*);
enum cflag_status cflag_int    (const struct cflag*, const char*);
enum cflag_status cflag_uint   (const struct cflag*, const char*);
enum cflag_status cflag_float  (const struct cflag*, const char*);
enum cflag_status cflag_double (const struct cflag*, const char*);
enum cflag_status cflag_string (const struct cflag*, const char*);
enum cflag_status cflag_bytes  (const struct cflag*, const char*);
enum cflag_status cflag_timei  (const struct cflag*, const char*);
enum cflag_status cflag_help   (const struct cflag*, const char*);

void cflag_usage(const struct cflag specs[],
                 const char        *progname,
                 const char        *syntax,
                 FILE              *out);

int  cflag_parse(const struct cflag specs[],
                 int               *pargc,
                 char            ***pargv);

const char* cflag_apply(const struct cflag specs[],
                        const char        *syntax,
                        int               *pargc,
                        char            ***pargv);

const char* cflag_status_name(enum cflag_status value);

#endif /* !CFLAG_H */
