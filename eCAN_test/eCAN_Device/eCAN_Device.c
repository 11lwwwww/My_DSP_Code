#include "DSP2833x_Device.h"
#include "DSP2833x_Examples.h"

#include "DSP2833x_ECan.h"
#include "interruptExample.h"
#include "eCAN_Device.h"
#include <stdio.h>

//#pragma DATA_ALIGN(g_tCanDeviceList, 4)
//static volatile tCanDevice g_tCanDeviceList = {0};             /* 队列，链表头 */

static tCanDevice* g_pCanDevices[4];
static int g_nCanDevices = 0;               /* 索引 */
#define MAX_CAN_DEVICES 4

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

/* 初始化CANa */
void CANaConfig(struct CanDevice *pdev, struct CanConfig *pConfig)
{
    InitECanaGpio();         /* 将GPIO复用为CAN */

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
    ECanaShadow.CANMC.bit.SCB = pConfig->Mode;
    ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;


    /* 初始化所有邮箱的消息控制寄存器，此步骤必须有 */
    InitMboxes();


    /* 清除所有标志位，相当于复位 */
    ECanaRegs.CANTA.all = 0xFFFFFFFF;       /* Clear all TAn bits */
    ECanaRegs.CANRMP.all = 0xFFFFFFFF;      /* Clear all RMPn bits */
    ECanaRegs.CANGIF0.all = 0xFFFFFFFF;     /* Clear all interrupt flag bits */
    ECanaRegs.CANGIF1.all = 0xFFFFFFFF;


    /* 进入初始化模式 */
    /* CCR位用来进入或退出初始化模式，置1则请求进入初始化模式修改CAN的参数，置0则进入正常工作模式 */
    ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;
    ECanaShadow.CANMC.bit.CCR = 1;                      // Set CCR = 1
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
        ECanaShadow.CANBTC.bit.BRPREG = pConfig->BRP;          //BRP
        ECanaShadow.CANBTC.bit.TSEG2REG = pConfig->TS2;        //TS2
        ECanaShadow.CANBTC.bit.TSEG1REG = pConfig->TS1;        //TS1
    #endif
    #if (CPU_FRQ_100MHZ)                       // CPU_FRQ_100MHz is defined in DSP2833x_Examples.h
    /* The following block is only for 100 MHz SYSCLKOUT (50 MHz CAN clock). Bit rate = 1 Mbps
       See Note at End of File */
        ECanaShadow.CANBTC.bit.BRPREG = 4;
        ECanaShadow.CANBTC.bit.TSEG2REG = 1;
        ECanaShadow.CANBTC.bit.TSEG1REG = 6;
    #endif
    ECanaShadow.CANBTC.bit.SAM = pConfig->SAM;            //使用单次采样，单次采样最高支持1Mbps，三次采样最高可能只支持500kbps甚至更低
    ECanaRegs.CANBTC.all = ECanaShadow.CANBTC.all;        //把影子的CANBTC寄存器的值赋给CAN真正的寄存器

    /* 进入正常工作模式 */
    ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;
    ECanaShadow.CANMC.bit.CCR = 0 ;
    ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;
    ECanaShadow.CANES.all = ECanaRegs.CANES.all;
    do
    {
       ECanaShadow.CANES.all = ECanaRegs.CANES.all;
    } while(ECanaShadow.CANES.bit.CCE != 0 );       // 死等CCE置位，CCE=0则说明进入了正常工作模式

    /* 是否开启环回模式 */
    ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;
    ECanaShadow.CANMC.bit.STM = pConfig->STM;    // Configure CAN for self-test mode，置1使能环回模式
    ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;

    /* 失能所有邮箱，邮箱ID寄存器只有邮箱被禁止时才能修改 */
    ECanaRegs.CANME.all = 0;

    EDIS;
}

