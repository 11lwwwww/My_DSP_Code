/*
 * can.c
 *
 *  Created on: 2024年1月24日
 *      Author: Free丶Chan
 *     Version: V1.0
 */

/* INCLUDE BEGIN */
#include "can.h"
/* INCLUDE END */

/*
 * Variable
 */
struct CAN_DATA_BUF senddatabuf;
struct CAN_REC_BUF RecDataBuf;

/*
 * Funtion
 */
void userInitECana(void);
void InitCanTran(Uint32 id);
void InitCanRec(Uint32 id);


/* 函数：void cana_init(void)
 * 用途：CANA初始化、初始化发送和接收邮箱
 */
void cana_init(void)
{
    userInitECana();
    InitCanTran(INIT_ID);
    InitCanRec(INIT_ID);
}

/* 函数：void InitECana(void)
 * 用途：初始化ECANA GPIO、eCAN功能
 * 调用：cana_init(void)
 */
void userInitECana(void)
{
    struct ECAN_REGS ECanaShadow;       //声明一个影子寄存器
    volatile struct MBOX *eCANa_MBOX;
    Uint16 i;

    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.ECANAENCLK=1;    // eCAN-A

    GpioCtrlRegs.GPAPUD.bit.GPIO18 = 0;     // Enable pull-up for GPIO18 (CANRXA)
    GpioCtrlRegs.GPAPUD.bit.GPIO19 = 0;     // Enable pull-up for GPIO19 (CANTXA)

    GpioCtrlRegs.GPAQSEL2.bit.GPIO18 = 3;   // Asynch qual for GPIO18 (CANRXA)

    GpioCtrlRegs.GPAMUX2.bit.GPIO18 = 3;    // Configure GPIO18 for CANRXA operation
    GpioCtrlRegs.GPAMUX2.bit.GPIO19 = 3;    // Configure GPIO19 for CANTXA operation
    EDIS;

    EALLOW;
    // Step1:配置eCAN的TX和RX引脚功能
    ECanaShadow.CANTIOC.all = ECanaRegs.CANTIOC.all;
    ECanaShadow.CANTIOC.bit.TXFUNC = 1;
    ECanaRegs.CANTIOC.all = ECanaShadow.CANTIOC.all;

    ECanaShadow.CANRIOC.all = ECanaRegs.CANRIOC.all;
    ECanaShadow.CANRIOC.bit.RXFUNC = 1;
    ECanaRegs.CANRIOC.all = ECanaShadow.CANRIOC.all;

    //Step2:配置CANMC寄存器
    ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;
    ECanaShadow.CANMC.bit.SCB = 1;      //eCAN模式
    ECanaShadow.CANMC.bit.ABO = 1;      //总线自动恢复使能
    ECanaShadow.CANMC.bit.DBO = 1;      //数据发送/接收：低位优先
    ECanaShadow.CANMC.bit.SUSP = 1;     //CAN功能不受调试（断点等）影响
    ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;

    //初始化MSGCTRL寄存器
    eCANa_MBOX= & ECanaMboxes.MBOX0;
    for(i = 0; i < 32; i++)
    {
        eCANa_MBOX->MSGCTRL.all = 0;
        eCANa_MBOX++;
    }

    //清除CANTA、CANRMP、CANGIFx寄存器
    ECanaRegs.CANTRR.all = 0xFFFFFFFF;      // Clear all TRRn bits
    ECanaRegs.CANTA.all = 0xFFFFFFFF;       // Clear all TAn bits
    ECanaRegs.CANRMP.all = 0xFFFFFFFF;      // Clear all RMPn bits
    ECanaRegs.CANGIF0.all = 0xFFFFFFFF;     // Clear all interrupt flag bits
    ECanaRegs.CANGIF1.all = 0xFFFFFFFF;

    //获取配置权限(配置CANBTC寄存器)
    ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;
    ECanaShadow.CANMC.bit.CCR = 1 ;
    ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;

    //CCR置位后，查询CANES.CCE置位，方可配置CANBTC
    ECanaShadow.CANES.all = ECanaRegs.CANES.all;
    do
    {
        ECanaShadow.CANES.all = ECanaRegs.CANES.all;
    } while(ECanaShadow.CANES.bit.CCE != 1 );

    ECanaShadow.CANBTC.all = 0;

    #if (CPU_FRQ_150MHZ)
    ECanaShadow.CANBTC.bit.BRPREG = 9;
    ECanaShadow.CANBTC.bit.TSEG2REG = 1;
    ECanaShadow.CANBTC.bit.TSEG1REG = 6;
    #endif

    #if (CPU_FRQ_100MHZ)
    ECanaShadow.CANBTC.bit.BRPREG = 4;
    ECanaShadow.CANBTC.bit.TSEG2REG = 1;
    ECanaShadow.CANBTC.bit.TSEG1REG = 6;
    #endif

    ECanaShadow.CANBTC.bit.SAM = 1;
    ECanaRegs.CANBTC.all = ECanaShadow.CANBTC.all;

    ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;
    ECanaShadow.CANMC.bit.CCR = 0 ;
    ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;

    ECanaShadow.CANES.all = ECanaRegs.CANES.all;

    do
    {
        ECanaShadow.CANES.all = ECanaRegs.CANES.all;
    } while(ECanaShadow.CANES.bit.CCE != 0 );// Wait for CCE bit to be cleared

    //失能所有邮箱
    ECanaRegs.CANME.all = 0;        // Required before writing the MSGIDs

    EDIS;
}

