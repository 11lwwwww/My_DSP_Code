已成功

/* cpu定时器的中断服务函数中要做的必要工作（以保证下次中断可以正常进入），此处已CpuTimer0为例 */
PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;     //清除应答寄存器
CpuTimer0Regs.TCR.bit.TIF = 1;
CpuTimer0Regs.TCR.bit.TRB = 1;              //重装载定时器

/* ePWM用作定时器的中断服务函数中要做的必要工作（以保证下次中断可以正常进入），此处已ePWM1为例，其中断位于中断向量表中的组3 */
EPwm1Regs.ETCLR.bit.INT = 1;        	 // 清除ePWM1中断标志，外设级
PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;  // 应答PIE组3中断，PIE级

注意区分