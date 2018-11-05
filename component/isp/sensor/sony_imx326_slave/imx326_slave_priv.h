/******************************************************************************

  Copyright (C), 2001-2015, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : IMX326_slave_priv.h
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2017/04/6
  Description   : this file was private for IMX326 slave mode sensor
  History       :
  1.Date        : 
    Author      : 
    Modification: Created file
******************************************************************************/
#ifndef __IMX326_SLAVE_PRIV_H_
#define __IMX326_SLAVE_PRIV_H_

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

// usefull macro
#define HIGH_8BITS(x) ((x & 0xff00) >> 8)
#define LOW_8BITS(x)  (x & 0x00ff)
#ifndef MAX
#define MAX(a, b) (((a) < (b)) ?  (b) : (a))
#endif

#ifndef MIN
#define MIN(a, b) (((a) > (b)) ?  (b) : (a))
#endif

#define IMX326_RES_IS_3072x1728_5M30fps(w, h)       (w <= 3072 && h <= 1728)
#define IMX326_RES_IS_2592x1944_5M30fps(w, h)       (w <= 2592 && h <= 1944)

#define CHECK_RET(express)\
do{\
    HI_S32 s32Ret;\
    s32Ret = express;\
    if (HI_SUCCESS != s32Ret)\
    {\
        printf("Failed at %s: LINE: %d with %#x!\n", __FUNCTION__, __LINE__, s32Ret);\
    }\
}while(0)

/*
--------------------------------------------------------------------------------
- Structure For Slave Mode Sensor Using
--------------------------------------------------------------------------------
*/
#define     FULL_LINES_MAX              (0xFFFF)
// registers to control exposure
#define     IMX326_SHR_L                (0x000C)
#define     IMX326_SHR_H                (0x000D)
#define     IMX326_SVR_L                (0x000E)
#define     IMX326_SVR_H                (0x000F)
#define     IMX326_PGC_L                (0x000A)
#define     IMX326_PGC_H                (0x000B)
#define     IMX326_DGAIN                (0x0012)

#define     NA                          (0xFFFF)

typedef struct hiIMX326_SENSOR_REG_S{
    HI_U16 u16Addr;
    HI_U8  u8Data;
} IMX326_SENSOR_REG_S;

typedef enum {
    IMX326_MODE0 = 0,
    IMX326_MODE1,
    IMX326_MODE_BUTT
} IMX326_RES_MODE_E;

typedef struct hiIMX326_VIDEO_MODE_TBL_S
{
    HI_U32  u32Inck;
    HI_U32  u32InckPerHs;
    HI_U32  u32InckPerVs;
    HI_U32  u32VertiLines;
    HI_FLOAT  f32MaxFps;
    const char *pszModeName;
} IMX326_VIDEO_MODE_TBL_S;
#endif /* __IMX117_SLAVE_PRIV_H_ */
