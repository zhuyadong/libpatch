#ifndef _ERR_H

#define _ERR_H
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

static void err(int eval, const char *fmt, ...)
{
    char msgbuff[4096];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(msgbuff, sizeof(msgbuff), fmt, ap);
    va_end(ap);
    fprintf(stderr, "error: %s\n", msgbuff);
    // exit(eval);
}

static void errx(int eval, const char *fmt, ...)
{
    char msgbuff[4096];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(msgbuff, sizeof(msgbuff), fmt, ap);
    va_end(ap);
    fprintf(stderr, "error: %s\n", msgbuff);
    exit(eval);
}
#endif