/* 函数：void InitTranMBOX(Uint16 MBOX_Num, Uint32 MSG_Id)
 * 用途：初始化发送邮箱
 * 参数：MBOX_Num：邮箱号
 *     MSG_Id：邮箱ID
 * 调用：void InitCanTran(Uint32 id);
 */
void InitTranMBOX(Uint16 MBOX_Num, Uint32 MSG_Id)
{
    volatile struct ECAN_REGS ECanShadow;
    volatile struct MBOX *ECanaMBOX;
    Uint32 temp;

    EALLOW;

    ECanaMBOX = &ECanaMboxes.MBOX0;
    temp = (Uint32) 1 << MBOX_Num;
    //失能邮箱
    ECanShadow.CANME.all = ECanaRegs.CANME.all;
    ECanShadow.CANME.all &= ~temp;
    ECanaRegs.CANME.all = ECanShadow.CANME.all;
    //邮箱方向：发送
    ECanShadow.CANMD.all = ECanaRegs.CANMD.all;
    ECanShadow.CANMD.all &= ~temp;
    ECanaRegs.CANMD.all = ECanShadow.CANMD.all;
    //配置邮箱信息
    ECanaMBOX += MBOX_Num;
    ECanaMBOX->MSGID.all = MSG_Id;
    ECanaMBOX->MSGCTRL.bit.DLC = 8;
    //使能邮箱
    ECanShadow.CANME.all = ECanaRegs.CANME.all;
    ECanShadow.CANME.all |= temp;
    ECanaRegs.CANME.all = ECanShadow.CANME.all;

    EDIS;
}

/* 函数：void InitCanTran(Uint32 id)
 * 用途：初始化多个发送邮箱
 * 参数：id：ID
 * 调用：cana_init(void)
 * 说明：开启3个发送邮箱，邮箱定义如下：
 * MSG 0-15 :发送邮箱，从15往下开始配置
 * MSG 16-31 :接收邮箱，从31往下开始配置
 */
void InitCanTran(Uint32 id)
{
    union CAN_MSGID_STRU msgId;
    Uint16 i;
    id &= ID_MASK;
    msgId.bit.msgID = id;           // ID标识位
    msgId.bit.aam = 0;              // 自动应答标志
    msgId.bit.ame = 0;              // 接收屏蔽使能，发送邮箱无效
    msgId.bit.ide = 1;              // 扩展帧标志

    //开启3个发送邮箱，邮箱定义如下：
    //MSG 0-15 :发送邮箱
    //MSG 16-31 :接收邮箱
    for(i = 0; i < TRAN_MSG_NUM; i++)
    {
        InitTranMBOX(15-i, msgId.all);      // 初始化发送邮箱
    }

}

/* 函数：void InitRecMBOX(Uint16 MBOX_Num, Uint32 MSG_Id, Uint32 lam, Uint16 opc)
 * 用途：初始化接收邮箱
 * 参数：MBOX_Num：邮箱号
 *     MSG_Id：邮箱ID
 *     lam：设置掩码
 *     opc：覆盖保护，1：保护，0：不保护
 * 调用：void InitCanRec(Uint32 id);
 */
