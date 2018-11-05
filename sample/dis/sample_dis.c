/******************************************************************************
  A simple program of Hisilicon Hi35xx video input and output implementation.
  Copyright (C), 2010-2011, Hisilicon Tech. Co., Ltd.
 ******************************************************************************
    Modification:  2011-8 Created
******************************************************************************/

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#ifndef __USE_GNU
#define __USE_GNU
#endif
#define _GNU_SOURCE

#include <sched.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "sample_comm.h"
#include "gyro_dev.h"

static pthread_t gs_DISPid;
VIDEO_NORM_E gs_enNorm = VIDEO_ENCODING_MODE_NTSC;
VO_INTF_TYPE_E g_enVoIntfType = VO_INTF_CVBS;
PIC_SIZE_E g_enPicSize = PIC_HD1080;
VI_DIS_MOTION_TYPE_E g_enMotionType = VI_DIS_MOTION_6DOF_SOFT;


HI_U32 gs_u32ViFrmRate = 0;

SAMPLE_VI_CONFIG_S g_stViChnConfig =
{
    .enViMode = PANASONIC_MN34220_SUBLVDS_1080P_30FPS,
    .enNorm   = VIDEO_ENCODING_MODE_AUTO,

    .enRotate = ROTATE_NONE,
    .enViChnSet = VI_CHN_SET_NORMAL,
    .enWDRMode  = WDR_MODE_NONE,
    .enFrmRate  = SAMPLE_FRAMERATE_DEFAULT,
    .enSnsNum = SAMPLE_SENSOR_SINGLE,

};

typedef struct st_sample_ctrl
{
    HI_CHAR *gyro_dev;
    HI_BOOL  bgyro_required;
    HI_S32  sensor_type;
}stSampleCtrl_S;

/*********************************************************************/

//ISP_EXCONFIG_S g_stISPExConfig = {0};


