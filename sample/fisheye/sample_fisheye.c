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

#include "mpi_vgs.h"
#include "mpi_fisheye.h"
#include "hi_comm_fisheye.h"
#include "hi_comm_vgs.h"
#include "sample_comm.h"

#ifndef ALIGN
#define ALIGN(x,a)            (((x)+(a)-1)&~(a-1)) 
#endif
VIDEO_NORM_E   gs_enNorm      = VIDEO_ENCODING_MODE_PAL;
VO_INTF_TYPE_E g_enVoIntfType = VO_INTF_CVBS;
PAYLOAD_TYPE_E g_enVencType   = PT_H264;
SAMPLE_RC_E    g_enRcMode     = SAMPLE_RC_CBR;
PIC_SIZE_E     g_enPicSize    = PIC_HD1080;


SAMPLE_VIDEO_LOSS_S gs_stVideoLoss;
HI_U32 gs_u32ViFrmRate = 0;

pthread_t ThreadId;
HI_BOOL bSetFisheyeAttr = HI_FALSE;

typedef struct hiFISHEYE_SET_ATTR_THREAD_INFO
{
	VPSS_GRP 	VpssGrp;
    VPSS_CHN 	VpssChn;
    VI_CHN 		ViChn;
}FISHEYE_SET_ATTR_THRD_INFO;


SAMPLE_VI_CONFIG_S g_stViChnConfig =
{
    .enViMode = PANASONIC_MN34220_SUBLVDS_1080P_30FPS,
    .enNorm   = VIDEO_ENCODING_MODE_AUTO,

    .enRotate = ROTATE_NONE,
    .enViChnSet = VI_CHN_SET_NORMAL,
    .enWDRMode  = WDR_MODE_NONE,
    .enFrmRate  = SAMPLE_FRAMERATE_DEFAULT,
};

static void SAMPLE_VPSS_FISHEYE_GetVpssConfig(SAMPLE_VPSS_ATTR_S *pstVpssAttr)
{
    if (HI_NULL == pstVpssAttr)
    {
        return;
    }
    memset(pstVpssAttr,0,sizeof(SAMPLE_VPSS_ATTR_S));
    pstVpssAttr->VpssGrp = 0;
    pstVpssAttr->VpssChn = VPSS_CHN0;

	pstVpssAttr->stVpssGrpAttr.bStitchBlendEn = HI_FALSE;
    pstVpssAttr->stVpssGrpAttr.bDciEn = HI_FALSE;
    pstVpssAttr->stVpssGrpAttr.bHistEn = HI_FALSE;
    pstVpssAttr->stVpssGrpAttr.bIeEn = HI_FALSE;
    pstVpssAttr->stVpssGrpAttr.bNrEn = HI_TRUE;
    pstVpssAttr->stVpssGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE;
    pstVpssAttr->stVpssGrpAttr.enPixFmt = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    pstVpssAttr->stVpssGrpAttr.stNrAttr.u32RefFrameNum = 1;
    switch (SENSOR_TYPE)
    {
        case APTINA_9M034_DC_720P_30FPS:
        case APTINA_AR0130_DC_720P_30FPS:
        case PANASONIC_MN34220_SUBLVDS_720P_120FPS:
        case PANASONIC_MN34220_MIPI_720P_120FPS:
        case SONY_IMX117_LVDS_720P_240FPS:
            pstVpssAttr->stVpssGrpAttr.u32MaxW = 1280;
            pstVpssAttr->stVpssGrpAttr.u32MaxH = 720;
            break;

        case APTINA_MT9P006_DC_1080P_30FPS:
        case PANASONIC_MN34220_SUBLVDS_1080P_30FPS:
        case PANASONIC_MN34220_MIPI_1080P_30FPS:
        case SONY_IMX178_LVDS_1080P_30FPS:
        case SONY_IMX185_MIPI_1080P_30FPS:
        case SONY_IMX290_MIPI_1080P_30FPS:
        case SONY_IMX122_DC_1080P_30FPS:
        case APTINA_AR0330_MIPI_1080P_30FPS:
        case OMNIVISION_OV4689_MIPI_1080P_30FPS:
        case SONY_IMX117_LVDS_1080P_120FPS:
        case APTINA_AR0230_HISPI_1080P_30FPS:
            pstVpssAttr->stVpssGrpAttr.u32MaxW  = 1920;
            pstVpssAttr->stVpssGrpAttr.u32MaxH = 1080;
            break;

        case APTINA_AR0330_MIPI_1296P_25FPS:
            pstVpssAttr->stVpssGrpAttr.u32MaxW  = 2304;
            pstVpssAttr->stVpssGrpAttr.u32MaxH = 1296;
            break;

        case APTINA_AR0330_MIPI_1536P_25FPS:
            pstVpssAttr->stVpssGrpAttr.u32MaxW  = 2048;
            pstVpssAttr->stVpssGrpAttr.u32MaxH = 1536;
            break;

        case OMNIVISION_OV4689_MIPI_4M_30FPS:
            pstVpssAttr->stVpssGrpAttr.u32MaxW  = 2592;
            pstVpssAttr->stVpssGrpAttr.u32MaxH = 1520;
            break;

        case SONY_IMX178_LVDS_5M_30FPS:
        case OMNIVISION_OV5658_MIPI_5M_30FPS:
            pstVpssAttr->stVpssGrpAttr.u32MaxW  = 2592;
            pstVpssAttr->stVpssGrpAttr.u32MaxH = 1944;
            break;
        case SONY_IMX226_LVDS_4K_30FPS:
	    case SONY_IMX226_LVDS_4K_60FPS:
        case SONY_IMX274_LVDS_4K_30FPS:
        case SONY_IMX117_LVDS_4K_30FPS:
            pstVpssAttr->stVpssGrpAttr.u32MaxW  = 3840;
            pstVpssAttr->stVpssGrpAttr.u32MaxH = 2160;
            break;
		case SONY_IMX226_LVDS_9M_30FPS:
			pstVpssAttr->stVpssGrpAttr.u32MaxW  = 3000;
            pstVpssAttr->stVpssGrpAttr.u32MaxH = 3000;
            break;

		case SONY_IMX226_LVDS_12M_30FPS:
			pstVpssAttr->stVpssGrpAttr.u32MaxW = 4000;
            pstVpssAttr->stVpssGrpAttr.u32MaxH = 3000;
            break;

        default:
            pstVpssAttr->stVpssGrpAttr.u32MaxW  = 1920;
            pstVpssAttr->stVpssGrpAttr.u32MaxH = 1080;
            break;
    }
    pstVpssAttr->stVpssChnMode.bDouble = HI_FALSE;
    pstVpssAttr->stVpssChnMode.enChnMode = VPSS_CHN_MODE_USER;
    pstVpssAttr->stVpssChnMode.enCompressMode = COMPRESS_MODE_NONE;
    pstVpssAttr->stVpssChnMode.enPixelFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    pstVpssAttr->stVpssChnMode.u32Height = pstVpssAttr->stVpssGrpAttr.u32MaxH;
    pstVpssAttr->stVpssChnMode.u32Width = pstVpssAttr->stVpssGrpAttr.u32MaxW;

    pstVpssAttr->stVpssChnAttr.bBorderEn= HI_FALSE;
    pstVpssAttr->stVpssChnAttr.bFlip = HI_FALSE;
    pstVpssAttr->stVpssChnAttr.bMirror = HI_FALSE;
    pstVpssAttr->stVpssChnAttr.bSpEn = HI_FALSE;
    pstVpssAttr->stVpssChnAttr.s32DstFrameRate = -1;
    pstVpssAttr->stVpssChnAttr.s32SrcFrameRate = -1;

    pstVpssAttr->stVpssExtChnAttr.enCompressMode = pstVpssAttr->stVpssChnMode.enCompressMode;
    pstVpssAttr->stVpssExtChnAttr.enPixelFormat = pstVpssAttr->stVpssChnMode.enPixelFormat;
    pstVpssAttr->stVpssExtChnAttr.s32BindChn = pstVpssAttr->VpssChn;
    pstVpssAttr->stVpssExtChnAttr.s32DstFrameRate = -1;
    pstVpssAttr->stVpssExtChnAttr.s32SrcFrameRate = -1;
    pstVpssAttr->stVpssExtChnAttr.u32Height = pstVpssAttr->stVpssChnMode.u32Height;
    pstVpssAttr->stVpssExtChnAttr.u32Width = pstVpssAttr->stVpssChnMode.u32Width;
}
/******************************************************************************
* function : show usage
******************************************************************************/
void SAMPLE_VIO_FISHEYE_Usage(char* sPrgNm)
{
    printf("Usage : %s <index> <vo intf> <venc type>\n", sPrgNm);
    printf("index:\n");
    printf("\t 0)online/offline fisheye 360 panorama 2 half with ceiling mount.\n");
    printf("\t 1)online/offline fisheye 360 panorama and 2 normal PTZ with desktop mount.\n");
    printf("\t 2)online/offline fisheye 180 panorama and 2 normal dynamic PTZ with wall mount.\n");
    printf("\t 3)online/offline fisheye source picture and 3 normal PTZ with wall mount.\n");
	printf("\t 4)offline nine_lattice preview(Only images larger than or equal to 8M are supported).\n");

    printf("vo intf:\n");
    printf("\t 0) vo cvbs output, default.\n");
    printf("\t 1) vo BT1120 output.\n");

    printf("venc type:\n");
    printf("\t 0) H264, default.\n");
    printf("\t 1) H265.\n");
    return;
}

/******************************************************************************
* function : to process abnormal case
******************************************************************************/
void SAMPLE_VIO_FISHEYE_HandleSig(HI_S32 signo)
{
    if (SIGINT == signo || SIGTERM == signo)
    {
    	bSetFisheyeAttr = HI_FALSE;
        SAMPLE_COMM_ISP_Stop();
        SAMPLE_COMM_SYS_Exit();
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
    }
    exit(-1);
}

static HI_S32 SAMPLE_VIO_FISHEYE_StartVO(void)
{
    VO_DEV VoDev = SAMPLE_VO_DEV_DSD0;
    VO_LAYER VoLayer = 0;
    SAMPLE_VO_MODE_E enVoMode = VO_MODE_1MUX;
    VO_PUB_ATTR_S stVoPubAttr;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;
    HI_S32 s32Ret = HI_SUCCESS;

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

    /* In HD, this item should be set to HI_FALSE */

    s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, &stVoPubAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VO_StartDev failed!\n");
        return s32Ret;
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
        SAMPLE_COMM_VO_StopDev(VoDev);
        return s32Ret;
    }

    stLayerAttr.stImageSize.u32Width  = stLayerAttr.stDispRect.u32Width;
    stLayerAttr.stImageSize.u32Height = stLayerAttr.stDispRect.u32Height;

    s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stLayerAttr, HI_FALSE);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VO_StartChn failed!\n");
        SAMPLE_COMM_VO_StopDev(VoDev);
        return s32Ret;
    }

    s32Ret = SAMPLE_COMM_VO_StartChn(VoDev, enVoMode);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VO_StartChn failed!\n");
        SAMPLE_COMM_VO_StopLayer(VoLayer);
        SAMPLE_COMM_VO_StopDev(VoDev);
        return s32Ret;
    }

    return HI_SUCCESS;
}

static HI_S32 SAMPLE_VIO_FISHEYE_StopVO(void)
{
    VO_DEV VoDev = SAMPLE_VO_DEV_DSD0;
    VO_LAYER VoLayer = 0;
    SAMPLE_VO_MODE_E enVoMode = VO_MODE_1MUX;
    
    SAMPLE_COMM_VO_StopChn(VoDev, enVoMode);
    SAMPLE_COMM_VO_StopLayer(VoLayer);
    SAMPLE_COMM_VO_StopDev(VoDev);

    return HI_SUCCESS;
}

