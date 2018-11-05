/******************************************************************************

  Copyright (C), 2016-2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : Sample_awb_correction.c
  Version       : Initial Draft
  Author        : Hisilicon BVT PQ group
  Created       : 2016/12/15
  Description   : 
  History       :
  1.Date        : 2016/12/15
    Author      : h00372898
    Modification: Created file

******************************************************************************/

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <math.h>


#include "mpi_isp.h"
#include "hi_comm_isp.h"
#include "mpi_ae.h"

#include "sample_comm.h"

void SAMPLE_VIO_Usage(char* sPrgNm)
{
    printf("Usage : %s <mode> <intf1> <intf2> <intf3>\n", sPrgNm);
    printf("mode:\n");
    printf("\t 0) Calculate Sample gain.\n");
    printf("\t 1) Adjust Sample gain according to Golden Sample.\n");

    printf("intf1:\n");
    printf("\t The value of Rgain of Golden Sample.\n");

    printf("intf2:\n");
    printf("\t The value of Bgain of Golden Sample.\n");

    printf("intf3:\n");
    printf("\t The value of Alpha ranging from 0 to 1024 (The strength of adusting Sampe Gain will increase with the value of Alpha) .\n");

    return;
}

int main(int argc, char *argv[])
{
    HI_S16 total_count = 0;
    HI_S16 stable_count = 0;
    ISP_DEV IspDev = 0;
    HI_U16 u16GoldenRgain = 0;
    HI_U16 u16GoldenBgain = 0;
    HI_S16 s16Alpha = 0;
        
    ISP_EXP_INFO_S stExpInfo;
    ISP_EXPOSURE_ATTR_S stExpAttr;

    HI_MPI_ISP_GetExposureAttr(IspDev, &stExpAttr);
    
    printf("set antiflicker enable and the value of frequency\n");
    stExpAttr.stAuto.stAntiflicker.bEnable = HI_TRUE;
    stExpAttr.stAuto.stAntiflicker.u8Frequency = 50;
    HI_MPI_ISP_SetExposureAttr(IspDev, &stExpAttr);

   #ifndef __HuaweiLite__
    if ( (argc < 2) || (1 != strlen(argv[1])))
    {
        SAMPLE_VIO_Usage(argv[0]);
        return HI_FAILURE;
    }
   #endif

    switch (*argv[1])
    {
            /* VI/VPSS - VO. Embeded isp, phychn channel preview. */
        case '0':
            u16GoldenRgain = 0;
            u16GoldenBgain = 0;
            break;
            
        case '1':
            u16GoldenRgain = atoi(argv[2]);
            u16GoldenBgain = atoi(argv[3]);
            s16Alpha = atoi(argv[4]);
            break;
            
        default:
            SAMPLE_PRT("the mode is invaild!\n");
            SAMPLE_VIO_Usage(argv[0]);
            return HI_FAILURE;
    }
    
    do
    {
        usleep(100000000 / DIV_0_TO_1(stExpInfo.u32Fps));               
   	    HI_MPI_ISP_QueryExposureInfo(IspDev, &stExpInfo);

        /*judge whether AE is stable*/
        if (stExpInfo.s16HistError > stExpAttr.stAuto.u8Tolerance)
        {
            stable_count = 0;
        }
        else
        {
            stable_count ++;
        }
        total_count ++;
    }
    while ((stable_count < 5) && (total_count < 20));


    if (stable_count >= 5)
    {
        ISP_AWB_Calibration_Gain_S stAWBCalibGain;
        HI_MPI_ISP_GetLightboxGain(IspDev, &stAWBCalibGain);
        /*Adjust the value of Rgain and Bgain of Sample according to Golden Sample*/
        if((u16GoldenRgain)&&(u16GoldenBgain))
        {
            stAWBCalibGain.u16AvgRgain =  (HI_U16)((HI_S16)(stAWBCalibGain.u16AvgRgain)  + ((((HI_S16)u16GoldenRgain - (HI_S16)(stAWBCalibGain.u16AvgRgain))* s16Alpha) >> 10));
            stAWBCalibGain.u16AvgBgain = (HI_U16)((HI_S16)(stAWBCalibGain.u16AvgBgain) + ((((HI_S16)u16GoldenBgain  - (HI_S16)(stAWBCalibGain.u16AvgBgain))* s16Alpha) >> 10 ));
        }

#if 0 
        printf("u16AvgRgain =%8d, u16AvgBgain = %8d\n", stAWBCalibGain.u16AvgRgain, stAWBCalibGain.u16AvgBgain);
#endif
        return HI_SUCCESS;
    }
    else
    {
        printf("AE IS NOT STABLE,PLEASE WAIT");
        return HI_FAILURE;
    }

}