#define VI_TRACE(level, fmt...)\
    do{ \
        HI_TRACE(level, HI_ID_VIU, "[Func]:%s [Line]:%d [Info]:", __FUNCTION__, __LINE__);\
        HI_TRACE(level, HI_ID_VIU, ##fmt);\
    }while(0)

#define VIU_CHECK_NULL_PTR(ptr)\
    do{\
        if(NULL == ptr)\
        {\
            VI_TRACE(HI_DBG_ERR,"NULL point \r\n");\
            return HI_ERR_VI_INVALID_NULL_PTR;\
        }\
    }while(0)


typedef struct hiVI_DIS_GYROINFO_S
{
    GYRO_DATA_S stDISGyroData[2];
    HI_U64   u64BeginPts;
    HI_U64   u64EndPts;
} VI_DIS_GYROINFO_S;

#define MOVINGSUBJECTLEVEL   (2)
#define NOMOVEMENTLEVEL      (0)
#define ROLL_COEF            (80)
#define FIXLEVEL             (4)
#define X_BUF_LEN            (1000)
#define GYRO_BUF_LEN         ((4*3*X_BUF_LEN)+8*X_BUF_LEN)
#define PI                   (3.14159)

#define DEC_TO_RAD  (0.01745333)  //PI/180

#define DEV  HI_S32
#define GRRO_MAX_DEV_NUM   (1)
HI_S32 g_s32GyroDevFd = -1;
GYRO_ATTR_S  g_stGyroAttr;
VI_DIS_GYROINFO_S g_stDISGyroInfo;
VI_DIS_GYRO_DATA_S g_stDISGyroData;
HI_DOUBLE g_GyroSensitivity = 16.4;

stSampleCtrl_S      stSampleCtrl;
VI_DIS_CALLBACK_S stDISCallback;

HI_U32 phyaddr = 0;

int fdphy = -1;

VI_DIS_CONFIG_S stDISConfigHard =
{
    .enAccuracy = VI_DIS_ACCURACY_HIGH,
    .enCameraMode = VI_DIS_CAMERA_MODE_NORMAL,
    .enMotionType = VI_DIS_MOTION_8DOF_HARD,
    .u32FixLevel = 7,
    .u32RollingShutterCoef = 80,
    .u32BufNum = 8,
    .u32CropRatio = 80,
    .bScale = HI_TRUE,
    .u32DelayFrmNum = 0,
    .u32RetCenterStrength = 7,
    .u32GyroWeight = 0
};

VI_DIS_CONFIG_S stDISConfigHybrid =
{
    .enAccuracy = VI_DIS_ACCURACY_HIGH,
    .enCameraMode = VI_DIS_CAMERA_MODE_NORMAL,
    .enMotionType = VI_DIS_MOTION_6DOF_HYBRID,
    .u32FixLevel = 5,
    .u32RollingShutterCoef = 70,
    .u32BufNum = 8,
    .u32CropRatio = 80,
    .bScale = HI_TRUE,
    .u32DelayFrmNum = 1,
    .u32RetCenterStrength = 7,
    .u32GyroWeight = 1
};

VI_DIS_CONFIG_S stDISConfigSoft=
{
    .enAccuracy = VI_DIS_ACCURACY_HIGH,
    .enCameraMode = VI_DIS_CAMERA_MODE_IPC,
    .enMotionType = VI_DIS_MOTION_4DOF_SOFT,
    .u32FixLevel = 5,
    .u32RollingShutterCoef = 60,
    .u32BufNum = 8,
    .u32CropRatio = 95,
    .bScale = HI_TRUE,
    .u32DelayFrmNum = 0,
    .u32RetCenterStrength = 7,
    .u32GyroWeight = 0
};

VI_DIS_ATTR_S stDISAttrHard =
{
    .bEnable = HI_TRUE,
    .u32MovingSubjectLevel = 0,
    .u32NoMovementLevel = 1,    
    .u32TimeLag = 16666,
    .u32Vangle = 410,
    .enAngleType = VI_DIS_ANGLE_TYPE_DIAGONAL,
    .bStillCrop = HI_FALSE
};

VI_DIS_ATTR_S stDISAttrSoft=
{
    .bEnable = HI_TRUE,
    .u32MovingSubjectLevel = 3,
    .u32NoMovementLevel = 0,    
    .u32TimeLag = 0,
    .u32Vangle = 410,
    .enAngleType = VI_DIS_ANGLE_TYPE_DIAGONAL,
    .bStillCrop = HI_FALSE
};

VI_DIS_ATTR_S stDISAttrHybrid=
{
    .bEnable = HI_TRUE,
    .u32MovingSubjectLevel = 5,
    .u32NoMovementLevel = 1,    
    .u32TimeLag = 16666,
    .u32Vangle = 1159,
    .enAngleType = VI_DIS_ANGLE_TYPE_DIAGONAL,
    .bStillCrop = HI_FALSE
};

VI_DIS_CONFIG_S *g_pstDISConfig = &stDISConfigSoft;
VI_DIS_ATTR_S *g_pstDISAttr = &stDISAttrSoft;
HI_U32 u32width = 1920;
HI_U32 u32height = 1080;


void SAMPLE_DIS_Usage(char *sPrgNm)
{
    printf("Usage : %s <index> <intf> <mode>\n", sPrgNm);
    printf("index:\n");
    printf("\t 0)vo preview \n");

    printf("intf:\n");
    printf("\t 0) vo cvbs output, 1)BT1120,default cvbs.\n");
    
    printf("mode:\n");
    printf("\t (0)software (1)hardware, (2)hybrid, default 0\n");

    return;
}

HI_VOID SAMPLE_DIS_Mode_Usage(HI_VOID)
{
    printf("mode:\n");
    printf("\t (0)software ,(1)hardware, (2)hybrid, default 0\n");
}

HI_VOID SAMPLE_DIS_VoIntf_Usage(HI_VOID)
{
    printf("intf:\n");
    printf("\t 0) vo cvbs output, default.\n");
    printf("\t 1) vo BT1120 output.\n");
}

HI_VOID SAMPLE_DIS_Index_Usage(HI_VOID)
{
    printf("index:\n");
    printf("\t 0) vo preview, default 0\n");
}

static void DC_clear_algorithm(   HI_S32    *pDataIn_x,
                                           HI_S32    *pDataOut_x,
                                           HI_S32    *pDataIn_y,
                                           HI_S32    *pDataOut_y,
                                           HI_S32    *pDataIn_z,
                                           HI_S32    *pDataOut_z,
                                           HI_S32      NrSamples)
{
    #define DC_D16_STEP 0.1
    static float GYROX=1,GYROY=1,GYROZ=1;
    HI_S32 Diff;
    HI_S32 j;

   
    for(j=NrSamples-1;j>=0;j--)
    { 
        /* Subtract DC an saturate */
        Diff=*(pDataIn_x++)-(HI_S32)(GYROX + 0.5);
        if (Diff > 32767) {
            Diff = 32767; }
        else if (Diff < -32768) {
            Diff = -32768; }
        *(pDataOut_x++)=(HI_S32)Diff;
        if (Diff < 0) {
            GYROX -= DC_D16_STEP; }
        else {
            GYROX += DC_D16_STEP; }

        /* Subtract DC an saturate */
        Diff=*(pDataIn_y++)-(HI_S32)(GYROY + 0.5);
        if (Diff > 32767) {
            Diff = 32767; }
        else if (Diff < -32768) {
            Diff = -32768; }
        *(pDataOut_y++)=(HI_S32)Diff;
        if (Diff < 0) {
            GYROY -= DC_D16_STEP; }
        else {
            GYROY += DC_D16_STEP; }

        /* Subtract DC an saturate */
        Diff=*(pDataIn_z++)-(HI_S32)(GYROZ + 0.5);
        if (Diff > 32767) {
            Diff = 32767; }
        else if (Diff < -32768) {
            Diff = -32768; }
        *(pDataOut_z++)=(HI_S32)Diff;
        if (Diff < 0) {
            GYROZ -= DC_D16_STEP; }
        else {
            GYROZ += DC_D16_STEP; }

    }
    
}


HI_S32 SAMPLE_GYRO_Init()
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32BufSize = 0;

    if(stSampleCtrl.bgyro_required != HI_TRUE)
    {
        return -1;
    }
     
    u32BufSize = GYRO_BUF_LEN;

    if(stSampleCtrl.gyro_dev != NULL)
    {
        g_s32GyroDevFd = open(stSampleCtrl.gyro_dev, O_RDWR);
    }

    if (g_s32GyroDevFd < 0)
    {
        printf("Error: cannot open GYRO device %s.\n",stSampleCtrl.gyro_dev);
        return -1;
    }

    s32Ret = HI_MPI_SYS_MmzAlloc(&g_stGyroAttr.u32PhyAddr, &g_stGyroAttr.pVirAddr,
                                 "GyroData", HI_NULL, u32BufSize);

    if (HI_SUCCESS != s32Ret)
    {
        VI_TRACE(HI_DBG_ERR, "GYRO Mmz alloc failed!\n");
        s32Ret =  HI_ERR_VI_NOMEM;
        return s32Ret;
    }
    memset(g_stGyroAttr.pVirAddr, 0, u32BufSize);

    phyaddr = g_stGyroAttr.u32PhyAddr;

    g_stGyroAttr.u32GyroFreq = 1000;
    g_stGyroAttr.u32Buflen = GYRO_BUF_LEN;

    g_stDISGyroData.pdRotX = (HI_DOUBLE *)malloc(sizeof(HI_DOUBLE) * X_BUF_LEN);
    if (NULL == g_stDISGyroData.pdRotX)
    {
        printf("pdRotX malloc failed !\n");
        return HI_ERR_VI_NOMEM;
    }
    g_stDISGyroData.pdRotY = (HI_DOUBLE *)malloc(sizeof(HI_DOUBLE) * X_BUF_LEN);
    if (NULL == g_stDISGyroData.pdRotY)
    {
        printf("pdRotX malloc failed !\n");
        return HI_ERR_VI_NOMEM;
    }
    g_stDISGyroData.pdRotZ = (HI_DOUBLE *)malloc(sizeof(HI_DOUBLE) * X_BUF_LEN);
    if (NULL == g_stDISGyroData.pdRotZ)
    {
        printf("pdRotX malloc failed !\n");
        return HI_ERR_VI_NOMEM;
    }
    g_stDISGyroData.ps64Time = (HI_S64 *)malloc(sizeof(HI_S64) * X_BUF_LEN);
    if (NULL == g_stDISGyroData.ps64Time)
    {
        printf("pdRotX malloc failed !\n");
        return HI_ERR_VI_NOMEM;
    }
    g_stDISGyroData.u32Num = 0;

    /*this value base on gyro range is 250*/
    g_GyroSensitivity = 131.072;

    s32Ret = ioctl(g_s32GyroDevFd, IOCTL_CMD_INIT_BUF, &g_stGyroAttr);

    return s32Ret;
}

