#ifndef ECAN_DEVICE_H
#define ECAN_DEVICE_H


typedef struct CanDevice tCanDevice, *ptCanDevice;              /* 实例位于eCAN_Device.c */
typedef struct CanConfig tCanConfig, *ptCanConfig;              /* 实例位于eCAN_test.c */
typedef struct MBOXConfig tMBOXConfig, *ptMBOXConfig;           /* 实例位于eCAN_test.c */


struct CanConfig
{
    Uint16 Mode;        /* 选择为SCC模式或eCAN模式 */
    Uint16 BRP;         /* 波特率配置 */
    Uint16 TS1;
    Uint16 TS2;
    Uint16 SAM;         /* 采样选择单次采样还是三次采样 */
    Uint16 STM;         /* 是否使用回环模式 */
    Uint16 TI_option;   /* 中断选择，可选中断0，中断1，0为开启中断0，1为开启中断1，仅起判断作用，不调用关于中断的函数的话也没用 */
};


struct MBOXConfig
{
    Uint32 MBOX_Num;    /* 邮箱编号 */
    Uint16 dire;        /* 方向，实际没用到 */
    Uint32 MSG_Id;      /* 消息ID（邮箱ID），实际上将MSGID的所有位都包含了，IDE位直接兼容在里面 */
    Uint32 RTR;         /* 是否使用远程帧 */
    Uint16 IDE;         /* 是否使用扩展模式，没用到，在MSG_Id中直接赋值了 */
    Uint16 DLC;         /* 数据大小 */
    Uint16 AME;         /* 是否开启屏蔽 */
    Uint32 Mask;        /* 掩码 */
    Uint16 TPL;         /* 发送优先级，仅用于eCAN模式 */
    Uint16 AAM;         /* 自动应答位 */
    Uint16 OPC;
    Uint16 AMI;
    Uint32 MSG_H;       /* 消息高位 */
    Uint32 MSG_L;       /* 消息低位 */
};


struct CanDevice
{
    char *name;
    void (*init)(struct CanDevice *pdev, struct CanConfig *pConfig);
    void (*initCanTran)(struct CanDevice *pdev, struct MBOXConfig *pMBOX);
    int  (*initCanRecv)(struct CanDevice *pdev, struct CanConfig *pConfig, struct MBOXConfig *pMBOX);
    void (*initManyTran)(struct CanDevice *pdev, Uint16 len, struct MBOXConfig *pMBOX);
    int  (*initManyRecv)(struct CanDevice *pdev, Uint16 len, struct CanConfig *pConfig, struct MBOXConfig *pMBOX);
    void (*initCanIT)(struct CanDevice *pdev, struct CanConfig *pConfig, struct MBOXConfig *pMBOX);
    void* PrivData;                /* 作句柄用,没有用到 */
    struct CanDevice *pNext;
};


int ResgisterCanDevice(void);
ptCanDevice GetCanDevice(char *name);


#endif