static HI_S32 SAMPLE_VIO_FISHEYE_StartVPSS(SAMPLE_VPSS_ATTR_S* pstVpssAttr, VPSS_CHN VpssExtChn)
{
    VPSS_GRP VpssGrp = 0;
	VPSS_CHN VpssChn = 0;
    VPSS_GRP_ATTR_S *pstVpssGrpAttr = HI_NULL;
    VPSS_CHN_ATTR_S *pstVpssChnAttr = HI_NULL;
    VPSS_CHN_MODE_S *pstVpssChnMode = HI_NULL;
    VPSS_EXT_CHN_ATTR_S *pstVpssExtChnAttr = HI_NULL;
    HI_S32 s32Ret = HI_SUCCESS;

    if (HI_NULL == pstVpssAttr)
    {
        SAMPLE_PRT("VPSS configs is NULL!\n");
        return HI_FAILURE;
    }    
    else
    {
    	VpssGrp        = pstVpssAttr->VpssGrp;
		VpssChn        = pstVpssAttr->VpssChn;
        pstVpssGrpAttr = &pstVpssAttr->stVpssGrpAttr;
        pstVpssChnAttr = &pstVpssAttr->stVpssChnAttr;
        pstVpssChnMode = &pstVpssAttr->stVpssChnMode;
        pstVpssExtChnAttr = &pstVpssAttr->stVpssExtChnAttr;
    }
    
    s32Ret = SAMPLE_COMM_VPSS_StartGroup(VpssGrp, pstVpssGrpAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start VPSS GROUP failed!\n");
        return s32Ret;
    }    

    s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp, VpssChn, pstVpssChnAttr, pstVpssChnMode, pstVpssExtChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start VPSS CHN failed!\n");
        SAMPLE_COMM_VPSS_StopGroup(VpssGrp);
        return s32Ret;
    }

	s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp, VpssExtChn, pstVpssChnAttr, pstVpssChnMode, pstVpssExtChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start VPSS ext CHN failed!\n");
		SAMPLE_COMM_VPSS_DisableChn(VpssGrp, VpssChn);
        SAMPLE_COMM_VPSS_StopGroup(VpssGrp);
        return s32Ret;
    }

    return s32Ret;
}

static HI_S32 SAMPLE_VIO_FISHEYE_StopVPSS(SAMPLE_VPSS_ATTR_S* pstVpssAttr, VPSS_CHN VpssExtChn)
{
    VPSS_GRP VpssGrp = 0;
    VPSS_CHN VpssChn = 0;

	if (HI_NULL != pstVpssAttr)
    {
    	VpssGrp        = pstVpssAttr->VpssGrp;
		VpssChn        = pstVpssAttr->VpssChn;
    }

	SAMPLE_COMM_VPSS_DisableChn(VpssGrp, VpssExtChn);
    SAMPLE_COMM_VPSS_DisableChn(VpssGrp, VpssChn);
    SAMPLE_COMM_VPSS_StopGroup(VpssGrp);

    return HI_SUCCESS;
}

/******************************************************************************
* function : venc bind vi
******************************************************************************/
static HI_S32 SAMPLE_VIU_FISHEYE_BindVenc(VENC_CHN VeChn, VI_DEV ViDev, VI_CHN ViExtChn)
{
    HI_S32 s32Ret = HI_SUCCESS;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId = HI_ID_VIU;
    stSrcChn.s32DevId = ViDev;
    stSrcChn.s32ChnId = ViExtChn;

    stDestChn.enModId = HI_ID_VENC;
    stDestChn.s32DevId = 0;
    stDestChn.s32ChnId = VeChn;

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return s32Ret;
}


/******************************************************************************
* function : venc unbind vi
******************************************************************************/
HI_S32 SAMPLE_VIU_FISHEYE_UnBindVenc(VENC_CHN VeChn, VI_DEV ViDev, VI_CHN ViExtChn)
{
    HI_S32 s32Ret = HI_SUCCESS;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId = HI_ID_VPSS;
    stSrcChn.s32DevId = ViDev;
    stSrcChn.s32ChnId = ViExtChn;

    stDestChn.enModId = HI_ID_VENC;
    stDestChn.s32DevId = 0;
    stDestChn.s32ChnId = VeChn;

    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return s32Ret;
}


static HI_S32 SAMPLE_VIO_FISHEYE_SysBind(SAMPLE_VI_CONFIG_S *pstViConfig, VI_CHN ViChn, VPSS_CHN VpssChn)
{
    VPSS_GRP VpssGrp = 0;
    VO_DEV   VoDev   = SAMPLE_VO_DEV_DSD0;
    VO_CHN   VoChn   = 0;
    HI_S32  s32Ret; 
    
    if (SAMPLE_COMM_IsViVpssOnline())       // vi-vpss online
    {
        s32Ret = SAMPLE_COMM_VI_BindVpss(pstViConfig->enViMode);   // VI --> VPSS
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("VI bind VPSS (vpss:%d)-(ViChn:%d) failed with %#x!\n", VpssChn, ViChn, s32Ret);
            return s32Ret;
        }
        
        s32Ret = SAMPLE_COMM_VO_BindVpss(VoDev, VoChn, VpssGrp, VpssChn);   // VPSS --> VO
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("VI bind VPSS (vpss:%d)-(ViChn:%d) failed with %#x!\n", VpssChn, ViChn, s32Ret);
            return s32Ret;
        }
    }
    else
    {
        s32Ret = SAMPLE_COMM_VO_BindVi(VoDev, VoChn, ViChn);    // VI --> VO
    }

    return HI_SUCCESS;
}

static HI_S32 SAMPLE_VIO_FISHEYE_SysUnBind(SAMPLE_VI_CONFIG_S *pstViConfig, VI_CHN ViChn, VPSS_CHN VpssChn, VENC_CHN VeChn)
{
    VPSS_GRP VpssGrp = 0;
    VO_DEV   VoDev   = SAMPLE_VO_DEV_DSD0;
    VO_CHN   VoChn   = 0;
    VI_DEV   ViDev 	= 0;

	SAMPLE_VIU_FISHEYE_UnBindVenc(VeChn, ViDev, ViChn);
    SAMPLE_COMM_VENC_UnBindVpss(VeChn, VpssGrp, VpssChn);
    SAMPLE_COMM_VO_UnBindVpss(VoDev, VoChn, VpssGrp, VpssChn);
    SAMPLE_COMM_VI_UnBindVpss(pstViConfig->enViMode);
    SAMPLE_COMM_VO_UnBindVi(VoDev, ViChn);

    return HI_SUCCESS;
}

/******************************************************************************
 * Function:    SAMPLE_VIO_FISHEYE_StartViVo
 * Description: online mode / offline mode. Embeded isp, phychn preview
******************************************************************************/
static HI_S32 SAMPLE_VIO_FISHEYE_StartViVo(SAMPLE_VI_CONFIG_S* pstViConfig, SAMPLE_VPSS_ATTR_S* pstVpssAttr, VI_CHN ViExtChn, VPSS_CHN VpssExtChn, VENC_CHN VencChn)
{
    HI_BOOL  bViVpssOnline = HI_FALSE;
    HI_BOOL  bStartVpss = HI_FALSE;
    HI_S32   s32Ret = HI_SUCCESS;
	VI_CHN_ATTR_S 		stChnAttr;
	VI_EXT_CHN_ATTR_S	stExtChnAttr;
    VENC_GOP_ATTR_S     stGopAttr;
	VI_CHN   ViChn   	= 0;
	HI_U32   u32Profile = 0;
    
    HI_ASSERT(HI_NULL != pstViConfig);
    HI_ASSERT(HI_NULL != pstVpssAttr);
    /******************************************
     step 1: start vi dev & chn to capture
    ******************************************/
	memset(&stChnAttr,0,sizeof(VI_CHN_ATTR_S));
	
    SAMPLE_VPSS_FISHEYE_GetVpssConfig(pstVpssAttr);
    
    s32Ret = SAMPLE_COMM_VI_StartVi(pstViConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        return s32Ret;
    }

	s32Ret = HI_MPI_VI_GetChnAttr(ViChn, &stChnAttr);
	if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("get vi chn:%d attr failed with:0x%x!\n", ViChn, s32Ret);
        return s32Ret;
    }
	
	stExtChnAttr.s32BindChn           = ViChn;
    stExtChnAttr.enCompressMode       = stChnAttr.enCompressMode;
    stExtChnAttr.enPixFormat          = stChnAttr.enPixFormat;
    stExtChnAttr.s32SrcFrameRate      = stChnAttr.s32SrcFrameRate;
    stExtChnAttr.s32DstFrameRate      = stChnAttr.s32DstFrameRate;
    stExtChnAttr.stDestSize.u32Width  = stChnAttr.stDestSize.u32Width;
    stExtChnAttr.stDestSize.u32Height = stChnAttr.stDestSize.u32Height;

	bViVpssOnline = SAMPLE_COMM_IsViVpssOnline();

	if (HI_FALSE == bViVpssOnline)
	{
		/* start vi dev extern chn */
		s32Ret = HI_MPI_VI_SetExtChnAttr(ViExtChn, &stExtChnAttr);
		if (HI_SUCCESS != s32Ret)
	    {
	        SAMPLE_PRT("set vi extern chn attr failed with: 0x%x.!\n", s32Ret);
	        return s32Ret;
	    }
		
		s32Ret = HI_MPI_VI_EnableChn(ViExtChn);
		if (HI_SUCCESS != s32Ret)
	    {
	        SAMPLE_PRT("enable vi extern chn failed with: 0x%x.!\n", s32Ret);
	        return s32Ret;
	    }
	}

    /******************************************
    step 2: start VENC
    ******************************************/
    s32Ret = SAMPLE_COMM_VENC_GetGopAttr(VENC_GOPMODE_NORMALP,&stGopAttr,gs_enNorm);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_VIO Get GopAttr failed!\n");
        goto exit1;
    }

    s32Ret = SAMPLE_COMM_VENC_Start(VencChn, g_enVencType, \
	                                    gs_enNorm, g_enPicSize, g_enRcMode,u32Profile,&stGopAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_VIO start VENC failed with %#x!\n", s32Ret);
        goto exit1;
    }
    
    /******************************************
    step 3: start VO SD0
    ******************************************/
    s32Ret = SAMPLE_VIO_FISHEYE_StartVO();
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_VIO start VO failed with %#x!\n", s32Ret);
        goto exit1;
    }

    /******************************************
    step 4: start VPSS
    ******************************************/
    if (bViVpssOnline)
    {
        s32Ret = SAMPLE_VIO_FISHEYE_StartVPSS(pstVpssAttr, VpssExtChn);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("SAMPLE_VIO_StartVPSS failed with %#x!\n", s32Ret);
            goto exit2;
        }
        s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, pstVpssAttr->VpssGrp, VpssExtChn);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("SAMPLE_COMM_VENC_BindVpss failed with %#x!\n", s32Ret);
            goto exit3;
        }
        bStartVpss = HI_TRUE;
    }
	else
    {
		s32Ret = SAMPLE_VIU_FISHEYE_BindVenc(VencChn, 0, ViExtChn);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("SAMPLE_VIU_FISHEYE_BindVenc failed with %#x!\n", s32Ret);
            goto exit2;
        }
    }
    
    s32Ret = SAMPLE_VIO_FISHEYE_SysBind(pstViConfig, ViExtChn, VpssExtChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("VIO sys bind failed with %#x!\n", s32Ret);
        goto exit3; 
    }

    return s32Ret;

exit3:
    if (bStartVpss)
    {
        SAMPLE_VIO_FISHEYE_StopVPSS(pstVpssAttr, VpssExtChn);
    }
exit2:
    SAMPLE_VIO_FISHEYE_StopVO();
    SAMPLE_COMM_VENC_Stop(VencChn);
exit1:
	s32Ret = HI_MPI_VI_DisableChn(ViExtChn);
	if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VI_DisableChn extchn:%d failed with %#x\n", ViExtChn, s32Ret);
    }
    SAMPLE_COMM_VI_StopVi(pstViConfig);
    return s32Ret;
}

HI_S32 SAMPLE_VIO_FISHEYE_StopViVo(SAMPLE_VI_CONFIG_S* pstViConfig, SAMPLE_VPSS_ATTR_S* pstVpssAttr, VI_CHN ViExtChn, VPSS_CHN VpssExtChn, VENC_CHN VencChn)
{
	HI_S32   s32Ret 	= HI_SUCCESS;
	HI_BOOL  bViVpssOnline = HI_FALSE;

	bViVpssOnline = SAMPLE_COMM_IsViVpssOnline();
	
    SAMPLE_VIO_FISHEYE_SysUnBind(pstViConfig, ViExtChn, VpssExtChn,VencChn);

	if (bViVpssOnline)
    {
        s32Ret = SAMPLE_VIO_FISHEYE_StopVPSS(pstVpssAttr, VpssExtChn);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("SAMPLE_VIO_StopVPSS failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        }
    }

    SAMPLE_VIO_FISHEYE_StopVO();
	SAMPLE_COMM_VENC_Stop(VencChn);
    
	s32Ret = HI_MPI_VI_DisableChn(ViExtChn);
	if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VI_DisableChn extchn:%d failed with %#x\n", ViExtChn, s32Ret);
        return HI_FAILURE;
    }
	
    SAMPLE_COMM_VI_StopVi(pstViConfig);

    return HI_SUCCESS;
}