HI_S32 SAMPLE_GYRO_DeInit(void)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if(g_s32GyroDevFd < 0)
    {
        return 0;
    }
    
    s32Ret = ioctl(g_s32GyroDevFd, IOCTL_CMD_DEINIT_BUF, NULL);

    sleep(5);
    s32Ret = HI_MPI_SYS_MmzFree(g_stGyroAttr.u32PhyAddr, g_stGyroAttr.pVirAddr);
    if (HI_SUCCESS != s32Ret)
    {
        VI_TRACE(HI_DBG_ERR, "gyro sys mmz free failed!");
    }
    g_stGyroAttr.u32PhyAddr = 0;
    g_stGyroAttr.pVirAddr = NULL;

    free(g_stDISGyroData.pdRotX);
    g_stDISGyroData.pdRotX = NULL;

    free(g_stDISGyroData.pdRotY);
    g_stDISGyroData.pdRotY = NULL;

    free(g_stDISGyroData.pdRotZ);
    g_stDISGyroData.pdRotZ = NULL;

    free(g_stDISGyroData.ps64Time);
    g_stDISGyroData.ps64Time = NULL;
    
    close(g_s32GyroDevFd);

    return s32Ret;
}

HI_S32 SAMPLE_GYRO_Start()
{
    return ioctl(g_s32GyroDevFd, IOCTL_CMD_START_GYRO, NULL);
}

HI_S32 SAMPLE_GYRO_Stop()
{
    return ioctl(g_s32GyroDevFd, IOCTL_CMD_STOP_GYRO, NULL);
}

