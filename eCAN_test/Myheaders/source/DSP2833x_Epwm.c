/*
 * DSP2833x_Epwm.c
 *
 *  Created on: 2025年4月28日
 *      Author: Administrator
 */
#include "DSP2833x_Device.h"            // Peripheral address definitions
#include "DSP2833x_Examples.h"              // Main include file
//#include "DSP2833x_EPwm.h"

extern Uint16 counter;

Uint16 Dead = 40;

//桥臂A的PWM输出配置
void InitPwm1AB()
{

    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;//禁用ePWM中的TBCLK,即禁用同步时钟   //没有SysCtrlRegs.PCLKCR2
    SysCtrlRegs.PCLKCR1.bit.EPWM1ENCLK = 1;//使能ePWM2时钟
    EDIS;

    /****TB模块配置****/
    EPwm1Regs.TBPRD = counter-1;//周期寄存器设计
    EPwm1Regs.TBPHS.half.TBPHS = 0x0000;//在相位寄存器中设置计数器的起始计数位置
    EPwm1Regs.TBCTR = 0x0000;   //计数器清0
    EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;//设置计数模式为连续增计数模式
    EPwm1Regs.TBCTL.bit.PHSEN = TB_DISABLE;//忽略PWM计数器同步信号,禁止相位装载，即设为主模式
    //EPwm2Regs.TBCTL.bit.PHSDIR = TB_UP;//选择相位加载是增计数还是减计数部分，只有选择增减计数模式该位才有效
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;//分频系数为2^x,时钟预定标不分频
    EPwm1Regs.TBCTL.bit.CLKDIV = TB_DIV1;//分频系数为2x,此时不分频
    EPwm1Regs.TBCTL.bit.PRDLD = TB_SHADOW;//使用映射模式CTR=TPRD时装载
    EPwm1Regs.TBCTL.bit.SYNCOSEL = TB_CTR_ZERO;//产生同步信号，选择CTR=0为同步输出信号
    /****CC模块配置****/
    EPwm1Regs.CMPA.half.CMPA = 250;//设置比较寄存器A的值
    //EPwm1Regs.CMPB = 1026;          //设置比较寄存器B的值
    EPwm1Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;//A比较映射模式使能
    EPwm1Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;//B比较映射模式使能
    EPwm1Regs.CMPCTL.bit.LOADAMODE = CC_CTR_PRD;//CTR=PRD装载
    EPwm1Regs.CMPCTL.bit.LOADBMODE = CC_CTR_PRD;//CTR=PRD
    /****AQ模块配置****/
    EPwm1Regs.AQCTLA.bit.CAU = AQ_CLEAR;  //增计数匹配置低
    EPwm1Regs.AQCTLA.bit.PRD = AQ_SET;    //减计数匹配置高
    //EPwm1Regs.AQCTLB.bit.CAU = AQ_SET;//增计数匹配置高，因为使能了死区所以这两行可以注释掉
    //EPwm1Regs.AQCTLB.bit.CAD = AQ_CLEAR;  //减计数匹配置低
    /****DB模块配置****/
    EPwm1Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;//使能死区
    EPwm1Regs.DBCTL.bit.IN_MODE = DBA_ALL;//EPWMA作为上升沿和下降沿延时信号源
    EPwm1Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC;//使能高有效互补
    EPwm1Regs.DBRED = Dead;//上升沿插入死区时间
    EPwm1Regs.DBFED = Dead;//下降沿插入死区时间
    /****ET模块配置****/
    EPwm1Regs.ETSEL.bit.INTEN = 1;//使能中断
    EPwm1Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;//选择0匹配触发中断
    EPwm1Regs.ETPS.bit.INTPRD = ET_1ST;//1个事件触发中断
    //AD触发设置
    EPwm1Regs.ETSEL.bit.SOCAEN = 0;//不允许比较器A触发ADC
    EPwm1Regs.ETSEL.bit.SOCASEL = ET_CTR_ZERO;//选择CTR=0触发ADC
    EPwm1Regs.ETPS.bit.SOCAPRD = ET_3RD;//2个事件申请触发ADC

    //使能时基计数器时钟
    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;
}