HI_S32 SAMPLE_VIO_FISHEYE_SetFisheyeConfig(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VI_DEV ViDev, FISHEYE_CONFIG_S *pstFisheyeConfig)
{
	HI_S32 s32Ret 			= HI_SUCCESS;
	HI_U32 u32ViVpssMode 	= HI_FALSE;
	
	s32Ret = HI_MPI_SYS_GetViVpssMode(&u32ViVpssMode);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("get vi/vpss mode failed with s32Ret:0x%x!\n", s32Ret);
	    return s32Ret; 
	}
	
	if (u32ViVpssMode)
    {
    	s32Ret =  HI_MPI_VPSS_SetFisheyeConfig(VpssGrp,  pstFisheyeConfig);
		if (HI_SUCCESS != s32Ret)
	    {
	        SAMPLE_PRT("set vpss fisheye config failed with s32Ret:0x%x!\n", s32Ret);
	        return s32Ret;   
	    }        
    }
    else
    {
    	s32Ret =  HI_MPI_VI_SetFisheyeDevConfig(ViDev, pstFisheyeConfig);
		if (HI_SUCCESS != s32Ret)
	    {
	        SAMPLE_PRT("set vi fisheye config failed with s32Ret:0x%x!\n", s32Ret);
	        return s32Ret;   
	    }        	
    }

	return HI_SUCCESS;
}

HI_S32 SAMPLE_VIO_FISHEYE_SetFisheyeAttr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VI_CHN ViChn, FISHEYE_ATTR_S *pstFisheyeAttr)
{
	HI_S32 s32Ret 			= HI_SUCCESS;
	HI_U32 u32ViVpssMode 	= HI_FALSE;
	
	s32Ret = HI_MPI_SYS_GetViVpssMode(&u32ViVpssMode);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("get vi/vpss mode failed with s32Ret:0x%x!\n", s32Ret);
	    return s32Ret; 
	}
	
	if (u32ViVpssMode)
    {
        s32Ret =  HI_MPI_VPSS_SetFisheyeAttr(VpssGrp, VpssChn,  pstFisheyeAttr);
		if (HI_SUCCESS != s32Ret)
	    {
	        SAMPLE_PRT("set vpss fisheye attr failed with s32Ret:0x%x!\n", s32Ret);
	        return s32Ret;   
	    }
    }
    else
    {
        s32Ret =  HI_MPI_VI_SetFisheyeAttr(ViChn, pstFisheyeAttr);
		if (HI_SUCCESS != s32Ret)
	    {
	        SAMPLE_PRT("set vi fisheye attr failed with s32Ret:0x%x!\n", s32Ret);
	        return s32Ret;  
	    }	
    }

	return HI_SUCCESS;
}

static HI_VOID *Proc_SetFisheyeAttrThread(HI_VOID *arg)
{
	HI_S32 		i 		= 0;
	VPSS_GRP 	VpssGrp = 0;
	VPSS_CHN 	VpssChn = 0;
    VI_CHN 		ViChn 	= 0;
	HI_S32 		s32Ret 	= HI_SUCCESS;
	HI_U32 		u32ViVpssMode 	= HI_FALSE;
	FISHEYE_ATTR_S				stFisheyeAttr;
    FISHEYE_SET_ATTR_THRD_INFO 	*pstFisheyeAttrThreadInfo = HI_NULL;
	
    if(HI_NULL == arg)
    {
        printf("arg is NULL\n");
        return HI_NULL;
    }

	memset(&stFisheyeAttr,0,sizeof(FISHEYE_ATTR_S));

	pstFisheyeAttrThreadInfo = (FISHEYE_SET_ATTR_THRD_INFO *)arg;

	VpssGrp = pstFisheyeAttrThreadInfo->VpssGrp;
	VpssChn = pstFisheyeAttrThreadInfo->VpssChn;
	ViChn 	= pstFisheyeAttrThreadInfo->ViChn;
		
    while (HI_TRUE == bSetFisheyeAttr)
    {	
		s32Ret = HI_MPI_SYS_GetViVpssMode(&u32ViVpssMode);
		if (HI_SUCCESS != s32Ret)
		{
			return HI_NULL;
		}
		
		if (u32ViVpssMode)
    	{
			s32Ret = HI_MPI_VPSS_GetFisheyeAttr(VpssGrp, VpssChn, &stFisheyeAttr);
			if (HI_SUCCESS != s32Ret)
			{
				return HI_NULL;
			}
		}
		else
		{
			s32Ret = HI_MPI_VI_GetFisheyeAttr(ViChn, &stFisheyeAttr);
			if (HI_SUCCESS != s32Ret)
			{
				return HI_NULL;
			}
		}
		
		for (i = 1; i < 3; i++)
		{
			if (360 == stFisheyeAttr.astFisheyeRegionAttr[i].u32Pan)
			{
				stFisheyeAttr.astFisheyeRegionAttr[i].u32Pan = 0;
			}
			else
			{
				stFisheyeAttr.astFisheyeRegionAttr[i].u32Pan++;
			}

			if (360 == stFisheyeAttr.astFisheyeRegionAttr[i].u32Tilt)
			{
				stFisheyeAttr.astFisheyeRegionAttr[i].u32Tilt = 0;
			}
			else
			{
				stFisheyeAttr.astFisheyeRegionAttr[i].u32Tilt++;
			}
		}

		if (u32ViVpssMode)
    	{
			s32Ret = HI_MPI_VPSS_SetFisheyeAttr(VpssGrp, VpssChn, &stFisheyeAttr);
			if (HI_SUCCESS != s32Ret)
			{
				return HI_NULL;
			}
		}
		else
		{
			s32Ret = HI_MPI_VI_SetFisheyeAttr(ViChn, &stFisheyeAttr);
			if (HI_SUCCESS != s32Ret)
			{
				return HI_NULL;
			}
		}		
		
		sleep(1);
		
	#if 0	
		printf("Func:%s, line:%d, pan1:%d, tilt1:%d, pan2:%d, tilt2:%d\n", __FUNCTION__, __LINE__, 
		stFisheyeAttr.astFisheyeRegionAttr[1].u32Pan, stFisheyeAttr.astFisheyeRegionAttr[1].u32Tilt,
		stFisheyeAttr.astFisheyeRegionAttr[2].u32Pan, stFisheyeAttr.astFisheyeRegionAttr[2].u32Tilt);
	#endif	
    }

    return HI_NULL;
}

HI_S32 SAMPLE_VIO_FISHEYE_StartSetFisheyeAttrThrd(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VI_CHN ViChn)
{
	FISHEYE_SET_ATTR_THRD_INFO stFisheyeAttrThreadInfo;

	stFisheyeAttrThreadInfo.VpssGrp = VpssGrp;
	stFisheyeAttrThreadInfo.VpssChn = VpssChn;
	stFisheyeAttrThreadInfo.ViChn   = ViChn;

	bSetFisheyeAttr = HI_TRUE;
	
    pthread_create(&ThreadId, HI_NULL, Proc_SetFisheyeAttrThread, &stFisheyeAttrThreadInfo);
	
    sleep(1);
	
    return HI_SUCCESS;
}

HI_S32 SAMPLE_VIO_FISHEYE_StopSetFisheyeAttrThrd()
{
    if (HI_FALSE != bSetFisheyeAttr)
    {
        bSetFisheyeAttr = HI_FALSE;
        pthread_join(ThreadId, HI_NULL);
    } 
	
    return HI_SUCCESS;
}