HI_S32 SAMPLE_GYRO_GetData(HI_U64 u64BeginPts, HI_U64 u64EndPts, VI_DIS_GYRO_DATA_S *pstDISGyroData)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 i = 0;
    HI_S32 *pdRotX1 = 0;
    HI_S32 *pdRotY1 = 0;
    HI_S32 *pdRotZ1 = 0;
    HI_U64 *pu64PTS1 = 0;
    HI_S32 *pdRotX2 = 0;
    HI_S32 *pdRotY2 = 0;
    HI_S32 *pdRotZ2 = 0;
    HI_U64 *pu64PTS2 = 0;

    VIU_CHECK_NULL_PTR(pstDISGyroData);

    memset(&g_stDISGyroInfo , 0, sizeof(VI_DIS_GYROINFO_S));

    g_stDISGyroInfo.u64BeginPts = u64BeginPts;
    g_stDISGyroInfo.u64EndPts = u64EndPts;

    s32Ret =  ioctl(g_s32GyroDevFd, IOCTL_CMD_GET_GYRO_DATA, &g_stDISGyroInfo);
    if (HI_SUCCESS != s32Ret)
    {
        printf("gyrodev get gyrodata failed ! s32Ret : 0x%x \r\n", s32Ret);
        return s32Ret;
    }

    g_stDISGyroData.u32Num = g_stDISGyroInfo.stDISGyroData[0].u32Num + g_stDISGyroInfo.stDISGyroData[1].u32Num;

    if (g_stDISGyroInfo.stDISGyroData[0].u32Num > 0)
    {
        pdRotX1 = (HI_S32 *)((HI_S32 *)g_stGyroAttr.pVirAddr + ((HI_S32 *)g_stDISGyroInfo.stDISGyroData[0].pdRotX - (HI_S32 *)phyaddr));
        pdRotY1 = (HI_S32 *)((HI_S32 *)g_stGyroAttr.pVirAddr + ((HI_S32 *)g_stDISGyroInfo.stDISGyroData[0].pdRotY - (HI_S32 *)phyaddr));
        pdRotZ1 = (HI_S32 *)((HI_S32 *)g_stGyroAttr.pVirAddr + ((HI_S32 *)g_stDISGyroInfo.stDISGyroData[0].pdRotZ - (HI_S32 *)phyaddr));
        pu64PTS1 = (HI_U64 *)((HI_U64 *)g_stGyroAttr.pVirAddr + ((HI_U64 *)g_stDISGyroInfo.stDISGyroData[0].pu64Time - (HI_U64 *)phyaddr));


        DC_clear_algorithm(pdRotX1,
                             pdRotX1,
                             pdRotY1,
                             pdRotY1,
                             pdRotZ1,
                             pdRotZ1,
                             g_stDISGyroInfo.stDISGyroData[0].u32Num);


        for (i = 0; i < g_stDISGyroInfo.stDISGyroData[0].u32Num ; i++)
        {
        /*The gyro data convert from gyro data got from gyro driver to gyro data that used by DIS algorithm */
#ifdef GYRO_INVENSENSE
            *(g_stDISGyroData.pdRotY + i) = (((HI_DOUBLE) * (pdRotX1 + i) / g_GyroSensitivity) * DEC_TO_RAD);
            *(g_stDISGyroData.pdRotX + i) = (((HI_DOUBLE) * (pdRotY1 + i) / g_GyroSensitivity) * DEC_TO_RAD);
            *(g_stDISGyroData.pdRotZ + i) = (((HI_DOUBLE) * (pdRotZ1 + i) / g_GyroSensitivity) * DEC_TO_RAD)*(-1);
#else
            *(g_stDISGyroData.pdRotZ + i) = (((HI_DOUBLE) * (pdRotX1 + i) / g_GyroSensitivity) * DEC_TO_RAD)*(-1);
            *(g_stDISGyroData.pdRotX + i) = (((HI_DOUBLE) * (pdRotY1 + i) / g_GyroSensitivity) * DEC_TO_RAD);
            *(g_stDISGyroData.pdRotY + i) = (((HI_DOUBLE) * (pdRotZ1 + i) / g_GyroSensitivity) * DEC_TO_RAD)*(-1);
#endif
            *(g_stDISGyroData.ps64Time + i) = (HI_S64)((*(pu64PTS1 + i)));
        }

    }

    if (g_stDISGyroInfo.stDISGyroData[1].u32Num > 0)
    {

        pdRotX2 = (HI_S32 *)((HI_S32 *)g_stGyroAttr.pVirAddr + ((HI_S32 *)g_stDISGyroInfo.stDISGyroData[1].pdRotX - (HI_S32 *)phyaddr));
        pdRotY2 = (HI_S32 *)((HI_S32 *)g_stGyroAttr.pVirAddr + ((HI_S32 *)g_stDISGyroInfo.stDISGyroData[1].pdRotY - (HI_S32 *)phyaddr));
        pdRotZ2 = (HI_S32 *)((HI_S32 *)g_stGyroAttr.pVirAddr + ((HI_S32 *)g_stDISGyroInfo.stDISGyroData[1].pdRotZ - (HI_S32 *)phyaddr));
        pu64PTS2 = (HI_U64 *)((HI_U64 *)g_stGyroAttr.pVirAddr + ((HI_U64 *)g_stDISGyroInfo.stDISGyroData[1].pu64Time - (HI_U64 *)phyaddr));

        DC_clear_algorithm(pdRotX2,
                             pdRotX2,
                             pdRotY2,
                             pdRotY2,
                             pdRotZ2,
                             pdRotZ2,
                             g_stDISGyroInfo.stDISGyroData[1].u32Num);


        for (i = g_stDISGyroInfo.stDISGyroData[0].u32Num ; i < g_stDISGyroInfo.stDISGyroData[0].u32Num + g_stDISGyroInfo.stDISGyroData[1].u32Num ; i++)
        {
        /*The gyro data convert from gyro data got from gyro driver to gyro data that used by DIS algorithm */
#ifdef GYRO_INVENSENSE
            *(g_stDISGyroData.pdRotY + i) = (HI_DOUBLE)((HI_DOUBLE)(*(pdRotX2 + i - g_stDISGyroInfo.stDISGyroData[0].u32Num) / g_GyroSensitivity) * DEC_TO_RAD);
            *(g_stDISGyroData.pdRotX + i) = (HI_DOUBLE)((HI_DOUBLE)(*(pdRotY2 + i - g_stDISGyroInfo.stDISGyroData[0].u32Num) / g_GyroSensitivity) * DEC_TO_RAD);
            *(g_stDISGyroData.pdRotZ + i)  = (HI_DOUBLE)((HI_DOUBLE)(*(pdRotZ2 + i - g_stDISGyroInfo.stDISGyroData[0].u32Num) / g_GyroSensitivity) * DEC_TO_RAD)*(-1);
#else
            *(g_stDISGyroData.pdRotZ + i) = (HI_DOUBLE)((HI_DOUBLE)(*(pdRotX2 + i - g_stDISGyroInfo.stDISGyroData[0].u32Num) / g_GyroSensitivity) * DEC_TO_RAD)*(-1);
            *(g_stDISGyroData.pdRotX + i) = (HI_DOUBLE)((HI_DOUBLE)(*(pdRotY2 + i - g_stDISGyroInfo.stDISGyroData[0].u32Num) / g_GyroSensitivity) * DEC_TO_RAD);
            *(g_stDISGyroData.pdRotY + i)  = (HI_DOUBLE)((HI_DOUBLE)(*(pdRotZ2 + i - g_stDISGyroInfo.stDISGyroData[0].u32Num) / g_GyroSensitivity) * DEC_TO_RAD)*(-1);
#endif
            *(g_stDISGyroData.ps64Time + i) = (HI_S64)((*(pu64PTS2 + i - g_stDISGyroInfo.stDISGyroData[0].u32Num)));
        }

    }

    memcpy(pstDISGyroData , &g_stDISGyroData, sizeof(VI_DIS_GYRO_DATA_S));

    return s32Ret;
}


