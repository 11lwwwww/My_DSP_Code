#include "DSP2833x_Device.h"
#include "DSP2833x_Examples.h"

#include "CPUTimerConfig.h"
#include "interruptExample.h"

void CpuTimer0Config(int us)
{
    /* 设置cpu定时器 */
    InitCpuTimers();
    ConfigCpuTimer(&CpuTimer0, 150, us);   //以us为单位，此处设置为1s
    StartCpuTimer0();

    /* 中断配置 */
    EALLOW;
    PieVectTable.TINT0 = &ISRTimer0;
    EDIS;
    /* 使能cpu中断 */
    IER |= M_INT1;
    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;
}
