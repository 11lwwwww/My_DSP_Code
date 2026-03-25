

/**
 * main.c
 */

/* 更新于2026.3.16
 */

#include "DSP2833x_Device.h"
#include "DSP2833x_Examples.h"

#include "ADCConfig.h"
#include "GPIOConfig.h"
//#include "ePWMConfig.h"
//#include "interruptExample.h"           /* 中断都放在这里 */
//#include "CPUTimerConfig.h"
#include "SCIConfig.h"
#include "vofa.h"
#include "CANConfig.h"
#include "eCAN_Device.h"
#include "eCAN_test.h"

//Uint32  TestMbox1 = 0;
//Uint32  TestMbox2 = 0;
//Uint32  TestMbox3 = 0;

void mailbox_read();


#define RamMode 0
#define FlashMode 1
#define RunType FlashMode


#if RunType==FlashMode
    extern Uint16 RamfuncsLoadStart;        /* DSP2833x_GlobalPrototypes.h中 */
    extern Uint16 RamfuncsLoadEnd;
    extern Uint16 RamfuncsRunStart;
    extern Uint16 RamfuncsLoadSize;
#endif


int main(void)
{
    InitSysCtrl();
    #if RunType==FlashMode          /* #if后面也是表达式 */
        MemCopy(&RamfuncsLoadStart, &RamfuncsLoadEnd, &RamfuncsRunStart);
        //MemCopy(&FPUmathTablesLoadStart, &FPUmathTablesLoadEnd, &FPUmathTablesRunStart);
        InitFlash();
        asm(" RPT #8 || NOP");      /* 等待一段时间，可以去掉，因为跳转到InitFlash后发现已经等待一段时间了 */
    #endif

    /* 每次都必须要做的 */
    DINT;                       //关闭总中断
    InitPieCtrl();              //初始化向量表,Pie的使能位、标志位清零

    IER = 0x0000;               //Cpu级的使能位、标志位清零，即关闭CPU级中断
    IFR = 0x0000;

    InitPieVectTable();         //配置各个中断服务函数的地址，放到表中
    asm(" RPT #8 || NOP");
    /* (每次都必须要做的)到这里 */

    /* user code */
    SCIAInit();
    CanTest();
    /* user code */
    EINT;                       /* 最后别忘了开启总中断 */



    while(1)
    {

    }
}



