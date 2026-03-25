/*
 * can.h
 *
 *  Created on: 2024年1月24日
 *      Author: Free丶Chan
 *     Version: V1.0
 */

#ifndef _CAN_H_
#define _CAN_H_


/* INCLUDE BEGIN */
#include "DSP2833x_Device.h"     // DSP2833x Headerfile Include File
#include "DSP2833x_Examples.h"   // DSP2833x Headerfile Include File
/* INCLUDE END */

// CAN消息ID位定义
struct  CAN_MSGID_BITS
{
    Uint32  msgID:29;                                   // ID       0-28
    Uint16  aam:1;                                      // 自动应答位    29
    Uint16  ame:1;                                      // 屏蔽使能位    30
    Uint16  ide:1;                                      // 扩展帧      31
};

// CAN 数据结构
union CAN_MSGID_STRU
{
    Uint32  all;
    struct  CAN_MSGID_BITS bit;
};

//32-bit high low 16 bit data
struct CAN_DATA_HL
{
    Uint16 data_h;
    Uint16 data_l;
};

// 32-bit data structure
union CAN_DATA_STRU
{
    Uint32 all;
    struct CAN_DATA_HL data;
};

// Email data structure
struct CAN_DATA_BUF
{
    union CAN_MSGID_STRU msgid;
    union CAN_DATA_STRU mdl;
    union CAN_DATA_STRU mdh;
    Uint32 len;
};

// Receive cache data structure
struct CAN_REC_BUF
{
    Uint16 bufFull;
    struct CAN_DATA_BUF buf[3];
};

extern struct CAN_DATA_BUF senddatabuf;
extern struct CAN_REC_BUF RecDataBuf;

#define INIT_ID       0x00000000   //任意设
#define ID_MASK       0x1FFFFFFF   //bit0-bit29 MAX:0x1FFFFFFF

//邮箱个数定义
#define REC_MSG_NUM   3            //MAX:16
#define TRAN_MSG_NUM  3            //MAX:16

/* 函数：void cana_init(void)
 * 用途：CANA初始化、初始化发送和接收邮箱
 */
void cana_init(void);

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
Uint16 DataTranforcan(Uint32 *dataPi, Uint16 len, Uint16 timeOut);

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
Uint16 DataRecforcan(struct CAN_REC_BUF *dataPi);

#endif /*_CAN_H_ */