/* 初始化发送邮箱 */
void InitCanTXMBOX(struct CanDevice *pdev, struct MBOXConfig *pMBOX)
{
    volatile struct ECAN_REGS ECanShadow;
    volatile struct MBOX *ECanaMBOX = NULL;
    Uint32 temp;

    ECanaMBOX = &ECanaMboxes.MBOX0;     /* 邮箱0的基地址，其他邮箱基地址为其往后推 */
    temp = (Uint32) 1 << pMBOX->MBOX_Num;

    EALLOW;

    //失能邮箱
    ECanShadow.CANME.all = ECanaRegs.CANME.all;
    ECanShadow.CANME.all &= ~temp;
    ECanaRegs.CANME.all = ECanShadow.CANME.all;
    //邮箱方向：发送
    ECanShadow.CANMD.all = ECanaRegs.CANMD.all;
    ECanShadow.CANMD.all &= ~temp;
    ECanaRegs.CANMD.all = ECanShadow.CANMD.all;
    //配置邮箱信息                                /* 得到该邮箱号对应的邮箱地址 */
    ECanaMBOX += pMBOX->MBOX_Num;
    ECanaMBOX->MSGID.all = pMBOX->MSG_Id;       /* 配置ID，兼容了IDE位，其他两个位后面有单独设置 */
    ECanaMBOX->MSGID.bit.AAM = pMBOX->AAM;
    ECanaMBOX->MSGCTRL.bit.DLC = pMBOX->DLC;
    ECanaMBOX->MSGCTRL.bit.RTR = pMBOX->RTR;
    ECanaMBOX->MSGCTRL.bit.TPL = pMBOX->TPL;
    //要发送的数据
    ECanaMBOX->MDL.all = pMBOX->MSG_L;
    ECanaMBOX->MDH.all = pMBOX->MSG_H;
    //使能邮箱
    ECanShadow.CANME.all = ECanaRegs.CANME.all;
    ECanShadow.CANME.all |= temp;
    ECanaRegs.CANME.all = ECanShadow.CANME.all;

    EDIS;
}

