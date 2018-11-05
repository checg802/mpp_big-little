#include <stdio.h>
#include <math.h>
//#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "mpi_sys.h"
#include "mpi_vi.h"
#include "mpi_vpss.h"
#include "histitch.h"


static void usage(void)
{
    printf(
        "\n"
        "*************************************************\n"
#ifndef __HuaweiLite__
        "Usage: ./stitch_adjust [x] [y] [theta] [mode] [vpssgrp].\n"
#else
        "Usage: stitch_adjust [x] [y] [theta] [mode] [vpssgrp].\n"
#endif
        "1)x: \n"
        "   move on horizental direction\n"
        "2)y: \n"
        "   move on vertical direction\n"
        "3)theta: \n"
        "   rotate\n"
        "4)mode: \n"
        "   stitch mode, 0: PERSPECTIVE, 1: CYLINDRICAL\n"
        "   Default: 0\n"
        "5)vpssgrp: \n"
        "   whitch group to adjust\n"
        "   Default: 0\n"
        "*)Example:\n"
#ifndef __HuaweiLite__
        "   e.g : ./stitch_adjust 10 0 0\n"
#else
        "   e.g : stitch_adjust 10 0 0\n"
#endif
        "*************************************************\n"
        "\n");
}


#ifdef __HuaweiLite__
HI_S32 stitch_adjust(int argc, char* argv[])
#else
HI_S32 main(int argc, char* argv[])
#endif
{
    HI_S32 s32Ret = HI_SUCCESS;

    STITCH_ADJUST_INPUT_S stAdjustInPut;
    STITCH_OUTPUT_PARAM_S stOutPut;

    MPP_CHN_S stDestChn, stSrcChn;
    VI_CHN ViLeftChn, ViRightChn;
    VI_CHN_ATTR_S stViAttr;
    VI_STITCH_CORRECTION_ATTR_S stCorretionAttr;
    VPSS_GRP VpssGrp = 0;
    VPSS_GRP_ATTR_S stGrpAttr;
    HI_S32 s32StitchMode = 0;


    printf("\nNOTICE: This tool only can be used for TESTING !!!\n");

    if (argc > 1)
    {
        if (!strncmp(argv[1], "-h", 2))
        {
            usage();
            exit(HI_SUCCESS);
        }
        stAdjustInPut.stAdjust.s32XAdjust = atoi(argv[1]);
    }

    if (argc > 2)
    {
        stAdjustInPut.stAdjust.s32YAdjust = atoi(argv[2]);
    }

    if (argc > 3)
    {
        stAdjustInPut.stAdjust.s32Theta    = atoi(argv[3]);
    }

    if (argc > 4)
    {
        s32StitchMode = atoi(argv[4]);
    }
    
    if (argc > 5)
    {
        VpssGrp =  atoi(argv[5]);
    }

    if (argc < 4)
    {
        printf("Need three parameters at least!\n");
        return -1;
    }

    stDestChn.enModId = HI_ID_VPSS;
    stDestChn.s32DevId = 0;
    stDestChn.s32ChnId = 0;

    s32Ret = HI_MPI_SYS_GetBindbyDest(&stDestChn, &stSrcChn);
    if (HI_SUCCESS != s32Ret)
    {
        printf("Get bind fail!\n");
        return -1;
    }

    ViLeftChn = stSrcChn.s32ChnId;
    ViRightChn = 1 - ViLeftChn;

    s32Ret = HI_MPI_VI_GetChnAttr(ViLeftChn , &stViAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("Get vi chn attr fail!\n");
        return -1;
    }

    stAdjustInPut.stInputParam.stLeftOriSize.u32Width = stViAttr.stDestSize.u32Width;
    stAdjustInPut.stInputParam.stLeftOriSize.u32Height = stViAttr.stDestSize.u32Height;


    s32Ret = HI_MPI_VI_GetChnAttr(ViRightChn , &stViAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("Get vi chn attr fail!\n");
        return -1;
    }

    stAdjustInPut.stInputParam.stRightOriSize.u32Width = stViAttr.stDestSize.u32Width;
    stAdjustInPut.stInputParam.stRightOriSize.u32Height = stViAttr.stDestSize.u32Height;


    /* get the old stitch param */
    s32Ret = HI_MPI_VI_GetStitchCorrectionAttr(ViLeftChn , &stCorretionAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("Get vi stitch correction attr fail!\n");
        return -1;
    }

    stAdjustInPut.stInputParam.stLeftPMFAttr.bEnable = stCorretionAttr.stPMFAttr.bEnable;
    memcpy(stAdjustInPut.stInputParam.stLeftPMFAttr.as32PMFCoef, stCorretionAttr.stPMFAttr.as32PMFCoef, sizeof(HI_S32) * 9);

    s32Ret = HI_MPI_VI_GetStitchCorrectionAttr(ViRightChn, &stCorretionAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("Get vi stitch correction attr fail!\n");
        return -1;
    }

    stAdjustInPut.stInputParam.stRightPMFAttr.bEnable = stCorretionAttr.stPMFAttr.bEnable;
    memcpy(stAdjustInPut.stInputParam.stRightPMFAttr.as32PMFCoef, stCorretionAttr.stPMFAttr.as32PMFCoef, sizeof(HI_S32) * 9);

    s32Ret = HI_MPI_VPSS_GetGrpAttr(VpssGrp, &stGrpAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("Get vpss grp attr fail!\n");
        return -1;
    }

    stAdjustInPut.stInputParam.stOutSize.u32Width = stGrpAttr.stStitchBlendAttr.u32OutWidth;
    stAdjustInPut.stInputParam.stOutSize.u32Height = stGrpAttr.stStitchBlendAttr.u32OutHeight;
    memcpy(stAdjustInPut.stInputParam.stOverlapPoint, stGrpAttr.stStitchBlendAttr.stOverlapPoint, sizeof(POINT_S) * 4);

    if (0 == s32StitchMode)
    {
        stAdjustInPut.stInputParam.enStitchMode = STITCH_MODE_PERSPECTIVE;
    }
    else
    {
        stAdjustInPut.stInputParam.enStitchMode = STITCH_MODE_CYLINDRICAL;
    }

    s32Ret = HI_STITCH_AdjustCompute(&stAdjustInPut, &stOutPut);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_STITCH_AdjustCompute fail!\n");
        return -1;
    }

    /* set the new stitch param */
    s32Ret = HI_MPI_VI_GetStitchCorrectionAttr(ViLeftChn , &stCorretionAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("Get vi stitch correction attr fail!\n");
        return -1;
    }

    stCorretionAttr.stPMFAttr.bEnable = stOutPut.stLeftPMFAttr.bEnable;
    stCorretionAttr.stPMFAttr.stDestSize.u32Width = stOutPut.stLeftSize.u32Width;
    stCorretionAttr.stPMFAttr.stDestSize.u32Height = stOutPut.stLeftSize.u32Height;
    memcpy(stCorretionAttr.stPMFAttr.as32PMFCoef, stOutPut.stLeftPMFAttr.as32PMFCoef, sizeof(HI_S32) * 9);

    s32Ret = HI_MPI_VI_SetStitchCorrectionAttr(ViLeftChn , &stCorretionAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("Set vi stitch correction attr fail!\n");
        return -1;
    }

    s32Ret = HI_MPI_VI_GetStitchCorrectionAttr(ViRightChn , &stCorretionAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("Get vi stitch correction attr fail!\n");
        return -1;
    }

    stCorretionAttr.stPMFAttr.bEnable = stOutPut.stRightPMFAttr.bEnable;
    stCorretionAttr.stPMFAttr.stDestSize.u32Width = stOutPut.stRightSize.u32Width;
    stCorretionAttr.stPMFAttr.stDestSize.u32Height = stOutPut.stRightSize.u32Height;
    memcpy(stCorretionAttr.stPMFAttr.as32PMFCoef, stOutPut.stRightPMFAttr.as32PMFCoef, sizeof(HI_S32) * 9);

    s32Ret = HI_MPI_VI_SetStitchCorrectionAttr(ViRightChn, &stCorretionAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("Set vi stitch correction attr fail!\n");
        return -1;
    }

    s32Ret = HI_MPI_VPSS_GetGrpAttr(VpssGrp, &stGrpAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("Get vpss grp attr fail!\n");
        return -1;
    }

    stGrpAttr.stStitchBlendAttr.u32OutWidth = stOutPut.stOutSize.u32Width;
    stGrpAttr.stStitchBlendAttr.u32OutHeight = stOutPut.stOutSize.u32Height;
    memcpy(stGrpAttr.stStitchBlendAttr.stOverlapPoint, stOutPut.stOverlapPoint, sizeof(POINT_S) * 4);

    s32Ret = HI_MPI_VPSS_SetGrpAttr(VpssGrp, &stGrpAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("Set vpss grp attr fail!\n");
        return -1;
    }
    printf("Stitch Adjust Over!\n");
    return 0;
}
