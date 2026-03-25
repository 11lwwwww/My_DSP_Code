#include "ePWMConfig.h"
#include "DSP2833x_Device.h"     // DSP2833x Headerfile Include File
#include "DSP2833x_Examples.h"   // DSP2833x Examples Include File
#include "DSP2833x_EPwm.h"
#include "DSP2833x_EPwm_defines.h"
#include "interruptExample.h"


/* 用来产生占空比,分为锯齿波比较模式(即使用上升计数模式)和三角波比较模式(即使用上升下降计数模式) */
/* 此处使用锯齿波 */
/* 未使用斩波模块PC和错误控制模块TZ，SysCtrlRegs受保护，必须用EALLOW和EDIS */
void ePWM1ABInit(int tbprd)
{
    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;  //禁用TBCLK，即禁用同步时钟
    SysCtrlRegs.PCLKCR1.bit.EPWM1ENCLK = 1; //使能ePWM1时钟
    EDIS;

    InitEPwm1Gpio();                        /* GPIO复用为ePWM功能 */

    /* TB模块 */
    /* 时钟预分频，得到TBCLK，此处未分频，即直接使用150MHz的时钟频率 */
    EPwm1Regs.TBCTL.bit.CLKDIV = TB_DIV1;               /* 分频系数为2^K */
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;            /* 大于0时分频系数为2K，为0时分频系数为1 */
    /* 得到fPWM,此处使用锯齿波 */
    EPwm1Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_DISABLE;     /* 不同步 */
    EPwm1Regs.TBCTL.bit.PHSEN = TB_DISABLE;             /* 禁止相位装载，使用主模式 */
    EPwm1Regs.TBPHS.half.TBPHS = 0;                     /* 不使用相位 */
    EPwm1Regs.TBCTR = 0x0000;                           /* 计数器清0 */
    EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;          /* 上升计数模式 */
    //EPwm1Regs.TBCTL.bit.PRDLD = TB_IMMEDIATE;         /* 使用周期映射模式，影子寄存器 */
    //EPwm1Regs.TBCTL.bit.SWFSYNC = TB_SYNC_IN;         //同步信号直通
    EPwm1Regs.TBPRD = tbprd - 1;                        /* 频率为20kHz，周期为50us */

#if ePWM1_Only_TIM==0
    /* CC模块 */
    //ePWM1A
    EPwm1Regs.CMPA.half.CMPA = 2250;                 /* 不能直接用EPwm1Regs.CMPA=7499;会报错 */
    EPwm1Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;      /* ePWM1A使用映射模式 */
    EPwm1Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;    /* ePWM1A周期匹配装载,CTR=0时装载 */
    //ePWM1B
    EPwm1Regs.CMPB = 2250;
    EPwm1Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;      /* ePWM1B使用映射模式 */
    EPwm1Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;    /* ePWM1B周期匹配装载,CTR=0时装载 */

    /* AQ模块 */
    //ePWM1A
    EPwm1Regs.AQCTLA.bit.CAU = AQ_CLEAR;            /* 增计数匹配置0，即增计数时计到CMP就置1 */
    EPwm1Regs.AQCTLA.bit.PRD = AQ_SET;              /* 周期匹配置1，即CNT=PRD时置0 */
    //ePWM1B
    EPwm1Regs.AQCTLB.bit.CBU = AQ_CLEAR;
    EPwm1Regs.AQCTLB.bit.PRD = AQ_SET;

    /* DB模块，AB一起考虑 */
    EPwm1Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;   /* 即10，即0x3，使能上升沿下降沿延时，宏定义位于DSP2833x_EPwm_defines.h */
    EPwm1Regs.DBCTL.bit.IN_MODE = DBA_RED_DBB_FED;   /* ePWM2A上升沿延时,2B下降沿延时 */
    EPwm1Regs.DBCTL.bit.POLSEL = 0x2;                /* ePWM2B极性反转 */
    EPwm1Regs.DBRED = 150;                           /* 上升沿死区时间，1us，50e-6(1/150e+6)=50*150 */
    EPwm1Regs.DBFED = 150;                           /* 下降沿死区时间，1us，7500个TBCLK */
#endif

    /* ET模块 */
    //此模块与发波无关，故不分AB
    //中断触发设置
    EPwm1Regs.ETSEL.bit.INTEN = 1;                  /* 打开外设中断 */
    EPwm1Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;       /* 选择CTR=0时触发中断 */
    EPwm1Regs.ETPS.bit.INTPRD = ET_1ST;             /* 来一个中断就触发事件 */

#if ePWM1_SOCA_ENA==1
    EPwm1Regs.ETSEL.bit.SOCAEN = 1;                 /* 向ADC发送SOCA转换信号 */
    EPwm1Regs.ETSEL.bit.SOCASEL = ET_CTRU_CMPA;     /* 即100，即0x4，CMPA加匹配时触发发送 */
    EPwm1Regs.ETPS.bit.SOCAPRD = ET_1ST;            /* 即0x1，一个事件就触发（相当于不分频） */
#endif

#if ePWM1_SOCB_ENA==1
    EPwm1Regs.ETSEL.bit.SOCBEN = 1;                 /* 向ADC发送SOCB转换信号 */
    EPwm1Regs.ETSEL.bit.SOCBSEL = ET_CTRU_CMPA;     /* 即100，即0x4，CMPA加匹配时触发发送 */
    EPwm1Regs.ETPS.bit.SOCBPRD = ET_1ST;            /* 即0x1，一个事件就触发（相当于不分频） */
#endif

    /* 开启时钟同步 */
    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;

#if ePWM1_INT_ENA==1
    EALLOW;
    PieVectTable.EPWM1_INT = &ISRepwm1;      /* 这里相当于重映射，也可以不给中断向量表重映射 */
    EDIS;

    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;      /* 使能PIE级中断 */
    PieCtrlRegs.PIEIER3.bit.INTx1 = 1;      /* 配置PIE级，ePWM1外设中断在INT3.1 */
    /* CPU级 */
    IER |= M_INT3;                          /* 配置CPU级，开启对应组中断 */
#endif
}



