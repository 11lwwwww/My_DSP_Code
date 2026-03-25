#ifndef EPWMCONFIG_H
#define EPWMCONFIG_H

/* 中断设置 */
#define ePWM1_INT_ENA   0
#define ePWM2_INT_ENA   0
#define ePWM3_INT_ENA   0
#define ePWM4_INT_ENA   0
#define ePWM5_INT_ENA   0
#define ePWM6_INT_ENA   0

/* ADC触发设置 */
#define ePWM1_SOCA_ENA    0
#define ePWM1_SOCB_ENA    0
#define ePWM2_SOCA_ENA    0
#define ePWM2_SOCB_ENA    0
#define ePWM3_SOCA_ENA    0
#define ePWM3_SOCB_ENA    0
#define ePWM4_SOCA_ENA    0
#define ePWM4_SOCB_ENA    0
#define ePWM5_SOCA_ENA    0
#define ePWM5_SOCB_ENA    0
#define ePWM6_SOCA_ENA    1
#define ePWM6_SOCB_ENA    0

/* 是否仅用作TIM定时器模式，为0则发波，为1则仅定时器 */
#define ePWM1_Only_TIM   1
#define ePWM2_Only_TIM   1
#define ePWM3_Only_TIM   1
#define ePWM4_Only_TIM   1
#define ePWM5_Only_TIM   1
#define ePWM6_Only_TIM   0


void ePWM1ABInit(int tbprd);
void ePWM2ABInit(int tbprd);
void ePWM3ABInit(int tbprd);
void ePWM4ABInit(int tbprd);
void ePWM5ABInit(int tbprd);
void ePWM6ABInit(int tbprd);

#endif