static HI_VOID *SAMPLE_VIO_Fisheye_nine_lattice_Thread(HI_VOID *arg)
{
    HI_S32				s32Ret = HI_SUCCESS;
	FISHEYE_HANDLE		hHandle;
    FISHEYE_TASK_ATTR_S	stTask;
	VGS_TASK_ATTR_S		stVgsTask;
	FISHEYE_ATTR_S		stFisheyeAttr;
	HI_U32				u32BufSize;
	HI_CHAR				*pMmzName = HI_NULL;
	HI_U32				u32OutPhyAddr;
    HI_U8				*pu8OutVirtAddr;
    VB_BLK				vbOutBlk;
	HI_U32				u32Width;
	HI_U32				u32Height;
	HI_U32				u32Stride;
    HI_U32				u32OutWidth ;
	HI_U32				u32OutHeight;
	HI_U32				u32OutStride;
	VI_CHN				ViChn = 0;
	HI_U32				u32OldDepth = 2;
	HI_U32				u32Depth = 2;
	VO_LAYER			VoLayer = 0;
	VO_CHN				VoChn = 0;
	HI_S32				s32MilliSec = -1;
	SIZE_S				*pstSize;
	
    if(NULL == arg)
    {
        SAMPLE_PRT("arg is NULL\n");
        return NULL;
    }	
	pstSize = (SIZE_S *)arg;

	u32Width = pstSize->u32Width;
	u32Height = pstSize->u32Height;
	u32Stride = ALIGN(pstSize->u32Width, 16);

	u32OutWidth = pstSize->u32Width;
	u32OutHeight = pstSize->u32Height;
	u32OutStride= ALIGN(pstSize->u32Width, 16);

	s32Ret = HI_MPI_VI_GetFrameDepth(ViChn,&u32OldDepth);
	if(HI_SUCCESS != s32Ret)
	{
	    SAMPLE_PRT("Err, s32Ret:0x%x\n",s32Ret);
	}
	
	s32Ret = HI_MPI_VI_SetFrameDepth(ViChn,u32Depth);
	if(HI_SUCCESS != s32Ret)
	{
	    SAMPLE_PRT("Err, s32Ret:0x%x\n",s32Ret);
	}
	
    while ( HI_TRUE == bSetFisheyeAttr )
    {	
	    s32Ret = HI_MPI_FISHEYE_BeginJob(&hHandle);
		if(s32Ret)
	    {
	        SAMPLE_PRT("Err, s32Ret:0x%x\n",s32Ret);
			return HI_NULL;
	    }

		u32BufSize = u32Stride * u32Height * 3 / 2;
		

		u32BufSize = u32OutStride * u32OutHeight * 3 / 2;
		vbOutBlk = HI_MPI_VB_GetBlock(VB_INVALID_POOLID, u32BufSize, pMmzName);  
		if (VB_INVALID_HANDLE == vbOutBlk)
		{
			SAMPLE_PRT("Info:HI_MPI_VB_GetBlock(size:%d) fail\n", u32BufSize);
			return HI_NULL;
		}

		u32OutPhyAddr = HI_MPI_VB_Handle2PhysAddr(vbOutBlk);
		if (0 == u32OutPhyAddr)
		{
			SAMPLE_PRT("Info:HI_MPI_VB_Handle2PhysAddr fail, u32OutPhyAddr:0x%x\n",u32OutPhyAddr);
			HI_MPI_VB_ReleaseBlock(vbOutBlk);
			return HI_NULL;
		}

		pu8OutVirtAddr = (HI_U8 *)HI_MPI_SYS_Mmap(u32OutPhyAddr, u32BufSize);
		if (NULL == pu8OutVirtAddr)
		{
			
			SAMPLE_PRT("Info:HI_MPI_SYS_Mmap fail, u8OutVirtAddr:0x%x\n",(HI_U32)pu8OutVirtAddr);
			HI_MPI_VB_ReleaseBlock(vbOutBlk);
			return HI_NULL;
		}



		s32Ret = HI_MPI_VI_GetFrame(ViChn,&stTask.stImgIn,s32MilliSec);
		if(HI_SUCCESS != s32Ret)
	    {
	        SAMPLE_PRT("Err, s32Ret:0x%x\n",s32Ret);
			HI_MPI_VB_ReleaseBlock(vbOutBlk);
			return HI_NULL;
	    }
		
		memcpy(&stTask.stImgOut, &stTask.stImgIn, sizeof(VIDEO_FRAME_INFO_S));

		stTask.stImgOut.u32PoolId 			   = HI_MPI_VB_Handle2PoolId(vbOutBlk);
		stTask.stImgOut.stVFrame.u32PhyAddr[0] = u32OutPhyAddr;
		stTask.stImgOut.stVFrame.u32PhyAddr[1] = u32OutPhyAddr + u32OutStride * u32OutHeight;
		stTask.stImgOut.stVFrame.u32PhyAddr[2] = u32OutPhyAddr + u32OutStride * u32OutHeight;
		stTask.stImgOut.stVFrame.pVirAddr[0]   = pu8OutVirtAddr;
		stTask.stImgOut.stVFrame.pVirAddr[1]   = pu8OutVirtAddr + u32OutStride * u32OutHeight;
		stTask.stImgOut.stVFrame.pVirAddr[2]   = pu8OutVirtAddr + u32OutStride * u32OutHeight;
		stTask.stImgOut.stVFrame.u32Stride[0]   = u32OutStride;
		stTask.stImgOut.stVFrame.u32Stride[1]   = u32OutStride;
		stTask.stImgOut.stVFrame.u32Stride[2]   = u32OutStride;	
        stTask.stImgOut.stVFrame.u32Width	   = u32OutWidth;
		stTask.stImgOut.stVFrame.u32Height	   = u32OutHeight;
        
		stFisheyeAttr.bEnable = HI_TRUE;
		stFisheyeAttr.bLMF = HI_FALSE;
		stFisheyeAttr.bBgColor = HI_TRUE;
		stFisheyeAttr.u32BgColor = 0xFC;
		stFisheyeAttr.s32HorOffset = 0;
		stFisheyeAttr.s32VerOffset = 0;
		stFisheyeAttr.u32TrapezoidCoef = 0;
		stFisheyeAttr.s32FanStrength = 0;
		stFisheyeAttr.enMountMode = FISHEYE_WALL_MOUNT;
		stFisheyeAttr.u32RegionNum = 4;

		stFisheyeAttr.astFisheyeRegionAttr[0].enViewMode = FISHEYE_VIEW_NORMAL;
		stFisheyeAttr.astFisheyeRegionAttr[0].u32InRadius = 0;
		stFisheyeAttr.astFisheyeRegionAttr[0].u32OutRadius = 640;
		stFisheyeAttr.astFisheyeRegionAttr[0].u32Pan = 180;
		stFisheyeAttr.astFisheyeRegionAttr[0].u32Tilt = 180;
		stFisheyeAttr.astFisheyeRegionAttr[0].u32HorZoom = 4095;
		stFisheyeAttr.astFisheyeRegionAttr[0].u32VerZoom = 4095;
		stFisheyeAttr.astFisheyeRegionAttr[0].stOutRect.s32X = 0;
		stFisheyeAttr.astFisheyeRegionAttr[0].stOutRect.s32Y = 0;
		stFisheyeAttr.astFisheyeRegionAttr[0].stOutRect.u32Width = ALIGN_BACK(u32Width/3,2);
		stFisheyeAttr.astFisheyeRegionAttr[0].stOutRect.u32Height = u32Height/3;

		stFisheyeAttr.astFisheyeRegionAttr[1].enViewMode = FISHEYE_VIEW_180_PANORAMA;
		stFisheyeAttr.astFisheyeRegionAttr[1].u32InRadius = 0;
		stFisheyeAttr.astFisheyeRegionAttr[1].u32OutRadius = 640;
		stFisheyeAttr.astFisheyeRegionAttr[1].u32Pan = 180;
		stFisheyeAttr.astFisheyeRegionAttr[1].u32Tilt = 180;
		stFisheyeAttr.astFisheyeRegionAttr[1].u32HorZoom = 4095;
		stFisheyeAttr.astFisheyeRegionAttr[1].u32VerZoom = 4095;
		stFisheyeAttr.astFisheyeRegionAttr[1].stOutRect.s32X = ALIGN_BACK(u32Width/3,16);
		stFisheyeAttr.astFisheyeRegionAttr[1].stOutRect.s32Y = 0;
		stFisheyeAttr.astFisheyeRegionAttr[1].stOutRect.u32Width = ALIGN_BACK(u32Width/3,2);
		stFisheyeAttr.astFisheyeRegionAttr[1].stOutRect.u32Height = ALIGN_BACK(u32Height/3,2);        

		stFisheyeAttr.astFisheyeRegionAttr[2].enViewMode = FISHEYE_VIEW_360_PANORAMA;
		stFisheyeAttr.astFisheyeRegionAttr[2].u32InRadius = 0;
		stFisheyeAttr.astFisheyeRegionAttr[2].u32OutRadius = 640;
		stFisheyeAttr.astFisheyeRegionAttr[2].u32Pan = 180;
		stFisheyeAttr.astFisheyeRegionAttr[2].u32Tilt = 180;
		stFisheyeAttr.astFisheyeRegionAttr[2].u32HorZoom = 4095;
		stFisheyeAttr.astFisheyeRegionAttr[2].u32VerZoom = 4095;
		stFisheyeAttr.astFisheyeRegionAttr[2].stOutRect.s32X = ALIGN_BACK(u32Width*2/3,16);
		stFisheyeAttr.astFisheyeRegionAttr[2].stOutRect.s32Y = 0;
		stFisheyeAttr.astFisheyeRegionAttr[2].stOutRect.u32Width = ALIGN_BACK(u32Width/3,2);
		stFisheyeAttr.astFisheyeRegionAttr[2].stOutRect.u32Height = ALIGN_BACK(u32Height/3,2); 

		stFisheyeAttr.astFisheyeRegionAttr[3].enViewMode = FISHEYE_VIEW_NORMAL;
		stFisheyeAttr.astFisheyeRegionAttr[3].u32InRadius = 0;
		stFisheyeAttr.astFisheyeRegionAttr[3].u32OutRadius = 640;
		stFisheyeAttr.astFisheyeRegionAttr[3].u32Pan = 180;
		stFisheyeAttr.astFisheyeRegionAttr[3].u32Tilt = 180;
		stFisheyeAttr.astFisheyeRegionAttr[3].u32HorZoom = 4095;
		stFisheyeAttr.astFisheyeRegionAttr[3].u32VerZoom = 4095;
		stFisheyeAttr.astFisheyeRegionAttr[3].stOutRect.s32X = 0;
		stFisheyeAttr.astFisheyeRegionAttr[3].stOutRect.s32Y = ALIGN_BACK(u32Height/3,2);
		stFisheyeAttr.astFisheyeRegionAttr[3].stOutRect.u32Width = ALIGN_BACK(u32Width/3,2);
		stFisheyeAttr.astFisheyeRegionAttr[3].stOutRect.u32Height = ALIGN_BACK(u32Height/3,2); 
        
		s32Ret = HI_MPI_FISHEYE_AddCorrectionTask(hHandle, &stTask, &stFisheyeAttr);
		if(HI_SUCCESS != s32Ret)
	    {
	        SAMPLE_PRT("Err, s32Ret:0x%x\n",s32Ret);
            HI_MPI_VB_ReleaseBlock(vbOutBlk);
			return HI_NULL;
	    }

        
		stFisheyeAttr.bEnable = HI_TRUE;
		stFisheyeAttr.bLMF = HI_FALSE;
		stFisheyeAttr.bBgColor = HI_TRUE;
		stFisheyeAttr.u32BgColor = 0xFC;
		stFisheyeAttr.s32HorOffset = 0;
		stFisheyeAttr.s32VerOffset = 0;
		stFisheyeAttr.u32TrapezoidCoef = 0;
		stFisheyeAttr.s32FanStrength = 0;
		stFisheyeAttr.enMountMode = FISHEYE_WALL_MOUNT;
		stFisheyeAttr.u32RegionNum = 4;

		stFisheyeAttr.astFisheyeRegionAttr[0].enViewMode = FISHEYE_VIEW_NORMAL;
		stFisheyeAttr.astFisheyeRegionAttr[0].u32InRadius = 0;
		stFisheyeAttr.astFisheyeRegionAttr[0].u32OutRadius = 640;
		stFisheyeAttr.astFisheyeRegionAttr[0].u32Pan = 180;
		stFisheyeAttr.astFisheyeRegionAttr[0].u32Tilt = 180;
		stFisheyeAttr.astFisheyeRegionAttr[0].u32HorZoom = 4095;
		stFisheyeAttr.astFisheyeRegionAttr[0].u32VerZoom = 4095;
		stFisheyeAttr.astFisheyeRegionAttr[0].stOutRect.s32X = ALIGN_BACK(u32Width/3,16);
		stFisheyeAttr.astFisheyeRegionAttr[0].stOutRect.s32Y = ALIGN_BACK(u32Height/3,2);
		stFisheyeAttr.astFisheyeRegionAttr[0].stOutRect.u32Width = ALIGN_BACK(u32Width/3,2);
		stFisheyeAttr.astFisheyeRegionAttr[0].stOutRect.u32Height = ALIGN_BACK(u32Height/3,2);

		stFisheyeAttr.astFisheyeRegionAttr[1].enViewMode = FISHEYE_VIEW_180_PANORAMA;
		stFisheyeAttr.astFisheyeRegionAttr[1].u32InRadius = 0;
		stFisheyeAttr.astFisheyeRegionAttr[1].u32OutRadius = 640;
		stFisheyeAttr.astFisheyeRegionAttr[1].u32Pan = 180;
		stFisheyeAttr.astFisheyeRegionAttr[1].u32Tilt = 180;
		stFisheyeAttr.astFisheyeRegionAttr[1].u32HorZoom = 4095;
		stFisheyeAttr.astFisheyeRegionAttr[1].u32VerZoom = 4095;
		stFisheyeAttr.astFisheyeRegionAttr[1].stOutRect.s32X = ALIGN_BACK(u32Width*2/3,16);
		stFisheyeAttr.astFisheyeRegionAttr[1].stOutRect.s32Y = ALIGN_BACK(u32Height/3,2);
		stFisheyeAttr.astFisheyeRegionAttr[1].stOutRect.u32Width = ALIGN_BACK(u32Width/3,2);
		stFisheyeAttr.astFisheyeRegionAttr[1].stOutRect.u32Height = ALIGN_BACK(u32Height/3,2);        

		stFisheyeAttr.astFisheyeRegionAttr[2].enViewMode = FISHEYE_VIEW_360_PANORAMA;
		stFisheyeAttr.astFisheyeRegionAttr[2].u32InRadius = 0;
		stFisheyeAttr.astFisheyeRegionAttr[2].u32OutRadius = 640;
		stFisheyeAttr.astFisheyeRegionAttr[2].u32Pan = 180;
		stFisheyeAttr.astFisheyeRegionAttr[2].u32Tilt = 180;
		stFisheyeAttr.astFisheyeRegionAttr[2].u32HorZoom = 4095;
		stFisheyeAttr.astFisheyeRegionAttr[2].u32VerZoom = 4095;
		stFisheyeAttr.astFisheyeRegionAttr[2].stOutRect.s32X = 0;
		stFisheyeAttr.astFisheyeRegionAttr[2].stOutRect.s32Y = ALIGN_BACK(u32Height*2/3,2);
		stFisheyeAttr.astFisheyeRegionAttr[2].stOutRect.u32Width = ALIGN_BACK(u32Width/3,2);
		stFisheyeAttr.astFisheyeRegionAttr[2].stOutRect.u32Height = ALIGN_BACK(u32Height/3,2); 

		stFisheyeAttr.astFisheyeRegionAttr[3].enViewMode = FISHEYE_VIEW_NORMAL;
		stFisheyeAttr.astFisheyeRegionAttr[3].u32InRadius = 0;
		stFisheyeAttr.astFisheyeRegionAttr[3].u32OutRadius = 640;
		stFisheyeAttr.astFisheyeRegionAttr[3].u32Pan = 180;
		stFisheyeAttr.astFisheyeRegionAttr[3].u32Tilt = 180;
		stFisheyeAttr.astFisheyeRegionAttr[3].u32HorZoom = 4095;
		stFisheyeAttr.astFisheyeRegionAttr[3].u32VerZoom = 4095;
		stFisheyeAttr.astFisheyeRegionAttr[3].stOutRect.s32X = ALIGN_BACK(u32Width/3,16);
		stFisheyeAttr.astFisheyeRegionAttr[3].stOutRect.s32Y = ALIGN_BACK(u32Height*2/3,2);
		stFisheyeAttr.astFisheyeRegionAttr[3].stOutRect.u32Width = ALIGN_BACK(u32Width/3,2);
		stFisheyeAttr.astFisheyeRegionAttr[3].stOutRect.u32Height = ALIGN_BACK(u32Height/3,2); 
        
		s32Ret = HI_MPI_FISHEYE_AddCorrectionTask(hHandle, &stTask, &stFisheyeAttr);
		if(HI_SUCCESS != s32Ret)
	    {
			SAMPLE_PRT("Err, s32Ret:0x%x\n",s32Ret);
            HI_MPI_VB_ReleaseBlock(vbOutBlk);
			return HI_NULL;
	    }

		s32Ret = HI_MPI_FISHEYE_EndJob(hHandle);
		if(HI_SUCCESS != s32Ret)
	    {
			SAMPLE_PRT("Err, s32Ret:0x%x\n",s32Ret);
            HI_MPI_VB_ReleaseBlock(vbOutBlk);
			return HI_NULL;
	    }

		/*Add Scale Task*/
		s32Ret = HI_MPI_VGS_BeginJob(&hHandle);
		if(HI_SUCCESS != s32Ret)
        {
			SAMPLE_PRT("Err, s32Ret:0x%x\n",s32Ret);
            HI_MPI_VB_ReleaseBlock(vbOutBlk);
			return HI_NULL;

        }

		stVgsTask.stImgIn.u32PoolId 			   = stTask.stImgIn.u32PoolId;
		stVgsTask.stImgIn.stVFrame.u32Width 	   = u32Width;
		stVgsTask.stImgIn.stVFrame.u32Height	   = u32Height;
		stVgsTask.stImgIn.stVFrame.u32Field 	   = VIDEO_FIELD_FRAME;
		stVgsTask.stImgIn.stVFrame.enPixelFormat  = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
		stVgsTask.stImgIn.stVFrame.u32Stride[0]   = u32Stride;
		stVgsTask.stImgIn.stVFrame.u32Stride[1]   = u32Stride;
		stVgsTask.stImgIn.stVFrame.u32Stride[2]   = u32Stride;
		stVgsTask.stImgIn.stVFrame.u32PhyAddr[0]  = stTask.stImgIn.stVFrame.u32PhyAddr[0];
		stVgsTask.stImgIn.stVFrame.u32PhyAddr[1]  = stTask.stImgIn.stVFrame.u32PhyAddr[1] ;
		stVgsTask.stImgIn.stVFrame.u32PhyAddr[2]  = stTask.stImgIn.stVFrame.u32PhyAddr[2] ;
		stVgsTask.stImgIn.stVFrame.pVirAddr[0]	   = stTask.stImgIn.stVFrame.pVirAddr[0];
		stVgsTask.stImgIn.stVFrame.pVirAddr[1]	   = stTask.stImgIn.stVFrame.pVirAddr[1];
		stVgsTask.stImgIn.stVFrame.pVirAddr[2]	   = stTask.stImgIn.stVFrame.pVirAddr[0];
		stVgsTask.stImgIn.stVFrame.u64pts		   = 0;
		stVgsTask.stImgIn.stVFrame.u32TimeRef	   = 0;
		stVgsTask.stImgIn.stVFrame.enCompressMode = COMPRESS_MODE_NONE;
		stVgsTask.stImgIn.stVFrame.enVideoFormat  = VIDEO_FORMAT_LINEAR;
		

		stVgsTask.stImgOut.u32PoolId			   = HI_MPI_VB_Handle2PoolId(vbOutBlk);
        stVgsTask.stImgOut.stVFrame.u32Width	   = ALIGN_BACK(u32Width/3,2);
        stVgsTask.stImgOut.stVFrame.u32Height	   = ALIGN_BACK(u32Height/3,2);
        stVgsTask.stImgOut.stVFrame.u32Field	   = VIDEO_FIELD_FRAME;
        stVgsTask.stImgOut.stVFrame.enPixelFormat	= PIXEL_FORMAT_YUV_SEMIPLANAR_420;
        stVgsTask.stImgOut.stVFrame.u32Stride[0]	= u32OutStride;
        stVgsTask.stImgOut.stVFrame.u32Stride[1]	= u32OutStride;
        stVgsTask.stImgOut.stVFrame.u32Stride[2]	= u32OutStride;
        stVgsTask.stImgOut.stVFrame.u32PhyAddr[0]	= ALIGN_BACK((u32OutPhyAddr + u32OutStride * u32OutHeight*2/3+u32OutWidth*2/3),16);
        stVgsTask.stImgOut.stVFrame.u32PhyAddr[1]	= ALIGN_BACK((u32OutPhyAddr + u32OutStride * u32OutHeight + ((u32OutStride*u32OutHeight*2/3)/2 +u32OutWidth*2/3)),16);
        stVgsTask.stImgOut.stVFrame.u32PhyAddr[2]	= ALIGN_BACK((u32OutPhyAddr + u32OutStride * u32OutHeight + ((u32OutStride*u32OutHeight*2/3)/2 +u32OutWidth*2/3)),16);
		stVgsTask.stImgOut.stVFrame.pVirAddr[0]	   = (HI_VOID *)ALIGN_BACK(((HI_U32)(pu8OutVirtAddr + u32OutStride * u32OutHeight*2/3+u32OutWidth*2/3)),16);
        stVgsTask.stImgOut.stVFrame.pVirAddr[1]	   = (HI_VOID *)ALIGN_BACK((HI_U32)(pu8OutVirtAddr + u32OutStride * u32OutHeight + ((u32OutStride*u32OutHeight*2/3)/2 +u32OutWidth*2/3)),16);
        stVgsTask.stImgOut.stVFrame.pVirAddr[2]	   = (HI_VOID *)ALIGN_BACK((HI_U32)(pu8OutVirtAddr + u32OutStride * u32OutHeight + ((u32OutStride*u32OutHeight*2/3)/2 +u32OutWidth*2/3)),16);
		stVgsTask.stImgOut.stVFrame.u64pts 	   = 0;
        stVgsTask.stImgOut.stVFrame.u32TimeRef    = 0;
        stVgsTask.stImgOut.stVFrame.enCompressMode = COMPRESS_MODE_NONE;
        stVgsTask.stImgOut.stVFrame.enVideoFormat	= VIDEO_FORMAT_LINEAR;

		
        s32Ret = HI_MPI_VGS_AddScaleTask(hHandle, &stVgsTask);
		if(HI_SUCCESS != s32Ret)
        {
	        SAMPLE_PRT("Err, s32Ret:0x%x\n",s32Ret);
            HI_MPI_VB_ReleaseBlock(vbOutBlk);
			return HI_NULL;
        }

        s32Ret = HI_MPI_VGS_EndJob(hHandle);
		if(HI_SUCCESS != s32Ret)
        {
	        SAMPLE_PRT("Err, s32Ret:0x%x\n",s32Ret);
            HI_MPI_VB_ReleaseBlock(vbOutBlk);
			return HI_NULL;

        }

        s32Ret = HI_MPI_VO_SendFrame(VoLayer, VoChn, &stTask.stImgOut, s32MilliSec);
		if (HI_SUCCESS != s32Ret)
		{				
			continue;
		}

		s32Ret = HI_MPI_VI_ReleaseFrame(ViChn,&stTask.stImgIn);
		if(HI_SUCCESS != s32Ret)
		{
			printf("Info:HI_MPI_VI_ReleaseFrame fail, s32Ret:0x%x\n",s32Ret );
			return HI_NULL;
		}	
		
		s32Ret = HI_MPI_SYS_Munmap(pu8OutVirtAddr,u32BufSize);
		if(HI_SUCCESS != s32Ret)
		{
			printf("Info:HI_MPI_SYS_Munmap fail,s32Ret:0x%x\n", s32Ret);
			return HI_NULL;
		}	
		

		s32Ret = HI_MPI_VB_ReleaseBlock(vbOutBlk);
		if(HI_SUCCESS != s32Ret)
		{
			printf("Info:HI_MPI_VB_ReleaseBlock fail,s32Ret:0x%x\n", s32Ret);
			return HI_NULL;
		}
		
		usleep(20000);
	}
	
	s32Ret = HI_MPI_VI_SetFrameDepth(ViChn,u32OldDepth);
	if(HI_SUCCESS != s32Ret)
	{
	    SAMPLE_PRT("Err, s32Ret:0x%x\n",s32Ret);
	}
    return HI_NULL;
}