void ePWM2ABInit(int tbprd)
{
    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;  //禁用TBCLK，即禁用同步时钟
    SysCtrlRegs.PCLKCR1.bit.EPWM2ENCLK = 1; //使能ePWM2时钟
    EDIS;

    InitEPwm2Gpio();                        /* GPIO复用为ePWM功能 */

    /* TB模块 */
    /* 时钟预分频，得到TBCLK，此处未分频，即直接使用150MHz的时钟频率 */
    EPwm2Regs.TBCTL.bit.CLKDIV = TB_DIV1;               /* 分频系数为2^K */
    EPwm2Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;            /* 大于0时分频系数为2K，为0时分频系数为1 */
    /* 得到fPWM,此处使用锯齿波 */
    EPwm2Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_DISABLE;     /* 不同步 */
    EPwm2Regs.TBCTL.bit.PHSEN = TB_DISABLE;             /* 禁止相位装载，使用主模式 */
    EPwm2Regs.TBPHS.half.TBPHS = 0;                     /* 不使用相位 */
    EPwm2Regs.TBCTR = 0x0000;                           /* 计数器清0 */
    EPwm2Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;          /* 上升计数模式 */
    //EPwm2Regs.TBCTL.bit.PRDLD = TB_IMMEDIATE;         /* 使用周期映射模式，影子寄存器 */
    //EPwm2Regs.TBCTL.bit.SWFSYNC = TB_SYNC_IN;         //同步信号直通
    EPwm2Regs.TBPRD = tbprd - 1;                        /* 频率为20kHz，周期为50us */

#if ePWM2_Only_TIM==0
    /* CC模块 */
    //ePWM2A
    EPwm2Regs.CMPA.half.CMPA = 2250;                 /* 不能直接用EPwm2Regs.CMPA=7499;会报错 */
    EPwm2Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;      /* ePWM2A使用映射模式 */
    EPwm2Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;    /* ePWM2A周期匹配装载,CTR=0时装载 */
    //ePWM2B
    EPwm2Regs.CMPB = 2250;
    EPwm2Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;      /* ePWM2B使用映射模式 */
    EPwm2Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;    /* ePWM2B周期匹配装载,CTR=0时装载 */

    /* AQ模块 */
    //ePWM2A
    EPwm2Regs.AQCTLA.bit.CAU = AQ_CLEAR;            /* 增计数匹配置0，即增计数时计到CMP就置1 */
    EPwm2Regs.AQCTLA.bit.PRD = AQ_SET;              /* 周期匹配置1，即CNT=PRD时置0 */
    //ePWM2B
    EPwm2Regs.AQCTLB.bit.CBU = AQ_CLEAR;
    EPwm2Regs.AQCTLB.bit.PRD = AQ_SET;

    /* DB模块，AB一起考虑 */
    EPwm2Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;   /* 即10，即0x3，使能上升沿下降沿延时，宏定义位于DSP2833x_EPwm_defines.h */
    EPwm2Regs.DBCTL.bit.IN_MODE = DBA_RED_DBB_FED;   /* ePWM2A上升沿延时,2B下降沿延时 */
    EPwm2Regs.DBCTL.bit.POLSEL = 0x2;                /* ePWM2B极性反转 */
    EPwm2Regs.DBRED = 150;                           /* 上升沿死区时间，1us，50e-6(1/150e+6)=50*150 */
    EPwm2Regs.DBFED = 150;                           /* 下降沿死区时间，1us，7500个TBCLK */
#endif

    /* ET模块 */
    //此模块与发波无关，故不分AB
    //中断触发设置
    EPwm2Regs.ETSEL.bit.INTEN = 1;                  /* 打开外设中断 */
    EPwm2Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;       /* 选择CTR=0时触发中断 */
    EPwm2Regs.ETPS.bit.INTPRD = ET_1ST;             /* 来一个中断就触发事件 */

#if ePWM2_SOCA_ENA==1
    EPwm2Regs.ETSEL.bit.SOCAEN = 1;                 /* 向ADC发送SOCA转换信号 */
    EPwm2Regs.ETSEL.bit.SOCASEL = ET_CTRU_CMPA;     /* 即100，即0x4，CMPA加匹配时触发发送 */
    EPwm2Regs.ETPS.bit.SOCAPRD = ET_1ST;            /* 即0x1，一个事件就触发（相当于不分频） */
#endif

#if ePWM2_SOCB_ENA==1
    EPwm2Regs.ETSEL.bit.SOCBEN = 1;                 /* 向ADC发送SOCB转换信号 */
    EPwm2Regs.ETSEL.bit.SOCBSEL = ET_CTRU_CMPA;     /* 即100，即0x4，CMPA加匹配时触发发送 */
    EPwm2Regs.ETPS.bit.SOCBPRD = ET_1ST;            /* 即0x1，一个事件就触发（相当于不分频） */
#endif

    /* 开启时钟同步 */
    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;

#if ePWM2_INT_ENA==1
    EALLOW;
    PieVectTable.EPWM2_INT = &ISRepwm2;      /* 这里相当于重映射，也可以不给中断向量表重映射 */
    EDIS;

    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;      /* 使能PIE级中断 */
    PieCtrlRegs.PIEIER3.bit.INTx2 = 1;      /* 配置PIE级，ePWM2外设中断在INT3.2 */
    /* CPU级 */
    IER |= M_INT3;                          /* 配置CPU级，开启对应组中断 */
#endif
}



