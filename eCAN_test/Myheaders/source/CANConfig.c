#include "DSP2833x_Device.h"
#include "DSP2833x_Examples.h"

#include "DSP2833x_ECan.h"
#include "interruptExample.h"
#include "CANConfig.h"

/*
void InitMboxes(void)
{
    ECanaMboxes.MBOX0.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX1.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX2.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX3.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX4.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX5.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX6.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX7.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX8.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX9.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX10.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX11.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX12.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX13.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX14.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX15.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX16.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX17.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX18.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX19.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX20.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX21.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX22.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX23.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX24.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX25.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX26.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX27.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX28.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX29.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX30.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX31.MSGCTRL.all = 0x00000000;
}
*/

void CANConfig(void)
{
    InitECanGpio();         /* 将GPIO复用为CAN */

    struct ECAN_REGS ECanaShadow;   /* 创建影子 */

    EALLOW;

    /* 打开时钟 */
    SysCtrlRegs.PCLKCR0.bit.ECANAENCLK=1;    // eCAN-A

    /* 将CANTX，CANRX引脚用于TX，RX功能 */
    ECanaShadow.CANTIOC.all = ECanaRegs.CANTIOC.all;        /* 先将原寄存器赋给影子，再给影子赋值，再把影子的值赋给原寄存器 */
    ECanaShadow.CANTIOC.bit.TXFUNC = 1;
    ECanaRegs.CANTIOC.all = ECanaShadow.CANTIOC.all;

    ECanaShadow.CANRIOC.all = ECanaRegs.CANRIOC.all;
    ECanaShadow.CANRIOC.bit.RXFUNC = 1;
    ECanaRegs.CANRIOC.all = ECanaShadow.CANRIOC.all;


    /* 设置为SCC模式（标准CAN控制器），即只使用15~0号邮箱，与HECC模式（增强型CAN控制器）相对应 */
    ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;
    ECanaShadow.CANMC.bit.SCB = 0;
    ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;


    /* 初始化所有邮箱的消息控制寄存器，此步骤必须有 */
//    InitMboxes();


    /* 清除所有标志位，相当于复位 */
    ECanaRegs.CANTA.all = 0xFFFFFFFF;       /* Clear all TAn bits */
    ECanaRegs.CANRMP.all = 0xFFFFFFFF;      /* Clear all RMPn bits */
    ECanaRegs.CANGIF0.all = 0xFFFFFFFF;     /* Clear all interrupt flag bits */
    ECanaRegs.CANGIF1.all = 0xFFFFFFFF;


    /* 进入初始化模式 */
    /* CCR位用来进入或退出初始化模式，置1则请求进入初始化模式修改CAN的参数，置0则进入正常工作模式 */
    ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;
    ECanaShadow.CANMC.bit.CCR = 1;            // Set CCR = 1
    ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;
    ECanaShadow.CANES.all = ECanaRegs.CANES.all;        /* 读取一次CANES寄存器的状态 */
    do
    {
        ECanaShadow.CANES.all = ECanaRegs.CANES.all;    //更新状态
    } while(ECanaShadow.CANES.bit.CCE != 1 );           //死等CCE置位，CCE=1则说明进入了初始化模式，才能继续配置位时序


    /* 配置位时序 */
    ECanaShadow.CANBTC.all = 0;
    #if (CPU_FRQ_150MHZ)                       // CPU_FRQ_150MHz is defined in DSP2833x_Examples.h
    /* The following block for all 150 MHz SYSCLKOUT (75 MHz CAN clock) - default. Bit rate = 1 Mbps
     * 波特率计算公式： Baud = CAN clock/(BRP+1)/(1+(TS1+1)+(TS2+1)) */
        ECanaShadow.CANBTC.bit.BRPREG = 4;          //BRP
        ECanaShadow.CANBTC.bit.TSEG2REG = 2;        //TS2
        ECanaShadow.CANBTC.bit.TSEG1REG = 10;       //TS1
    #endif
    #if (CPU_FRQ_100MHZ)                       // CPU_FRQ_100MHz is defined in DSP2833x_Examples.h
    /* The following block is only for 100 MHz SYSCLKOUT (50 MHz CAN clock). Bit rate = 1 Mbps
       See Note at End of File */
        ECanaShadow.CANBTC.bit.BRPREG = 4;
        ECanaShadow.CANBTC.bit.TSEG2REG = 1;
        ECanaShadow.CANBTC.bit.TSEG1REG = 6;
    #endif
    ECanaShadow.CANBTC.bit.SAM = 0;                     //使用单次采样，单次采样最高支持1Mbps，三次采样最高可能只支持500kbps甚至更低
    ECanaRegs.CANBTC.all = ECanaShadow.CANBTC.all;      //把影子的CANBTC寄存器的值赋给CAN真正的寄存器

    /* 配置屏蔽位 */
    ECanaMboxes.MBOX8.MSGID.bit.AME = 1;
//    /* 方式一：在eCAN模式下配置单独的本地接收屏蔽 */
//    ECanaLAMRegs.LAM8.all = 0xFFFFFFFF;
    /* 方式二：在SCC模式下配置全局屏蔽中断（或者LAM0和LAM3） */
    ECanaRegs.CANGAM.all = 0;                   // 先清零
    ECanaRegs.CANGAM.all = 0xFFFFFFFF;          // 屏蔽所有位，接收所有报文
    ECanaRegs.CANGAM.bit.AMI = 0;               // 不屏蔽IDE位

    /* 进入正常工作模式 */
    ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;
    ECanaShadow.CANMC.bit.CCR = 0 ;
    ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;
    ECanaShadow.CANES.all = ECanaRegs.CANES.all;
    do
    {
       ECanaShadow.CANES.all = ECanaRegs.CANES.all;
    } while(ECanaShadow.CANES.bit.CCE != 0 );       // 死等CCE置位，CCE=0则说明进入了正常工作模式

    /* 失能所有邮箱，邮箱ID寄存器只有邮箱被禁止时才能修改 */
    ECanaRegs.CANME.all = 0;

    EDIS;
}

void CAN_RX_IT_Config(void)
{
    EALLOW;
    ECanaRegs.CANMIM.bit.MIM8 = 1;          //使能中断邮箱8的中断;
    ECanaRegs.CANMIL.bit.MIL8 = 1;          //将中断8连接至中断1;
    ECanaRegs.CANGIM.bit.I1EN = 1;          //使能中断1;
    EDIS;

    /* 中断配置 */
    EALLOW;
    PieVectTable.ECAN1INTA = &eCANaINT1Isr;    //eCANa的中断1
    EDIS;

    /* 使能eCANa中断1 */
    IER |= M_INT9;
    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;
    PieCtrlRegs.PIEIER9.bit.INTx6 = 1;
}

void CAN_TX_IT_Config(void)
{
    EALLOW;
    ECanaRegs.CANMIM.bit.MIM0 = 1;          //使能中断邮箱0的中断;
    ECanaRegs.CANMIL.bit.MIL0 = 1;          //将中断0连接至中断1;
    ECanaRegs.CANGIM.bit.I1EN = 1;          //使能中断1;
    EDIS;

    /* 中断配置 */
    EALLOW;
    PieVectTable.ECAN1INTA = &eCANaINT1Isr;    //eCANa的中断1
    EDIS;

    /* 使能eCANa中断1 */
    IER |= M_INT9;
    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;
    PieCtrlRegs.PIEIER9.bit.INTx6 = 1;
    EINT;
}


