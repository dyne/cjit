/* cflag.c
 * Copyright (C) 2020 Adrian Perez de Castro <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "cflag.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#if defined(linux) || defined(__linux) || defined(__linux__) || \
    defined(__OpenBSD__) || defined(__FreeBSD__) || defined(__NetBSD__)
# define HAVE_TIOCGWINZS 1
# include <sys/ioctl.h>
#else
# define HAVE_TIOCGWINZS 0
#endif

static unsigned
term_columns(FILE *f)
{
#if HAVE_TIOCGWINZS
    struct winsize ws;
    int fd = fileno(f);
    if (fd >= 0 && ioctl(fd, TIOCGWINSZ, &ws) == 0)
        return ws.ws_col;
#endif /* HAVE_TIOCGWINZS */

    return 80;
}

static bool
findent(FILE *out, unsigned rcol, const char *text)
{
    if (rcol < 20)
        return false;

    if (!(text && *text))
        return true;

    fputs("   ", out);
    for (unsigned col = 3; text && *text;) {
        const char *spc = strchr(text, ' ');
        const size_t len = spc ? (++spc - text) : strlen(text);

        if (col + len > rcol) {
            /* Move over to the next line. */
            fputc('\n', out);
            return findent(out, rcol, text);
        }

        fwrite(text, sizeof(char), len, out);
        col += len;
        text = spc;
    }

    return true;
}

static inline bool
is_long_flag(const char *s)
{
    return (s[0] == '-' && s[1] == '-' && s[2] != '\0');
}

static inline bool
is_short_flag(const char *s)
{
    return (s[0] == '-' && s[1] != '\0' && s[2] == '\0');
}

static inline bool
is_negated(const char *s)
{
    return strncmp("--no-", s, 5) == 0;
}

static const struct cflag*
find_short(const struct cflag specs[],
           int                letter)
{
    for (unsigned i = 0; specs[i].name != NULL || specs[i].letter != '\0'; ++i) {
        if (specs[i].letter == '\0')
            continue;
        if (specs[i].letter == letter)
            return &specs[i];
    }
    return NULL;
}

static const struct cflag*
find_long(const struct cflag specs[],
          const char        *name)
{
    for (unsigned i = 0; specs[i].name != NULL || specs[i].letter != '\0'; ++i) {
        if (!specs[i].name)
            continue;
        if (strcmp(specs[i].name, name) == 0)
            return &specs[i];
    }
    return NULL;
}

static inline bool
needs_arg(const struct cflag *spec)
{
    return (*spec->func)(NULL, NULL) == CFLAG_NEEDS_ARG;
}

void
cflag_usage(const struct cflag specs[],
            const char        *progname,
            const char        *usage,
            FILE              *out)
{
    assert(specs);
    assert(progname);
    assert(usage);

    if (!out)
        out = stderr;
    {
        const char *slash = strrchr(progname, '/');
        if (slash)
            progname = slash + 1;
    }

    const unsigned rcol = term_columns(out) - 3;

    fprintf(out, "Usage: %s %s\n", progname, usage);
    fprintf(out, "Command line options:\n\n");

    for (unsigned i = 0;; ++i) {
        const struct cflag *spec = &specs[i];

        const bool has_letter = spec->letter != '\0';
        const bool has_name = spec->name != NULL;

        if (!(has_name || has_letter))
            break;

        if (has_letter && has_name)
            fprintf(out, "-%c, --%s", spec->letter, spec->name);
        else if (has_name)
            fprintf(out, "--%s", spec->name);
        else
            fprintf(out, "-%c", spec->letter);

        if (needs_arg(spec))
            fprintf(out, " <ARG>");

        fputc('\n', out);
        if (!findent(out, rcol, spec->help)) {
            fputs("   ", out);
            fputs(spec->help, out);
        }
        fputc('\n', out);
        fputc('\n', out);
    }
}

int
cflag_parse(const struct cflag specs[],
            int               *pargc,
            char            ***pargv)
{
    assert(specs);
    assert(pargc);
    assert(pargv);

    int argc = *pargc;
    char **argv = *pargv;

    for (; argc > 0; --argc, ++argv) {
        const char *arg = *argv;

        bool negated = false;
        const struct cflag *spec;
        if (is_short_flag(arg)) {
            if (arg[1] == '-') /* -- stop processing command line flags */
                break;
            spec = find_short(specs, arg[1]);
        } else if (is_long_flag(arg)) {
            spec = find_long(specs, &arg[2]);
            if (!spec && is_negated(arg)) {
                const struct cflag *negspec = find_long(specs, &arg[5]);
                if (negspec->func == cflag_bool) {
                    spec = negspec;
                    negated = true;
                }
            }
        } else {
            *pargc = argc; *pargv = argv;
            return CFLAG_OK;
        }

        if (!spec) {
            *pargc = argc; *pargv = argv;
            return CFLAG_UNDEFINED;
        }

        arg = NULL;
        if (needs_arg(spec)) {
            if (argc == 1) {
                *pargc = argc; *pargv = argv;
                return CFLAG_NEEDS_ARG;
            }
            arg = *(++argv);
            --argc;
        }

        const enum cflag_status status = (*spec->func)(spec, arg);
        if (status != CFLAG_OK) {
            *pargc = argc; *pargv = argv;
            return status;
        }

        /*
         * XXX: This fixup here is ugly, but avoids needing to pass
         *      additional parameters to cflag_<type> functions.
         */
        if (spec->func == cflag_bool && negated)
            *((bool*) spec->data) = false;
    }

    *pargc = argc; *pargv = argv;
    return CFLAG_OK;
}