void ePWM3ABInit(int tbprd)
{
    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;    //禁用TBCLK，即禁用同步时钟
    SysCtrlRegs.PCLKCR1.bit.EPWM3ENCLK = 1;   //使能ePWM3时钟
    EDIS;

    InitEPwm3Gpio();                        /* GPIO复用为ePWM功能 */

    /* TB模块 */
    /* 时钟预分频，得到TBCLK，此处未分频，即直接使用150MHz的时钟频率 */
    EPwm3Regs.TBCTL.bit.CLKDIV = TB_DIV1;               /* 分频系数为2^K */
    EPwm3Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;            /* 大于0时分频系数为2K，为0时分频系数为1 */
    /* 得到fPWM,此处使用锯齿波 */
    EPwm3Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_DISABLE;     /* 不同步 */
    EPwm3Regs.TBCTL.bit.PHSEN = TB_DISABLE;             /* 禁止相位装载，使用主模式 */
    EPwm3Regs.TBPHS.half.TBPHS = 0;                     /* 不使用相位 */
    EPwm3Regs.TBCTR = 0x0000;                           /* 计数器清0 */
    EPwm3Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;          /* 上升计数模式 */
    //EPwm3Regs.TBCTL.bit.PRDLD = TB_IMMEDIATE;         /* 使用周期映射模式，影子寄存器 */
    //EPwm3Regs.TBCTL.bit.SWFSYNC = TB_SYNC_IN;         //同步信号直通
    EPwm3Regs.TBPRD = tbprd - 1;                        /* 频率为20kHz，周期为50us */

#if ePWM3_Only_TIM==0
    /* CC模块 */
    //ePWM3A
    EPwm3Regs.CMPA.half.CMPA = 2250;                 /* 不能直接用EPwm2Regs.CMPA=7499;会报错 */
    EPwm3Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;      /* ePWM2A使用映射模式 */
    EPwm3Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;    /* ePWM2A周期匹配装载,CTR=0时装载 */
    //ePWM3B
    EPwm3Regs.CMPB = 2250;
    EPwm3Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;      /* ePWM2B使用映射模式 */
    EPwm3Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;    /* ePWM2B周期匹配装载,CTR=0时装载 */

    /* AQ模块 */
    //ePWM3A
    EPwm3Regs.AQCTLA.bit.CAU = AQ_CLEAR;            /* 增计数匹配置0，即增计数时计到CMP就置1 */
    EPwm3Regs.AQCTLA.bit.PRD = AQ_SET;              /* 周期匹配置1，即CNT=PRD时置0 */
    //ePWM3B
    EPwm3Regs.AQCTLB.bit.CBU = AQ_CLEAR;
    EPwm3Regs.AQCTLB.bit.PRD = AQ_SET;

    /* DB模块，AB一起考虑 */
    EPwm3Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;   /* 即10，即0x3，使能上升沿下降沿延时，宏定义位于DSP2833x_EPwm_defines.h */
    EPwm3Regs.DBCTL.bit.IN_MODE = DBA_RED_DBB_FED;   /* ePWM2A上升沿延时,2B下降沿延时 */
    EPwm3Regs.DBCTL.bit.POLSEL = 0x2;                /* ePWM2B极性反转 */
    EPwm3Regs.DBRED = 150;                           /* 上升沿死区时间，1us，50e-6(1/150e+6)=50*150 */
    EPwm3Regs.DBFED = 150;                           /* 下降沿死区时间，1us，7500个TBCLK */
#endif

    /* ET模块 */
    //此模块与发波无关，故不分AB
    //中断触发设置
    EPwm3Regs.ETSEL.bit.INTEN = 1;                  /* 打开外设中断 */
    EPwm3Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;       /* 选择CTR=0时触发中断 */
    EPwm3Regs.ETPS.bit.INTPRD = ET_1ST;             /* 来一个中断就触发事件 */

#if ePWM3_SOCA_ENA==1
    EPwm3Regs.ETSEL.bit.SOCAEN = 1;                 /* 向ADC发送SOCA转换信号 */
    EPwm3Regs.ETSEL.bit.SOCASEL = ET_CTRU_CMPA;     /* 即100，即0x4，CMPA加匹配时触发发送 */
    EPwm3Regs.ETPS.bit.SOCAPRD = ET_1ST;            /* 即0x1，一个事件就触发（相当于不分频） */
#endif

#if ePWM3_SOCB_ENA==1
    EPwm3Regs.ETSEL.bit.SOCBEN = 1;                 /* 向ADC发送SOCB转换信号 */
    EPwm3Regs.ETSEL.bit.SOCBSEL = ET_CTRU_CMPA;     /* 即100，即0x4，CMPA加匹配时触发发送 */
    EPwm3Regs.ETPS.bit.SOCBPRD = ET_1ST;            /* 即0x1，一个事件就触发（相当于不分频） */
#endif

    /* 开启时钟同步 */
    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;

#if ePWM3_INT_ENA==1
    EALLOW;
    PieVectTable.EPWM3_INT = &ISRepwm3;      /* 这里相当于重映射，也可以不给中断向量表重映射 */
    EDIS;

    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;      /* 使能PIE级中断 */
    PieCtrlRegs.PIEIER3.bit.INTx3 = 1;      /* 配置PIE级，ePWM3外设中断在INT3.3 */
    /* CPU级 */
    IER |= M_INT3;                          /* 配置CPU级，开启对应组中断 */
#endif
}



