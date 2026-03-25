#include "DSP2833x_Device.h"     // DSP2833x Headerfile Include File
#include "DSP2833x_Examples.h"   // DSP2833x Examples Include File
#include "interruptExample.h"
#include "SCIConfig.h"
#include "vofa.h"

int Voltage1[2];
//int i;
volatile Uint16 tbctr = 0;      // 应该看到变化

typedef unsigned char uint8_t;

// Global variables
Uint16 sdataA[8] = {0};     // Send data for SCI-A
Uint16 sdataB[8] = {0};     // Send data for SCI-B
Uint16 rdataA[8] = {0};     // Received data for SCI-A
Uint16 rdataB[8] = {0};     // Received data for SCI-B
Uint16 rdata_pointA = 0;    // Used for checking the received data
Uint16 rdata_pointB = 0;

#if RunType==FlashMode
    #ifdef __TI_COMPILER_VERSION__
        #if __TI_COMPILER_VERSION__ >= 15009000
            #pragma CODE_SECTION(ISRepwm1, ".TI.ramfunc");
        #else
            #pragma CODE_SECTION(ISRepwm1, "ramfuncs");
        #endif
    #endif

    #ifdef __TI_COMPILER_VERSION__
        #if __TI_COMPILER_VERSION__ >= 15009000
            #pragma CODE_SECTION(ISRepwm2, ".TI.ramfunc");
        #else
            #pragma CODE_SECTION(ISRepwm2, "ramfuncs");
        #endif
    #endif

    #ifdef __TI_COMPILER_VERSION__
        #if __TI_COMPILER_VERSION__ >= 15009000
            #pragma CODE_SECTION(ISRepwm3, ".TI.ramfunc");
        #else
            #pragma CODE_SECTION(ISRepwm3, "ramfuncs");
        #endif
    #endif

    #ifdef __TI_COMPILER_VERSION__
        #if __TI_COMPILER_VERSION__ >= 15009000
            #pragma CODE_SECTION(ISRepwm4, ".TI.ramfunc");
        #else
            #pragma CODE_SECTION(ISRepwm4, "ramfuncs");
        #endif
    #endif

    #ifdef __TI_COMPILER_VERSION__
        #if __TI_COMPILER_VERSION__ >= 15009000
            #pragma CODE_SECTION(ISRepwm5, ".TI.ramfunc");
        #else
            #pragma CODE_SECTION(ISRepwm5, "ramfuncs");
        #endif
    #endif

    #ifdef __TI_COMPILER_VERSION__
        #if __TI_COMPILER_VERSION__ >= 15009000
            #pragma CODE_SECTION(ISRepwm6, ".TI.ramfunc");
        #else
            #pragma CODE_SECTION(ISRepwm6, "ramfuncs");
        #endif
    #endif

    #ifdef __TI_COMPILER_VERSION__
        #if __TI_COMPILER_VERSION__ >= 15009000
            #pragma CODE_SECTION(ISRTimer0, ".TI.ramfunc");
        #else
            #pragma CODE_SECTION(ISRTimer0, "ramfuncs");
        #endif
    #endif
#endif

/* 定时器中断服务函数 */
interrupt void ISRepwm1(void)     // ePWM1
{
    /* 中断中要执行的操作 */
    GpioDataRegs.GPBTOGGLE.bit.GPIO49 = 0x1;    //翻转s

    /* 必要工作，清除标志位 */
    EPwm1Regs.ETCLR.bit.INT = 1;            // 清除ePWM1中断标志
    PieCtrlRegs.PIEACK.all |= PIEACK_GROUP3;  // 应答PIE组3中断
}

interrupt void ISRepwm2(void)     // ePWM2
{

    /* 中断中要执行的操作 */

    /* 必要工作，清除标志位 */
    EPwm2Regs.ETCLR.bit.INT = 1;            // 清除ePWM2中断标志
    PieCtrlRegs.PIEACK.all |= PIEACK_GROUP3;  // 应答PIE组3中断
}

interrupt void ISRepwm3(void)     // ePWM3
{

    /* 中断中要执行的操作 */

    /* 必要工作，清除标志位 */
    EPwm3Regs.ETCLR.bit.INT = 1;            // 清除ePWM3中断标志
    PieCtrlRegs.PIEACK.all |= PIEACK_GROUP3;  // 应答PIE组3中断
}

interrupt void ISRepwm4(void)     // ePWM4
{

    /* 中断中要执行的操作 */

    /* 必要工作，清除标志位 */
    EPwm4Regs.ETCLR.bit.INT = 1;            // 清除ePWM4中断标志
    PieCtrlRegs.PIEACK.all |= PIEACK_GROUP3;  // 应答PIE组3中断
}

interrupt void ISRepwm5(void)     // ePWM5
{

    /* 中断中要执行的操作 */

    /* 必要工作，清除标志位 */
    EPwm5Regs.ETCLR.bit.INT = 1;            // 清除ePWM5中断标志
    PieCtrlRegs.PIEACK.all |= PIEACK_GROUP3;  // 应答PIE组3中断
}

interrupt void ISRepwm6(void)     // ePWM6
{

    /* 中断中要执行的操作 */

    /* 必要工作，清除标志位 */
    EPwm6Regs.ETCLR.bit.INT = 1;            // 清除ePWM6中断标志
    PieCtrlRegs.PIEACK.all |= PIEACK_GROUP3;  // 应答PIE组3中断
}