void Pwm1ABOut_Stop()
{
    EALLOW;
    //SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;//禁用ePWM中的TBCLK
    SysCtrlRegs.PCLKCR1.bit.EPWM1ENCLK = 0;//失能ePWM2时钟
    EDIS;
    EPwm1Regs.TBCTR = 0x0000;   //计数器清0
    EALLOW;
    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO1 = 0;//将GPIO1设置为通用IO功能
    GpioDataRegs.GPACLEAR.bit.GPIO0 = 1;//设置为低电平
    GpioDataRegs.GPACLEAR.bit.GPIO1 = 1;
    EDIS;
}

void Pwm1ABOut_Start()
{
    EALLOW;
    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 1;//将GPIO0设置为EPWM功能
    GpioCtrlRegs.GPAMUX1.bit.GPIO1 = 1;//将GPIO1设置为EPWM功能
    EDIS;
    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;//使能ePWM中的TBCLK
    SysCtrlRegs.PCLKCR1.bit.EPWM1ENCLK = 1;//使能ePWM2时钟
    EDIS;
}


//桥臂B的PWM输出配置
void InitPwm2AB()
{

    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;//禁用ePWM中的TBCLK
    SysCtrlRegs.PCLKCR1.bit.EPWM2ENCLK = 1;//使能ePWM2时钟
    EDIS;

    /****TB模块配置****/
    EPwm2Regs.TBPRD = counter-1;//周期寄存器设计
    EPwm2Regs.TBPHS.half.TBPHS = 0;//在相位寄存器中设置计数器的起始计数位置
    EPwm2Regs.TBCTR = 0x0000;   //计数器清0
    EPwm2Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;//设置计数模式为连续增计数模式
    EPwm2Regs.TBCTL.bit.PHSEN = TB_ENABLE;//使能PWM计数器同步信号，即允许同步设置为从模式
    //EPwm2Regs.TBCTL.bit.PHSDIR = TB_UP;//选择相位加载是增计数还是减计数部分，只有选择增减计数模式该位才有效
    EPwm2Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;//分频系数为2^x,时钟预定标不分频
    EPwm2Regs.TBCTL.bit.CLKDIV = TB_DIV1;//分频系数为2x,2分频
    EPwm2Regs.TBCTL.bit.PRDLD = TB_SHADOW;//使用映射模式CTR=TPRD时装载
    EPwm2Regs.TBCTL.bit.SWFSYNC = TB_SYNC_IN;//同步信号直通
    /****CC模块配置****/
    EPwm2Regs.CMPA.half.CMPA = 250;//设置比较寄存器A的值
    //EPwm1Regs.CMPB = 1026;          //设置比较寄存器B的值
    EPwm2Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;//A比较映射模式使能
    EPwm2Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;//B比较映射模式使能
    EPwm2Regs.CMPCTL.bit.LOADAMODE = CC_CTR_PRD;//CTR=PRD装载
    EPwm2Regs.CMPCTL.bit.LOADBMODE = CC_CTR_PRD;//CTR=PRD
    /****AQ模块配置****/
    EPwm2Regs.AQCTLA.bit.CAU = AQ_CLEAR;  //增计数匹配置低
    EPwm2Regs.AQCTLA.bit.PRD = AQ_SET;    //减计数匹配置高
    //EPwm2Regs.AQCTLB.bit.CAU = AQ_SET;//增计数匹配置高，因为使能了死区所以这两行可以注释掉
    //EPwm2Regs.AQCTLB.bit.CAD = AQ_CLEAR;  //减计数匹配置低
    /****DB模块配置****/
    EPwm2Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;//使能死区
    EPwm2Regs.DBCTL.bit.IN_MODE = DBA_ALL;//EPWMA作为上升沿和下降沿延时信号源
    EPwm2Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC;//使能高有效互补
    EPwm2Regs.DBRED = Dead;//上升沿插入死区时间
    EPwm2Regs.DBFED = Dead;//下降沿插入死区时间
    /****ET模块配置****/
    EPwm2Regs.ETSEL.bit.INTEN = 0;//禁止中断
    EPwm2Regs.ETSEL.bit.INTSEL = ET_CTR_PRD;//选择0匹配触发中断
    EPwm2Regs.ETPS.bit.INTPRD = ET_2ND;//2个事件触发中断
    //AD触发设置
    EPwm2Regs.ETSEL.bit.SOCAEN = 0;//禁止出发比较器A触发ADC
    EPwm2Regs.ETSEL.bit.SOCASEL = ET_CTRU_CMPA;//选择EPWMA比较器A加匹配触发ADC
    EPwm2Regs.ETPS.bit.SOCAPRD = ET_2ND;//2个事件触发ADC

    //使能时基计数器时钟
    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;
}