void InitRecMBOX(Uint16 MBOX_Num, Uint32 MSG_Id, Uint32 lam, Uint16 opc)
{
    volatile struct ECAN_REGS ECanShadow;
    volatile struct MBOX *ECanaMBOX;
    volatile union CANLAM_REG *MBOX_LAM;
    Uint32 temp;

    ECanaMBOX = &ECanaMboxes.MBOX0;
    MBOX_LAM = &ECanaLAMRegs.LAM0;
    temp = (Uint32) 1 << MBOX_Num;
    //失能邮箱
    ECanShadow.CANME.all = ECanaRegs.CANME.all;
    ECanShadow.CANME.all &= ~temp;
    ECanaRegs.CANME.all = ECanShadow.CANME.all;
    //设置方向：接收
    ECanShadow.CANMD.all = ECanaRegs.CANMD.all;
    ECanShadow.CANMD.all |= temp;
    ECanaRegs.CANMD.all = ECanShadow.CANMD.all;
    //设置邮箱覆盖保护
    if(opc == 1) //禁止被覆盖
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
    //配置邮箱信息
    ECanaMBOX += MBOX_Num;
    ECanaMBOX->MSGID.all = MSG_Id;
    ECanaMBOX->MSGCTRL.bit.DLC = 8;
    //使能邮箱
    ECanShadow.CANME.all = ECanaRegs.CANME.all;
    ECanShadow.CANME.all |= temp;
    ECanaRegs.CANME.all = ECanShadow.CANME.all;
    //设置邮箱掩码
    MBOX_LAM += MBOX_Num;
    MBOX_LAM->all = lam;
}

/* 函数：void InitCanRec(Uint32 id)
 * 用途：初始化多个接收邮箱
 * 参数：id：ID
 * 调用：cana_init(void)
 * 说明：开启3个接收邮箱，邮箱定义如下：
 * MSG 0-15 :发送邮箱，从15往下开始配置
 * MSG 16-31 :接收邮箱，从31往下开始配置
 * 初始化的最后一个邮箱可以覆盖
 */
void InitCanRec(Uint32 id)
{
    union CAN_MSGID_STRU msgId;
    Uint32 lam;
    Uint16 i;
    id &= ID_MASK;
    msgId.bit.msgID = id;                   // id
    msgId.bit.ame = 1;                      // 接收屏蔽使能
    msgId.bit.ide = 1;                      // 扩展帧标志
    //~(7ul<<29) 高3位为0，其他位为1
    lam = (~(7ul<<29));                     //只接收扩展帧，不掩码
    //开启3个接收邮箱，邮箱定义如下：
    //MSG 0-15 :发送邮箱
    //MSG 16-31 :接收邮箱
    for(i = 0; i < REC_MSG_NUM-1; i++)
    {
        InitRecMBOX(31-i, msgId.all, lam, 1);   //开启邮箱覆盖保护
    }
    InitRecMBOX(31-i, msgId.all, lam, 0);       //不保护

}

/* 函数：Uint16 eCanSendData(Uint16 mbox_num, Uint16 len, Uint32 msgid, Uint32 *dataPi)
 * 用途：发送数据
 * 参数：mbox_num：邮箱号
 *     len：数据长度
 *     msgid：邮箱ID
 *     dataPi：需要发送数据指针
 * 调用：Uint16 DataTranforcan(Uint32 *dataPi, Uint16 len, Uint16 timeOut)
 * 返回：1：CAN邮箱忙
 *     0：成功发送
 */
Uint16 eCanSendData(Uint16 mbox_num, Uint16 len, Uint32 msgid, Uint32 *dataPi)
{
    volatile struct ECAN_REGS ECanShadow;
    volatile struct MBOX *ECanaMBOX;
    Uint32 temp;
    mbox_num &= 0x1f;               /* 将 mbox_num 限制在 0-31 的范围内 */
    temp = 1ul << mbox_num;
    ECanaMBOX = &ECanaMboxes.MBOX0;
    // 检查上次发送是否完成，发送请求标志置位
    if (ECanaRegs.CANTRS.all & temp)
    {
        return 1;   // CAN邮箱忙
    }

    ECanaRegs.CANTA.all = temp;  // 清空发送响应标志

    ECanaMBOX += mbox_num;
    msgid &= ~(0x7ul<<29);                         // 清除高三位
    msgid |= ECanaMBOX->MSGID.all & (0x7ul << 29); // 不修改ID原始配置位
    // 禁止对应邮箱
    ECanShadow.CANME.all = ECanaRegs.CANME.all;
    ECanShadow.CANME.all &= ~temp;
    ECanaRegs.CANME.all = ECanShadow.CANME.all;
    // 邮箱信息配置
    ECanaMBOX->MSGID.all = msgid;
    ECanaMBOX->MSGCTRL.bit.DLC = len;
    ECanaMBOX->MDL.all = *dataPi++;
    ECanaMBOX->MDH.all = *dataPi;
    // 使能对应邮箱
    ECanShadow.CANME.all = ECanaRegs.CANME.all;
    ECanShadow.CANME.all |= temp;
    ECanaRegs.CANME.all = ECanShadow.CANME.all;
    // 使能发送
    ECanaRegs.CANTRS.all = temp;
    return (0);
}

