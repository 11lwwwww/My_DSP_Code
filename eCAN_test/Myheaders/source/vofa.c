#include "DSP2833x_Device.h"
#include "DSP2833x_Examples.h"
#include <stdio.h>
#include <string.h>

#include "SCIConfig.h"
#include "vofa.h"

typedef unsigned char  uint8_t;

/* 若要使用printf重定向发送数据，则一定不要使能SCI的中断 */

int fputc(int _c, register FILE *_fp)
{
    while (SciaRegs.SCICTL2.bit.TXEMPTY == 0);
    SciaRegs.SCITXBUF = _c;
    return _c;
}

int putc(int _c, register FILE *_fp)
{
    while (SciaRegs.SCICTL2.bit.TXEMPTY == 0);
    SciaRegs.SCITXBUF = _c;
    return _c;
}



int putchar(int data)
{
  while (SciaRegs.SCICTL2.bit.TXRDY == 0);
  SciaRegs.SCITXBUF=data;
  return data;
}



int fputs(const char *_ptr, register FILE *_fp)
{
  unsigned int i, len;
  len = strlen(_ptr);
  for(i=0 ; i<len ; i++)
  {
        while (SciaRegs.SCICTL2.bit.TXEMPTY == 0);
        SciaRegs.SCITXBUF = (uint8_t) _ptr[i];
  }
  return len;
}