HI_S32 SAMPLE_GYRO_Release()
{
    memset(g_stDISGyroData.pdRotX, 0 , sizeof(HI_DOUBLE) * X_BUF_LEN);
    memset(g_stDISGyroData.pdRotY, 0 , sizeof(HI_DOUBLE) * X_BUF_LEN);
    memset(g_stDISGyroData.pdRotZ, 0 , sizeof(HI_DOUBLE) * X_BUF_LEN);
    memset(g_stDISGyroData.ps64Time, 0 , sizeof(HI_S64) * X_BUF_LEN);

    return  ioctl(g_s32GyroDevFd, IOCTL_CMD_RELEASE_BUF, &g_stDISGyroInfo);
}


/******************************************************************************
* function : show usage
******************************************************************************/

static void *SAMPLE_DIS_Run(void *param)
{
    VI_CHN ViChn = 0;

	#ifndef __HuaweiLite__
    /******************************************
      To set the DIS_Run thread run at CPU 1 (a17)
    ******************************************/
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(1, &mask);
    if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0)
    {
        perror("pthread_setaffinity_np");
    }
	#endif
    ViChn = *((VI_CHN *)param);
    HI_MPI_VI_DISRun(ViChn);

    return NULL;
}

static HI_S32 SAMPLE_DIS_Exit(void)
{
    HI_S32 s32Ret = HI_SUCCESS;
    VI_CHN ViChn = 0;

    s32Ret = HI_MPI_VI_DISExit(ViChn);

    if (gs_DISPid)
    {
        pthread_join(gs_DISPid, 0);
        gs_DISPid = 0;
    }
    return s32Ret;
}

static HI_S32 SAMPLE_SENSOR_ADAPTIVE(void)
{
    /* GYRO_BOSCH / GYRO_INVENSENSE maco is defined in ../Makefile.param*/
#ifdef GYRO_INVENSENSE
    stSampleCtrl.gyro_dev = "/dev/mpu20628";   
#elif defined(GYRO_BOSCH)
    stSampleCtrl.gyro_dev = "/dev/BMI160";
#endif 

    switch(g_stViChnConfig.enViMode)
    {
    	
        case PANASONIC_MN34220_SUBLVDS_1080P_30FPS:
        case SONY_IMX117_LVDS_1080P_120FPS:
            g_stViChnConfig.enFrmRate = SAMPLE_FRAMERATE_60FPS;
            break;
        case SONY_IMX117_LVDS_720P_240FPS:
            g_stViChnConfig.enFrmRate = SAMPLE_FRAMERATE_120FPS;
            u32width = 1280;
            u32height = 720;
            if(g_pstDISConfig->enMotionType != VI_DIS_MOTION_8DOF_HARD)
            {
            	g_pstDISConfig->enAccuracy = VI_DIS_ACCURACY_LOW;
            }
            
            break;
           
        default:
            break;
    }
    return HI_SUCCESS;
}