const char*
cflag_status_name(enum cflag_status value)
{
    switch (value) {
        case CFLAG_OK: return "success";
        case CFLAG_SHOW_HELP: return "help requested";
        case CFLAG_UNDEFINED: return "no such option";
        case CFLAG_BAD_FORMAT: return "argument has invalid format";
        case CFLAG_NEEDS_ARG: return "missing argument";
    }
    assert(!"Unreachable");
    abort();
}

const char*
cflag_apply(const struct cflag specs[],
            const char        *syntax,
            int               *pargc,
            char            ***pargv)
{
    assert(specs);
    assert(syntax);
    assert(pargc);
    assert(pargv);

    int argc = *pargc;
    char **argv = *pargv;

    const char *argv0 = *argv++; argc--;
    {
        const char *slash = strrchr(argv0, '/');
        if (slash) argv0 = slash + 1;
    }

    const enum cflag_status status = cflag_parse(specs, &argc, &argv);
    switch (status) {
        case CFLAG_SHOW_HELP:
            cflag_usage(specs, argv0, syntax, stdout);
            exit(EXIT_SUCCESS);
        case CFLAG_OK:
            *pargc = argc;
            *pargv = argv;
            return argv0;
        default:
            break;
    }

    fprintf(stderr, "%s: %s: '%s'\n", argv0, cflag_status_name(status), *argv);
    exit(EXIT_FAILURE);
}

enum cflag_status
cflag_bool(const struct cflag *spec,
           const char         *arg)
{
    (void) arg;

    if (!spec)
        return CFLAG_OK;

    *((bool*) spec->data) = true;
    return CFLAG_OK;
}

enum cflag_status
cflag_int(const struct cflag *spec,
          const char         *arg)
{
    if (!spec)
        return CFLAG_NEEDS_ARG;

    return (sscanf(arg, "%d", (int*) spec->data) == 1) ? CFLAG_OK : CFLAG_BAD_FORMAT;
}

enum cflag_status
cflag_uint(const struct cflag *spec,
           const char         *arg)
{
    if (!spec)
        return CFLAG_NEEDS_ARG;

    return (sscanf(arg, "%u", (unsigned*) spec->data) == 1) ? CFLAG_OK : CFLAG_BAD_FORMAT;
}

enum cflag_status
cflag_float(const struct cflag *spec,
            const char         *arg)
{
    if (!spec)
        return CFLAG_NEEDS_ARG;

    char *endptr;
    float v = strtof(arg, &endptr);
    if (errno == ERANGE || *endptr != '\0')
        return CFLAG_BAD_FORMAT;

    *((float*) spec->data) = v;
    return CFLAG_OK;
}

enum cflag_status
cflag_double(const struct cflag *spec,
             const char         *arg)
{
    if (!spec)
        return CFLAG_NEEDS_ARG;

    char *endptr;
    double v = strtod(arg, &endptr);
    if (errno == ERANGE || *endptr != '\0')
        return CFLAG_BAD_FORMAT;

    *((double*) spec->data) = v;
    return CFLAG_OK;
}

enum cflag_status
cflag_string(const struct cflag *spec,
             const char         *arg)
{
    if (!spec)
        return CFLAG_NEEDS_ARG;

    *((const char**) spec->data) = arg;
    return CFLAG_OK;
}

enum cflag_status
cflag_bytes(const struct cflag *spec,
            const char         *arg)
{
    if (!spec)
        return CFLAG_NEEDS_ARG;

    char *endpos;
    unsigned long long v = strtoull(arg, &endpos, 0);
    if (v == ULLONG_MAX && errno == ERANGE)
        return CFLAG_BAD_FORMAT;

    if (endpos) {
        switch (*endpos) {
            case 'g': case 'G': v *= 1024 * 1024 * 1024; break; /* gigabytes */
            case 'm': case 'M': v *= 1024 * 1024;        break; /* megabytes */
            case 'k': case 'K': v *= 1024;               break; /* kilobytes */
            case 'b': case 'B': case '\0':               break; /* bytes     */
        }
    }

    *((size_t*) spec->data) = v;
    return CFLAG_OK;
}

enum cflag_status
cflag_timei(const struct cflag *spec,
            const char         *arg)
{
    if (!spec)
        return CFLAG_NEEDS_ARG;

    char *endpos;
    unsigned long long v = strtoull(arg, &endpos, 0);

    if (v == ULLONG_MAX && errno == ERANGE)
        return CFLAG_BAD_FORMAT;

    if (endpos) {
        switch (*endpos) {
            case 'y': v *= 60 * 60 * 24 * 365; break; /* years   */
            case 'M': v *= 60 * 60 * 24 * 30;  break; /* months  */
            case 'w': v *= 60 * 60 * 24 * 7;   break; /* weeks   */
            case 'd': v *= 60 * 60 * 24;       break; /* days    */
            case 'h': v *= 60 * 60;            break; /* hours   */
            case 'm': v *= 60;                 break; /* minutes */
            case 's': case '\0':               break; /* seconds */
            default : return CFLAG_BAD_FORMAT;
        }
    }

    *((unsigned long long*) spec->data) = v;
    return CFLAG_OK;
}

enum cflag_status
cflag_help(const struct cflag *spec,
           const char         *arg)
{
    (void) spec;
    (void) arg;

    return CFLAG_SHOW_HELP;
}