HI_S32 FISHEYE_MST_StopSwitchModeThrd()
{
    if (HI_FALSE != bSetFisheyeAttr)
    {
        bSetFisheyeAttr = HI_FALSE;
        pthread_join(ThreadId, NULL);
    }
	
    return HI_SUCCESS;
}


/******************************************************************************
* function : vi/vpss: offline/online fisheye mode VI-VO. Embeded isp, phychn channel preview.
******************************************************************************/
HI_S32 SAMPLE_VIO_Fisheye_360Panorama_celing_2half_PreView(SAMPLE_VI_CONFIG_S* pstViConfig)
{
    HI_U32 u32ViChnCnt = 2;
    VI_CHN ViExtChn = 2;
    VI_DEV ViDev 	= 0;
    VENC_CHN VencChn = 0;
    VPSS_GRP VpssGrp = 0;
	VPSS_CHN VpssExtChn = 4;
    SIZE_S stSize;
    HI_U32 u32BlkSize;
    VB_CONF_S stVbConf;
    PIC_SIZE_E enPicSize = g_enPicSize;
    SAMPLE_VPSS_ATTR_S stVpssAttr;
    HI_S32 s32ChnNum = 1;
    HI_S32 s32Ret = HI_SUCCESS;
    FISHEYE_CONFIG_S stFisheyeConfig;
	FISHEYE_ATTR_S   stFisheyeAttr;

    s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, enPicSize, &stSize);
    if (HI_SUCCESS != s32Ret) 
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        return s32Ret;
    }

    /******************************************
      step  1: init global  variable
     ******************************************/
    gs_u32ViFrmRate = (VIDEO_ENCODING_MODE_PAL == gs_enNorm) ? 25 : 30;

    /******************************************
      step  2: mpp system init
     ******************************************/
    memset(&stVbConf, 0, sizeof(VB_CONF_S));
    stVbConf.u32MaxPoolCnt = 128;

    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize2(&stSize, 
            SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);

    /* comm video buffer */
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt  = u32ViChnCnt * 15;    

    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        SAMPLE_COMM_SYS_Exit();
        return s32Ret;
    }

    /******************************************
      step  3: start VI VO  VENC
     ******************************************/
    s32Ret = SAMPLE_VIO_FISHEYE_StartViVo(pstViConfig, &stVpssAttr, ViExtChn, VpssExtChn, VencChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_VIO_FISHEYE_StartViVo failed witfh %d\n", s32Ret);
        goto exit;   
    }

	 /******************************************
     step   4: stream venc process -- get stream, then save it to file. 
    ******************************************/
    s32Ret = SAMPLE_COMM_VENC_StartGetStream(s32ChnNum);
	if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VENC_StartGetStream failed witfh %d\n", s32Ret);
        goto exit1;   
    }

    /******************************************
      step  5: set fisheye Attr
     ******************************************/
    HI_U16 temp[128] = {0, 15, 31, 47, 63, 79, 95, 111, 127, 143, 159, 175, 
    191, 207, 223, 239, 255, 271, 286, 302, 318, 334, 350, 365, 381, 397, 412, 
    428, 443, 459, 474, 490, 505, 520, 536, 551, 566, 581, 596, 611, 626, 641, 
    656, 670, 685, 699, 713, 728, 742, 756, 769, 783, 797, 810, 823, 836, 848, 
    861, 873, 885, 896, 908, 919, 929, 940, 950, 959, 969, 984, 998, 1013, 1027, 
    1042, 1056, 1071, 1085, 1100, 1114, 1129, 1143, 1158, 1172, 1187, 1201, 1215, 
    1230, 1244, 1259, 1273, 1288, 1302, 1317, 1331, 1346, 1360, 1375, 1389, 1404, 
    1418, 1433, 1447, 1462, 1476, 1491, 1505, 1519, 1534, 1548, 1563, 1577, 1592, 
    1606, 1621, 1635, 1650, 1664, 1679, 1693, 1708, 1722, 1737, 1751, 1766, 1780, 1795, 1809, 1823, 1838};
    memcpy(stFisheyeConfig.au16LMFCoef,temp,256);
    
    stFisheyeAttr.bEnable 			= HI_TRUE;
	stFisheyeAttr.bLMF 				= HI_FALSE;
	stFisheyeAttr.bBgColor 			= HI_TRUE;
	stFisheyeAttr.u32BgColor 		= 0x0fFC;
	stFisheyeAttr.s32HorOffset 		= 0;
    stFisheyeAttr.s32FanStrength    = 0;
	stFisheyeAttr.s32VerOffset 		= 0;
	stFisheyeAttr.u32TrapezoidCoef 	= 0;
	stFisheyeAttr.s32FanStrength 	= 0;
	stFisheyeAttr.enMountMode 		= FISHEYE_CEILING_MOUNT;
	stFisheyeAttr.u32RegionNum 		= 2;

	stFisheyeAttr.astFisheyeRegionAttr[0].enViewMode 			= FISHEYE_VIEW_360_PANORAMA;
	stFisheyeAttr.astFisheyeRegionAttr[0].u32InRadius 			= 0;
	stFisheyeAttr.astFisheyeRegionAttr[0].u32OutRadius 			= ALIGN_BACK(stVpssAttr.stVpssChnMode.u32Height/2, 2);
	stFisheyeAttr.astFisheyeRegionAttr[0].u32Pan 				= 180;
	stFisheyeAttr.astFisheyeRegionAttr[0].u32Tilt 				= 180;
	stFisheyeAttr.astFisheyeRegionAttr[0].u32HorZoom 			= 4095;
	stFisheyeAttr.astFisheyeRegionAttr[0].u32VerZoom 			= 4095;
	stFisheyeAttr.astFisheyeRegionAttr[0].stOutRect.s32X 		= 0;
	stFisheyeAttr.astFisheyeRegionAttr[0].stOutRect.s32Y 		= 0;
	stFisheyeAttr.astFisheyeRegionAttr[0].stOutRect.u32Width 	= stVpssAttr.stVpssChnMode.u32Width;
	stFisheyeAttr.astFisheyeRegionAttr[0].stOutRect.u32Height 	= ALIGN_BACK(stVpssAttr.stVpssChnMode.u32Height/3, 2);

	stFisheyeAttr.astFisheyeRegionAttr[1].enViewMode 			= FISHEYE_VIEW_360_PANORAMA;
	stFisheyeAttr.astFisheyeRegionAttr[1].u32InRadius 			= 0;
	stFisheyeAttr.astFisheyeRegionAttr[1].u32OutRadius 			= ALIGN_BACK(stVpssAttr.stVpssChnMode.u32Height/2, 2);
	stFisheyeAttr.astFisheyeRegionAttr[1].u32Pan 				= 180;
	stFisheyeAttr.astFisheyeRegionAttr[1].u32Tilt 				= 180;
	stFisheyeAttr.astFisheyeRegionAttr[1].u32HorZoom 			= 4095;
	stFisheyeAttr.astFisheyeRegionAttr[1].u32VerZoom 			= 4095;
	stFisheyeAttr.astFisheyeRegionAttr[1].stOutRect.s32X 		= 0;
	stFisheyeAttr.astFisheyeRegionAttr[1].stOutRect.s32Y 		= ALIGN_BACK(stVpssAttr.stVpssChnMode.u32Height/3, 2);
	stFisheyeAttr.astFisheyeRegionAttr[1].stOutRect.u32Width 	= stVpssAttr.stVpssChnMode.u32Width;
	stFisheyeAttr.astFisheyeRegionAttr[1].stOutRect.u32Height 	= ALIGN_BACK(stVpssAttr.stVpssChnMode.u32Height/3, 2);

    s32Ret = SAMPLE_VIO_FISHEYE_SetFisheyeConfig(VpssGrp, VpssExtChn, ViDev, &stFisheyeConfig);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("set fisheye config failed with s32Ret:0x%x!\n", s32Ret);
		goto exit1;   
	}

	s32Ret = SAMPLE_VIO_FISHEYE_SetFisheyeAttr(VpssGrp, VpssExtChn, ViExtChn, &stFisheyeAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("set fisheye attr failed with s32Ret:0x%x!\n", s32Ret);
        goto exit1;   
    }

    printf("\nplease press enter, disable fisheye\n\n");
    VI_PAUSE();

    stFisheyeAttr.bEnable = HI_FALSE;
	s32Ret = SAMPLE_VIO_FISHEYE_SetFisheyeAttr(VpssGrp, VpssExtChn, ViExtChn, &stFisheyeAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("set fisheye attr failed with s32Ret:0x%x!\n", s32Ret);
        goto exit1;   
    }    

    printf("\nplease press enter, enable fisheye\n");
    VI_PAUSE();

    stFisheyeAttr.bEnable = HI_TRUE;
    s32Ret = SAMPLE_VIO_FISHEYE_SetFisheyeAttr(VpssGrp, VpssExtChn, ViExtChn, &stFisheyeAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("set fisheye attr failed with s32Ret:0x%x!\n", s32Ret);
        goto exit1;   
    }    

	printf("\nplease press enter to exit!\n");
    VI_PAUSE();
	
	SAMPLE_COMM_VENC_StopGetStream();
    