/******************************************************************************
* function : to process abnormal case
******************************************************************************/
void SAMPLE_DIS_HandleSig(HI_S32 signo)
{
    signal(SIGINT, SIG_IGN);
    if (SIGINT == signo || SIGTERM == signo ||  SIGBUS == SIGTERM)
    {
        SAMPLE_DIS_Exit();
        if(g_s32GyroDevFd >0)
        {
        	SAMPLE_GYRO_Stop();
        	SAMPLE_GYRO_DeInit();	
        }
        SAMPLE_COMM_ISP_Stop();
        SAMPLE_COMM_SYS_Exit();
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
    }
    exit(-1);
}

HI_S32 SAMPLE_VI_DIS_VPSS_VO_PreView_Venc(SAMPLE_VI_CONFIG_S *pstViConfig)
{
    HI_U32 u32ViChnCnt = 2;
    VB_CONF_S stVbConf;
    VO_DEV VoDev = SAMPLE_VO_DEV_DSD0;;
    VO_CHN VoChn = 0;
    VI_CHN ViChn = 0;
    VO_PUB_ATTR_S stVoPubAttr;
    SAMPLE_VO_MODE_E enVoMode = VO_MODE_1MUX;
    PIC_SIZE_E enPicSize = g_enPicSize;
    VO_LAYER VoLayer = 0;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;
    VPSS_GRP VpssGrp = 0;
    VPSS_CHN VpssChn[2] = {0,1};
    VPSS_GRP_ATTR_S stVpssGrpAttr = {0};
    VPSS_CHN_ATTR_S stVpssChnAttr;
    VPSS_CHN_MODE_S stVpssChnMode;
    VI_CHN_ATTR_S stChnAttr;
    

    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32BlkSize;
    SIZE_S stSize;

    VENC_CHN VencChn = 0;
    SAMPLE_RC_E enRcMode= SAMPLE_RC_CBR;
    PAYLOAD_TYPE_E enPayLoad= PT_H264;
    HI_U32 u32Profile = 0;
    HI_S32 s32ChnNum=1;   
	VENC_GOP_ATTR_S stGopAttr;


    /******************************************
     step  1: init global  variable
    ******************************************/
    gs_u32ViFrmRate = (VIDEO_ENCODING_MODE_PAL == gs_enNorm) ? 25 : 30;
    memset(&stVbConf, 0, sizeof(VB_CONF_S));

    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm, enPicSize,
                 SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.u32MaxPoolCnt = 128;

    /*ddr0 video buffer*/
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt = u32ViChnCnt * 15;

    /******************************************
     step 2: start vpss and vi bind vpss (subchn needn't bind vpss in this mode)
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, enPicSize, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        goto EXIT_0;
    }

    /******************************************
     step 3: mpp system init.
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        goto EXIT_0;
    }

    /******************************************
     step 4: start vi dev & chn to capture
    ******************************************/
    s32Ret = SAMPLE_COMM_VI_StartVi(pstViConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        goto EXIT_1;
    }

    s32Ret =  HI_MPI_VI_GetChnAttr(ViChn,&stChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("getChnAttr failed with %#x!\n", s32Ret);
        goto EXIT_1;
    }

    /******************************************
     step 5: setDISConfig & DISInit & setDISAttr
    ******************************************/    

    if(stChnAttr.stDestSize.u32Width == 3840 && stChnAttr.stDestSize.u32Height == 2160)
    {
    	g_pstDISConfig->s32FrameRate = 30;
    }
    else if(stChnAttr.stDestSize.u32Width == 1920 && stChnAttr.stDestSize.u32Height == 1080)
    {
    	g_pstDISConfig->s32FrameRate = 60;
    }
    else if (stChnAttr.stDestSize.u32Width == 1280 && stChnAttr.stDestSize.u32Height == 720)
    {
    	g_pstDISConfig->s32FrameRate = 120;
    }
    else
    {
    	g_pstDISConfig->s32FrameRate = 30;
    }
    

    s32Ret =  HI_MPI_VI_SetDISConfig(ViChn, g_pstDISConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SetDISConfig failed with %#x!\n", s32Ret);
        goto EXIT_1;
    }

    s32Ret =  HI_MPI_VI_DISInit(ViChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        goto EXIT_1;
    }

  	/******************************************
     step 6:  setDISAttr
    ******************************************/

    s32Ret =  HI_MPI_VI_SetDISAttr(ViChn, g_pstDISAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        goto EXIT_2;
    }
   
    if( SAMPLE_GYRO_Init() == HI_SUCCESS)
    {
        SAMPLE_GYRO_Start();
        stDISCallback.pfnGetGyroDataCallback = SAMPLE_GYRO_GetData;
        stDISCallback.pfnReleaseGyroDataCallback = SAMPLE_GYRO_Release;
        HI_MPI_VI_RegisterDISCallback(0, &stDISCallback);
    }

    /******************************************
     step 7: start DISRun thread
    ******************************************/

    if (0 != pthread_create(&gs_DISPid, NULL, SAMPLE_DIS_Run, &ViChn))
    {
        SAMPLE_PRT("create dis run thread failed!\r\n");
        goto EXIT_2;
    }

    /******************************************
     step 8: start vpss group
    ******************************************/    

    stVpssGrpAttr.bDciEn = HI_FALSE;
    stVpssGrpAttr.bHistEn = HI_FALSE;
    stVpssGrpAttr.bIeEn = HI_FALSE;
    stVpssGrpAttr.bNrEn = HI_TRUE;
    stVpssGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE;
    stVpssGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;
    stVpssGrpAttr.bStitchBlendEn = HI_FALSE;
    stVpssGrpAttr.stNrAttr.u32RefFrameNum = 1;
    stVpssGrpAttr.u32MaxH = stSize.u32Height;
    stVpssGrpAttr.u32MaxW = stSize.u32Width;

    s32Ret = SAMPLE_COMM_VPSS_StartGroup(VpssGrp, &stVpssGrpAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start VPSS GROUP failed!\n");
        goto EXIT_3;
    }

    memset(&stVpssChnAttr, 0, sizeof(stVpssChnAttr));
    stVpssChnAttr.s32SrcFrameRate = -1;
    stVpssChnAttr.s32DstFrameRate = -1;

    
    stVpssChnMode.bDouble = HI_FALSE;
    stVpssChnMode.enChnMode = VPSS_CHN_MODE_USER;
    stVpssChnMode.enCompressMode = COMPRESS_MODE_SEG;
    stVpssChnMode.enPixelFormat = SAMPLE_PIXEL_FORMAT;
    stVpssChnMode.u32Width  = stSize.u32Width;
    stVpssChnMode.u32Height = stSize.u32Height;

    /******************************************
    step 9: start vpss chn 0 for venc
    ******************************************/
    s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp, VpssChn[0], &stVpssChnAttr, &stVpssChnMode, HI_NULL);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start VPSS CHN failed!\n");
        goto EXIT_3;
    }

    /******************************************
     start vpss chn 1 for vo
    ******************************************/
    
    stVpssChnMode.u32Width  = u32width;
    stVpssChnMode.u32Height = u32height;
    s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp, VpssChn[1], &stVpssChnAttr, &stVpssChnMode, HI_NULL);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start VPSS CHN failed!\n");
        goto EXIT_3;
    }

    /******************************************
    step 10: start VO SD0 (bind * vpss )
    ******************************************/
    stVoPubAttr.enIntfType = g_enVoIntfType;
    if (VO_INTF_BT1120 == g_enVoIntfType)
    {
        stVoPubAttr.enIntfSync = VO_OUTPUT_1080P30;
        gs_u32ViFrmRate = 50;
    }
    else
    {
        stVoPubAttr.enIntfSync = VO_OUTPUT_PAL;
    }
    stVoPubAttr.u32BgColor = 0x000000ff;

    s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, &stVoPubAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VO_StartDev failed!\n");
        goto EXIT_3;
    }

    stLayerAttr.bClusterMode = HI_FALSE;
    stLayerAttr.bDoubleFrame = HI_FALSE;
    stLayerAttr.enPixFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;

    stLayerAttr.stDispRect.s32X = 0;
    stLayerAttr.stDispRect.s32Y = 0;

    s32Ret = SAMPLE_COMM_VO_GetWH(stVoPubAttr.enIntfSync,
                                  &stLayerAttr.stDispRect.u32Width, &stLayerAttr.stDispRect.u32Height,
                                  &stLayerAttr.u32DispFrmRt);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VO_GetWH failed!\n");
        goto EXIT_4;
    }
    stLayerAttr.stImageSize.u32Width  = stLayerAttr.stDispRect.u32Width;
    stLayerAttr.stImageSize.u32Height = stLayerAttr.stDispRect.u32Height;

    s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stLayerAttr, HI_TRUE);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VO_StartLayer failed!\n");
        goto EXIT_4;
    }

    s32Ret = SAMPLE_COMM_VO_StartChn(VoDev, enVoMode);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VO_StartChn failed!\n");
        goto EXIT_5;
    }
  
    s32Ret = SAMPLE_COMM_VI_BindVpss(pstViConfig->enViMode);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VI_BindVpss failed!\n");
        goto EXIT_5;
    }

    s32Ret = SAMPLE_COMM_VO_BindVpss(VoDev, VoChn, VpssGrp, VpssChn[1]);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VO_BindVpss(vo:%d)-(VpssChn:%d) failed with %#x!\n", VoDev, VoChn, s32Ret);
        goto EXIT_6;
    }
    /******************************************
     step 11: start stream venc
    ******************************************/
	s32Ret = SAMPLE_COMM_VENC_GetGopAttr(VENC_GOPMODE_NORMALP,&stGopAttr,gs_enNorm);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Get GopAttr failed!\n");
		goto EXIT_7;
	}		

    s32Ret = SAMPLE_COMM_VENC_Start(VencChn, enPayLoad,\
                                   gs_enNorm, enPicSize, enRcMode,u32Profile,&stGopAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto EXIT_7;
    }

    s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VpssChn[0]);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto EXIT_7;
    }

    /******************************************
     step 12: stream venc process -- get stream, then save it to file. 
    ******************************************/
    s32Ret = SAMPLE_COMM_VENC_StartGetStream(s32ChnNum);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto EXIT_7;
    }

    printf("\nplease hit the Enter key, disable DIS\n");
    getchar();

    g_pstDISAttr->bEnable = HI_FALSE;

    s32Ret =  HI_MPI_VI_SetDISAttr(ViChn, g_pstDISAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        goto EXIT_7;
    }


    printf("\nplease hit the Enter key, enable DIS\n");
    getchar();

    g_pstDISAttr->bEnable = HI_TRUE;

    s32Ret =  HI_MPI_VI_SetDISAttr(ViChn, g_pstDISAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        goto EXIT_7;
    }

    VI_PAUSE();

    /******************************************
     step 13: exit process
    ******************************************/
    
