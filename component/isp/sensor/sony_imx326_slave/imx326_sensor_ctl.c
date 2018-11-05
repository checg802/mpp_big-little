/******************************************************************************

  Copyright (C), 2001-2013, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : imx326_sensor_ctl.c
  Version       : Initial Draft
  Author        : Hisilicon BVT ISP group
  Created       : 2017/04/05
  Description   : Sony IMX326 sensor driver
  History       :
  1.Date        : 2017/04/05
  Author        : 
  Modification  : Created file

******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include "hi_comm_video.h"
#include "hi_sns_ctrl.h"
#include "mpi_sys.h"

#include "hi_spi.h"
#include "imx326_slave_priv.h"
#include "mpi_isp.h"

extern const IMX326_VIDEO_MODE_TBL_S g_astImx326ModeTbl[];
extern ISP_SLAVE_SNS_SYNC_S gstImx326Sync[];

static int g_fd[ISP_MAX_DEV_NUM] = {-1,-1};
extern ISP_SNS_STATE_S          g_astImx326[ISP_MAX_DEV_NUM];
extern ISP_SNS_COMMBUS_U        g_aunImx326BusInfo[];


void imx326_linear_3072x1728_5M30fps_init(ISP_DEV IspDev);
void imx326_linear_2592x1944_5M29fps_init(ISP_DEV IspDev);

int imx326_spi_init(ISP_DEV IspDev)
{
    char acDevFile[16] = {0};
    HI_U8 u8DevNum, u8Cs;

    if(g_fd[IspDev] >= 0)
    {
        return 0;
    }
    unsigned int value;
    int ret = 0;

	u8DevNum = g_aunImx326BusInfo[IspDev].s8SspDev.bit4SspDev;
    u8Cs = g_aunImx326BusInfo[IspDev].s8SspDev.bit4SspCs;
    snprintf_s(acDevFile, sizeof(acDevFile), sizeof(acDevFile)-1, "/dev/spidev%d.%d", u8DevNum, u8Cs);

    g_fd[IspDev] = open(acDevFile, 0);
    if (g_fd[IspDev] < 0)
    {
        printf("Open %s error!\n",acDevFile);
        return -1;
    }

    value = SPI_MODE_3 | SPI_LSB_FIRST;
    ret = ioctl(g_fd[IspDev], SPI_IOC_WR_MODE, &value);
    if (ret < 0)
    {
        printf("ioctl SPI_IOC_WR_MODE err, value = %d ret = %d\n", value, ret);
        return ret;
    }

    value = 8;
    ret = ioctl(g_fd[IspDev], SPI_IOC_WR_BITS_PER_WORD, &value);
    if (ret < 0)
    {
        printf("ioctl SPI_IOC_WR_BITS_PER_WORD err, value = %d ret = %d\n",value, ret);
        return ret;
    }

    value = 2000000;
    ret = ioctl(g_fd[IspDev], SPI_IOC_WR_MAX_SPEED_HZ, &value);
    if (ret < 0)
    {
        printf("ioctl SPI_IOC_WR_MAX_SPEED_HZ err, value = %d ret = %d\n",value, ret);
        return ret;
    }

    return 0;
}

int imx326_spi_exit(ISP_DEV IspDev)
{
    if (g_fd[IspDev] >= 0)
    {
        close(g_fd[IspDev]);
        g_fd[IspDev] = -1;
        return 0;
    }
    return -1;
}

int imx326_write_register(ISP_DEV IspDev,unsigned int addr, unsigned char data)
{
    int ret;
    struct spi_ioc_transfer mesg[1];
    unsigned char  tx_buf[8] = {0};
    unsigned char  rx_buf[8] = {0};

    if (0 > g_fd[IspDev])
    {
        return 0;
    }

    tx_buf[0] = 0x81;
    tx_buf[1] = addr >> 8;
    tx_buf[2] = addr & 0xff;
    tx_buf[3] = data;

    memset(mesg, 0, sizeof(mesg));
    mesg[0].tx_buf = (__u32)tx_buf;
    mesg[0].len = 4;
    mesg[0].rx_buf = (__u32)rx_buf;
    mesg[0].cs_change = 1;

    ret = ioctl(g_fd[IspDev], SPI_IOC_MESSAGE(1), mesg);
    if (ret < 0) {
        printf("SPI_IOC_MESSAGE error \n");
        return -1;
    }

    return 0;
}

int imx326_read_register(ISP_DEV IspDev,unsigned int addr)
{
    int ret = 0;
    struct spi_ioc_transfer mesg[1];
    unsigned char  tx_buf[8] = {0};
    unsigned char  rx_buf[8] = {0};

    tx_buf[0] = 0x80;
    tx_buf[1] = addr >> 8;
    tx_buf[2] = addr & 0xff;
    tx_buf[3] = 0;

    memset(mesg, 0, sizeof(mesg));
    mesg[0].tx_buf = (__u32)tx_buf;
    mesg[0].len = 4;
    mesg[0].rx_buf = (__u32)rx_buf;
    mesg[0].cs_change = 1;

    ret = ioctl(g_fd[IspDev], SPI_IOC_MESSAGE(1), mesg);
    if (ret  < 0) {
        printf("SPI_IOC_MESSAGE error \n");
        return -1;
    }

    return rx_buf[3];
}

static void delay_ms(int ms) {
    hi_usleep(ms*1000);
}

void imx326_standby(ISP_DEV IspDev)
{
    // TODO:
    return;
}

void imx326_restart(ISP_DEV IspDev)
{
    // TODO:
    return;
}

void imx326_init(ISP_DEV IspDev)
{
    HI_U8   u8ImgMode;
    HI_U32  i;

    u8ImgMode   = g_astImx326[IspDev].u8ImgMode;

    /* hold sync signal as fixed */
    CHECK_RET(HI_MPI_ISP_GetHvSync(IspDev, &gstImx326Sync[IspDev]));
    gstImx326Sync[IspDev].unCfg.stBits.bitHEnable = 0;
    gstImx326Sync[IspDev].unCfg.stBits.bitVEnable = 0;
    CHECK_RET(HI_MPI_ISP_SetHvSync(IspDev, &gstImx326Sync[IspDev]));

    /* 2. imx326 spi init */
    imx326_spi_init(IspDev);

    /* When sensor first init, config all registers */
    if(IMX326_MODE0 == u8ImgMode)
    {
        imx326_linear_3072x1728_5M30fps_init(IspDev);
    }
    else if(IMX326_MODE1 == u8ImgMode)
    {
        imx326_linear_2592x1944_5M29fps_init(IspDev);
    }
    else
    {
        printf("Not Support this Mode\n");
    }

    // release hv sync
    gstImx326Sync[IspDev].u32HsTime = g_astImx326ModeTbl[u8ImgMode].u32InckPerHs;
	if(g_astImx326[IspDev].astRegsInfo[0].stSlvSync.u32SlaveVsTime == 0)
	{
		gstImx326Sync[IspDev].u32VsTime = g_astImx326ModeTbl[u8ImgMode].u32InckPerVs;
	}
	else
	{
		gstImx326Sync[IspDev].u32VsTime = g_astImx326[IspDev].astRegsInfo[0].stSlvSync.u32SlaveVsTime;
	}
    gstImx326Sync[IspDev].unCfg.u32Bytes = 0xc0030000;
    gstImx326Sync[IspDev].u32HsCyc = 0x3;
    gstImx326Sync[IspDev].u32VsCyc = 0x3;
    CHECK_RET(HI_MPI_ISP_SetHvSync(IspDev, &gstImx326Sync[IspDev]));

	for (i=0; i<g_astImx326[IspDev].astRegsInfo[0].u32RegNum; i++)
	{
		imx326_write_register(IspDev, g_astImx326[IspDev].astRegsInfo[0].astSspData[i].u32RegAddr, g_astImx326[IspDev].astRegsInfo[0].astSspData[i].u32Data);
	}

    printf("IMX326 %s init succuss!\n", g_astImx326ModeTbl[u8ImgMode].pszModeName);
	g_astImx326[IspDev].bInit = HI_TRUE;

    return ;
}