exit1:
    SAMPLE_VIO_FISHEYE_StopViVo(pstViConfig, &stVpssAttr, ViExtChn, VpssExtChn, VencChn);

exit:
    SAMPLE_COMM_SYS_Exit();
    return s32Ret;

}

/******************************************************************************
* function : vi/vpss: offline/online fisheye mode VI-VO. Embeded isp, phychn channel preview.
******************************************************************************/
HI_S32 SAMPLE_VIO_Fisheye_360Panorama_desktop_and_2normal_PreView(SAMPLE_VI_CONFIG_S* pstViConfig)
{
    HI_U32 u32ViChnCnt = 2;
    VI_CHN ViExtChn = 2;
    VENC_CHN VencChn = 0;
    VPSS_GRP VpssGrp = 0;
	VPSS_CHN VpssExtChn = 4;
    SIZE_S stSize;
    HI_U32 u32BlkSize;
    VB_CONF_S stVbConf;
    PIC_SIZE_E enPicSize = g_enPicSize;
    SAMPLE_VPSS_ATTR_S stVpssAttr;
    HI_S32 s32ChnNum = 1;
    HI_S32 s32Ret = HI_SUCCESS;
	FISHEYE_ATTR_S   stFisheyeAttr;
    
	s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, enPicSize, &stSize);
	if (HI_SUCCESS != s32Ret) 
	{
		SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
		return s32Ret;
	}

	/******************************************
	  step	1: init global	variable
	 ******************************************/
	gs_u32ViFrmRate = (VIDEO_ENCODING_MODE_PAL == gs_enNorm) ? 25 : 30;

	/******************************************
	  step	2: mpp system init
	 ******************************************/
	memset(&stVbConf, 0, sizeof(VB_CONF_S));
	stVbConf.u32MaxPoolCnt = 128;

	u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize2(&stSize, 
			SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);

	/* comm video buffer */
	stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt  = u32ViChnCnt * 15;	 

	s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("system init failed with %d!\n", s32Ret);
		SAMPLE_COMM_SYS_Exit();
		return s32Ret;
	}

    /******************************************
      step  3: start VI VO  VENC
     ******************************************/
    s32Ret = SAMPLE_VIO_FISHEYE_StartViVo(pstViConfig, &stVpssAttr, ViExtChn, VpssExtChn, VencChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_VIO_FISHEYE_StartViVo failed witfh %d\n", s32Ret);
        goto exit;   
    }

	 /******************************************
     step   4: stream venc process -- get stream, then save it to file. 
    ******************************************/
    s32Ret = SAMPLE_COMM_VENC_StartGetStream(s32ChnNum);
	if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VENC_StartGetStream failed witfh %d\n", s32Ret);
        goto exit1;   
    }

    /******************************************
      step  5: set fisheye Attr
     ******************************************/
	
	stFisheyeAttr.bEnable			= HI_TRUE;
	stFisheyeAttr.bLMF				= HI_FALSE;
	stFisheyeAttr.bBgColor			= HI_TRUE;
	stFisheyeAttr.u32BgColor		= 0xFC;
	stFisheyeAttr.s32HorOffset		= 0;
    stFisheyeAttr.s32FanStrength    = 0;
	stFisheyeAttr.s32VerOffset		= 0;
	stFisheyeAttr.u32TrapezoidCoef	= 0;
	stFisheyeAttr.s32FanStrength 	= 0;
	stFisheyeAttr.enMountMode		= FISHEYE_DESKTOP_MOUNT;
	stFisheyeAttr.u32RegionNum		= 3;

	stFisheyeAttr.astFisheyeRegionAttr[0].enViewMode			= FISHEYE_VIEW_360_PANORAMA;
	stFisheyeAttr.astFisheyeRegionAttr[0].u32InRadius			= 0;
	stFisheyeAttr.astFisheyeRegionAttr[0].u32OutRadius			= ALIGN_BACK(stVpssAttr.stVpssChnMode.u32Height/2, 2);
	stFisheyeAttr.astFisheyeRegionAttr[0].u32Pan				= 180;
	stFisheyeAttr.astFisheyeRegionAttr[0].u32Tilt				= 180;
	stFisheyeAttr.astFisheyeRegionAttr[0].u32HorZoom			= 4095;
	stFisheyeAttr.astFisheyeRegionAttr[0].u32VerZoom			= 4095;
	stFisheyeAttr.astFisheyeRegionAttr[0].stOutRect.s32X		= 0;
	stFisheyeAttr.astFisheyeRegionAttr[0].stOutRect.s32Y		= 0;
	stFisheyeAttr.astFisheyeRegionAttr[0].stOutRect.u32Width	= stVpssAttr.stVpssChnMode.u32Width;
	stFisheyeAttr.astFisheyeRegionAttr[0].stOutRect.u32Height	= ALIGN_BACK(stVpssAttr.stVpssChnMode.u32Height/3, 2);

	stFisheyeAttr.astFisheyeRegionAttr[1].enViewMode			= FISHEYE_VIEW_NORMAL;
	stFisheyeAttr.astFisheyeRegionAttr[1].u32InRadius			= 0;
	stFisheyeAttr.astFisheyeRegionAttr[1].u32OutRadius			= ALIGN_BACK(stVpssAttr.stVpssChnMode.u32Height/2, 2);
	stFisheyeAttr.astFisheyeRegionAttr[1].u32Pan				= 0;
	stFisheyeAttr.astFisheyeRegionAttr[1].u32Tilt				= 90;
	stFisheyeAttr.astFisheyeRegionAttr[1].u32HorZoom			= 2048;
	stFisheyeAttr.astFisheyeRegionAttr[1].u32VerZoom			= 4095;
	stFisheyeAttr.astFisheyeRegionAttr[1].stOutRect.s32X		= 0;
	stFisheyeAttr.astFisheyeRegionAttr[1].stOutRect.s32Y		= ALIGN_BACK(stVpssAttr.stVpssChnMode.u32Height/3, 2);
	stFisheyeAttr.astFisheyeRegionAttr[1].stOutRect.u32Width	= ALIGN_BACK(stVpssAttr.stVpssChnMode.u32Width/2, 2);
	stFisheyeAttr.astFisheyeRegionAttr[1].stOutRect.u32Height	= ALIGN_BACK(stVpssAttr.stVpssChnMode.u32Height/2, 2);

	stFisheyeAttr.astFisheyeRegionAttr[2].enViewMode			= FISHEYE_VIEW_NORMAL;
	stFisheyeAttr.astFisheyeRegionAttr[2].u32InRadius			= 0;
	stFisheyeAttr.astFisheyeRegionAttr[2].u32OutRadius			= ALIGN_BACK(stVpssAttr.stVpssChnMode.u32Height/2, 2);
	stFisheyeAttr.astFisheyeRegionAttr[2].u32Pan				= 180;
	stFisheyeAttr.astFisheyeRegionAttr[2].u32Tilt				= 270;
	stFisheyeAttr.astFisheyeRegionAttr[2].u32HorZoom			= 2048;
	stFisheyeAttr.astFisheyeRegionAttr[2].u32VerZoom			= 4095;
	stFisheyeAttr.astFisheyeRegionAttr[2].stOutRect.s32X		= ALIGN_BACK(stVpssAttr.stVpssChnMode.u32Width/2, 16);
	stFisheyeAttr.astFisheyeRegionAttr[2].stOutRect.s32Y		= ALIGN_BACK(stVpssAttr.stVpssChnMode.u32Height/3, 2);
	stFisheyeAttr.astFisheyeRegionAttr[2].stOutRect.u32Width	= ALIGN_BACK(stVpssAttr.stVpssChnMode.u32Width/2, 2);
	stFisheyeAttr.astFisheyeRegionAttr[2].stOutRect.u32Height	= ALIGN_BACK(stVpssAttr.stVpssChnMode.u32Height/2, 2);


	s32Ret = SAMPLE_VIO_FISHEYE_SetFisheyeAttr(VpssGrp, VpssExtChn, ViExtChn, &stFisheyeAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("set fisheye attr failed with s32Ret:0x%x!\n", s32Ret);
        goto exit1;   
    }	

	printf("\nplease press enter, disable fisheye\n\n");
	VI_PAUSE();

	stFisheyeAttr.bEnable = HI_FALSE;
	s32Ret = SAMPLE_VIO_FISHEYE_SetFisheyeAttr(VpssGrp, VpssExtChn, ViExtChn, &stFisheyeAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("set fisheye attr failed with s32Ret:0x%x!\n", s32Ret);
        goto exit1;   
    }

	printf("\nplease press enter, enable fisheye\n");
	VI_PAUSE();

	stFisheyeAttr.bEnable = HI_TRUE;
	s32Ret = SAMPLE_VIO_FISHEYE_SetFisheyeAttr(VpssGrp, VpssExtChn, ViExtChn, &stFisheyeAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("set fisheye attr failed with s32Ret:0x%x!\n", s32Ret);
        goto exit1;   
    }

	printf("\nplease press enter to exit!\n");
	VI_PAUSE();
    SAMPLE_COMM_VENC_StopGetStream();