void Pwm2ABOut_Stop()
{
    EALLOW;
    //SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;//禁用ePWM中的TBCLK
    SysCtrlRegs.PCLKCR1.bit.EPWM2ENCLK = 0;//禁用ePWM2时钟
    EDIS;
    EPwm2Regs.TBCTR = 0x0000;   //计数器清0
    EALLOW;
    GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO3 = 0;//将GPIO3设置为通用IO功能
    GpioDataRegs.GPACLEAR.bit.GPIO2 = 1;//设置为低电平
    GpioDataRegs.GPACLEAR.bit.GPIO3 = 1;
    EDIS;
}

void Pwm2ABOut_Start()
{
    EALLOW;
    GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 1;//将GPIO2设置为EPWM功能
    GpioCtrlRegs.GPAMUX1.bit.GPIO3 = 1;//将GPIO3设置为EPWM功能
    EDIS;
    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;//使能ePWM中的TBCLK
    SysCtrlRegs.PCLKCR1.bit.EPWM2ENCLK = 1;//使能ePWM2时钟
    EDIS;
}


//桥臂C的PWM输出配置
void InitPwm3AB()
{
    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;//禁用ePWM中的TBCLK
    SysCtrlRegs.PCLKCR1.bit.EPWM3ENCLK = 1;//使能ePWM3时钟
    EDIS;

    /****TB模块配置****/
    EPwm3Regs.TBPRD = counter-1;//周期寄存器设计
    EPwm3Regs.TBPHS.half.TBPHS = 0;//在相位寄存器中设置计数器的起始计数位置
    EPwm3Regs.TBCTR = 0x0000;   //计数器清0
    EPwm3Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;//设置计数模式为连续增计数模式
    EPwm3Regs.TBCTL.bit.PHSEN = TB_ENABLE;//使能PWM计数器同步信号，即允许同步设置为从模式
    //EPwm2Regs.TBCTL.bit.PHSDIR = TB_UP;//选择相位加载是增计数还是减计数部分，只有选择增减计数模式该位才有效
    EPwm3Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;//分频系数为2^x,时钟预定标不分频
    EPwm3Regs.TBCTL.bit.CLKDIV = TB_DIV1;//分频系数为2x,2分频
    EPwm3Regs.TBCTL.bit.PRDLD = TB_SHADOW;//使用映射模式CTR=TPRD时装载
    EPwm3Regs.TBCTL.bit.SWFSYNC = TB_SYNC_IN;//同步信号直通
    /****CC模块配置****/
    EPwm3Regs.CMPA.half.CMPA = 250;//设置比较寄存器A的值
    //EPwm1Regs.CMPB = 1026;          //设置比较寄存器B的值
    EPwm3Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;//A比较映射模式使能
    EPwm3Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;//B比较映射模式使能
    EPwm3Regs.CMPCTL.bit.LOADAMODE = CC_CTR_PRD;//CTR=PRD时装载CMPA
    EPwm3Regs.CMPCTL.bit.LOADBMODE = CC_CTR_PRD;//CTR=PRD时装载CMPB
    /****AQ模块配置****/
    EPwm3Regs.AQCTLA.bit.CAU = AQ_CLEAR;  //增计数匹配置低
    EPwm3Regs.AQCTLA.bit.PRD = AQ_SET;    //减计数匹配置高
    //EPwm2Regs.AQCTLB.bit.CAU = AQ_SET;//增计数匹配置高，因为使能了死区所以这两行可以注释掉
    //EPwm2Regs.AQCTLB.bit.CAD = AQ_CLEAR;  //减计数匹配置低
    /****DB模块配置****/
    EPwm3Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;//使能死区
    EPwm3Regs.DBCTL.bit.IN_MODE = DBA_ALL;//EPWMA作为上升沿和下降沿延时信号源
    EPwm3Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC;//使能高有效互补
    EPwm3Regs.DBRED = Dead;//上升沿插入死区时间
    EPwm3Regs.DBFED = Dead;//下降沿插入死区时间
    /****ET模块配置****/
    EPwm3Regs.ETSEL.bit.INTEN = 0;//禁止中断
    EPwm3Regs.ETSEL.bit.INTSEL = ET_CTR_PRD;//选择0匹配触发中断
    EPwm3Regs.ETPS.bit.INTPRD = ET_2ND;//2个事件触发中断
    //AD触发设置
    EPwm3Regs.ETSEL.bit.SOCAEN = 0;//禁止出发比较器A触发ADC
    EPwm3Regs.ETSEL.bit.SOCASEL = ET_CTRU_CMPA;//选择EPWMA比较器A加匹配触发ADC
    EPwm3Regs.ETPS.bit.SOCAPRD = ET_2ND;//2个事件触发ADC

    //使能时基计数器时钟
    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;
}