void imx326_exit(ISP_DEV IspDev)
{
    imx326_spi_exit(IspDev);
    return;
}

void imx326_linear_3072x1728_5M30fps_init(ISP_DEV IspDev)
{
    imx326_write_register(IspDev, 0x0000, 0x1A);
    imx326_write_register(IspDev, 0x003E, 0x00);
    imx326_write_register(IspDev, 0x0120, 0xC0);
    imx326_write_register(IspDev, 0x0121, 0x00);
    imx326_write_register(IspDev, 0x0122, 0x02);
    imx326_write_register(IspDev, 0x0129, 0x9C);
    imx326_write_register(IspDev, 0x012A, 0x02);
    imx326_write_register(IspDev, 0x012D, 0x02);
    imx326_write_register(IspDev, 0x010B, 0x00);
    imx326_write_register(IspDev, 0x00EE, 0x01);
    imx326_write_register(IspDev, 0x032C, 0x6E);
    imx326_write_register(IspDev, 0x032D, 0x0F);
    imx326_write_register(IspDev, 0x034A, 0x6E);
    imx326_write_register(IspDev, 0x034B, 0x0F);
    imx326_write_register(IspDev, 0x05B6, 0x6E);
    imx326_write_register(IspDev, 0x05B7, 0x0F);
    imx326_write_register(IspDev, 0x05B8, 0x6E);
    imx326_write_register(IspDev, 0x05B9, 0x0F);
    imx326_write_register(IspDev, 0x004C, 0x00);
    imx326_write_register(IspDev, 0x004D, 0x03);
    imx326_write_register(IspDev, 0x031C, 0x1A);
    imx326_write_register(IspDev, 0x031D, 0x00);
    imx326_write_register(IspDev, 0x0502, 0x02);
    imx326_write_register(IspDev, 0x0529, 0x0E);
    imx326_write_register(IspDev, 0x052A, 0x0E);
    imx326_write_register(IspDev, 0x052B, 0x0E);
    imx326_write_register(IspDev, 0x0538, 0x0E);
    imx326_write_register(IspDev, 0x0539, 0x0E);
    imx326_write_register(IspDev, 0x0553, 0x00);
    imx326_write_register(IspDev, 0x057D, 0x05);
    imx326_write_register(IspDev, 0x057F, 0x05);
    imx326_write_register(IspDev, 0x0581, 0x04);
    imx326_write_register(IspDev, 0x0583, 0x76);
    imx326_write_register(IspDev, 0x0587, 0x01);
    imx326_write_register(IspDev, 0x05BB, 0x0E);
    imx326_write_register(IspDev, 0x05BC, 0x0E);
    imx326_write_register(IspDev, 0x05BD, 0x0E);
    imx326_write_register(IspDev, 0x05BE, 0x0E);
    imx326_write_register(IspDev, 0x05BF, 0x0E);
    imx326_write_register(IspDev, 0x066E, 0x00);
    imx326_write_register(IspDev, 0x066F, 0x00);
    imx326_write_register(IspDev, 0x0670, 0x00);
    imx326_write_register(IspDev, 0x0671, 0x00);
    delay_ms(10);
    imx326_write_register(IspDev, 0x00E6, 0x01);
    imx326_write_register(IspDev, 0x00E8, 0x01);
    imx326_write_register(IspDev, 0x0000, 0x18);
    delay_ms(7);
    imx326_write_register(IspDev, 0x0001, 0x10);
    imx326_write_register(IspDev, 0x0009, 0x01);
    imx326_write_register(IspDev, 0x0003, 0x22);
    imx326_write_register(IspDev, 0x0004, 0x00);
    imx326_write_register(IspDev, 0x0005, 0x07);
    imx326_write_register(IspDev, 0x0006, 0x00);
    imx326_write_register(IspDev, 0x0007, 0xA2);
    imx326_write_register(IspDev, 0x001A, 0x00);
    imx326_write_register(IspDev, 0x0037, 0x01);
    imx326_write_register(IspDev, 0x0038, 0x80);
    imx326_write_register(IspDev, 0x0039, 0x01);
    imx326_write_register(IspDev, 0x003A, 0x98);
    imx326_write_register(IspDev, 0x003B, 0x0D);
    imx326_write_register(IspDev, 0x006B, 0x05);
    imx326_write_register(IspDev, 0x00DD, 0x01);
    imx326_write_register(IspDev, 0x00DE, 0xD8);
    imx326_write_register(IspDev, 0x00DF, 0x00);
    imx326_write_register(IspDev, 0x00E0, 0xD8);
    imx326_write_register(IspDev, 0x00E1, 0x00);
    imx326_write_register(IspDev, 0x00E2, 0x00);
    imx326_write_register(IspDev, 0x0342, 0xFF);
    imx326_write_register(IspDev, 0x0343, 0x01);
    imx326_write_register(IspDev, 0x0344, 0xFF);
    imx326_write_register(IspDev, 0x0345, 0x01);
    imx326_write_register(IspDev, 0x03A6, 0x00);
    imx326_write_register(IspDev, 0x0528, 0x0F);
    imx326_write_register(IspDev, 0x0554, 0x00);
    imx326_write_register(IspDev, 0x0555, 0x00);
    imx326_write_register(IspDev, 0x0556, 0x00);
    imx326_write_register(IspDev, 0x0557, 0x00);
    imx326_write_register(IspDev, 0x0558, 0x00);
    imx326_write_register(IspDev, 0x0559, 0x1F);
    imx326_write_register(IspDev, 0x055A, 0x1F);
    imx326_write_register(IspDev, 0x05BA, 0x0F);
    imx326_write_register(IspDev, 0x066A, 0x00);
    imx326_write_register(IspDev, 0x066B, 0x00);
    imx326_write_register(IspDev, 0x066C, 0x00);
    imx326_write_register(IspDev, 0x066D, 0x00);
    imx326_write_register(IspDev, 0x000E, 0x00);
    imx326_write_register(IspDev, 0x000F, 0x00);
    imx326_write_register(IspDev, 0x000C, 0x0C);
    imx326_write_register(IspDev, 0x000D, 0x00);
    imx326_write_register(IspDev, 0x0019, 0x10);
    imx326_write_register(IspDev, 0x002E, 0x04);
    imx326_write_register(IspDev, 0x002F, 0x00);
    imx326_write_register(IspDev, 0x0030, 0x43);
    imx326_write_register(IspDev, 0x0031, 0x05);
    imx326_write_register(IspDev, 0x0032, 0x08);
    imx326_write_register(IspDev, 0x0033, 0x00);
    imx326_write_register(IspDev, 0x0041, 0x30);
    imx326_write_register(IspDev, 0x0042, 0x08);
    imx326_write_register(IspDev, 0x0043, 0x01);
    imx326_write_register(IspDev, 0x00E9, 0x00);
    printf("===imx326 linear 3072x1728 5M30fps Init OK!===\n");
    return;
}