/* 初始化接收邮箱，返回-1则初始化失败 */
int InitCanRXMBOX(struct CanDevice *pdev, struct CanConfig *pConfig, struct MBOXConfig *pMBOX)
{
    volatile struct ECAN_REGS ECanShadow;
    volatile struct MBOX *ECanaMBOX = NULL;             /* 定义时就赋值，防止出现野指针 */
    volatile union CANLAM_REG *MBOX_LAM = NULL;
    Uint32 temp;

    ECanaMBOX = &ECanaMboxes.MBOX0;         /* 邮箱0的基地址，其他邮箱基地址为其往后推 */
    MBOX_LAM = &ECanaLAMRegs.LAM0;
    temp = (Uint32) 1 << pMBOX->MBOX_Num;

    EALLOW;

    //失能邮箱
    ECanShadow.CANME.all = ECanaRegs.CANME.all;
    ECanShadow.CANME.all &= ~temp;
    ECanaRegs.CANME.all = ECanShadow.CANME.all;
    //邮箱方向：接收
    ECanShadow.CANMD.all = ECanaRegs.CANMD.all;
    ECanShadow.CANMD.all |= temp;
    ECanaRegs.CANMD.all = ECanShadow.CANMD.all;
    //设置邮箱覆盖保护
    if(pMBOX->OPC == 1) //禁止被覆盖
    {
        ECanShadow.CANOPC.all = ECanaRegs.CANOPC.all;
        ECanShadow.CANOPC.all |= temp;
        ECanaRegs.CANOPC.all = ECanShadow.CANOPC.all;
    }
    else //允许被覆盖
    {
        ECanShadow.CANOPC.all = ECanaRegs.CANOPC.all;
        ECanShadow.CANOPC.all &= ~temp;
        ECanaRegs.CANOPC.all = ECanShadow.CANOPC.all;
    }
    //配置邮箱信息                                /* 得到该邮箱号对应的邮箱地址 */
    ECanaMBOX += pMBOX->MBOX_Num;
    ECanaMBOX->MSGID.all = pMBOX->MSG_Id;       /* 配置ID，兼容了IDE位，其他两个位后面有单独设置 */
    ECanaMBOX->MSGCTRL.bit.DLC = pMBOX->DLC;    /* 配置要接收的数据的长度 */
    ECanaMBOX->MSGCTRL.bit.RTR = pMBOX->RTR;    /* 配置要接收的数据的长度 */
    /* 屏蔽设置 */
    if(pMBOX->AME == 1)
    {
        /* 进入初始化模式 */
        ECanShadow.CANMC.all = ECanaRegs.CANMC.all;
        ECanShadow.CANMC.bit.CCR = 1;            // Set CCR = 1
        ECanaRegs.CANMC.all = ECanShadow.CANMC.all;
        ECanShadow.CANES.all = ECanaRegs.CANES.all;        /* 读取一次CANES寄存器的状态 */
        do
        {
            ECanShadow.CANES.all = ECanaRegs.CANES.all;    //更新状态
        } while(ECanShadow.CANES.bit.CCE != 1 );           //死等CCE置位，CCE=1则说明进入了初始化模式，才能继续配置位时序
        /* 开启屏蔽 */
        ECanaMBOX->MSGID.bit.AME = pMBOX->AME;
        if(pConfig->Mode == 1)      /* eCAN模式 */
        {
            MBOX_LAM += pMBOX->MBOX_Num;
            MBOX_LAM->all = pMBOX->Mask;
        }
        else                        /* SCC模式 */
        {
            if((pMBOX->MBOX_Num >= 6) && (pMBOX->MBOX_Num <= 15))     /* 若为邮箱6到15 */
            {
                ECanaRegs.CANGAM.all = 0;                    // 先清零
                ECanaRegs.CANGAM.all = pMBOX->Mask;          // 设置掩码
            }
            else if((pMBOX->MBOX_Num >= 3) && (pMBOX->MBOX_Num <= 5))
            {
                ECanaLAMRegs.LAM3.all = pMBOX->Mask;
            }
            else if((pMBOX->MBOX_Num >= 0) && (pMBOX->MBOX_Num <= 2))
            {
                ECanaLAMRegs.LAM0.all = pMBOX->Mask;
            }
            else
            {
                return -1;              /* 邮箱号有误 */
            }
        }
        ECanaRegs.CANGAM.bit.AMI = pMBOX->AMI;               // 不屏蔽IDE位
        /* 进入正常工作模式 */
        ECanShadow.CANMC.all = ECanaRegs.CANMC.all;
        ECanShadow.CANMC.bit.CCR = 0 ;
        ECanaRegs.CANMC.all = ECanShadow.CANMC.all;
        ECanShadow.CANES.all = ECanaRegs.CANES.all;
        do
        {
           ECanShadow.CANES.all = ECanaRegs.CANES.all;
        } while(ECanShadow.CANES.bit.CCE != 0 );       // 死等CCE置位，CCE=0则说明进入了正常工作模式
    }

    //要接收的消息ID
    ECanaMBOX->MDL.all = pMBOX->MSG_L;
    ECanaMBOX->MDH.all = pMBOX->MSG_H;
    //使能邮箱
    ECanShadow.CANME.all = ECanaRegs.CANME.all;
    ECanShadow.CANME.all |= temp;
    ECanaRegs.CANME.all = ECanShadow.CANME.all;

    EDIS;

    return 0;
}

/* 初始化多个发送邮箱 */
void InitManyTXMBOX(struct CanDevice *pdev, Uint16 len, struct MBOXConfig *pMBOX)
{
    Uint16 i = 0;
    for(i = 0; i < len; i++)
    {
        InitCanTXMBOX(pdev, pMBOX);
    }
}

