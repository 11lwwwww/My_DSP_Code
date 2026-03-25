#include "DSP2833x_Gpio.h"
#include "GPIOConfig.h"
#include "interruptExample.h"

/* GPIO口配置为普通的输入输出 */
void GPIOInit(void)
{
    EALLOW;
    /* 开启时钟 */
    SysCtrlRegs.PCLKCR3.bit.GPIOINENCLK = 1;
    /* 初始化配置，五个LED */
//    GpioCtrlRegs.GPBMUX2.bit.GPIO50 = 0;        //设置为普通IO口
//    GpioCtrlRegs.GPBDIR.bit.GPIO50 = 1;         //设置方向  1为输出    0为输入
//    GpioCtrlRegs.GPBPUD.bit.GPIO50 = 1;         //是否上拉，0为使能上拉

    GpioCtrlRegs.GPCMUX2.bit.GPIO80 = 0;        //设置为普通IO口
    GpioCtrlRegs.GPCDIR.bit.GPIO80 = 1;         //设置方向  1为输出    0为输入
    GpioCtrlRegs.GPCPUD.bit.GPIO80 = 1;         //是否上拉，0为使能上拉

    GpioCtrlRegs.GPCMUX2.bit.GPIO81 = 0;        //设置为普通IO口
    GpioCtrlRegs.GPCDIR.bit.GPIO81 = 1;         //设置方向  1为输出    0为输入
    GpioCtrlRegs.GPCPUD.bit.GPIO81 = 1;         //是否上拉，0为使能上拉

    GpioCtrlRegs.GPCMUX2.bit.GPIO82 = 0;        //设置为普通IO口
    GpioCtrlRegs.GPCDIR.bit.GPIO82 = 1;         //设置方向  1为输出    0为输入
    GpioCtrlRegs.GPCPUD.bit.GPIO82 = 1;         //是否上拉，0为使能上拉

    GpioCtrlRegs.GPCMUX2.bit.GPIO83 = 0;        //设置为普通IO口
    GpioCtrlRegs.GPCDIR.bit.GPIO83 = 1;         //设置方向  1为输出    0为输入
    GpioCtrlRegs.GPCPUD.bit.GPIO83 = 1;         //是否上拉，0为使能上拉

    //GpioDataRegs.GPBDAT.bit.GPIO52 = 0;
    EDIS;
}

/* GPIO配置为复用模式 */
/* GPIO时钟在InitSysCtrl()中的InitPeripheralClocks()中开启过了，可以不用再开启 */
void GPIOInit_reuse(void)
{
    /* ePWM1 */
    EALLOW;
    GpioCtrlRegs.GPAPUD.bit.GPIO0 =1;       /* 禁用内部上拉 */
    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 1;     /* 配置为ePWM1A */
    GpioCtrlRegs.GPADIR.bit.GPIO0 = 1;      //设置方向  1为输出    0为输入
    EDIS;

    /* ePWM2 */
    //ePWM2A
    EALLOW;
    GpioCtrlRegs.GPAPUD.bit.GPIO2 = 1;      /* 禁用内部上拉 */
    GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 1;     /* 配置为ePWM2A */
    GpioCtrlRegs.GPADIR.bit.GPIO2 = 1;      //设置方向  1为输出    0为输入
    EDIS;
    //ePWM2B
    EALLOW;
    GpioCtrlRegs.GPAPUD.bit.GPIO3 = 1;      /* 禁用内部上拉 */
    GpioCtrlRegs.GPAMUX1.bit.GPIO3 = 1;     /* 配置为ePWM2B */
    GpioCtrlRegs.GPADIR.bit.GPIO3 = 1;      //设置方向  1为输出    0为输入
    EDIS;
}



/* XINT1,XINT2,XNMI的可选GPIO引脚为GPIO0-GPIO31，选择其他GPIO无效 */
void GPIOInit_xint1(void)
{
    /* XINT1配置 */
    /* 先配置为普通GPIO输入模式 */
    EALLOW;
    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 0;         // GPIO模式
    GpioCtrlRegs.GPADIR.bit.GPIO0 = 0;          // 设置为输入
    GpioCtrlRegs.GPAQSEL1.bit.GPIO0 = 0;        // Xint1 Synch to SYSCLKOUT only
    GpioCtrlRegs.GPAPUD.bit.GPIO0 =0;           /* 使能内部上拉 */
    EDIS;

    /* 设置GPIO0为Xint1 */
    EALLOW;
    GpioIntRegs.GPIOXINT1SEL.bit.GPIOSEL = 0;   // Xint1 is GPIO0
    EDIS;

    /* 初始化Xint1，此处设置为下降沿触发 */
    XIntruptRegs.XINT1CR.bit.POLARITY = 0;      // Falling edge interrupt

    /* 使能Xint1和Xint2 */
    XIntruptRegs.XINT1CR.bit.ENABLE = 1;        // Enable Xint1

    /* 中断配置 */
    EALLOW;
    PieVectTable.XINT1 = &xint1_isr;
    EDIS;

    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;          // Enable the PIE block
    PieCtrlRegs.PIEIER1.bit.INTx4 = 1;          // Enable PIE Gropu 1 INT4
    IER |= M_INT1;
}