EXIT_7:
    SAMPLE_COMM_VENC_StopGetStream();
    SAMPLE_COMM_VENC_UnBindVpss(VencChn, VpssGrp, VpssChn[0]);
    SAMPLE_COMM_VENC_Stop(VencChn);
EXIT_6:
    SAMPLE_COMM_VO_UnBindVi(VoDev, VoChn);
    SAMPLE_COMM_VO_StopChn(VoDev, enVoMode);
EXIT_5:
    SAMPLE_COMM_VO_StopLayer(VoLayer);
EXIT_4:
    SAMPLE_COMM_VO_StopDev(VoDev);
EXIT_3:
	SAMPLE_COMM_VI_UnBindVpss(pstViConfig->enViMode);
    SAMPLE_COMM_VPSS_DisableChn(VpssGrp, VpssChn[0]);
    SAMPLE_COMM_VPSS_DisableChn(VpssGrp, VpssChn[1]);
    SAMPLE_COMM_VPSS_StopGroup(VpssGrp);
EXIT_2:
    SAMPLE_DIS_Exit();
    SAMPLE_GYRO_Stop();
    SAMPLE_GYRO_DeInit();
EXIT_1:
    SAMPLE_COMM_VI_StopVi(pstViConfig);
EXIT_0:
    SAMPLE_COMM_SYS_Exit();

    return s32Ret;
}