/* ADC中断服务函数 */
interrupt void ISRADC(void)
{
    /* 中断中要执行的操作 */
//    Voltage1[0] = AdcRegs.ADCRESULT0 >> 4;
//    Voltage1[1] = AdcRegs.ADCRESULT1 >> 4;

    /* 必要工作，清除标志位 */
    AdcRegs.ADCST.bit.INT_SEQ1_CLR = 1;             // 清除ADC外设中断标志位
    //AdcRegs.ADCST.bit.INT_SEQ2_CLR = 1;             // 清除ADC外设中断标志位
    PieCtrlRegs.PIEACK.all |= PIEACK_GROUP1;         // 应答PIE组1中断
}


/* CpuTimer0定时器中断服务函数 */
interrupt void ISRTimer0(void)
{
    /* 中断中要执行的操作 */
    //GpioDataRegs.GPBTOGGLE.bit.GPIO49 = 0x1;    //翻转
    AdcRegs.ADCTRL2.bit.SOC_SEQ1 = 1;             //要使用ADC，一定要检查有没有开启ADC，这里是在定时器中手动开启
    tbctr = EPwm2Regs.TBCTR;
    Voltage1[0] = AdcRegs.ADCRESULT0 >> 4;
    Voltage1[1] = AdcRegs.ADCRESULT1 >> 4;

    /* 必要工作 */
    PieCtrlRegs.PIEACK.all |= PIEACK_GROUP1;     //清除应答寄存器
    CpuTimer0Regs.TCR.bit.TIF = 1;
    CpuTimer0Regs.TCR.bit.TRB = 1;              //重装载定时器
}


/* GPIO外部中断中断服务函数 */
interrupt void xint1_isr(void)
{
    EALLOW;
    GpioDataRegs.GPBTOGGLE.bit.GPIO50 = 1;
    EDIS;
    /* 必要工作 */
    PieCtrlRegs.PIEACK.all |= PIEACK_GROUP1;         // 应答PIE组1中断
}

interrupt void xint2_isr(void)
{
    EALLOW;
    GpioDataRegs.GPCTOGGLE.bit.GPIO82 = 1;
    EDIS;
    /* 必要工作 */
    PieCtrlRegs.PIEACK.all |= PIEACK_GROUP1;         // 应答PIE组1中断
}

interrupt void xint3_isr(void)
{
    EALLOW;
    GpioDataRegs.GPCTOGGLE.bit.GPIO83 = 1;
    EDIS;
    /* 必要工作 */
    PieCtrlRegs.PIEACK.all |= PIEACK_GROUP12;         // 应答PIE组12中断
}


interrupt void xint4_isr(void)
{
    EALLOW;
    GpioDataRegs.GPCTOGGLE.bit.GPIO80 = 1;
    EDIS;
    /* 必要工作 */
    PieCtrlRegs.PIEACK.all |= PIEACK_GROUP12;         // 应答PIE组12中断
}


interrupt void xint5_isr(void)
{
    EALLOW;
    GpioDataRegs.GPCTOGGLE.bit.GPIO81 = 1;
    EDIS;
    /* 必要工作 */
    PieCtrlRegs.PIEACK.all |= PIEACK_GROUP12;         // 应答PIE组12中断
}


/* SCI中断服务函数 */
/* SCIA */
interrupt void sciaTxFifoIsr(void)
{
    //SCIA_Xmit(0x01);
    /* 必要工作 */
    SciaRegs.SCIFFTX.bit.TXFFINTCLR=1;             // Clear SCI Interrupt flag
    PieCtrlRegs.PIEACK.all |= PIEACK_GROUP9;        // Issue PIE ACK
}

//void error(void)
//{
//    asm("     ESTOP0"); // Test failed!! Stop!
//    for (;;);
//}


interrupt void sciaRxFifoIsr(void)
{
    SciaRegs.SCIFFRX.bit.RXFFOVRCLR=1;   // Clear Overflow flag
    SciaRegs.SCIFFRX.bit.RXFFINTCLR=1;   // Clear Interrupt flag

    PieCtrlRegs.PIEACK.all |= PIEACK_GROUP9;       // Issue PIE ack

    /* 必要工作 */
//    SciaRegs.SCIFFRX.bit.RXFFOVRCLR=1;            // Clear Overflow flag
//    SciaRegs.SCIFFRX.bit.RXFFINTCLR=1;            // Clear Interrupt flag
//    PieCtrlRegs.PIEACK.all = PIEACK_GROUP9;       // Issue PIE ack
}

interrupt void eCANaINT0Isr(void)
{

}


interrupt void eCANaINT1Isr(void)
{
    if(ECanaRegs.CANRMP.all != 0)
    {
        if(ECanaRegs.CANRMP.all == 0x00000100)
        {
            Uint32  TestMbox1 = 0;
            Uint32  TestMbox2 = 0;
            Uint32  TestMbox3 = 0;
            volatile struct MBOX *Mailbox;
            Mailbox = &ECanaMboxes.MBOX8;
            TestMbox1 = Mailbox->MDL.all;
            TestMbox2 = Mailbox->MDH.all;
            TestMbox3 = Mailbox->MSGID.all;

            printf("TestMbox2_H = %x\r\n",TestMbox2);           //TestMbox2为32位的数值，打印时使用%x，只打印出了高16位，即0x9555
            printf("TestMbox2_L = %x\r\n",(TestMbox2 << 16));   //0xAAA0
            ECanaRegs.CANRMP.all = 0x00000100;
        }
    }
    if(ECanaRegs.CANTA.all != 0)
    {
        printf("This is eCAN TX interrupt!");
    }
    PieCtrlRegs.PIEACK.all |= PIEACK_GROUP9;       // Issue PIE ack
    //ECanbRegs.CANGIF1.bit.GMIF1 = 1;
    EINT;
}