void GPIOInit_xint2(void)
{
    /* 先配置为普通GPIO输入模式 */
    EALLOW;
    GpioCtrlRegs.GPAMUX1.bit.GPIO1 = 0;         // GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO1 = 0;          // 配置为输入模式
    GpioCtrlRegs.GPAQSEL1.bit.GPIO1 = 0;        // Xint2 Synch to SYSCLKOUT only
    GpioCtrlRegs.GPAPUD.bit.GPIO1 =0;           /* 使能内部上拉 */
    EDIS;

    /* 选择使用哪个GPIO口 */
    EALLOW;
    GpioIntRegs.GPIOXINT2SEL.bit.GPIOSEL = 1;   // XINT2 is GPIO1
    EDIS;

    XIntruptRegs.XINT2CR.bit.POLARITY = 0;      // Falling edge interrupt，上升沿触发为1，下降沿触发为0
    /* 使能XINT2 */
    XIntruptRegs.XINT2CR.bit.ENABLE = 1;        // Enable XINT2

    /* 中断配置 */
    EALLOW;
    PieVectTable.XINT2 = &xint2_isr;
    EDIS;

    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;          // Enable the PIE block
    PieCtrlRegs.PIEIER1.bit.INTx5 = 1;          // Enable PIE Gropu 1 INT5
    IER |= M_INT1;
}



/* XINT3-XINT7的可选GPIO引脚为GPIO32到GPIO63，选择其他GPIO会无效 */
void GPIOInit_xint3(void)
{
    /* 先配置为普通GPIO输入模式 */
    EALLOW;
    GpioCtrlRegs.GPBMUX1.bit.GPIO44 = 0;         // GPIO
    GpioCtrlRegs.GPBDIR.bit.GPIO44 = 0;          // 配置为输入模式
    GpioCtrlRegs.GPBQSEL1.bit.GPIO44 = 0;        // Xint3 Synch to SYSCLKOUT only
    GpioCtrlRegs.GPBPUD.bit.GPIO44 =0;           /* 使能内部上拉 */
    EDIS;

    /* 选择使用哪个GPIO口 */
    EALLOW;
    GpioIntRegs.GPIOXINT3SEL.bit.GPIOSEL = 44;   // XINT3 is GPIO44
    EDIS;

    XIntruptRegs.XINT3CR.bit.POLARITY = 0;      // Falling edge interrupt，上升沿触发为1，下降沿触发为0
    /* 使能XINT3 */
    XIntruptRegs.XINT3CR.bit.ENABLE = 1;        // Enable XINT3

    /* 中断配置 */
    EALLOW;
    PieVectTable.XINT3 = &xint3_isr;
    EDIS;

    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;          // Enable the PIE block
    PieCtrlRegs.PIEIER12.bit.INTx1 = 1;          // Enable PIE Gropu 12 INT1

    /* 清除可能存在的挂起中断 */
    IER |= M_INT12;
}

void GPIOInit_xint4(void)
{
    /* 先配置为普通GPIO输入模式 */
    EALLOW;
    GpioCtrlRegs.GPBMUX1.bit.GPIO45 = 0;         // GPIO
    GpioCtrlRegs.GPBDIR.bit.GPIO45 = 0;          // 配置为输入模式
    GpioCtrlRegs.GPBQSEL1.bit.GPIO45 = 0;        // Xint4 Synch to SYSCLKOUT only
    GpioCtrlRegs.GPBPUD.bit.GPIO45 =0;           /* 使能内部上拉 */
    EDIS;

    /* 选择使用哪个GPIO口 */
    EALLOW;
    GpioIntRegs.GPIOXINT4SEL.bit.GPIOSEL = 45;   // XINT4 is GPIO45
    EDIS;

    XIntruptRegs.XINT4CR.bit.POLARITY = 0;      // Falling edge interrupt，上升沿触发为1，下降沿触发为0
    /* 使能XINT4 */
    XIntruptRegs.XINT4CR.bit.ENABLE = 1;        // Enable XINT4

    /* 中断配置 */
    EALLOW;
    PieVectTable.XINT4 = &xint4_isr;
    EDIS;

    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;          // Enable the PIE block
    PieCtrlRegs.PIEIER12.bit.INTx2 = 1;          // Enable PIE Gropu 12 INT2
    IER |= M_INT12;
}


void GPIOInit_xint5(void)
{
    /* 先配置为普通GPIO输入模式 */
    EALLOW;
    GpioCtrlRegs.GPBMUX1.bit.GPIO46 = 0;         // GPIO
    GpioCtrlRegs.GPBDIR.bit.GPIO46 = 0;          // 配置为输入模式
    GpioCtrlRegs.GPBQSEL1.bit.GPIO46 = 0;        // Xint4 Synch to SYSCLKOUT only
    GpioCtrlRegs.GPBPUD.bit.GPIO46 =0;           /* 使能内部上拉 */
    EDIS;

    /* 选择使用哪个GPIO口 */
    EALLOW;
    GpioIntRegs.GPIOXINT5SEL.bit.GPIOSEL = 46;   // XINT5 is GPIO46
    EDIS;

    XIntruptRegs.XINT5CR.bit.POLARITY = 0;      // Falling edge interrupt，上升沿触发为1，下降沿触发为0
    /* 使能XINT5 */
    XIntruptRegs.XINT5CR.bit.ENABLE = 1;        // Enable XINT5

    /* 中断配置 */
    EALLOW;
    PieVectTable.XINT5 = &xint5_isr;
    EDIS;

    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;          // Enable the PIE block
    PieCtrlRegs.PIEIER12.bit.INTx3 = 1;          // Enable PIE Gropu 12 INT3
    IER |= M_INT12;
}