/* 初始化多个接收邮箱，返回-1则初始化失败 */
int InitManyRXMBOX(struct CanDevice *pdev, Uint16 len, struct CanConfig *pConfig, struct MBOXConfig *pMBOX)
{
    Uint16 i = 0;
    int result = 0;
    for(i = 0; i < len; i++)
    {
        result = InitCanRXMBOX(pdev, pConfig, pMBOX);
    }
    return result;
}

/* 对邮箱设置中断，传入的是发送邮箱就是开启发送中断，接收邮箱就是接收中断 */
void CAN_IT_Config(struct CanDevice *pdev, struct CanConfig *pConfig, struct MBOXConfig *pMBOX)
{
    Uint32 temp;

    temp = (Uint32) 1 << pMBOX->MBOX_Num;   /* 只对某特定邮箱赋值0时使用"&=~temp"，只对某特定邮箱赋值1时使用"|=temp"，不影响其他位 */

    EALLOW;
    ECanaRegs.CANMIM.all |= temp;           //使能某邮箱的中断
    EDIS;
    if(pConfig->TI_option == 0)             //选用中断0
    {
        /* 外设级 */
        EALLOW;
        ECanaRegs.CANMIL.all &=~temp;
        ECanaRegs.CANGIM.bit.I0EN = 1;          //使能中断0;

        /* 中断配置,PIE级 */
        PieVectTable.ECAN1INTA = &eCANaINT0Isr;    //eCANa的中断1
        EDIS;

        /* 使能eCANa中断0，CPU级，INT9.5 */
        IER |= M_INT9;
        PieCtrlRegs.PIECTRL.bit.ENPIE = 1;
        PieCtrlRegs.PIEIER9.bit.INTx5 = 1;
    }
    else if(pConfig->TI_option == 1)
    {
        /* 外设级 */
        EALLOW;
        ECanaRegs.CANMIL.all |= temp;           //将中断8连接至中断1;
        ECanaRegs.CANGIM.bit.I1EN = 1;          //使能中断1;

        /* 中断配置,PIE级 */
        PieVectTable.ECAN1INTA = &eCANaINT1Isr;    //eCANa的中断1
        EDIS;

        /* 使能eCANa中断1，CPU级，INT9.6 */
        IER |= M_INT9;
        PieCtrlRegs.PIECTRL.bit.ENPIE = 1;
        PieCtrlRegs.PIEIER9.bit.INTx6 = 1;
    }
    else
    {
        printf("RX_IT Config error");
    }

    EINT;
}



#pragma DATA_SECTION(g_tCanDev, ".ebss")
#pragma DATA_ALIGN(g_tCanDev, 4)
static volatile tCanDevice g_tCanDev =          /* 其中所有函数都是在eCANA的前提下 */
{
    .name = "canA",
    .init = CANaConfig,
    .initCanTran = InitCanTXMBOX,
    .initCanRecv = InitCanRXMBOX,
    .initManyTran = InitManyTXMBOX,
    .initManyRecv = InitManyRXMBOX,
    .initCanIT = CAN_IT_Config,
    .PrivData = NULL,
    .pNext = NULL,                              /* 别打成分号了 */
};



int _ResgisterCanDevice(tCanDevice* pdev)
{
    if (g_nCanDevices >= MAX_CAN_DEVICES) {
        printf("Device table full!\r\n");
        return -1;
    }

    g_pCanDevices[g_nCanDevices] = pdev;
    g_nCanDevices++;

//    printf("Registered %s at index %d, addr=%p\r\n",
//           pdev->name, g_nCanDevices-1, pdev);
    return 0;
}

int ResgisterCanDevice(void)
{
    return _ResgisterCanDevice(&g_tCanDev);  // 传递地址
}

ptCanDevice GetCanDevice(char *name)
{
    int i = 0;
    for (i = 0; i < g_nCanDevices; i++) {
        if (strcmp(g_pCanDevices[i]->name, name) == 0) {
//            printf("Found %s at index %d\r\n", name, i);
            return g_pCanDevices[i];
        }
    }
    printf("Device %s not found\r\n", name);
    return NULL;
}