void ePWM4ABInit(int tbprd)
{
    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;    //禁用TBCLK，即禁用同步时钟
    SysCtrlRegs.PCLKCR1.bit.EPWM4ENCLK = 1;   //使能ePWM4时钟
    EDIS;

    InitEPwm4Gpio();                        /* GPIO复用为ePWM功能 */

    /* TB模块 */
    /* 时钟预分频，得到TBCLK，此处未分频，即直接使用150MHz的时钟频率 */
    EPwm4Regs.TBCTL.bit.CLKDIV = TB_DIV1;               /* 分频系数为2^K */
    EPwm4Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;            /* 大于0时分频系数为2K，为0时分频系数为1 */
    /* 得到fPWM,此处使用锯齿波 */
    EPwm4Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_DISABLE;     /* 不同步 */
    EPwm4Regs.TBCTL.bit.PHSEN = TB_DISABLE;             /* 禁止相位装载，使用主模式 */
    EPwm4Regs.TBPHS.half.TBPHS = 0;                     /* 不使用相位 */
    EPwm4Regs.TBCTR = 0x0000;                           /* 计数器清0 */
    EPwm4Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;          /* 上升计数模式 */
    //EPwm4Regs.TBCTL.bit.PRDLD = TB_IMMEDIATE;         /* 使用周期映射模式，影子寄存器 */
    //EPwm4Regs.TBCTL.bit.SWFSYNC = TB_SYNC_IN;         //同步信号直通
    EPwm4Regs.TBPRD = tbprd - 1;                        /* 频率为20kHz，周期为50us */

#if ePWM4_Only_TIM==0
    /* CC模块 */
    //ePWM4A
    EPwm4Regs.CMPA.half.CMPA = 2250;                 /* 不能直接用EPwm2Regs.CMPA=7499;会报错 */
    EPwm4Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;      /* ePWM2A使用映射模式 */
    EPwm4Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;    /* ePWM2A周期匹配装载,CTR=0时装载 */
    //ePWM4B
    EPwm4Regs.CMPB = 2250;
    EPwm4Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;      /* ePWM2B使用映射模式 */
    EPwm4Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;    /* ePWM2B周期匹配装载,CTR=0时装载 */

    /* AQ模块 */
    //ePWM4A
    EPwm4Regs.AQCTLA.bit.CAU = AQ_CLEAR;            /* 增计数匹配置0，即增计数时计到CMP就置1 */
    EPwm4Regs.AQCTLA.bit.PRD = AQ_SET;              /* 周期匹配置1，即CNT=PRD时置0 */
    //ePWM4B
    EPwm4Regs.AQCTLB.bit.CBU = AQ_CLEAR;
    EPwm4Regs.AQCTLB.bit.PRD = AQ_SET;

    /* DB模块，AB一起考虑 */
    EPwm4Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;   /* 即10，即0x3，使能上升沿下降沿延时，宏定义位于DSP2833x_EPwm_defines.h */
    EPwm4Regs.DBCTL.bit.IN_MODE = DBA_RED_DBB_FED;   /* ePWM2A上升沿延时,2B下降沿延时 */
    EPwm4Regs.DBCTL.bit.POLSEL = 0x2;                /* ePWM2B极性反转 */
    EPwm4Regs.DBRED = 150;                           /* 上升沿死区时间，1us，50e-6(1/150e+6)=50*150 */
    EPwm4Regs.DBFED = 150;                           /* 下降沿死区时间，1us，7500个TBCLK */
#endif

    /* ET模块 */
    //此模块与发波无关，故不分AB
    //中断触发设置
    EPwm4Regs.ETSEL.bit.INTEN = 1;                  /* 打开外设中断 */
    EPwm4Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;       /* 选择CTR=0时触发中断 */
    EPwm4Regs.ETPS.bit.INTPRD = ET_1ST;             /* 来一个中断就触发事件 */

#if ePWM4_SOCA_ENA==1
    EPwm4Regs.ETSEL.bit.SOCAEN = 1;                 /* 向ADC发送SOCA转换信号 */
    EPwm4Regs.ETSEL.bit.SOCASEL = ET_CTRU_CMPA;     /* 即100，即0x4，CMPA加匹配时触发发送 */
    EPwm4Regs.ETPS.bit.SOCAPRD = ET_1ST;            /* 即0x1，一个事件就触发（相当于不分频） */
#endif

#if ePWM4_SOCB_ENA==1
    EPwm4Regs.ETSEL.bit.SOCBEN = 1;                 /* 向ADC发送SOCB转换信号 */
    EPwm4Regs.ETSEL.bit.SOCBSEL = ET_CTRU_CMPA;     /* 即100，即0x4，CMPA加匹配时触发发送 */
    EPwm4Regs.ETPS.bit.SOCBPRD = ET_1ST;            /* 即0x1，一个事件就触发（相当于不分频） */
#endif

    /* 开启时钟同步 */
    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;

#if ePWM4_INT_ENA==1
    EALLOW;
    PieVectTable.EPWM4_INT = &ISRepwm4;      /* 这里相当于重映射，也可以不给中断向量表重映射 */
    EDIS;

    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;      /* 使能PIE级中断 */
    PieCtrlRegs.PIEIER3.bit.INTx4 = 1;      /* 配置PIE级，ePWM4外设中断在INT3.4 */
    /* CPU级 */
    IER |= M_INT3;                          /* 配置CPU级，开启对应组中断 */
#endif
}



