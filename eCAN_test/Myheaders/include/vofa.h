#ifndef VOFA_H
#define VOFA_H

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

extern char  SCITXDA_TX_BUF[400];

int fputc(int _c, register FILE *_fp);
int putc(int _c, register FILE *_fp);
int putchar(int data);
int fputs(const char *_ptr, register FILE *_fp);

#endif