void Pwm3ABOut_Stop()
{
    EALLOW;
    //SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;//禁用ePWM中的TBCLK
    SysCtrlRegs.PCLKCR1.bit.EPWM3ENCLK = 0;//禁用ePWM3时钟
    EDIS;
    EPwm3Regs.TBCTR = 0x0000;   //计数器清0
    EALLOW;
    GpioCtrlRegs.GPAMUX1.bit.GPIO4 = 0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO5 = 0;//将GPIO5设置为通用IO功能
    GpioDataRegs.GPACLEAR.bit.GPIO4 = 1;//设置为低电平
    GpioDataRegs.GPACLEAR.bit.GPIO5 = 1;
    EDIS;
}

void Pwm3ABOut_Start()
{
    EALLOW;
    GpioCtrlRegs.GPAMUX1.bit.GPIO4 = 1;//将GPIO4设置为EPWM功能
    GpioCtrlRegs.GPAMUX1.bit.GPIO5 = 1;//将GPIO5设置为EPWM功能
    EDIS;
    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;//使能ePWM中的TBCLK
    SysCtrlRegs.PCLKCR1.bit.EPWM3ENCLK = 1;//使能ePWM3时钟
    EDIS;
}

//EPWM中断配置1，用来跑锁相环，锁q轴来提取直流信号
void InitPwm4AB()
{
    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;//禁用ePWM中的TBCLK
    SysCtrlRegs.PCLKCR1.bit.EPWM4ENCLK = 1;//使能ePWM4时钟
    EDIS;

    /****TB模块配置****/
    EPwm4Regs.TBPRD = 2500-1;//周期寄存器设计
    EPwm4Regs.TBPHS.half.TBPHS = 0;//在相位寄存器中设置计数器的起始计数位置
    EPwm4Regs.TBCTR = 0x0000;   //计数器清0
    EPwm4Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;//设置计数模式为连续增计数模式
    EPwm4Regs.TBCTL.bit.PHSEN = TB_DISABLE;//不使能PWM计数器同步信号，即不允许同步设置为从模式
    //EPwm3Regs.TBCTL.bit.PHSDIR = TB_UP;//选择相位加载是增计数还是减计数部分，只有选择增减计数模式该位才有效
    EPwm4Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;//分频系数为2^x,时钟预定标不分频
    EPwm4Regs.TBCTL.bit.CLKDIV = TB_DIV1;//分频系数为2x,2分频
    EPwm4Regs.TBCTL.bit.PRDLD = TB_SHADOW;//使用映射模式(即使用辅助寄存器),CTR=TPRD时装载
    EPwm4Regs.TBCTL.bit.SWFSYNC = TB_SYNC_IN;//同步信号直通
    /****CC模块配置****/
    EPwm4Regs.CMPA.half.CMPA = 1250;//设置比较寄存器A的值
    //EPwm1Regs.CMPB = 1026;          //设置比较寄存器B的值
    EPwm4Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;//A比较映射模式使能
    EPwm4Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;//B比较映射模式使能
    EPwm4Regs.CMPCTL.bit.LOADAMODE = CC_CTR_PRD;//CTR=PRD装载
    EPwm4Regs.CMPCTL.bit.LOADBMODE = CC_CTR_PRD;//CTR=PRD
    /****AQ模块配置****/
    EPwm4Regs.AQCTLA.bit.CAU = AQ_CLEAR;  //增计数匹配置低
    EPwm4Regs.AQCTLA.bit.PRD = AQ_SET;    //减计数匹配置高
    //EPwm2Regs.AQCTLB.bit.CAU = AQ_SET;//增计数匹配置高，因为使能了死区所以这两行可以注释掉
    //EPwm2Regs.AQCTLB.bit.CAD = AQ_CLEAR;  //减计数匹配置低
    /****DB模块配置****/
    EPwm4Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;//使能死区
    EPwm4Regs.DBCTL.bit.IN_MODE = DBA_ALL;//EPWMA作为上升沿和下降沿延时信号源
    EPwm4Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC;//使能高有效互补
    EPwm4Regs.DBRED = 20;//上升沿插入死区时间
    EPwm4Regs.DBFED = 20;//下降沿插入死区时间
    /****ET模块配置****/
    EPwm4Regs.ETSEL.bit.INTEN = 1;//允许中断
    EPwm4Regs.ETSEL.bit.INTSEL = ET_CTR_PRD;//选择0匹配触发中断
    EPwm4Regs.ETPS.bit.INTPRD = ET_2ND;//2个事件触发中断
    //AD触发设置
    EPwm4Regs.ETSEL.bit.SOCAEN = 0;//禁止出发比较器A触发ADC
    EPwm4Regs.ETSEL.bit.SOCASEL = ET_CTRU_CMPA;//选择EPWMA比较器A加匹配触发ADC
    EPwm4Regs.ETPS.bit.SOCAPRD = ET_2ND;//2个事件触发ADC

    //使能时基计数器时钟
    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;
}

