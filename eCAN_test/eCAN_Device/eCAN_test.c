#include "DSP2833x_Device.h"
#include "DSP2833x_Examples.h"
#include <stdio.h>


#include "eCAN_test.h"
#include "eCAN_Device.h"


static volatile ptCanDevice pdev;

int CanTest(void)
{
    int ret;
    tCanConfig eCANaConfigData;
    tMBOXConfig MBOX0ConfigData;
    tMBOXConfig MBOX8ConfigData;

    /* register dev */
    ResgisterCanDevice();

    /* get dev */
    pdev = GetCanDevice("canA");
    if (pdev == NULL)
    {
        printf("get canA dev err\r\n");
        return -1;
    }

    /* can dev init */
    eCANaConfigData.Mode = 0;
    eCANaConfigData.SAM = 0;
    eCANaConfigData.STM = 0;
    eCANaConfigData.BRP = 4;
    eCANaConfigData.TS2 = 2;
    eCANaConfigData.TS1 = 10;
    eCANaConfigData.TI_option = 1;
    pdev->init(pdev, &eCANaConfigData);   /* 别忘了取地址 */

    /* transmit MBOX init */
    MBOX0ConfigData.AAM = 1;
    MBOX0ConfigData.DLC = 8;
    MBOX0ConfigData.IDE = 0;
    MBOX0ConfigData.MBOX_Num = 0;
    MBOX0ConfigData.MSG_Id = 0x4FF80000;
    MBOX0ConfigData.RTR = 0;
    MBOX0ConfigData.TPL = 0;
    MBOX0ConfigData.MSG_L = 0x11112222;
    MBOX0ConfigData.MSG_H = 0x33334444;
    pdev->initCanTran(pdev, &MBOX0ConfigData);

    /* receive MBOX init */
    MBOX8ConfigData.AME = 1;
    MBOX8ConfigData.AMI = 0;
    MBOX8ConfigData.DLC = 8;
    MBOX8ConfigData.IDE = 0;
    MBOX8ConfigData.MBOX_Num = 8;
    MBOX8ConfigData.MSG_Id = 0x5FF80000;
    MBOX8ConfigData.Mask = 0xFFFFFFFF;
    MBOX8ConfigData.OPC = 0;
    MBOX8ConfigData.RTR = 0;
    pdev->initCanRecv(pdev, &eCANaConfigData, &MBOX8ConfigData);

    /* IT init */
    pdev->initCanIT(pdev, &eCANaConfigData, &MBOX8ConfigData);

    while(1)
    {
//        ECanaRegs.CANTRS.all = 0x00000001;  // 发送函数，将发送邮箱中的数据发送出去
//        while(ECanaRegs.CANTA.all != 0x00000001 ) {}  // Wait for all TAn bits to be set..，等待发送成功
//        ECanaRegs.CANTA.all = 0x00000001;   // 清除标志位
//        DELAY_US(10);

//        if(ECanaRegs.CANRMP.all == 0x00000100)
//        {
//            Uint32  TestMbox1 = 0;
//            Uint32  TestMbox2 = 0;
//            Uint32  TestMbox3 = 0;
//            volatile struct MBOX *Mailbox;
//            Mailbox = &ECanaMboxes.MBOX8;
//            TestMbox1 = Mailbox->MDL.all;
//            TestMbox2 = Mailbox->MDH.all;
//            TestMbox3 = Mailbox->MSGID.all;
//
//            printf("TestMbox2_H = %x\r\n",TestMbox2);           //TestMbox2为32位的数值，打印时使用%x，只打印出了高16位，即0x9555
//            printf("TestMbox2_L = %x\r\n",(TestMbox2 << 16));   //0xAAA0
//            ECanaRegs.CANRMP.all = 0x00000100;
//        }
    }
}