/******************************************************************************
* function    : main()
* Description : video preview sample
******************************************************************************/
#ifdef __HuaweiLite__
int app_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
    HI_S32 s32Ret = HI_FAILURE;
#ifdef __HuaweiLite__    
    HI_S32 VoInf,ch;
#endif

#ifndef __HuaweiLite__
    if ((argc < 2) || (1 != strlen(argv[1])))
    {
        SAMPLE_DIS_Usage(argv[0]);
        return HI_FAILURE;
    }

    signal(SIGINT, SAMPLE_DIS_HandleSig);
    signal(SIGTERM, SAMPLE_DIS_HandleSig);
    signal(SIGBUS, SAMPLE_DIS_HandleSig);
#endif

#ifdef __HuaweiLite__
    SAMPLE_DIS_VoIntf_Usage();
    VoInf = getchar();
    while((ch = getchar()) != '\n' && (ch != EOF)); 
    if ('1' == VoInf)
#else
    if ((argc > 2) && *argv[2] == '1')  /* '1': VO_INTF_CVBS, else: BT1120 */
#endif
    {
        g_enVoIntfType = VO_INTF_BT1120;
    }

#ifdef __HuaweiLite__
    SAMPLE_DIS_Mode_Usage();
    VoInf = getchar();
    while((ch = getchar()) != '\n' && (ch != EOF)); 
    if ('1' == VoInf)
#else
    if ((argc > 3) && *argv[3] == '1')  /* '1': hardware mode, else: software mode */
#endif
    {
        stSampleCtrl.bgyro_required = HI_TRUE;
        g_pstDISConfig = &stDISConfigHard;
        g_pstDISAttr = &stDISAttrHard;
    }
#ifdef __HuaweiLite__
    else if('2' == VoInf)
#else
    else if((argc > 3) && *argv[3] == '2')
#endif
    {
        stSampleCtrl.bgyro_required = HI_TRUE;
        g_pstDISConfig = &stDISConfigHybrid;
        g_pstDISAttr = &stDISAttrHybrid;
    }


    g_stViChnConfig.enViMode = SENSOR_TYPE;
    printf("~~~~~~~~~~~~~~ vimode %d ~~~~~~~~~~~~~~\n", g_stViChnConfig.enViMode);
    SAMPLE_COMM_VI_GetSizeBySensor(&g_enPicSize);

    SAMPLE_SENSOR_ADAPTIVE();


#ifndef __HuaweiLite__
    switch (*argv[1])
#else
	SAMPLE_DIS_Index_Usage();	
    ch = getchar();
    switch (ch)
#endif
    {
            /* VI_DIS_VPSS_VO, phychn channel preview. */
        case '0':
            s32Ret = SAMPLE_VI_DIS_VPSS_VO_PreView_Venc(&g_stViChnConfig);
            break;
        default:
            SAMPLE_PRT("the index is invaild!\n");
            SAMPLE_DIS_Usage(argv[0]);
            return HI_FAILURE;
    }

    
    if (HI_SUCCESS == s32Ret)
    {
        SAMPLE_PRT("program exit normally!\n");
    }
    else
    {
        SAMPLE_PRT("program exit abnormally!\n");
    }
    exit(s32Ret);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