void ePWM5ABInit(int tbprd)
{
    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;    //禁用TBCLK，即禁用同步时钟
    SysCtrlRegs.PCLKCR1.bit.EPWM5ENCLK = 1;   //使能ePWM5时钟
    EDIS;

    InitEPwm5Gpio();                        /* GPIO复用为ePWM功能 */

    /* TB模块 */
    /* 时钟预分频，得到TBCLK，此处未分频，即直接使用150MHz的时钟频率 */
    EPwm5Regs.TBCTL.bit.CLKDIV = TB_DIV1;               /* 分频系数为2^K */
    EPwm5Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;            /* 大于0时分频系数为2K，为0时分频系数为1 */
    /* 得到fPWM,此处使用锯齿波 */
    EPwm5Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_DISABLE;     /* 不同步 */
    EPwm5Regs.TBCTL.bit.PHSEN = TB_DISABLE;             /* 禁止相位装载，使用主模式 */
    EPwm5Regs.TBPHS.half.TBPHS = 0;                     /* 不使用相位 */
    EPwm5Regs.TBCTR = 0x0000;                           /* 计数器清0 */
    EPwm5Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;          /* 上升计数模式 */
    //EPwm5Regs.TBCTL.bit.PRDLD = TB_IMMEDIATE;         /* 使用周期映射模式，影子寄存器 */
    //EPwm5Regs.TBCTL.bit.SWFSYNC = TB_SYNC_IN;         //同步信号直通
    EPwm5Regs.TBPRD = tbprd - 1;                        /* 频率为20kHz，周期为50us */

#if ePWM5_Only_TIM==0
    /* CC模块 */
    //ePWM5A
    EPwm5Regs.CMPA.half.CMPA = 2250;                 /* 不能直接用EPwm2Regs.CMPA=7499;会报错 */
    EPwm5Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;      /* ePWM2A使用映射模式 */
    EPwm5Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;    /* ePWM2A周期匹配装载,CTR=0时装载 */
    //ePWM5B
    EPwm5Regs.CMPB = 2250;
    EPwm5Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;      /* ePWM2B使用映射模式 */
    EPwm5Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;    /* ePWM2B周期匹配装载,CTR=0时装载 */

    /* AQ模块 */
    //ePWM5A
    EPwm5Regs.AQCTLA.bit.CAU = AQ_CLEAR;            /* 增计数匹配置0，即增计数时计到CMP就置1 */
    EPwm5Regs.AQCTLA.bit.PRD = AQ_SET;              /* 周期匹配置1，即CNT=PRD时置0 */
    //ePWM5B
    EPwm5Regs.AQCTLB.bit.CBU = AQ_CLEAR;
    EPwm5Regs.AQCTLB.bit.PRD = AQ_SET;

    /* DB模块，AB一起考虑 */
    EPwm5Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;   /* 即10，即0x3，使能上升沿下降沿延时，宏定义位于DSP2833x_EPwm_defines.h */
    EPwm5Regs.DBCTL.bit.IN_MODE = DBA_RED_DBB_FED;   /* ePWM2A上升沿延时,2B下降沿延时 */
    EPwm5Regs.DBCTL.bit.POLSEL = 0x2;                /* ePWM2B极性反转 */
    EPwm5Regs.DBRED = 150;                           /* 上升沿死区时间，1us，50e-6(1/150e+6)=50*150 */
    EPwm5Regs.DBFED = 150;                           /* 下降沿死区时间，1us，7500个TBCLK */
#endif

    /* ET模块 */
    //此模块与发波无关，故不分AB
    //中断触发设置
    EPwm5Regs.ETSEL.bit.INTEN = 1;                  /* 打开外设中断 */
    EPwm5Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;       /* 选择CTR=0时触发中断 */
    EPwm5Regs.ETPS.bit.INTPRD = ET_1ST;             /* 来一个中断就触发事件 */

#if ePWM5_SOCA_ENA==1
    EPwm5Regs.ETSEL.bit.SOCAEN = 1;                 /* 向ADC发送SOCA转换信号 */
    EPwm5Regs.ETSEL.bit.SOCASEL = ET_CTRU_CMPA;     /* 即100，即0x4，CMPA加匹配时触发发送 */
    EPwm5Regs.ETPS.bit.SOCAPRD = ET_1ST;            /* 即0x1，一个事件就触发（相当于不分频） */
#endif

#if ePWM5_SOCB_ENA==1
    EPwm5Regs.ETSEL.bit.SOCBEN = 1;                 /* 向ADC发送SOCB转换信号 */
    EPwm5Regs.ETSEL.bit.SOCBSEL = ET_CTRU_CMPA;     /* 即100，即0x4，CMPA加匹配时触发发送 */
    EPwm5Regs.ETPS.bit.SOCBPRD = ET_1ST;            /* 即0x1，一个事件就触发（相当于不分频） */
#endif

    /* 开启时钟同步 */
    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;

#if ePWM5_INT_ENA==1
    EALLOW;
    PieVectTable.EPWM5_INT = &ISRepwm5;      /* 这里相当于重映射，也可以不给中断向量表重映射 */
    EDIS;

    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;      /* 使能PIE级中断 */
    PieCtrlRegs.PIEIER3.bit.INTx5 = 1;      /* 配置PIE级，ePWM5外设中断在INT3.5 */
    /* CPU级 */
    IER |= M_INT3;                          /* 配置CPU级，开启对应组中断 */
#endif
}



