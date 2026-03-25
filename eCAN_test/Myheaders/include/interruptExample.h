#ifndef INTERRUPTEXAMPLE_H
#define INTERRUPTEXAMPLE_H

extern int Voltage1[2];

/* 定时器中断服务函数 */
interrupt void ISRepwm1(void);
interrupt void ISRepwm2(void);
interrupt void ISRepwm3(void);
interrupt void ISRepwm4(void);
interrupt void ISRepwm5(void);
interrupt void ISRepwm6(void);

/* ADC中断服务函数 */
interrupt void ISRADC(void);

/* Cpu定时器中断服务函数 */
interrupt void ISRTimer0(void);

/* GPIO外部中断中断服务函数 */
interrupt void xint1_isr(void);
interrupt void xint2_isr(void);
interrupt void xint3_isr(void);
interrupt void xint4_isr(void);
interrupt void xint5_isr(void);

/* SCI中断服务函数 */
interrupt void sciaTxFifoIsr(void);
interrupt void sciaRxFifoIsr(void);

/* eCAN中断服务函数 */
interrupt void eCANaINT0Isr(void);
interrupt void eCANaINT1Isr(void);

#endif