/* 函数：Uint16 eCanRecData(Uint16 mbox_num, Uint32 *dataPi)
 * 用途：接收数据
 * 参数：mbox_num：邮箱号
 *     dataPi：需要发送数据指针
 * 调用：Uint16 DataRecforcan(struct CAN_REC_BUF *dataPi)
 * 返回：0：未接收到数据
 *     1：邮箱有被覆盖
 *     2：接收成功
 */
Uint16 eCanRecData(Uint16 mbox_num, Uint32 *dataPi)
{
    volatile struct MBOX *ECanaMBOX;
    Uint32 temp;
    temp = 1ul << mbox_num;
    mbox_num&= 0x1f;
    ECanaMBOX = &ECanaMboxes.MBOX0;
    ECanaMBOX += mbox_num;
    if (ECanaRegs.CANRMP.all & temp)                // 检查是否有接收消息挂起
    {
        *dataPi++ = ECanaMBOX->MSGID.all;           // 读ID，读数据
        *dataPi++ = ECanaMBOX->MDL.all;
        *dataPi++ = ECanaMBOX->MDH.all;
        *dataPi   = ECanaMBOX->MSGCTRL.bit.DLC;     // 读取接收数据长度

        if (ECanaRegs.CANRML.all & temp)            // 检查邮箱是否被覆盖过
        {
            ECanaRegs.CANRMP.all = temp;            // 清除消息挂起寄存器
            return (1);                             // 邮箱被覆盖，返回数据溢出
        }
        else
        {
            ECanaRegs.CANRMP.all = temp;             // 清除消息挂起寄存器
            return (2);                              // 接收成功
        }
    }
    else
    {
        return (0);                                  // 邮箱无数据
    }
}

/* 函数：Uint16 DataTranforcan(Uint32 *dataPi, Uint16 len, Uint16 timeOut)
 * 用途：发送数据
 * 参数：dataPi：需要发送数据指针
 *     len:需要发送的长度
 *     timeOut：超时
 * 返回：0：发送成功
 *     1：发送超时
 *     2：发送邮箱忙
 * 用法：
{
    //1.定义发送数据
    struct CAN_DATA_BUF senddatabuf;
    Uint16 Status;
    //2.配置ID、DLC、DATA等（略）
    ...
    //3.发送数据，读取返回值
    Status = DataTranforcan((Uint32*)(&senddatabuf), 4, 1000);
    //4.根据返回状态Status作出处理事件
}
*/
Uint16 DataTranforcan(Uint32 *dataPi, Uint16 len, Uint16 timeOut)
{
    Uint16 rec, i;
    Uint32 msgid;
    static Uint16 timecount = 0;

    msgid = *dataPi++;
    for (i=0; i<3; i++)
    {
        rec = eCanSendData(15-i, len, msgid, dataPi);
        if (0 == rec)
        {
            timecount = 0;
            return (0);  // 发送成功
        }
    }
    if (++timecount >= timeOut)
    {
        timecount = 0;
        return (1);      // 发送超时
    }
    else
    {
        return (2);     // 发送邮箱忙
    }
}

/* 函数：Uint16 DataRecforcan(struct CAN_REC_BUF *dataPi)
 * 用途：发送数据
 * 参数：dataPi：需要发送数据指针
 * 返回：0：有数据接收
 *     3：接收邮箱无数据
 *     4：有数据被覆盖
 * 用法：
{
    //1.定义接收缓冲区
    struct CAN_REC_BUF RecDataBuf;
    Uint16 rec,i;
    //2.接收数据
    rec = DataRecforcan(&RecDataBuf);
    //3.根据返回值处理数据
    if ((0 == rec) || (4 == rec) )
    {
        for (i = 0; i < 3; i++)
        {
            if ( (RecDataBuf.bufFull & (1 << i) ) == (1 << i))
            {
                //处理函数自行定义
                //处理函数的传入参数为：(struct CAN_REC_BUF*)(&RecDataBuf.buf[i]);
            }
        }
    }
}
 */
Uint16 DataRecforcan(struct CAN_REC_BUF *dataPi)
{
    Uint16 i, rec;

    dataPi->bufFull = 0;
    for (i=0; i<3; i++)
    {
        rec = eCanRecData(31-i, (Uint32 *)(&(dataPi->buf[i])) );
        if (0 != rec)
            dataPi->bufFull |= 1<<i;                        // 接收缓存有效
    }
    if ( 0 == dataPi->bufFull )                             // 未收数据
        return (3);                                         // 接收邮箱空，返回
    if (1 == rec)
        return (4);
    else
        return (0);
}