void Pwm4ABOut_Stop()
{
    EALLOW;
    //SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;//禁用ePWM中的TBCLK
    SysCtrlRegs.PCLKCR1.bit.EPWM4ENCLK = 0;//禁用ePWM4时钟
    EDIS;
    EPwm4Regs.TBCTR = 0x0000;   //计数器清0
    EALLOW;
    GpioCtrlRegs.GPAMUX1.bit.GPIO6 = 0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO7 = 0;//将GPIO7设置为通用IO功能
    GpioDataRegs.GPACLEAR.bit.GPIO6 = 1;//设置为低电平
    GpioDataRegs.GPACLEAR.bit.GPIO7 = 1;
    EDIS;
}

void Pwm4ABOut_Start()
{
    EALLOW;
    GpioCtrlRegs.GPAMUX1.bit.GPIO6 = 1;//将GPIO6设置为EPWM功能
    GpioCtrlRegs.GPAMUX1.bit.GPIO7 = 1;//将GPIO7设置为EPWM功能
    EDIS;
    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;//使能ePWM中的TBCLK
    SysCtrlRegs.PCLKCR1.bit.EPWM4ENCLK = 1;//使能ePWM4时钟
    EDIS;
}

//EPWM中断配置2，用来更新相位，这样做是为了为并联做提前的准备
void InitPwm5AB()
{
    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;//禁用ePWM中的TBCLK
    SysCtrlRegs.PCLKCR1.bit.EPWM5ENCLK = 1;//使能ePWM5时钟
    EDIS;

    /****TB模块配置****/
    EPwm5Regs.TBPRD = 5000-1;//周期寄存器设计
    EPwm5Regs.TBPHS.half.TBPHS = 0;//在相位寄存器中设置计数器的起始计数位置
    EPwm5Regs.TBCTR = 0x0000;   //计数器清0
    EPwm5Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;//设置计数模式为连续增计数模式
    EPwm5Regs.TBCTL.bit.PHSEN = TB_DISABLE;//不使能PWM计数器同步信号，即不允许同步设置为从模式
    //EPwm3Regs.TBCTL.bit.PHSDIR = TB_UP;//选择相位加载是增计数还是减计数部分，只有选择增减计数模式该位才有效
    EPwm5Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;//分频系数为2^x,时钟预定标不分频
    EPwm5Regs.TBCTL.bit.CLKDIV = TB_DIV1;//分频系数为2x,2分频
    EPwm5Regs.TBCTL.bit.PRDLD = TB_SHADOW;//使用映射模式CTR=TPRD时装载
    EPwm5Regs.TBCTL.bit.SWFSYNC = TB_SYNC_DISABLE;//同步信号直通
    /****CC模块配置****/
    EPwm5Regs.CMPA.half.CMPA = 200;//设置比较寄存器A的值
    //EPwm1Regs.CMPB = 1026;          //设置比较寄存器B的值
    EPwm5Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;//A比较映射模式使能
    EPwm5Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;//B比较映射模式使能
    EPwm5Regs.CMPCTL.bit.LOADAMODE = CC_CTR_PRD;//CTR=PRD装载
    EPwm5Regs.CMPCTL.bit.LOADBMODE = CC_CTR_PRD;//CTR=PRD
    /****AQ模块配置****/
    EPwm5Regs.AQCTLA.bit.CAU = AQ_CLEAR;  //增计数匹配置低
    EPwm5Regs.AQCTLA.bit.PRD = AQ_SET;    //减计数匹配置高
    //EPwm2Regs.AQCTLB.bit.CAU = AQ_SET;//增计数匹配置高，因为使能了死区所以这两行可以注释掉
    //EPwm2Regs.AQCTLB.bit.CAD = AQ_CLEAR;  //减计数匹配置低
    /****DB模块配置****/
    EPwm5Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;//使能死区
    EPwm5Regs.DBCTL.bit.IN_MODE = DBA_ALL;//EPWMA作为上升沿和下降沿延时信号源
    EPwm5Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC;//使能高有效互补
    EPwm5Regs.DBRED = 20;//上升沿插入死区时间
    EPwm5Regs.DBFED = 20;//下降沿插入死区时间
    /****ET模块配置****/
    EPwm5Regs.ETSEL.bit.INTEN = 0;//不允许中断
    EPwm5Regs.ETSEL.bit.INTSEL = ET_CTR_PRD;//选择0匹配触发中断
    EPwm5Regs.ETPS.bit.INTPRD = ET_2ND;//2个事件触发中断
    //AD触发设置
    EPwm5Regs.ETSEL.bit.SOCAEN = 0;//禁止出发比较器A触发ADC
    EPwm5Regs.ETSEL.bit.SOCASEL = ET_CTRU_CMPA;//选择EPWMA比较器A加匹配触发ADC
    EPwm5Regs.ETPS.bit.SOCAPRD = ET_2ND;//2个事件触发ADC

    //使能时基计数器时钟
    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;
}

void Pwm5ABOut_Stop()
{
    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;//禁用ePWM中的TBCLK
    SysCtrlRegs.PCLKCR1.bit.EPWM5ENCLK = 0;//禁用ePWM5时钟
    EDIS;
    EPwm5Regs.TBCTR = 0x0000;   //计数器清0
    EALLOW;
    GpioCtrlRegs.GPAMUX1.bit.GPIO8 = 0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO9 = 0;//将GPIO9设置为通用IO功能
    GpioDataRegs.GPACLEAR.bit.GPIO8 = 1;//设置为低电平
    GpioDataRegs.GPACLEAR.bit.GPIO9 = 1;
    EDIS;
}

void Pwm5ABOut_Start()
{
    EALLOW;
    GpioCtrlRegs.GPAMUX1.bit.GPIO8 = 1;//将GPIO8设置为EPWM功能
    GpioCtrlRegs.GPAMUX1.bit.GPIO9 = 1;//将GPIO9设置为EPWM功能
    EDIS;
    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;//使能ePWM中的TBCLK
    SysCtrlRegs.PCLKCR1.bit.EPWM5ENCLK = 1;//使能ePWM5时钟
    EDIS;
}