void ePWM6ABInit(int tbprd)
{
    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;    //禁用TBCLK，即禁用同步时钟
    SysCtrlRegs.PCLKCR1.bit.EPWM6ENCLK = 1;   //使能ePWM6时钟
    EDIS;

    InitEPwm6Gpio();                        /* GPIO复用为ePWM功能 */

    /* TB模块 */
    /* 时钟预分频，得到TBCLK，此处未分频，即直接使用150MHz的时钟频率 */
    EPwm6Regs.TBCTL.bit.CLKDIV = TB_DIV1;               /* 分频系数为2^K */
    EPwm6Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;            /* 大于0时分频系数为2K，为0时分频系数为1 */
    /* 得到fPWM,此处使用锯齿波 */
    EPwm6Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_DISABLE;     /* 不同步 */
    EPwm6Regs.TBCTL.bit.PHSEN = TB_DISABLE;             /* 禁止相位装载，使用主模式 */
    EPwm6Regs.TBPHS.half.TBPHS = 0;                     /* 不使用相位 */
    EPwm6Regs.TBCTR = 0x0000;                           /* 计数器清0 */
    EPwm6Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;          /* 上升计数模式 */
    //EPwm6Regs.TBCTL.bit.PRDLD = TB_IMMEDIATE;         /* 使用周期映射模式，影子寄存器 */
    //EPwm6Regs.TBCTL.bit.SWFSYNC = TB_SYNC_IN;         //同步信号直通
    EPwm6Regs.TBPRD = tbprd - 1;                        /* 频率为20kHz，周期为50us */

#if ePWM6_Only_TIM==0
    /* CC模块 */
    //ePWM6A
    EPwm6Regs.CMPA.half.CMPA = 2250;                 /* 不能直接用EPwm2Regs.CMPA=7499;会报错 */
    EPwm6Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;      /* ePWM2A使用映射模式 */
    EPwm6Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;    /* ePWM2A周期匹配装载,CTR=0时装载 */
    //ePWM6B
    EPwm6Regs.CMPB = 2250;
    EPwm6Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;      /* ePWM2B使用映射模式 */
    EPwm6Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;    /* ePWM2B周期匹配装载,CTR=0时装载 */

    /* AQ模块 */
    //ePWM6A
    EPwm6Regs.AQCTLA.bit.CAU = AQ_CLEAR;            /* 增计数匹配置0，即增计数时计到CMP就置1 */
    EPwm6Regs.AQCTLA.bit.PRD = AQ_SET;              /* 周期匹配置1，即CNT=PRD时置0 */
    //ePWM6B
    EPwm6Regs.AQCTLB.bit.CBU = AQ_CLEAR;
    EPwm6Regs.AQCTLB.bit.PRD = AQ_SET;

    /* DB模块，AB一起考虑 */
    EPwm6Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;   /* 即10，即0x3，使能上升沿下降沿延时，宏定义位于DSP2833x_EPwm_defines.h */
    EPwm6Regs.DBCTL.bit.IN_MODE = DBA_RED_DBB_FED;   /* ePWM2A上升沿延时,2B下降沿延时 */
    EPwm6Regs.DBCTL.bit.POLSEL = 0x2;                /* ePWM2B极性反转 */
    EPwm6Regs.DBRED = 150;                           /* 上升沿死区时间，1us，50e-6(1/150e+6)=50*150 */
    EPwm6Regs.DBFED = 150;                           /* 下降沿死区时间，1us，7500个TBCLK */
#endif

    /* ET模块 */
    //此模块与发波无关，故不分AB
    //中断触发设置
    EPwm6Regs.ETSEL.bit.INTEN = 1;                  /* 打开外设中断 */
    EPwm6Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;       /* 选择CTR=0时触发中断 */
    EPwm6Regs.ETPS.bit.INTPRD = ET_1ST;             /* 来一个中断就触发事件 */

#if ePWM6_SOCA_ENA==1
    EPwm6Regs.ETSEL.bit.SOCAEN = 1;                 /* 向ADC发送SOCA转换信号 */
    EPwm6Regs.ETSEL.bit.SOCASEL = ET_CTRU_CMPA;     /* 即100，即0x4，CMPA加匹配时触发发送 */
    EPwm6Regs.ETPS.bit.SOCAPRD = ET_1ST;            /* 即0x1，一个事件就触发（相当于不分频） */
#endif

#if ePWM6_SOCB_ENA==1
    EPwm6Regs.ETSEL.bit.SOCBEN = 1;                 /* 向ADC发送SOCB转换信号 */
    EPwm6Regs.ETSEL.bit.SOCBSEL = ET_CTRU_CMPA;     /* 即100，即0x4，CMPA加匹配时触发发送 */
    EPwm6Regs.ETPS.bit.SOCBPRD = ET_1ST;            /* 即0x1，一个事件就触发（相当于不分频） */
#endif

    /* 开启时钟同步 */
    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;

#if ePWM6_INT_ENA==1
    EALLOW;
    PieVectTable.EPWM6_INT = &ISRepwm6;      /* 这里相当于重映射，也可以不给中断向量表重映射 */
    EDIS;

    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;      /* 使能PIE级中断 */
    PieCtrlRegs.PIEIER3.bit.INTx6 = 1;      /* 配置PIE级，ePWM6外设中断在INT3.6 */
    /* CPU级 */
    IER |= M_INT3;                          /* 配置CPU级，开启对应组中断 */
#endif
}