exit1:
	SAMPLE_VIO_FISHEYE_StopViVo(pstViConfig, &stVpssAttr, ViExtChn, VpssExtChn, VencChn);

exit:
	SAMPLE_COMM_SYS_Exit();
	return s32Ret;

}


/******************************************************************************
* function : vi/vpss: offline/online fisheye mode VI-VO. Embeded isp, phychn channel preview.
******************************************************************************/
HI_S32 SAMPLE_VIO_Fisheye_180Panorama_wall_and_2DynamicNormal_PreView(SAMPLE_VI_CONFIG_S* pstViConfig)
{
    HI_U32 u32ViChnCnt = 2;
    VI_CHN ViExtChn = 2;
    VENC_CHN VencChn = 0;
    VPSS_GRP VpssGrp = 0;
	VPSS_CHN VpssExtChn = 4;
    SIZE_S stSize;
    HI_U32 u32BlkSize;
    VB_CONF_S stVbConf;
    PIC_SIZE_E enPicSize = g_enPicSize;
    SAMPLE_VPSS_ATTR_S stVpssAttr;
    HI_S32 s32ChnNum = 1;
    HI_S32 s32Ret = HI_SUCCESS;
	FISHEYE_ATTR_S   stFisheyeAttr;

	s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, enPicSize, &stSize);
	if (HI_SUCCESS != s32Ret) 
	{
		SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
		return s32Ret;
	}

	/******************************************
	  step	1: init global	variable
	 ******************************************/
	gs_u32ViFrmRate = (VIDEO_ENCODING_MODE_PAL == gs_enNorm) ? 25 : 30;

	/******************************************
	  step	2: mpp system init
	 ******************************************/
	memset(&stVbConf, 0, sizeof(VB_CONF_S));
	stVbConf.u32MaxPoolCnt = 128;

	u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize2(&stSize, 
			SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);

	/* comm video buffer */
	stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt  = u32ViChnCnt * 15;	 

	s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("system init failed with %d!\n", s32Ret);
		SAMPLE_COMM_SYS_Exit();
		return s32Ret;
	}

    /******************************************
      step  3: start VI VO  VENC
     ******************************************/
    s32Ret = SAMPLE_VIO_FISHEYE_StartViVo(pstViConfig, &stVpssAttr, ViExtChn, VpssExtChn, VencChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_VIO_FISHEYE_StartViVo failed witfh %d\n", s32Ret);
        goto exit;   
    }

	 /******************************************
     step   4: stream venc process -- get stream, then save it to file. 
    ******************************************/
    s32Ret = SAMPLE_COMM_VENC_StartGetStream(s32ChnNum);
	if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VENC_StartGetStream failed witfh %d\n", s32Ret);
        goto exit1;   
    }

    /******************************************
      step  5: set fisheye Attr
     ******************************************/
	
	stFisheyeAttr.bEnable			= HI_TRUE;
	stFisheyeAttr.bLMF				= HI_FALSE;
	stFisheyeAttr.bBgColor			= HI_TRUE;
	stFisheyeAttr.u32BgColor		= 0x0011FC;
	stFisheyeAttr.s32HorOffset		= 0;
	stFisheyeAttr.s32VerOffset		= 0;
	stFisheyeAttr.u32TrapezoidCoef	= 10;
    stFisheyeAttr.s32FanStrength    = 300;
	stFisheyeAttr.enMountMode		= FISHEYE_WALL_MOUNT;
	stFisheyeAttr.u32RegionNum		= 3;

	stFisheyeAttr.astFisheyeRegionAttr[0].enViewMode			= FISHEYE_VIEW_180_PANORAMA;
	stFisheyeAttr.astFisheyeRegionAttr[0].u32InRadius			= 0;
	stFisheyeAttr.astFisheyeRegionAttr[0].u32OutRadius			= ALIGN_BACK(stVpssAttr.stVpssChnMode.u32Height/2, 2);
	stFisheyeAttr.astFisheyeRegionAttr[0].u32Pan				= 180;
	stFisheyeAttr.astFisheyeRegionAttr[0].u32Tilt				= 180;
	stFisheyeAttr.astFisheyeRegionAttr[0].u32HorZoom			= 4095;
	stFisheyeAttr.astFisheyeRegionAttr[0].u32VerZoom			= 4095;
	stFisheyeAttr.astFisheyeRegionAttr[0].stOutRect.s32X		= 0;
	stFisheyeAttr.astFisheyeRegionAttr[0].stOutRect.s32Y		= 0;
	stFisheyeAttr.astFisheyeRegionAttr[0].stOutRect.u32Width	= stVpssAttr.stVpssChnMode.u32Width;
	stFisheyeAttr.astFisheyeRegionAttr[0].stOutRect.u32Height	= ALIGN_BACK(stVpssAttr.stVpssChnMode.u32Height/2, 2);

	stFisheyeAttr.astFisheyeRegionAttr[1].enViewMode			= FISHEYE_VIEW_NORMAL;
	stFisheyeAttr.astFisheyeRegionAttr[1].u32InRadius			= 0;
	stFisheyeAttr.astFisheyeRegionAttr[1].u32OutRadius			= ALIGN_BACK(stVpssAttr.stVpssChnMode.u32Height/2, 2);
	stFisheyeAttr.astFisheyeRegionAttr[1].u32Pan				= 180;
	stFisheyeAttr.astFisheyeRegionAttr[1].u32Tilt				= 180;
	stFisheyeAttr.astFisheyeRegionAttr[1].u32HorZoom			= 4095;
	stFisheyeAttr.astFisheyeRegionAttr[1].u32VerZoom			= 4095;
	stFisheyeAttr.astFisheyeRegionAttr[1].stOutRect.s32X		= 0;
	stFisheyeAttr.astFisheyeRegionAttr[1].stOutRect.s32Y		= ALIGN_BACK(stVpssAttr.stVpssChnMode.u32Height/2, 2);
	stFisheyeAttr.astFisheyeRegionAttr[1].stOutRect.u32Width	= ALIGN_BACK(stVpssAttr.stVpssChnMode.u32Width/2, 2);
	stFisheyeAttr.astFisheyeRegionAttr[1].stOutRect.u32Height	= ALIGN_BACK(stVpssAttr.stVpssChnMode.u32Height/2, 2);

	stFisheyeAttr.astFisheyeRegionAttr[2].enViewMode			= FISHEYE_VIEW_NORMAL;
	stFisheyeAttr.astFisheyeRegionAttr[2].u32InRadius			= 0;
	stFisheyeAttr.astFisheyeRegionAttr[2].u32OutRadius			= ALIGN_BACK(stVpssAttr.stVpssChnMode.u32Height/2, 2);
	stFisheyeAttr.astFisheyeRegionAttr[2].u32Pan				= 200;
	stFisheyeAttr.astFisheyeRegionAttr[2].u32Tilt				= 200;
	stFisheyeAttr.astFisheyeRegionAttr[2].u32HorZoom			= 4095;
	stFisheyeAttr.astFisheyeRegionAttr[2].u32VerZoom			= 4095;
	stFisheyeAttr.astFisheyeRegionAttr[2].stOutRect.s32X		= ALIGN_BACK(stVpssAttr.stVpssChnMode.u32Width/2, 16);
	stFisheyeAttr.astFisheyeRegionAttr[2].stOutRect.s32Y		= ALIGN_BACK(stVpssAttr.stVpssChnMode.u32Height/2, 2);
	stFisheyeAttr.astFisheyeRegionAttr[2].stOutRect.u32Width	= ALIGN_BACK(stVpssAttr.stVpssChnMode.u32Width/2, 2);
	stFisheyeAttr.astFisheyeRegionAttr[2].stOutRect.u32Height	= ALIGN_BACK(stVpssAttr.stVpssChnMode.u32Height/2, 2);


	s32Ret = SAMPLE_VIO_FISHEYE_SetFisheyeAttr(VpssGrp, VpssExtChn, ViExtChn, &stFisheyeAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("set fisheye attr failed with s32Ret:0x%x!\n", s32Ret);
		goto exit1;   
	}	 

	/* create a pthread to change the fisheye attr */
	SAMPLE_VIO_FISHEYE_StartSetFisheyeAttrThrd(VpssGrp, VpssExtChn, ViExtChn);
	
	printf("\nplease press enter to exit!\n");
	VI_PAUSE();
    SAMPLE_COMM_VENC_StopGetStream();
	SAMPLE_VIO_FISHEYE_StopSetFisheyeAttrThrd();
	
exit1:
	SAMPLE_VIO_FISHEYE_StopViVo(pstViConfig, &stVpssAttr, ViExtChn, VpssExtChn, VencChn);

exit:
	SAMPLE_COMM_SYS_Exit();
	return s32Ret;

}