void imx326_linear_2592x1944_5M29fps_init(ISP_DEV IspDev)
{
	imx326_write_register(IspDev, 0x0000, 0x1A);
	imx326_write_register(IspDev, 0x003E, 0x00);
	imx326_write_register(IspDev, 0x0120, 0xC0);
	imx326_write_register(IspDev, 0x0121, 0x00);
	imx326_write_register(IspDev, 0x0122, 0x02);
	imx326_write_register(IspDev, 0x0129, 0x9C);
	imx326_write_register(IspDev, 0x012A, 0x02);
	imx326_write_register(IspDev, 0x012D, 0x02);
	imx326_write_register(IspDev, 0x010B, 0x00);
	imx326_write_register(IspDev, 0x00EE, 0x01);
	imx326_write_register(IspDev, 0x032C, 0xAE);
	imx326_write_register(IspDev, 0x032D, 0x11);
	imx326_write_register(IspDev, 0x034A, 0xAE);
	imx326_write_register(IspDev, 0x034B, 0x11);
	imx326_write_register(IspDev, 0x05B6, 0xAE);
	imx326_write_register(IspDev, 0x05B7, 0x11);
	imx326_write_register(IspDev, 0x05B8, 0xAE);
	imx326_write_register(IspDev, 0x05B9, 0x11);
	imx326_write_register(IspDev, 0x004C, 0x00);
	imx326_write_register(IspDev, 0x004D, 0x03);
	imx326_write_register(IspDev, 0x031C, 0x1A);
	imx326_write_register(IspDev, 0x031D, 0x00);
	imx326_write_register(IspDev, 0x0502, 0x02);
	imx326_write_register(IspDev, 0x0529, 0x0E);
	imx326_write_register(IspDev, 0x052A, 0x0E);
	imx326_write_register(IspDev, 0x052B, 0x0E);
	imx326_write_register(IspDev, 0x0538, 0x0E);
	imx326_write_register(IspDev, 0x0539, 0x0E);
	imx326_write_register(IspDev, 0x0553, 0x00);
	imx326_write_register(IspDev, 0x057D, 0x05);
	imx326_write_register(IspDev, 0x057F, 0x05);
	imx326_write_register(IspDev, 0x0581, 0x04);
	imx326_write_register(IspDev, 0x0583, 0x76);
	imx326_write_register(IspDev, 0x0587, 0x01);
	imx326_write_register(IspDev, 0x05BB, 0x0E);
	imx326_write_register(IspDev, 0x05BC, 0x0E);
	imx326_write_register(IspDev, 0x05BD, 0x0E);
	imx326_write_register(IspDev, 0x05BE, 0x0E);
	imx326_write_register(IspDev, 0x05BF, 0x0E);
	imx326_write_register(IspDev, 0x066E, 0x00);
	imx326_write_register(IspDev, 0x066F, 0x00);
	imx326_write_register(IspDev, 0x0670, 0x00);
	imx326_write_register(IspDev, 0x0671, 0x00);
	delay_ms(10);
	imx326_write_register(IspDev, 0x00E6, 0x01);
	imx326_write_register(IspDev, 0x00E8, 0x01);
	imx326_write_register(IspDev, 0x0000, 0x18);
	delay_ms(7);
	imx326_write_register(IspDev, 0x0001, 0x10);
	imx326_write_register(IspDev, 0x0009, 0x01);
	imx326_write_register(IspDev, 0x0003, 0x22);
	imx326_write_register(IspDev, 0x0004, 0x00);
	imx326_write_register(IspDev, 0x0005, 0x07);
	imx326_write_register(IspDev, 0x0006, 0x00);
	imx326_write_register(IspDev, 0x0007, 0xA2);
	imx326_write_register(IspDev, 0x001A, 0x00);
	imx326_write_register(IspDev, 0x0037, 0x01);
	imx326_write_register(IspDev, 0x0038, 0x70);
	imx326_write_register(IspDev, 0x0039, 0x02);
	imx326_write_register(IspDev, 0x003A, 0xA8);
	imx326_write_register(IspDev, 0x003B, 0x0C);
	imx326_write_register(IspDev, 0x006B, 0x07);
	imx326_write_register(IspDev, 0x00DD, 0x01);
	imx326_write_register(IspDev, 0x00DE, 0x6C);
	imx326_write_register(IspDev, 0x00DF, 0x00);
	imx326_write_register(IspDev, 0x00E0, 0x36);
	imx326_write_register(IspDev, 0x00E1, 0x00);
	imx326_write_register(IspDev, 0x00E2, 0x00);
	imx326_write_register(IspDev, 0x0342, 0xFF);
	imx326_write_register(IspDev, 0x0343, 0x01);
	imx326_write_register(IspDev, 0x0344, 0xFF);
	imx326_write_register(IspDev, 0x0345, 0x01);
	imx326_write_register(IspDev, 0x03A6, 0x01);
	imx326_write_register(IspDev, 0x0528, 0x0F);
	imx326_write_register(IspDev, 0x0554, 0x00);
	imx326_write_register(IspDev, 0x0555, 0x00);
	imx326_write_register(IspDev, 0x0556, 0x00);
	imx326_write_register(IspDev, 0x0557, 0x00);
	imx326_write_register(IspDev, 0x0558, 0x00);
	imx326_write_register(IspDev, 0x0559, 0x1F);
	imx326_write_register(IspDev, 0x055A, 0x1F);
	imx326_write_register(IspDev, 0x05BA, 0x0F);
	imx326_write_register(IspDev, 0x066A, 0x00);
	imx326_write_register(IspDev, 0x066B, 0x00);
	imx326_write_register(IspDev, 0x066C, 0x00);
	imx326_write_register(IspDev, 0x066D, 0x00);
	imx326_write_register(IspDev, 0x000E, 0x00);
	imx326_write_register(IspDev, 0x000F, 0x00);
	imx326_write_register(IspDev, 0x000C, 0x0C);
	imx326_write_register(IspDev, 0x000D, 0x00);
	imx326_write_register(IspDev, 0x0019, 0x10);
	imx326_write_register(IspDev, 0x002E, 0x04);
	imx326_write_register(IspDev, 0x002F, 0x00);
	imx326_write_register(IspDev, 0x0030, 0x43);
	imx326_write_register(IspDev, 0x0031, 0x05);
	imx326_write_register(IspDev, 0x0032, 0x08);
	imx326_write_register(IspDev, 0x0033, 0x00);
	imx326_write_register(IspDev, 0x0041, 0x30);
	imx326_write_register(IspDev, 0x0042, 0x08);
	imx326_write_register(IspDev, 0x0043, 0x01);
	imx326_write_register(IspDev, 0x00E9, 0x00);
	printf("===imx326 linear 2592x1944 5M29.97fps Init OK!===\n");
	return;
}

