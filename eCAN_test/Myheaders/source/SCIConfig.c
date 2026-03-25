#include "DSP2833x_Device.h"
#include "DSP2833x_Examples.h"

#include "SCIConfig.h"
#include "interruptExample.h"

#define CPU_FRQ_150MHz 1
#define CPU_FRQ_100MHz 0
#define CPU_FRQ_Now CPU_FRQ_150MHz


/* SCI串口，使用35，36是因为在板子上集成了USB转TTL模块，即CH340，可以直接用USB线将电脑与板子连接，方便查看数据 */
void SCIAInit(void)
{
    InitSciaGpio();                                 /* GPIO复用为SCIA,使用的是GPIO28(RX)和GPIO29(TX) */
/*
    EALLOW;
    GpioCtrlRegs.GPBPUD.bit.GPIO36 = 0;    // Enable pull-up for GPIO36 (SCIRXDA)
    GpioCtrlRegs.GPBPUD.bit.GPIO35 = 0;    // Enable pull-up for GPIO35 (SCITXDA)
    GpioCtrlRegs.GPBQSEL1.bit.GPIO36 = 3;  // Asynch input GPIO35 (SCIRXDA)
    GpioCtrlRegs.GPBMUX1.bit.GPIO36 = 1;   // Configure GPIO35 for SCIRXDA operation
    GpioCtrlRegs.GPBMUX1.bit.GPIO35 = 1;   // Configure GPIO36 for SCITXDA operation

    EDIS;
    */

    SciaRegs.SCICCR.bit.LOOPBKENA = 0;              /* 禁止自测(回环)模式，即禁止自发自收 */
    SciaRegs.SCICCR.bit.PARITYENA = 0;              /* 禁止奇偶校验位 */
    SciaRegs.SCICCR.bit.PARITY = 0;                 /* 奇校验 */
    SciaRegs.SCICCR.bit.ADDRIDLE_MODE = 0;          /* 空闲线模式 */
    SciaRegs.SCICCR.bit.STOPBITS = 0;               /* 1位停止位 */
    SciaRegs.SCICCR.bit.SCICHAR = 0x7;              /* 7+1位数据位 */

    SciaRegs.SCICTL1.bit.TXENA = 1;                 /* 使能TX，SCI可发送 */
    SciaRegs.SCICTL1.bit.RXENA = 1;                 /* 使能RX，SCI可接收 */

    /* 设置波特率，此处设置为115200 */
#if CPU_FRQ_Now == CPU_FRQ_150MHz                   /* LSPCLK=37.5MHz */
    SciaRegs.SCIHBAUD = 0x0000;
    SciaRegs.SCILBAUD = 0x0027;
#endif
#if CPU_FRQ_Now == CPU_FRQ_100MHz                   /* LSPCLK=25MHz */
    SciaRegs.SCIHBAUD = 0x0000;
    SciaRegs.SCILBAUD = 0x001A;
#endif
    /* 复位一下 */
    SciaRegs.SCICTL1.all =0x0023;                   // Relinquish SCI from Reset

#if SCIA_FIFO_ENA==1
    /* FIFO配置 */
    SciaRegs.SCIFFTX.all = 0xC028;                  /* 1100 0000 0010 1000 */
    SciaRegs.SCIFFRX.all = 0x0028;                  /* 0000 0000 0010 1000 */
    SciaRegs.SCIFFCT.all = 0x0;                     /* 禁用FIFO的高级功能 */
#endif

#if SCIA_INT_ENA==1
    /* 中断配置 */
    /* 外设级 */
    SciaRegs.SCICTL2.bit.TXINTENA = 1;              /* 发送中断使能 */
    SciaRegs.SCICTL2.bit.RXBKINTENA = 1;            /* 接收中断使能 */
    //SciaRegs.SCICTL1.bit.SWRESET = 1;               /* 复位一下 */
    /* 复位一下（使用FIFO的话后两句必须要有，否则无法发送） */
    SciaRegs.SCICTL1.all =0x0023;                   // Relinquish SCI from Reset
    SciaRegs.SCIFFTX.bit.TXFIFOXRESET=1;            /* ！！！必须要复位，必须要复位，必须要复位，否则会无法发送 */
    SciaRegs.SCIFFRX.bit.RXFIFORESET=1;

    /* PIE级 */
    EALLOW;
    PieVectTable.SCIRXINTA = &sciaRxFifoIsr;        /* 这里相当于重映射，也可以不给中断向量表重映射 */
    PieVectTable.SCITXINTA = &sciaTxFifoIsr;
    EDIS;
    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;              // Enable the PIE block
    PieCtrlRegs.PIEIER9.bit.INTx1 = 1;              // PIE Group 9, int1, SCIA的RX中断
    PieCtrlRegs.PIEIER9.bit.INTx2 = 1;              // PIE Group 9, int2, SCIA的TX中断

    /* CPU级 */
    IER |= M_INT9;                                  /* 配置CPU级，开启对应组中断 */
#endif
}


void SCIA_Xmit(int a)
{
    while(SciaRegs.SCIFFTX.bit.TXFFST != 0);        /* 等待FIFO中字全部发送完 */
    SciaRegs.SCITXBUF = a;
}

void SCIA_Transmsg(char* msg)
{
    int i;
    i = 0;
    while(msg[i]!='\0')
    {
        SCIA_Xmit(msg[i]);
        i++;
    }
}
