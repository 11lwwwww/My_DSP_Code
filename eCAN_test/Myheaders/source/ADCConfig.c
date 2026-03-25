#include "DSP2833x_Device.h"
#include "DSP2833x_Examples.h"

#include "interruptExample.h"
#include "ADCConfig.h"

/* 在DSP中，DMA也有相关的段，可以去.cmd文件中搜索DMA找到 */

#define SYSCLKNow 150
#define ADC_usDELAY 5000L


/* 对于ADC模块，不需要复用GPIO模块，因为ADC是专有的引脚，没有GPIO功能，可在C2000助手引脚分布那一节看到 */
void AdcConfig(void)
{
    /* ADC转换前准备工作 */
    /* 开启ADC外设时钟，SysCtrlRegs寄存器受保护，需使用EALLOW和EDIS */
    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.ADCENCLK = 1;
    ADC_cal();              //ADC校准
    EDIS;
    /* ADC上电配置 */
    AdcRegs.ADCTRL3.all = 0x00E0;//启动A/D采样，顺序采样，0X00E0=0000 0000 1110 0000  //最右边是第0位，即这里是第5~7位为1，与低功耗模式有关
    DELAY_US(ADC_usDELAY);
    /* 将高速外设时钟设置为25MHz */
    #if SYSCLKNow==150
        EALLOW;
        SysCtrlRegs.HISPCP.bit.HSPCLK = 0x3;
        EDIS;
    #elif SYSCLKNow==100
        EALLOW;
        SysCtrlRegs.HISPCP.bit.HSPCLK = 0x2;
        EDIS;
    #endif

    /* ADC配置 */
    AdcRegs.ADCTRL3.all = 0x00E0;               /* ADC电源配置上电，采用顺序采样，分频为不分频 */
    AdcRegs.ADCTRL1.bit.ACQ_PS = 0x05;          /* 采样窗口设置为6个时钟周期 */
    AdcRegs.ADCTRL1.bit.SEQ_CASC = 1;           /* 级联模式 */
    AdcRegs.ADCTRL1.bit.CONT_RUN = 0;           /* 连续转换,不配置的话默认为0，即启动/停止模式 */

    /* ADC通道排序 */
    AdcRegs.ADCMAXCONV.bit.MAX_CONV1 = 0x1;     /* 转换两个通道 */
    AdcRegs.ADCCHSELSEQ1.bit.CONV00 = 0x0;      /* 选择ADCINA0，CONV00占四位，故也可以赋值为0000 */
    AdcRegs.ADCCHSELSEQ1.bit.CONV01 = 0x1;      /* 选择ADCINA1，也可以赋值为0001 */

    /* 是否手动开启ADC转换（也可以使用ePWM触发开启） */
    AdcRegs.ADCTRL2.bit.SOC_SEQ1 = 0;           //0为关闭，不手动开启，可在ePWM中断里开启

    /* 是否允许ePWM的SOC触发转换 */
    AdcRegs.ADCTRL2.bit.EPWM_SOCA_SEQ1 = 1;     /* 允许ePWM发出的SOCA触发SEQ或SEQ1 */
    AdcRegs.ADCTRL2.bit.EPWM_SOCB_SEQ = 1;      /* 级联模式下，允许ePWM发出的SOCB触发SEQ，级联模式下才有用 */
    //AdcRegs.ADCTRL2.bit.EPWM_SOCB_SEQ2 = 1;   /* 在双排序器模式下，允许ePWM发出的SOCB触发SEQ2 */

#if ADC_INT_ENA==1
    /* 外设级配置 */
    AdcRegs.ADCTRL2.bit.INT_ENA_SEQ1 = 1;       /* 开启排序器SEQ1转换完成中断 */
    AdcRegs.ADCTRL2.bit.INT_MOD_SEQ1 = 0;      /* 每转换完成一个序列就置中断标志位 */
    AdcRegs.ADCTRL2.bit.RST_SEQ1 = 1;           /* 复位一下 */

    /* PIE级 */
    EALLOW;
    PieVectTable.ADCINT = &ISRADC;      /* 这里相当于重映射，也可以不给中断向量表重映射 */
    EDIS;
    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;      /* 使能PIE级中断 */
    PieCtrlRegs.PIEIER1.bit.INTx6 = 1;      /* 配置PIE级，ADC中断在INT1.6 */

    /* CPU级 */
    IER |= M_INT1;                          /* 配置CPU级，开启对应组中断 */
#endif
}