/******************************************************************************
* function : vi/vpss: offline/online fisheye mode VI-VO. Embeded isp, phychn channel preview.
******************************************************************************/
HI_S32 SAMPLE_VIO_Fisheye_source_and_3Normal_PreView(SAMPLE_VI_CONFIG_S* pstViConfig)
{
    HI_U32 u32ViChnCnt = 2;
    VI_CHN ViExtChn = 2;
    VENC_CHN VencChn = 0;
    VPSS_GRP VpssGrp = 0;
	VPSS_CHN VpssExtChn = 4;
    SIZE_S stSize;
    HI_U32 u32BlkSize;
    VB_CONF_S stVbConf;
    PIC_SIZE_E enPicSize = g_enPicSize;
    SAMPLE_VPSS_ATTR_S stVpssAttr;
    HI_S32 s32ChnNum = 1;
    HI_S32 s32Ret = HI_SUCCESS;
	FISHEYE_ATTR_S   stFisheyeAttr;

	s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, enPicSize, &stSize);
	if (HI_SUCCESS != s32Ret) 
	{
		SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
		return s32Ret;
	}

	/******************************************
	  step	1: init global	variable
	 ******************************************/
	gs_u32ViFrmRate = (VIDEO_ENCODING_MODE_PAL == gs_enNorm) ? 25 : 30;

	/******************************************
	  step	2: mpp system init
	 ******************************************/
	memset(&stVbConf, 0, sizeof(VB_CONF_S));
	stVbConf.u32MaxPoolCnt = 128;

	u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize2(&stSize, 
			SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);

	/* comm video buffer */
	stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt  = u32ViChnCnt * 15;	 

	s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("system init failed with %d!\n", s32Ret);
		SAMPLE_COMM_SYS_Exit();
		return s32Ret;
	}

    /******************************************
      step  3: start VI VO  VENC
     ******************************************/
    s32Ret = SAMPLE_VIO_FISHEYE_StartViVo(pstViConfig, &stVpssAttr, ViExtChn, VpssExtChn, VencChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_VIO_FISHEYE_StartViVo failed witfh %d\n", s32Ret);
        goto exit;   
    }

	 /******************************************
     step   4: stream venc process -- get stream, then save it to file. 
    ******************************************/
    s32Ret = SAMPLE_COMM_VENC_StartGetStream(s32ChnNum);
	if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VENC_StartGetStream failed witfh %d\n", s32Ret);
        goto exit1;   
    }

    /******************************************
      step  5: set fisheye Attr
     ******************************************/
	
	stFisheyeAttr.bEnable			= HI_TRUE;
	stFisheyeAttr.bLMF				= HI_FALSE;
	stFisheyeAttr.bBgColor			= HI_FALSE;
	stFisheyeAttr.u32BgColor		= 0xFC;
	stFisheyeAttr.s32HorOffset		= 0;
	stFisheyeAttr.s32VerOffset		= 0;
	stFisheyeAttr.u32TrapezoidCoef	= 10;
	stFisheyeAttr.s32FanStrength 	= 0;
	stFisheyeAttr.enMountMode		= FISHEYE_WALL_MOUNT;
	stFisheyeAttr.u32RegionNum		= 4;

	stFisheyeAttr.astFisheyeRegionAttr[0].enViewMode			= FISHEYE_NO_TRANSFORMATION;
	stFisheyeAttr.astFisheyeRegionAttr[0].u32InRadius			= 0;
	stFisheyeAttr.astFisheyeRegionAttr[0].u32OutRadius			= 600;
	stFisheyeAttr.astFisheyeRegionAttr[0].u32Pan				= 0;
	stFisheyeAttr.astFisheyeRegionAttr[0].u32Tilt				= 0;
	stFisheyeAttr.astFisheyeRegionAttr[0].u32HorZoom			= 2048;
	stFisheyeAttr.astFisheyeRegionAttr[0].u32VerZoom			= 4095;
	stFisheyeAttr.astFisheyeRegionAttr[0].stOutRect.s32X		= 0;
	stFisheyeAttr.astFisheyeRegionAttr[0].stOutRect.s32Y		= 0;
	stFisheyeAttr.astFisheyeRegionAttr[0].stOutRect.u32Width	= ALIGN_BACK(stVpssAttr.stVpssChnMode.u32Width/2, 2);
	stFisheyeAttr.astFisheyeRegionAttr[0].stOutRect.u32Height	= ALIGN_BACK(stVpssAttr.stVpssChnMode.u32Height/2, 2);

	stFisheyeAttr.astFisheyeRegionAttr[1].enViewMode			= FISHEYE_VIEW_NORMAL;
	stFisheyeAttr.astFisheyeRegionAttr[1].u32InRadius			= 0;
	stFisheyeAttr.astFisheyeRegionAttr[1].u32OutRadius			= ALIGN_BACK(stVpssAttr.stVpssChnMode.u32Height/2, 2);
	stFisheyeAttr.astFisheyeRegionAttr[1].u32Pan				= 180;
	stFisheyeAttr.astFisheyeRegionAttr[1].u32Tilt				= 135;
	stFisheyeAttr.astFisheyeRegionAttr[1].u32HorZoom			= 2048;
	stFisheyeAttr.astFisheyeRegionAttr[1].u32VerZoom			= 4095;
	stFisheyeAttr.astFisheyeRegionAttr[1].stOutRect.s32X		= ALIGN_BACK(stVpssAttr.stVpssChnMode.u32Width/2, 16);
	stFisheyeAttr.astFisheyeRegionAttr[1].stOutRect.s32Y		= 0;
	stFisheyeAttr.astFisheyeRegionAttr[1].stOutRect.u32Width	= ALIGN_BACK(stVpssAttr.stVpssChnMode.u32Width/2, 2);
	stFisheyeAttr.astFisheyeRegionAttr[1].stOutRect.u32Height	= ALIGN_BACK(stVpssAttr.stVpssChnMode.u32Height/2, 2);

	stFisheyeAttr.astFisheyeRegionAttr[2].enViewMode			= FISHEYE_VIEW_NORMAL;
	stFisheyeAttr.astFisheyeRegionAttr[2].u32InRadius			= 0;
	stFisheyeAttr.astFisheyeRegionAttr[2].u32OutRadius			= ALIGN_BACK(stVpssAttr.stVpssChnMode.u32Height/2, 2);
	stFisheyeAttr.astFisheyeRegionAttr[2].u32Pan				= 135;
	stFisheyeAttr.astFisheyeRegionAttr[2].u32Tilt				= 180;
	stFisheyeAttr.astFisheyeRegionAttr[2].u32HorZoom			= 2048;
	stFisheyeAttr.astFisheyeRegionAttr[2].u32VerZoom			= 4095;
	stFisheyeAttr.astFisheyeRegionAttr[2].stOutRect.s32X		= 0;
	stFisheyeAttr.astFisheyeRegionAttr[2].stOutRect.s32Y		= ALIGN_BACK(stVpssAttr.stVpssChnMode.u32Height/2, 2);
	stFisheyeAttr.astFisheyeRegionAttr[2].stOutRect.u32Width	= ALIGN_BACK(stVpssAttr.stVpssChnMode.u32Width/2, 2);
	stFisheyeAttr.astFisheyeRegionAttr[2].stOutRect.u32Height	= ALIGN_BACK(stVpssAttr.stVpssChnMode.u32Height/2, 2);

	stFisheyeAttr.astFisheyeRegionAttr[3].enViewMode			= FISHEYE_VIEW_NORMAL;
	stFisheyeAttr.astFisheyeRegionAttr[3].u32InRadius			= 0;
	stFisheyeAttr.astFisheyeRegionAttr[3].u32OutRadius			= ALIGN_BACK(stVpssAttr.stVpssChnMode.u32Height/2, 2);
	stFisheyeAttr.astFisheyeRegionAttr[3].u32Pan				= 215;
	stFisheyeAttr.astFisheyeRegionAttr[3].u32Tilt				= 180;
	stFisheyeAttr.astFisheyeRegionAttr[3].u32HorZoom			= 2048;
	stFisheyeAttr.astFisheyeRegionAttr[3].u32VerZoom			= 4095;
	stFisheyeAttr.astFisheyeRegionAttr[3].stOutRect.s32X		= ALIGN_BACK(stVpssAttr.stVpssChnMode.u32Width/2, 16);
	stFisheyeAttr.astFisheyeRegionAttr[3].stOutRect.s32Y		= ALIGN_BACK(stVpssAttr.stVpssChnMode.u32Height/2, 2);
	stFisheyeAttr.astFisheyeRegionAttr[3].stOutRect.u32Width	= ALIGN_BACK(stVpssAttr.stVpssChnMode.u32Width/2, 2);
	stFisheyeAttr.astFisheyeRegionAttr[3].stOutRect.u32Height	= ALIGN_BACK(stVpssAttr.stVpssChnMode.u32Height/2, 2);

	s32Ret = SAMPLE_VIO_FISHEYE_SetFisheyeAttr(VpssGrp, VpssExtChn, ViExtChn, &stFisheyeAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("set fisheye attr failed with s32Ret:0x%x!\n", s32Ret);
		goto exit1;   
	}

	printf("\nplease press enter, disable fisheye\n\n");
	VI_PAUSE();

	stFisheyeAttr.bEnable = HI_FALSE;
	s32Ret = SAMPLE_VIO_FISHEYE_SetFisheyeAttr(VpssGrp, VpssExtChn, ViExtChn, &stFisheyeAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("set fisheye attr failed with s32Ret:0x%x!\n", s32Ret);
		goto exit1;   
	}	 

	printf("\nplease press enter, enable fisheye\n");
	VI_PAUSE();

	stFisheyeAttr.bEnable = HI_TRUE;
	s32Ret = SAMPLE_VIO_FISHEYE_SetFisheyeAttr(VpssGrp, VpssExtChn, ViExtChn, &stFisheyeAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("set fisheye attr failed with s32Ret:0x%x!\n", s32Ret);
		goto exit1;   
	}	 

	printf("\nplease press enter to exit!\n");
	VI_PAUSE();
    SAMPLE_COMM_VENC_StopGetStream();

exit1:
	SAMPLE_VIO_FISHEYE_StopViVo(pstViConfig, &stVpssAttr, ViExtChn, VpssExtChn, VencChn);

exit:
	SAMPLE_COMM_SYS_Exit();
	return s32Ret;

}

/******************************************************************************
* function : vi/: online fisheye mode VI-VO. Embeded isp, phychn channel preview.
******************************************************************************/
HI_S32 SAMPLE_VIO_Fisheye_nine_lattice_Preview(SAMPLE_VI_CONFIG_S* pstViConfig)
{
	HI_U32	u32ViChnCnt = 1;
    SIZE_S	stSize;
    HI_U32	u32BlkSize;
    VB_CONF_S	stVbConf;
    PIC_SIZE_E	enPicSize = g_enPicSize;
    HI_S32 s32Ret = HI_SUCCESS;
	HI_U32	u32Mode;
    
    HI_ASSERT(HI_NULL != pstViConfig);

   /******************************************
     step 1: start vi dev & chn to capture
    ******************************************/
	bSetFisheyeAttr = HI_TRUE;
   
	s32Ret = HI_MPI_SYS_GetViVpssMode(&u32Mode);
	if(HI_SUCCESS != s32Ret)
	{
        SAMPLE_PRT("HI_MPI_SYS_GetViVpssMode failed!\n");
		return s32Ret;
	}
	
	if(!u32Mode)
	{
		printf("\n\033[0;35mVI,VPSS is offline\033[0;39m\n");
	}
	else
	{
		SAMPLE_PRT("only support offline!\n");
        return s32Ret;
	}

	s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, enPicSize, &stSize);
	if (HI_SUCCESS != s32Ret) 
	{
		SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
		return s32Ret;
	}

	/******************************************
	  step	1: init global	variable
	 ******************************************/
	gs_u32ViFrmRate = (VIDEO_ENCODING_MODE_PAL == gs_enNorm) ? 25 : 30;

	/******************************************
	  step	2: mpp system init
	 ******************************************/
	memset(&stVbConf, 0, sizeof(VB_CONF_S));
	stVbConf.u32MaxPoolCnt = 128;

	u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize2(&stSize, 
			SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);

	/* comm video buffer */
	stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt  = u32ViChnCnt * 15;	 

	s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("system init failed with %d!\n", s32Ret);
		SAMPLE_COMM_SYS_Exit();
		return s32Ret;
	}
	    
    s32Ret = SAMPLE_COMM_VI_StartVi(pstViConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        return s32Ret;
    }

	/******************************************
   	 step 3: start VO SD0
    	******************************************/
    s32Ret = SAMPLE_VIO_FISHEYE_StartVO();
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_VIO start VO failed with %#x!\n", s32Ret);
        goto exit;
    }

	pthread_create(&ThreadId, NULL, SAMPLE_VIO_Fisheye_nine_lattice_Thread, &stSize);
	VI_PAUSE();
	FISHEYE_MST_StopSwitchModeThrd();
		 
exit:	
	SAMPLE_VIO_FISHEYE_StopVO();    	
    SAMPLE_COMM_VI_StopVi(pstViConfig);
	SAMPLE_COMM_SYS_Exit();
	return s32Ret;
}

/******************************************************************************
* function    : main()
* Description : video fisheye preview sample
******************************************************************************/
#ifdef __HuaweiLite__
	int app_main(int argc, char* argv[])
#else
	int main(int argc, char* argv[])
#endif
{
    HI_S32 s32Ret = HI_FAILURE;

    if ( (argc < 2) || (1 != strlen(argv[1])) )
    {
        SAMPLE_VIO_FISHEYE_Usage(argv[0]);
        return HI_FAILURE;
    }

#ifndef __HuaweiLite__
    signal(SIGINT, SAMPLE_VIO_FISHEYE_HandleSig);
    signal(SIGTERM, SAMPLE_VIO_FISHEYE_HandleSig);
#endif

    if ((argc > 2) && *argv[2] == '1')  /* '1': VO_INTF_CVBS, else: BT1120 */
    {
        g_enVoIntfType = VO_INTF_BT1120;
    }
    if ((argc > 3) && *argv[3] == '1')  /* '1': PT_H265, else: PT_H264 */
    {
        g_enVencType   = PT_H265;
    }
    
    g_stViChnConfig.enViMode = SENSOR_TYPE;
    SAMPLE_COMM_VI_GetSizeBySensor(&g_enPicSize);

    switch (*argv[1])
    {
        /* VI/VPSS - VO. Embeded isp, phychn channel preview. */
        case '0':
            s32Ret = SAMPLE_VIO_Fisheye_360Panorama_celing_2half_PreView(&g_stViChnConfig);
            break;
			
        case '1':            
            s32Ret = SAMPLE_VIO_Fisheye_360Panorama_desktop_and_2normal_PreView(&g_stViChnConfig);
            break;

        case '2':
            s32Ret = SAMPLE_VIO_Fisheye_180Panorama_wall_and_2DynamicNormal_PreView(&g_stViChnConfig);
            break;

        case '3':
            s32Ret = SAMPLE_VIO_Fisheye_source_and_3Normal_PreView(&g_stViChnConfig);
            break;
			
		case '4':
			s32Ret = SAMPLE_VIO_Fisheye_nine_lattice_Preview(&g_stViChnConfig);
            break;
			
        default:
            SAMPLE_PRT("the index is invaild!\n");
            SAMPLE_VIO_FISHEYE_Usage(argv[0]);
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

