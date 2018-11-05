/******************************************************************************
  A simple program of Hisilicon Hi35xx snap implementation.
  Copyright (C), 2016-2020, Hisilicon Tech. Co., Ltd.
 ******************************************************************************
    Modification:  2016-05 Created
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
#include <sys/prctl.h>
#include <sys/select.h>

#include "sample_comm.h"

#ifdef HI_SUPPORT_PHOTO_ALGS
#include "mpi_photo.h"
#endif

#ifdef __HuaweiLite__
#define AppMain app_main
#else
#define AppMain main
#endif

#define SNAP_PAUSE(...) ({ \
    HI_S32 ch1, ch2; \
    printf(__VA_ARGS__); \
    ch1 = getchar(); \
    if (ch1 != '\n' && ch1 != EOF) { \
        while((ch2 = getchar()) != '\n' && (ch2 != EOF)); \
    } \
    ch1; \
})

typedef void* THREAD_HANDLE_S;
#define THREAD_INVALID_HANDLE   ((THREAD_HANDLE_S)-1)
typedef int (*FN_THREAD_LOOP)(void *pUserData);
typedef struct hiSNAP_THREAD_CTX_S
{
    pthread_t       thread;
    pthread_mutex_t mutex;
    HI_BOOL         bRun;
    char *pThreadName;

    FN_THREAD_LOOP entryFunc;
    
    void      *pUserData;
}SNAP_THREAD_CTX_S;

typedef void* MEDIA_RECORDER_S;
#define INVALID_MEDIA_RECORDER  ((MEDIA_RECORDER_S)-1)
typedef void (*FN_SNAP_VENC_CALLBACK_S)(VENC_CHN VencChn, VENC_STREAM_S *pstStream, void *parg);

typedef enum {
    MEDIA_RECORDER_IDLE                  = 1 << 0, // Recorder was just created.
    MEDIA_RECORDER_INITIALIZED           = 1 << 1, // Recorder has been initialized.
    MEDIA_RECORDER_PREPARED              = 1 << 3, // Recorder is ready to start.
    MEDIA_RECORDER_RECORDING             = 1 << 4, // Recording is in progress.
}MEDIA_RECORDER_STATE_E;

typedef enum {
    VIDEO_ENCODER_H264 = 264,
    VIDEO_ENCODER_H265 = 265,
}VIDEO_ENCODER_E;

typedef struct hiMEDIA_RECORDER_CTX_S
{
    MEDIA_RECORDER_STATE_E enState;
    THREAD_HANDLE_S hThread;

    VENC_CHN  VencChn;
    MPP_CHN_S stSrcChn;  // src chn;
    VIDEO_ENCODER_E enEncoder;
    HI_CHAR *pszFileName;
    FILE *pFile;
    
    FN_SNAP_VENC_CALLBACK_S fnCallback;
} MEDIA_RECORDER_CTX_S;

typedef enum
{
    SNAP_PIPE_MODE_SINGLE = 0,
    SNAP_PIPE_MODE_DUAL,
    SNAP_PIPE_MODE_BUTT
}SNAP_PIPE_MODE_E;

static VIDEO_NORM_E   s_enNorm = VIDEO_ENCODING_MODE_PAL;
static VO_INTF_TYPE_E s_enVoIntfType = VO_INTF_CVBS;
static PIC_SIZE_E     s_enSnsSize = PIC_HD1080;
static HI_BOOL s_bExit = HI_FALSE;

static SAMPLE_VI_CONFIG_S s_stViConfig =
{
    .enViMode = OMNIVISION_OV4689_MIPI_1080P_30FPS,
    .enNorm   = VIDEO_ENCODING_MODE_AUTO,

    .enRotate = ROTATE_NONE,
    .enViChnSet = VI_CHN_SET_NORMAL,
    .enWDRMode  = WDR_MODE_NONE,
    .enFrmRate  = SAMPLE_FRAMERATE_DEFAULT,
    .enSnsNum = SAMPLE_SENSOR_DOUBLE,
};

static SAMPLE_VPSS_ATTR_S s_stVpssAttr =
{
    .VpssGrp = 0,
    .VpssChn = 0,
    .stVpssGrpAttr =
    {
        .bDciEn    = HI_FALSE,
        .bHistEn   = HI_FALSE,
        .bIeEn     = HI_FALSE,
        .bNrEn     = HI_TRUE,
        .bStitchBlendEn = HI_FALSE,
        .stNrAttr  =
        {
            .enNrType       = VPSS_NR_TYPE_VIDEO,
            .u32RefFrameNum = 2,
            .stNrVideoAttr =
            {
                .enNrRefSource = VPSS_NR_REF_FROM_RFR,
                .enNrOutputMode = VPSS_NR_OUTPUT_NORMAL
            }
        },
        .enDieMode = VPSS_DIE_MODE_NODIE,
        .enPixFmt  = PIXEL_FORMAT_YUV_SEMIPLANAR_420,
        .u32MaxW   = 1920,
        .u32MaxH   = 1080
    },
    
    .stVpssChnAttr =
    {
        .bBorderEn       = HI_FALSE,
        .bFlip           = HI_FALSE,
        .bMirror         = HI_FALSE,
        .bSpEn           = HI_FALSE,
        .s32DstFrameRate = -1,
        .s32SrcFrameRate = -1,
    },

    .stVpssChnMode =
    {
        .bDouble         = HI_FALSE,
        .enChnMode       = VPSS_CHN_MODE_USER,
        .enCompressMode  = COMPRESS_MODE_NONE,
        .enPixelFormat   = PIXEL_FORMAT_YUV_SEMIPLANAR_420,
        .u32Width        = 1920,
        .u32Height       = 1080
    },

    .stVpssExtChnAttr =
    {
    }
};

static void SNAP_HandleSig(HI_S32 signo)
{
    printf("catch signal %d\n", signo);
    if (SIGINT == signo || SIGTERM == signo || SIGQUIT == signo)
    {
        s_bExit = HI_TRUE;
        fclose(stdin);  // close stdin, so getchar will return EOF
    }
}

/***********************************************************************
* function : show usage
***********************************************************************/
static void SNAP_Usage(void)
{
    printf("Usage : sample_snap <index> <intf>\n");
    printf("index:\n");
    printf("\t 0) dualpipe ZSL, ref frame num = 0.\n");
    printf("\t 1) dualpipe ZSL, ref frame num = 2.\n");
#ifdef HI_SUPPORT_PHOTO_ALGS
    printf("\t 2) dualpipe offline HDR.\n");
    printf("\t 3) dualpipe LL.\n");
    printf("\t 4) dualpipe SR.\n");
	printf("\t 5) dualpipe onine HDR.\n");
#endif
	printf("\t 6) dualpipe user mode.\n");

    printf("intf:\n");
    printf("\t 0) vo cvbs output, default.\n");
    printf("\t 1) vo BT1120 output.\n");
}

static HI_S32 SNAP_StartVO(VO_DEV VoDev, VO_LAYER VoLayer, 
                    SAMPLE_VO_MODE_E enVoMode, HI_BOOL bVgsBypass)
{
    VO_PUB_ATTR_S stVoPubAttr;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;
    HI_S32 s32Ret = HI_SUCCESS;

    stVoPubAttr.enIntfType = s_enVoIntfType;
    if (VO_INTF_BT1120 == stVoPubAttr.enIntfType)
    {
        stVoPubAttr.enIntfSync = VO_OUTPUT_1080P30;
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
                    &stLayerAttr.stDispRect.u32Width,
                    &stLayerAttr.stDispRect.u32Height,
                    &stLayerAttr.u32DispFrmRt);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VO_GetWH failed!\n");
        SAMPLE_COMM_VO_StopDev(VoDev);
        return s32Ret;
    }
    stLayerAttr.stImageSize.u32Width  = stLayerAttr.stDispRect.u32Width;
    stLayerAttr.stImageSize.u32Height = stLayerAttr.stDispRect.u32Height;

    s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stLayerAttr, bVgsBypass);
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

static HI_S32 SNAP_StopVO(VO_DEV VoDev, VO_LAYER VoLayer, SAMPLE_VO_MODE_E enVoMode)
{
    SAMPLE_COMM_VO_StopChn(VoDev, enVoMode);
    SAMPLE_COMM_VO_StopLayer(VoLayer);
    SAMPLE_COMM_VO_StopDev(VoDev);

    return HI_SUCCESS;
}

static HI_S32 SNAP_VencCreateChn(VENC_CHN VencChn, PAYLOAD_TYPE_E enType, SIZE_S *pstSize,
                    SAMPLE_RC_E enRcMode, HI_U32 u32SrcFrmRate, HI_U32 u32DstFrmRate)
{
    VENC_CHN_ATTR_S stVencChnAttr;
    HI_S32 s32Ret;

    stVencChnAttr.stVeAttr.enType = enType;
    switch (enType)
    {
    case PT_JPEG:
        {
            VENC_ATTR_JPEG_S stJpegAttr;

            stJpegAttr.u32PicWidth   = pstSize->u32Width;
            stJpegAttr.u32PicHeight  = pstSize->u32Height;
            stJpegAttr.u32MaxPicWidth  = pstSize->u32Width;
            stJpegAttr.u32MaxPicHeight = pstSize->u32Height;
            stJpegAttr.u32BufSize   = pstSize->u32Width * pstSize->u32Height * 3;
            stJpegAttr.bByFrame     = HI_TRUE;/*get stream mode is field mode  or frame mode*/
            stJpegAttr.bSupportDCF  = HI_TRUE;
            memcpy(&stVencChnAttr.stVeAttr.stAttrJpege, &stJpegAttr, sizeof(VENC_ATTR_JPEG_S));
        }
        break;
    case PT_H264:
        {
            VENC_ATTR_H264_S stH264Attr;
            VENC_ATTR_H264_CBR_S stH264Cbr;

            printf("Create Venc H264 Chn...\n");

            stH264Attr.u32MaxPicWidth  = pstSize->u32Width;
            stH264Attr.u32MaxPicHeight = pstSize->u32Height;
            stH264Attr.u32PicWidth  = pstSize->u32Width;  /*the picture width*/
            stH264Attr.u32PicHeight = pstSize->u32Height; /*the picture height*/
            stH264Attr.u32BufSize   = pstSize->u32Width * pstSize->u32Height * 2;/*stream buffer size*/
            stH264Attr.u32Profile = 0;
            stH264Attr.bByFrame = HI_TRUE;    /* get stream mode is slice mode or frame mode?*/
            memcpy(&stVencChnAttr.stVeAttr.stAttrH264e, &stH264Attr, sizeof(VENC_ATTR_H264_S));

            if (SAMPLE_RC_CBR == enRcMode)
            {
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
                stH264Cbr.u32Gop         = 30;
                stH264Cbr.u32StatTime    = 1;   /* stream rate statics time(s) */
                stH264Cbr.u32SrcFrmRate  = u32SrcFrmRate;  /* input frame rate */
                stH264Cbr.fr32DstFrmRate = u32DstFrmRate; /* target frame rate */

                if (pstSize->u32Width * pstSize->u32Height <= 352 * 288) // CIF
                {
                    stH264Cbr.u32BitRate = 512;
                }
                else if (pstSize->u32Width * pstSize->u32Height <= 640 * 480 ) // VGA
                {
                    stH264Cbr.u32BitRate = 1024 * 2;
                }
                else if (pstSize->u32Width * pstSize->u32Height <= 1280 * 720 ) // HD720
                {
                    stH264Cbr.u32BitRate = 1024 * 2;
                }
                else if (pstSize->u32Width * pstSize->u32Height <= 1920 * 1080 ) // HD1080
                {
                    stH264Cbr.u32BitRate = 1024 * 4;
                }
                else if (pstSize->u32Width * pstSize->u32Height <= 2592 * 1944 ) // 5M
                {
                    stH264Cbr.u32BitRate = 1024 * 6;
                }
                else if (pstSize->u32Width * pstSize->u32Height <= 3840 * 2160 ) // UHD4K
                {
                    stH264Cbr.u32BitRate = 1024 * 8;
                }
                else
                {
                    stH264Cbr.u32BitRate = 1024 * 4;
                }

                stH264Cbr.u32FluctuateLevel = 1; /* average bit rate */
                memcpy(&stVencChnAttr.stRcAttr.stAttrH264Cbr, &stH264Cbr, sizeof(VENC_ATTR_H264_CBR_S));
            }
            else if (SAMPLE_RC_FIXQP == enRcMode)
            {
                VENC_ATTR_H264_FIXQP_S  stH264FixQp;

                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264FIXQP;
                stH264FixQp.u32Gop = 30;
                stH264FixQp.u32SrcFrmRate  = u32SrcFrmRate;
                stH264FixQp.fr32DstFrmRate = u32DstFrmRate;
                stH264FixQp.u32IQp = 20;
                stH264FixQp.u32PQp = 23;
                stH264FixQp.u32BQp = 23;
                memcpy(&stVencChnAttr.stRcAttr.stAttrH264FixQp, &stH264FixQp, sizeof(VENC_ATTR_H264_FIXQP_S));
            }
            else if (SAMPLE_RC_VBR == enRcMode)
            {
                VENC_ATTR_H264_VBR_S stH264Vbr;

                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264VBR;
                stH264Vbr.u32Gop = 25;
                stH264Vbr.u32StatTime = 1;
                stH264Vbr.u32SrcFrmRate = u32SrcFrmRate;
                stH264Vbr.fr32DstFrmRate = u32DstFrmRate;
                stH264Vbr.u32MinQp  = 10;
                stH264Vbr.u32MinIQp = 10;
                stH264Vbr.u32MaxQp  = 50;

                if (pstSize->u32Width * pstSize->u32Height <= 352 * 288) // CIF
                {
                    stH264Vbr.u32MaxBitRate = 512 * 3;
                }
                else if (pstSize->u32Width * pstSize->u32Height <= 640 * 480 ) // VGA
                {
                    stH264Vbr.u32MaxBitRate = 1024 * 2;
                }
                else if (pstSize->u32Width * pstSize->u32Height <= 1280 * 720 ) // HD720
                {
                    stH264Vbr.u32MaxBitRate = 1024 * 3;
                }
                else if (pstSize->u32Width * pstSize->u32Height <= 1920 * 1080 ) // HD1080
                {
                    stH264Vbr.u32MaxBitRate = 1024 * 4;
                }
                else if (pstSize->u32Width * pstSize->u32Height <= 2592 * 1944 ) // 5M
                {
                    stH264Vbr.u32MaxBitRate = 1024 * 6;
                }
                else if (pstSize->u32Width * pstSize->u32Height <= 3840 * 2160 ) // UHD4K
                {
                    stH264Vbr.u32MaxBitRate = 1024 * 8;
                }
                else
                {
                    stH264Vbr.u32MaxBitRate = 1024 * 4;
                }

                memcpy(&stVencChnAttr.stRcAttr.stAttrH264Vbr, &stH264Vbr, sizeof(VENC_ATTR_H264_VBR_S));
            }
            else
            {
                return HI_FAILURE;
            }
        }
        break;
    case PT_H265:
        {
            VENC_ATTR_H265_S stH265Attr;
            VENC_ATTR_H265_CBR_S stH265Cbr;

            stH265Attr.u32MaxPicWidth  = pstSize->u32Width;
            stH265Attr.u32MaxPicHeight = pstSize->u32Height;
            stH265Attr.u32PicWidth  = pstSize->u32Width;  /*the picture width*/
            stH265Attr.u32PicHeight = pstSize->u32Height; /*the picture height*/
            stH265Attr.u32BufSize   = pstSize->u32Width * pstSize->u32Height * 2;/*stream buffer size*/

            stH265Attr.u32Profile = 0;        /* 0:MP; */
            stH265Attr.bByFrame = HI_TRUE;    /* get stream mode is slice mode or frame mode?*/
            //stH265Attr.u32BFrameNum = 0; /* 0: not support B frame; >=1: number of B frames */
            //stH265Attr.u32RefNum = 1;    /* 0: default; number of refrence frame*/
            memcpy(&stVencChnAttr.stVeAttr.stAttrH265e, &stH265Attr, sizeof(VENC_ATTR_H265_S));

            if (SAMPLE_RC_CBR == enRcMode)
            {
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H265CBR;
                stH265Cbr.u32Gop         = 30;
                stH265Cbr.u32StatTime    = 1;   /* stream rate statics time(s) */
                stH265Cbr.u32SrcFrmRate  = u32SrcFrmRate;  /* input frame rate */
                stH265Cbr.fr32DstFrmRate = u32DstFrmRate; /* target frame rate */

                if (pstSize->u32Width * pstSize->u32Height <= 352 * 288) // CIF
                {
                    stH265Cbr.u32BitRate = 512;
                }
                else if (pstSize->u32Width * pstSize->u32Height <= 640 * 480 ) // VGA
                {
                    stH265Cbr.u32BitRate = 1024 * 2;
                }
                else if (pstSize->u32Width * pstSize->u32Height <= 1280 * 720 ) // HD720
                {
                    stH265Cbr.u32BitRate = 1024 * 3;
                }
                else if (pstSize->u32Width * pstSize->u32Height <= 1920 * 1080 ) // HD1080
                {
                    stH265Cbr.u32BitRate = 1024 * 4;
                }
                else if (pstSize->u32Width * pstSize->u32Height <= 2592 * 1944 ) // 5M
                {
                    stH265Cbr.u32BitRate = 1024 * 6;
                }
                else if (pstSize->u32Width * pstSize->u32Height <= 3840 * 2160 ) // UHD4K
                {
                    stH265Cbr.u32BitRate = 1024 * 8;
                }
                else
                {
                    stH265Cbr.u32BitRate = 1024 * 4;
                }

                stH265Cbr.u32FluctuateLevel = 1; /* average bit rate */
                memcpy(&stVencChnAttr.stRcAttr.stAttrH265Cbr, &stH265Cbr, sizeof(VENC_ATTR_H265_CBR_S));
            }
            else if (SAMPLE_RC_FIXQP == enRcMode)
            {
                VENC_ATTR_H265_FIXQP_S  stH265FixQp;

                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H265FIXQP;
                stH265FixQp.u32Gop = 30;
                stH265FixQp.u32SrcFrmRate  = u32SrcFrmRate;
                stH265FixQp.fr32DstFrmRate = u32DstFrmRate;
                stH265FixQp.u32IQp = 20;
                stH265FixQp.u32PQp = 23;
                stH265FixQp.u32BQp = 25;
                memcpy(&stVencChnAttr.stRcAttr.stAttrH265FixQp, &stH265FixQp, sizeof(VENC_ATTR_H265_FIXQP_S));
            }
            else if (SAMPLE_RC_VBR == enRcMode)
            {
                VENC_ATTR_H265_VBR_S stH265Vbr;

                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H265VBR;
                stH265Vbr.u32Gop = 25;
                stH265Vbr.u32StatTime = 1;
                stH265Vbr.u32SrcFrmRate = u32SrcFrmRate;
                stH265Vbr.fr32DstFrmRate = u32DstFrmRate;
                stH265Vbr.u32MinQp  = 10;
				stH265Vbr.u32MinIQp = 10;
                stH265Vbr.u32MaxQp  = 40;

                if (pstSize->u32Width * pstSize->u32Height <= 352 * 288) // CIF
                {
                    stH265Cbr.u32BitRate = 512 * 3;
                }
                else if (pstSize->u32Width * pstSize->u32Height <= 640 * 480 ) // VGA
                {
                    stH265Cbr.u32BitRate = 1024 * 2;
                }
                else if (pstSize->u32Width * pstSize->u32Height <= 1280 * 720 ) // HD720
                {
                    stH265Cbr.u32BitRate = 1024 * 3;
                }
                else if (pstSize->u32Width * pstSize->u32Height <= 1920 * 1080 ) // HD1080
                {
                    stH265Cbr.u32BitRate = 1024 * 6;
                }
                else if (pstSize->u32Width * pstSize->u32Height <= 2592 * 1944 ) // 5M
                {
                    stH265Cbr.u32BitRate = 1024 * 8;
                }
                else if (pstSize->u32Width * pstSize->u32Height <= 3840 * 2160 ) // UHD4K
                {
                    stH265Cbr.u32BitRate = 1024 * 8;
                }
                else
                {
                    stH265Cbr.u32BitRate = 1024 * 4;
                }

                stH265Vbr.u32MaxBitRate = 1024 * 6;
                memcpy(&stVencChnAttr.stRcAttr.stAttrH265Vbr, &stH265Vbr, sizeof(VENC_ATTR_H265_VBR_S));
            }
            else
            {
                return HI_FAILURE;
            }
        }
        break;

    default:
        return HI_FAILURE;
    }

    stVencChnAttr.stGopAttr.enGopMode  = VENC_GOPMODE_NORMALP;
    stVencChnAttr.stGopAttr.stNormalP.s32IPQpDelta = 0;

    s32Ret = HI_MPI_VENC_CreateChn(VencChn, &stVencChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VENC_CreateChn [%d] faild with %#x! ===\n", VencChn, s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

static HI_S32 SNAP_VencDestroyChn(VENC_CHN VencChn)
{
    HI_S32 s32Ret;

    s32Ret = HI_MPI_VENC_StopRecvPic(VencChn);
	if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VENC_StopRecvPic [%d] failed with %#x!\n", VencChn, s32Ret);
        return s32Ret;
    }

    s32Ret = HI_MPI_VENC_DestroyChn(VencChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("VENC DestroyChn [%d] failed with %#x!\n", VencChn, s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 SNAP_VencSaveStream(VENC_STREAM_S *pstStream, PAYLOAD_TYPE_E enType,
            const char *pszFileName, const char *pszOpenMode)
{
    FILE *pFile;
    VENC_PACK_S *pstData;
    HI_S32 i;
    HI_S32 s32Ret = HI_SUCCESS;

    pFile = fopen(pszFileName, pszOpenMode);
    if (NULL == pFile)
    {
        fprintf(stderr, "open %s failed\n", pszFileName);
        return HI_FAILURE;
    }

    printf("saving %s ...\n", pszFileName);
    for (i = 0; i < pstStream->u32PackCount; i++)
    {
        pstData = &pstStream->pstPack[i];
        fwrite(pstData->pu8Addr + pstData->u32Offset, 1, pstData->u32Len-pstData->u32Offset, pFile);
    }

    fclose(pFile);

    return s32Ret;
}

/***********************************************************************
* funciton : get file postfix according palyload_type.
***********************************************************************/
HI_S32 SNAP_VencGetFilePostfix(PAYLOAD_TYPE_E enPayload, char* szFilePostfix)
{
    if (PT_H264 == enPayload)
    {
        strcpy(szFilePostfix, ".h264");
    }
    else if (PT_H265 == enPayload)
    {
        strcpy(szFilePostfix, ".h265");
    }
    else if (PT_JPEG == enPayload)
    {
        strcpy(szFilePostfix, ".jpg");
    }
    else if (PT_MJPEG == enPayload)
    {
        strcpy(szFilePostfix, ".mjp");
    }
    else if (PT_MP4VIDEO == enPayload)
    {
        strcpy(szFilePostfix, ".mp4");
    }
    else
    {
        SAMPLE_PRT("payload type err!\n");
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}

static HI_S32 SNAP_VencGetStream(VENC_CHN VencChn, VENC_STREAM_S *pstStream, HI_S32 s32TimeoutMs)
{
    HI_S32 s32Ret;
    fd_set read_fds;
    HI_S32 VencFd;
    VENC_CHN_STAT_S stStat;
    struct timeval stTimeoutVal;

    FD_ZERO(&read_fds);

    VencFd = HI_MPI_VENC_GetFd(VencChn);
    if (VencFd < 0)
    {
        fprintf(stderr, "VencChn %d GetFd faild with%#x!\n", VencChn, VencFd);
        return HI_FAILURE;
    }

    FD_SET(VencFd, &read_fds);

    stTimeoutVal.tv_sec  = s32TimeoutMs / 1000;
    stTimeoutVal.tv_usec = (s32TimeoutMs % 1000) * 1000;
    s32Ret = select(VencFd + 1, &read_fds, NULL, NULL, &stTimeoutVal);
    if (s32Ret < 0)
    {
        fprintf(stderr, "snap jepg select failed!\n");
        return HI_FAILURE;
    }
    else if (0 == s32Ret)
    {
        fprintf(stderr, "venc chn %d get stream time out!\n", VencChn);
        return HI_FAILURE;
    }

    if (!FD_ISSET(VencFd, &read_fds))
    {
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VENC_Query(VencChn, &stStat);
    if (s32Ret != HI_SUCCESS)
    {
        fprintf(stderr, "VencChn %d Query failed with %#x!\n", VencChn, s32Ret);
        return s32Ret;
    }

    if(0 == stStat.u32CurPacks)
    {
        fprintf(stderr, "NOTE: Current  frame is NULL!\n");
        return HI_FAILURE;
    }

    pstStream->pstPack = (VENC_PACK_S*)malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
    if (NULL == pstStream->pstPack)
    {
        fprintf(stderr, "malloc memory failed!\n");
        return HI_FAILURE;
    }

    pstStream->u32PackCount = stStat.u32CurPacks;
    s32Ret = HI_MPI_VENC_GetStream(VencChn, pstStream, -1);
    if (HI_SUCCESS != s32Ret)
    {
        fprintf(stderr, "Venc GetStream failed with %#x!\n", s32Ret);
        free(pstStream->pstPack);
        pstStream->pstPack = NULL;
        return s32Ret;
    }

    return s32Ret;
}

static HI_S32 SNAP_VencReleaseStream(VENC_CHN VencChn, VENC_STREAM_S *pstStream)
{
    if (NULL == pstStream || NULL == pstStream->pstPack)
    {
        return HI_FAILURE;
    }

    HI_MPI_VENC_ReleaseStream(VencChn, pstStream);
    free(pstStream->pstPack);
    pstStream->pstPack = NULL;
    
    return HI_SUCCESS;
}

void* SNAP_ThreadLoop(void *parg)
{
    SNAP_THREAD_CTX_S *pstThreadCtx = (SNAP_THREAD_CTX_S*)parg;

    prctl(PR_SET_NAME, (unsigned long)pstThreadCtx->pThreadName, 0, 0, 0);
    free(pstThreadCtx->pThreadName);
    pstThreadCtx->pThreadName = NULL;
    pstThreadCtx->bRun = HI_TRUE;
    
    do
    {
        HI_S32 s32Ret;

        s32Ret = pstThreadCtx->entryFunc(pstThreadCtx->pUserData);
        
        pthread_mutex_lock(&pstThreadCtx->mutex); 
        if (0 == s32Ret || HI_FALSE == pstThreadCtx->bRun)
        {
            pstThreadCtx->bRun = HI_FALSE;
            pthread_mutex_unlock(&pstThreadCtx->mutex);
            break;
        }
        
        pthread_mutex_unlock(&pstThreadCtx->mutex);
    } while(1);
    
    return NULL;
}

static HI_S32 SNAP_StartThread(THREAD_HANDLE_S *handle, FN_THREAD_LOOP entryFunc,
                    const char *pThreadName, void *pUserData)
{
    HI_S32 s32Ret;
    SNAP_THREAD_CTX_S *pstThreadCtx;

    pstThreadCtx = malloc(sizeof(SNAP_THREAD_CTX_S));
	if(NULL == pstThreadCtx)
	{
		return HI_FAILURE;
	}

    pthread_mutex_init(&pstThreadCtx->mutex, NULL);
    pstThreadCtx->bRun  = HI_FALSE;
    pstThreadCtx->entryFunc = entryFunc;
    pstThreadCtx->pUserData = pUserData;
    pstThreadCtx->pThreadName = pThreadName ? strdup(pThreadName) : NULL;
    
    s32Ret = pthread_create(&pstThreadCtx->thread, NULL, SNAP_ThreadLoop, pstThreadCtx);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("create VencGetStream thread failed, %s\n", strerror(s32Ret));
        pthread_mutex_destroy(&pstThreadCtx->mutex);
        if (NULL != pstThreadCtx->pThreadName)
        {
            free(pstThreadCtx->pThreadName);
        }
        free(pstThreadCtx);
        return HI_FAILURE;
    }

    *handle = (THREAD_HANDLE_S)pstThreadCtx;

    return HI_SUCCESS;
}

static HI_S32 SNAP_StopThread(THREAD_HANDLE_S *handle)
{
    SNAP_THREAD_CTX_S *pstThreadCtx;
    
    if (THREAD_INVALID_HANDLE == *handle)
    {
        SAMPLE_PRT("invalid handle\n");
        return HI_FAILURE;
    }
    pstThreadCtx = (SNAP_THREAD_CTX_S*)*handle;
    if (NULL == pstThreadCtx)
    {
        SAMPLE_PRT("null ptr\n");
        return HI_FAILURE;
    }

    pthread_mutex_lock(&pstThreadCtx->mutex);
    if (HI_FALSE == pstThreadCtx->bRun)
    {
        pthread_mutex_unlock(&pstThreadCtx->mutex);
        return HI_SUCCESS;
    }

    pstThreadCtx->bRun = HI_FALSE;
    pthread_mutex_unlock(&pstThreadCtx->mutex);
    
    pthread_join(pstThreadCtx->thread, NULL);

    pthread_mutex_destroy(&pstThreadCtx->mutex);
    free(pstThreadCtx);

    *handle = THREAD_INVALID_HANDLE;

    return HI_SUCCESS;
}


int MediaRecorder_GetStreamLoop(void *parg)
{
    VENC_CHN VencChn;
    VENC_STREAM_S stStream;
    VENC_PACK_S *pstData;
    MEDIA_RECORDER_CTX_S *pstMediaRecorder = (MEDIA_RECORDER_CTX_S*)parg;
    FILE *pFile;
    HI_S32 i, s32Ret;

    VencChn = pstMediaRecorder->VencChn;
    pFile = pstMediaRecorder->pFile;

    s32Ret = SNAP_VencGetStream(VencChn, &stStream, 1000);
    if (HI_SUCCESS != s32Ret)
    {
        return 1;
    }

    for (i = 0; i < stStream.u32PackCount; i++)
    {
        pstData = &stStream.pstPack[i];
        fwrite(pstData->pu8Addr + pstData->u32Offset, 1, pstData->u32Len-pstData->u32Offset, pFile);
    }

    SNAP_VencReleaseStream(VencChn, &stStream);

    return 1;
}

HI_S32 MediaRecorder_Create(MEDIA_RECORDER_S *phMRecorder)
{
    MEDIA_RECORDER_CTX_S *pstRecorderCtx;

    CHECK_NULL_PTR(phMRecorder);

    pstRecorderCtx = malloc(sizeof(MEDIA_RECORDER_CTX_S));
	if (NULL == pstRecorderCtx)
	{
		SAMPLE_PRT("null ptr\n");
		return HI_FAILURE;
	}
    memset(pstRecorderCtx, 0, sizeof(MEDIA_RECORDER_CTX_S));

    pstRecorderCtx->hThread = THREAD_INVALID_HANDLE;

    pstRecorderCtx->enState = MEDIA_RECORDER_IDLE;

    *phMRecorder = (MEDIA_RECORDER_CTX_S*)pstRecorderCtx;

    return HI_SUCCESS;
}

HI_S32 MediaRecorder_Init(MEDIA_RECORDER_S hMRecorder, VENC_CHN VencChn, MPP_CHN_S stSrcChn)
{
    MEDIA_RECORDER_CTX_S *pstRecorderCtx;

    if (INVALID_MEDIA_RECORDER == hMRecorder)
    {
        return HI_FAILURE;
    }
    pstRecorderCtx = (MEDIA_RECORDER_CTX_S*)hMRecorder;
    CHECK_NULL_PTR(hMRecorder);

    if (MEDIA_RECORDER_IDLE != pstRecorderCtx->enState)
    {
        SAMPLE_PRT("Mediarecorder init called in an invalid state(%d)", pstRecorderCtx->enState);
        return HI_FAILURE;
    }


    if (HI_ID_VPSS != stSrcChn.enModId)
    {
        SAMPLE_PRT("invalid src chn %d\n", stSrcChn.enModId);
        return HI_FAILURE;
    }
    
    pstRecorderCtx->VencChn  = VencChn;
    pstRecorderCtx->stSrcChn = stSrcChn;
    
    pstRecorderCtx->fnCallback = NULL;
    
    pstRecorderCtx->enState = MEDIA_RECORDER_INITIALIZED;
    //printf("MEDIA_RECORDER_INITIALIZED\n");
    
    return HI_SUCCESS;
}
HI_S32 MediaRecorder_SetVideoEncoder(MEDIA_RECORDER_S hMRecorder, VIDEO_ENCODER_E enEncoder)
{
    MEDIA_RECORDER_CTX_S *pstRecorderCtx;

    if (INVALID_MEDIA_RECORDER == hMRecorder)
    {
        return HI_FAILURE;
    }
    pstRecorderCtx = (MEDIA_RECORDER_CTX_S*)hMRecorder;
    CHECK_NULL_PTR(pstRecorderCtx);

    if (MEDIA_RECORDER_INITIALIZED != pstRecorderCtx->enState)
    {
        SAMPLE_PRT("Set video encoder in an invalid state(%d)\n", pstRecorderCtx->enState);
        return HI_FAILURE;
    }

    pstRecorderCtx->enEncoder = enEncoder;

    return HI_SUCCESS;
}
HI_S32 MediaRecorder_SetOutputFile(MEDIA_RECORDER_S hMRecorder, HI_CHAR *pszFileName)
{
    MEDIA_RECORDER_CTX_S *pstRecorderCtx;

    if (INVALID_MEDIA_RECORDER == hMRecorder)
    {
        return HI_FAILURE;
    }
    pstRecorderCtx = (MEDIA_RECORDER_CTX_S*)hMRecorder;
    CHECK_NULL_PTR(pstRecorderCtx);

    if (MEDIA_RECORDER_INITIALIZED != pstRecorderCtx->enState)
    {
        SAMPLE_PRT("Set video output file in an invalid state(%d)\n", pstRecorderCtx->enState);
        return HI_FAILURE;
    }

    pstRecorderCtx->pszFileName = (pszFileName != NULL) ? strdup(pszFileName) : NULL;

    return HI_SUCCESS;
}
HI_S32 MediaRecorder_Prepare(MEDIA_RECORDER_S hMRecorder)
{
    VENC_CHN VencChn;
    SIZE_S stSize = {0};
    MEDIA_RECORDER_CTX_S *pstRecorderCtx;
    PAYLOAD_TYPE_E enPayload;
    HI_S32 s32Ret;

    if (INVALID_MEDIA_RECORDER == hMRecorder)
    {
        return HI_FAILURE;
    }
    pstRecorderCtx = (MEDIA_RECORDER_CTX_S*)hMRecorder;
    CHECK_NULL_PTR(pstRecorderCtx);

    if (MEDIA_RECORDER_INITIALIZED != pstRecorderCtx->enState)
    {
        SAMPLE_PRT("Mediarecorder prepare in an invalid state(%d)\n", pstRecorderCtx->enState);
        return HI_FAILURE;
    }

    VencChn = pstRecorderCtx->VencChn;

    if (HI_ID_VPSS == pstRecorderCtx->stSrcChn.enModId)
    {
        VPSS_GRP VpssGrp = pstRecorderCtx->stSrcChn.s32DevId;
        VPSS_CHN VpssChn = pstRecorderCtx->stSrcChn.s32ChnId;
        VPSS_CHN_MODE_S stVpssChnMode;

        HI_MPI_VPSS_GetChnMode(VpssGrp, VpssChn, &stVpssChnMode);
        stSize.u32Width  = stVpssChnMode.u32Width;
        stSize.u32Height = stVpssChnMode.u32Height;
        SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VpssChn);
    }

    if (VIDEO_ENCODER_H265 == pstRecorderCtx->enEncoder)
    {
        enPayload = PT_H265;
    }
    else
    {
        enPayload = PT_H264;
    }

    s32Ret = SNAP_VencCreateChn(VencChn, enPayload, &stSize, SAMPLE_RC_CBR, 30, 30);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start venc chn: %d failed with %#x\n", VencChn, s32Ret);
        return s32Ret;
    }

    pstRecorderCtx->enState = MEDIA_RECORDER_PREPARED;
    //printf("MEDIA_RECORDER_PREPARED\n");

    return HI_SUCCESS;
}
HI_S32 MediaRecorder_Start(MEDIA_RECORDER_S hMRecorder)
{
    HI_S32 s32Ret;
    MEDIA_RECORDER_CTX_S *pstRecorderCtx;
    VENC_CHN VencChn;
    FILE *pFile;

    if (INVALID_MEDIA_RECORDER == hMRecorder)
    {
        return HI_FAILURE;
    }
    pstRecorderCtx = (MEDIA_RECORDER_CTX_S*)hMRecorder;
    CHECK_NULL_PTR(pstRecorderCtx);
    
    if (MEDIA_RECORDER_PREPARED != pstRecorderCtx->enState)
    {
        SAMPLE_PRT("Mediarecorder start in an invalid state(%d)\n", pstRecorderCtx->enState);
        return HI_FAILURE;
    }

    VencChn = pstRecorderCtx->VencChn;

    s32Ret = HI_MPI_VENC_StartRecvPic(VencChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("venc chn %d StartRecvPic failed with %#x\n", VencChn, s32Ret);
        return s32Ret;
    }

    pFile = fopen(pstRecorderCtx->pszFileName, "wb+");
    if (NULL == pFile)
    {
        fprintf(stderr, "open %s failed\n", pstRecorderCtx->pszFileName);
		s32Ret = HI_MPI_VENC_StopRecvPic(VencChn);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("venc chn %d StopRecvPic failed with %#x\n", VencChn, s32Ret);
		}
        return HI_FAILURE;
    }
    pstRecorderCtx->pFile = pFile;

    s32Ret = SNAP_StartThread(&pstRecorderCtx->hThread, MediaRecorder_GetStreamLoop,
                    "GetStream", pstRecorderCtx);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start get stream thread failed\n");
        s32Ret = HI_MPI_VENC_StopRecvPic(VencChn);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("venc chn %d StopRecvPic failed with %#x\n", VencChn, s32Ret);
			return s32Ret;
		}
		
        return s32Ret;
    }

    pstRecorderCtx->enState = MEDIA_RECORDER_RECORDING;
    //printf("MEDIA_RECORDER_RECORDING\n");

    return HI_SUCCESS;
}
HI_S32 MediaRecorder_Stop(MEDIA_RECORDER_S hMRecorder)
{
    HI_S32 s32Ret;
    MEDIA_RECORDER_CTX_S *pstRecorderCtx;
    VENC_CHN VencChn;

    if (INVALID_MEDIA_RECORDER == hMRecorder)
    {
        return HI_SUCCESS;
    }

    pstRecorderCtx = (MEDIA_RECORDER_CTX_S*)hMRecorder;
    CHECK_NULL_PTR(pstRecorderCtx);
    
    if (MEDIA_RECORDER_RECORDING != pstRecorderCtx->enState)
    {
        SAMPLE_PRT("Mediarecorder stop in an invalid state(%d)\n", pstRecorderCtx->enState);
        return HI_FAILURE;
    }

    VencChn = pstRecorderCtx->VencChn;
    
    s32Ret = HI_MPI_VENC_StopRecvPic(VencChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("venc chn %d StopRecvPic failed with %#x\n", VencChn, s32Ret);
        return s32Ret;
    }

    s32Ret = SNAP_StopThread(&pstRecorderCtx->hThread);

    fflush(pstRecorderCtx->pFile);
    fclose(pstRecorderCtx->pFile);
    pstRecorderCtx->pFile = NULL;

    pstRecorderCtx->enState = MEDIA_RECORDER_PREPARED;
    //printf("MEDIA_RECORDER_PREPARED\n");

    return HI_SUCCESS;
}
HI_S32 MediaRecorder_Reset(MEDIA_RECORDER_S hMRecorder)
{
    VENC_CHN VencChn;
    MEDIA_RECORDER_CTX_S *pstRecorderCtx;
    HI_S32 s32Ret;

    if (INVALID_MEDIA_RECORDER == hMRecorder)
    {
        return HI_SUCCESS;
    }
    pstRecorderCtx = (MEDIA_RECORDER_CTX_S*)(hMRecorder);
    CHECK_NULL_PTR(pstRecorderCtx);

    VencChn = pstRecorderCtx->VencChn;

    if (MEDIA_RECORDER_RECORDING == pstRecorderCtx->enState)
    {
        s32Ret = MediaRecorder_Stop(hMRecorder);
        if (HI_SUCCESS == s32Ret)
        {
            MediaRecorder_Reset(hMRecorder);
        }
    }
    else if (MEDIA_RECORDER_PREPARED == pstRecorderCtx->enState)
    {
        if (HI_ID_VPSS == pstRecorderCtx->stSrcChn.enModId)
        {
            SAMPLE_COMM_VENC_UnBindVpss(VencChn, pstRecorderCtx->stSrcChn.s32DevId,
                    pstRecorderCtx->stSrcChn.s32ChnId);
        }
        SNAP_VencDestroyChn(VencChn);
        pstRecorderCtx->enState = MEDIA_RECORDER_IDLE;
        //printf("MEDIA_RECORDER_IDLE\n");
    }
    else if (MEDIA_RECORDER_INITIALIZED == pstRecorderCtx->enState)
    {
        pstRecorderCtx->enState = MEDIA_RECORDER_IDLE;
        //printf("MEDIA_RECORDER_IDLE\n");
    }

    return HI_SUCCESS;
}
HI_S32 MediaRecorder_Release(MEDIA_RECORDER_S *phMRecorder)
{
    MEDIA_RECORDER_CTX_S *pstRecorderCtx;

    if (INVALID_MEDIA_RECORDER == *phMRecorder)
    {
        return HI_SUCCESS;
    }

    pstRecorderCtx = (MEDIA_RECORDER_CTX_S*)(*phMRecorder);
    CHECK_NULL_PTR(pstRecorderCtx);

    MediaRecorder_Reset(*phMRecorder);

    if (NULL != pstRecorderCtx->pszFileName)
    {
        free(pstRecorderCtx->pszFileName);
    }
    
    free(pstRecorderCtx);
    *phMRecorder = INVALID_MEDIA_RECORDER;
    
    return HI_SUCCESS;
}

HI_S32 SNAP_CreateVencEncoder(VENC_CHN VenChn, MPP_CHN_S stSrcChn,
            VIDEO_ENCODER_E enEncoder, MEDIA_RECORDER_S *phMRecorder)
{
    HI_CHAR szFileName[64] = {0};
    HI_S32 s32Ret;

    if (VIDEO_ENCODER_H265 == enEncoder)
    {
        snprintf(szFileName,sizeof(szFileName), "stream_%d.h265", VenChn);
    }
    else if (VIDEO_ENCODER_H264 == enEncoder)
    {
        snprintf(szFileName, sizeof(szFileName),"stream_%d.h264", VenChn);
    }
    else
    {
        SAMPLE_PRT("invalid video encoder %d\n", enEncoder);
        return HI_FAILURE;
    }

    s32Ret = MediaRecorder_Create(phMRecorder);
    if (HI_SUCCESS != s32Ret)
    {
        return s32Ret;    
    }
    
    if ( HI_SUCCESS != (s32Ret = MediaRecorder_Init(*phMRecorder, VenChn, stSrcChn)))
    {
        goto release;
    }
    if ( HI_SUCCESS != (s32Ret = MediaRecorder_SetVideoEncoder(*phMRecorder, enEncoder)))
    {
        goto release;
    }
    if ( HI_SUCCESS != (s32Ret = MediaRecorder_SetOutputFile(*phMRecorder, szFileName)))
    {
        goto release;
    }
    if ( HI_SUCCESS != (s32Ret = MediaRecorder_Prepare(*phMRecorder)))
    {
        goto release;
    }
    if ( HI_SUCCESS != (s32Ret = MediaRecorder_Start(*phMRecorder)))
    {
        goto release;
    }

    return HI_SUCCESS;

release:
    MediaRecorder_Release(phMRecorder);
    return s32Ret;
}

HI_S32 SNAP_DestroyVencEncoder(VENC_CHN VenChn, MEDIA_RECORDER_S *phMRecorder)
{
    MediaRecorder_Stop(*phMRecorder);
    MediaRecorder_Release(phMRecorder);

    return HI_SUCCESS;
}


/*
 * convert YUV semi-plannar420 to YUV plannar420
 * convert YUV semi-plannar422 to YUV plannar422
 * */
static HI_S32 save_yuvframe(VIDEO_FRAME_S *pstFrame, FILE* pfd)
{
    HI_U32 u32Size, u32UvHeight; // UV height in plannar format
    HI_U8 *pUserPageAddr;
    HI_U8 *pVBufVirt_Y, *pVBufVirt_C;
    HI_U8 *pMemContent;
    HI_U8 *pLineBuf;
    HI_S32 h, w;
    PIXEL_FORMAT_E  enPixelFormat = pstFrame->enPixelFormat;

    if (PIXEL_FORMAT_YUV_SEMIPLANAR_420 == enPixelFormat)
    {
        u32Size = pstFrame->u32Stride[0] * pstFrame->u32Height * 3 / 2;
        u32UvHeight = pstFrame->u32Height / 2;
    }
    else if (PIXEL_FORMAT_YUV_SEMIPLANAR_422 == enPixelFormat)
    {
        u32Size = pstFrame->u32Stride[0] * pstFrame->u32Height * 2;
        u32UvHeight = pstFrame->u32Height;
    }
    else if (PIXEL_FORMAT_YUV_400 == enPixelFormat)
    {
		u32Size = pstFrame->u32Stride[0] * pstFrame->u32Height;
		u32UvHeight = 0;
	}
    else
    {
        SAMPLE_PRT("invalid pixel format %d\n", enPixelFormat);
        return HI_FAILURE;
    }

    pLineBuf = (HI_U8*)malloc(pstFrame->u32Width);
    if (NULL == pLineBuf)
    {
        SAMPLE_PRT("malloc line buf failed\n");
        return HI_FAILURE;
    }

    printf("phy_addr=0x%x, width=%d, size=%d, stride0=%d, stride1=%d\n", pstFrame->u32PhyAddr[0], 
                pstFrame->u32Width, u32Size,
                pstFrame->u32Stride[0], pstFrame->u32Stride[1]);
    pUserPageAddr = (HI_U8 *)HI_MPI_SYS_Mmap(pstFrame->u32PhyAddr[0], u32Size);
    if (NULL == pUserPageAddr)
    {
        free(pLineBuf);
        return HI_FAILURE;
    }

    pVBufVirt_Y = pUserPageAddr;
    pVBufVirt_C = pVBufVirt_Y + pstFrame->u32Stride[0] * pstFrame->u32Height;

    fprintf(stderr, "saving......Y......");
    fflush(stderr);
    for( h = 0; h < pstFrame->u32Height; h++)
    {
        pMemContent = pVBufVirt_Y + h * pstFrame->u32Stride[0];
        fwrite(pMemContent, pstFrame->u32Width, 1, pfd);
    }
    fflush(pfd);

    fprintf(stderr, "U......");
    fflush(stderr);
    for(h = 0; h < u32UvHeight; h++)
    {
        pMemContent = pVBufVirt_C + h * pstFrame->u32Stride[1];
        pMemContent += 1;

        for(w = 0; w < pstFrame->u32Width / 2; w++)
        {
            pLineBuf[w] = *pMemContent;
            pMemContent += 2;
        }
        fwrite(pLineBuf, pstFrame->u32Width / 2, 1, pfd);
    }
    fflush(pfd);

    fprintf(stderr, "V......\n");
    fflush(stderr);
    for(h = 0; h < u32UvHeight; h++)
    {
        pMemContent = pVBufVirt_C + h * pstFrame->u32Stride[1];

        for(w = 0; w < pstFrame->u32Width / 2; w++)
        {
            pLineBuf[w] = *pMemContent;
            pMemContent += 2;
        }
        fwrite(pLineBuf, pstFrame->u32Width / 2, 1, pfd);
    }
    fflush(pfd);

    fflush(stderr);

    free(pLineBuf);
    HI_MPI_SYS_Munmap(pUserPageAddr, u32Size);

    return HI_SUCCESS;
}

static inline HI_S32 pixelFormat2BitWidth(PIXEL_FORMAT_E enPixelFormat)
{
    HI_U32 u32Nbit;

    if (PIXEL_FORMAT_RGB_BAYER_8BPP == enPixelFormat)
    {
        u32Nbit = 8;
    }
    else if (PIXEL_FORMAT_RGB_BAYER_10BPP == enPixelFormat)
    {
        u32Nbit = 10;
    }
    else if (PIXEL_FORMAT_RGB_BAYER_12BPP == enPixelFormat)
    {
        u32Nbit = 12;
    }
    else if (PIXEL_FORMAT_RGB_BAYER_14BPP == enPixelFormat)
    {
        u32Nbit = 14;
    }
    else
    {
        u32Nbit = 16;
    }
    
    return u32Nbit;
}

/*
 * Convert the input compact pixel data to 16bit data
 * support input 10bit/12bit/14bit
 * pu8Data:     input data, bit width is u32BitWidth
 * u32DataNum:  input pixel number
 * pu16OutData: save result data
 *
 * RETURN:      return the converted pixel number
 * */
HI_S32 convertBitPixel(HI_U8 *pu8Data, HI_U32 u32DataNum, HI_U32 u32BitWidth, HI_U16 *pu16OutData)
{
    HI_S32 i, u32Tmp, s32OutCnt;
    HI_U32 u32Val;
    HI_U64 u64Val;
    HI_U8 *pu8Tmp = pu8Data;
    
    s32OutCnt = 0;
    switch(u32BitWidth) 
    {
    case 10:
        {
            /* 4 pixels consist of 5 bytes  */
            u32Tmp = u32DataNum / 4;

            for (i = 0; i < u32Tmp; i++)
            {
                /* byte4 byte3 byte2 byte1 byte0 */
                pu8Tmp = pu8Data + 5 * i;
                u64Val = pu8Tmp[0] + ((HI_U32)pu8Tmp[1] << 8) + ((HI_U32)pu8Tmp[2] << 16) + 
                         ((HI_U32)pu8Tmp[3] << 24) + ((HI_U64)pu8Tmp[4] << 32);
                
                pu16OutData[s32OutCnt++] = u64Val & 0x3ff;
                pu16OutData[s32OutCnt++] = (u64Val >> 10) & 0x3ff;
                pu16OutData[s32OutCnt++] = (u64Val >> 20) & 0x3ff;
                pu16OutData[s32OutCnt++] = (u64Val >> 30) & 0x3ff;
            }
        }
        break;
    case 12:
        {
            /* 2 pixels consist of 3 bytes  */
            u32Tmp = u32DataNum / 2;

            for (i = 0; i < u32Tmp; i++)
            {
                /* byte2 byte1 byte0 */
                pu8Tmp = pu8Data + 3 * i;
                u32Val = pu8Tmp[0] + (pu8Tmp[1] << 8) + (pu8Tmp[2] << 16);
                pu16OutData[s32OutCnt++] = u32Val & 0xfff;
                pu16OutData[s32OutCnt++] = (u32Val >> 12) & 0xfff;
            }
        }
        break;
    case 14:
        {
            /* 4 pixels consist of 7 bytes  */
            u32Tmp = u32DataNum / 4;

            for (i = 0; i < u32Tmp; i++)
            {
                pu8Tmp = pu8Data + 7 * i;
                u64Val = pu8Tmp[0] + ((HI_U32)pu8Tmp[1] << 8) + ((HI_U32)pu8Tmp[2] << 16) + 
                         ((HI_U32)pu8Tmp[3] << 24) + ((HI_U64)pu8Tmp[4] << 32) + 
                         ((HI_U64)pu8Tmp[5] << 40) + ((HI_U64)pu8Tmp[6] << 48);

                pu16OutData[s32OutCnt++] = u64Val & 0x3fff;
                pu16OutData[s32OutCnt++] = (u64Val >> 14) & 0x3fff;
                pu16OutData[s32OutCnt++] = (u64Val >> 28) & 0x3fff;
                pu16OutData[s32OutCnt++] = (u64Val >> 42) & 0x3fff;
            }
        }
        break;
    default:
        fprintf(stderr, "unsuport bitWidth: %d\n", u32BitWidth);
        return -1;
        break;
    }

    return s32OutCnt;
}

static HI_S32 save_rawframe(VIDEO_FRAME_S* pstFrame, FILE* pfd)
{
    HI_U32 u32H;
    HI_U32 u32Size, u32Nbit;
    HI_VOID* pUserPageAddr;
    HI_U16 *pu16Data = NULL;
    HI_U8 *pu8Data;

    u32Nbit = pixelFormat2BitWidth(pstFrame->enPixelFormat);
    
    u32Size = (pstFrame->u32Stride[0]) * (pstFrame->u32Height) * 2;
    pUserPageAddr = HI_MPI_SYS_Mmap(pstFrame->u32PhyAddr[0], u32Size);
    if (NULL == pUserPageAddr)
    {
        return -1;
    }

    if ((8 != u32Nbit) && (16 != u32Nbit))
    {
        pu16Data = (HI_U16*)malloc(pstFrame->u32Width * 2);
        if (NULL == pu16Data)
        {
            fprintf(stderr, "alloc memory failed\n");
            HI_MPI_SYS_Munmap(pUserPageAddr, u32Size);
            return HI_FAILURE;
        }
    }

    fprintf(stderr, "saving raw data...... PixelFormat: %d,  u32Stride[0]: %d, width: %d, u32TimeRef: %d\n",
                pstFrame->enPixelFormat, pstFrame->u32Stride[0], pstFrame->u32Width, pstFrame->u32TimeRef);

    pu8Data = pUserPageAddr;
    for (u32H = 0; u32H < pstFrame->u32Height; u32H++)
    {
        if (8 == u32Nbit)
        {
            fwrite(pu8Data, pstFrame->u32Width, 1, pfd);
        }
        else if (16 == u32Nbit)
        {
            fwrite(pu8Data, pstFrame->u32Width, 2, pfd);
        }
        else
        {
            convertBitPixel(pu8Data, pstFrame->u32Width, u32Nbit, pu16Data);
            fwrite(pu16Data, pstFrame->u32Width, 2, pfd);
        }

        pu8Data += pstFrame->u32Stride[0];
    }
    fflush(pfd);

    if (NULL != pu16Data)
    {
        free(pu16Data);
    }
    HI_MPI_SYS_Munmap(pUserPageAddr, u32Size);

    return HI_SUCCESS;
}

/*
 * save YUV frame or raw frame
 */
HI_S32 SNAP_SaveFrame(VIDEO_FRAME_S* pstFrame, const HI_CHAR *pszName)
{
    FILE *pFile;
    HI_S32 s32Ret = HI_FAILURE;

    pFile = fopen(pszName, "w+");
    if (NULL == pFile)
    {
        SAMPLE_PRT("open %s failed\n", pszName);
        return -1;
    }

    if (PIXEL_FORMAT_RGB_BAYER_8BPP <= pstFrame->enPixelFormat
        && PIXEL_FORMAT_RGB_BAYER >= pstFrame->enPixelFormat)
    {
        s32Ret = save_rawframe(pstFrame, pFile);
    }
    else if (PIXEL_FORMAT_YUV_SEMIPLANAR_422 == pstFrame->enPixelFormat
        || PIXEL_FORMAT_YUV_SEMIPLANAR_420 == pstFrame->enPixelFormat
        || PIXEL_FORMAT_YUV_400 == pstFrame->enPixelFormat)
    {
        s32Ret = save_yuvframe(pstFrame, pFile);
    }

    fclose(pFile);

    return s32Ret;
}

HI_S32 SNAP_GetFrame(MPP_CHN_S *pstChn, VIDEO_FRAME_INFO_S *pstFrameInfo, HI_S32 s32MilliSec)
{
    HI_S32 s32Ret = HI_FAILURE;
    const char *pszModName = "unkown";
    
    if (HI_ID_VIU == pstChn->enModId)
    {
        s32Ret = HI_MPI_VI_GetFrame(pstChn->s32ChnId, pstFrameInfo, s32MilliSec);
        pszModName = "VI";
    }
    else if (HI_ID_VPSS == pstChn->enModId)
    {
        s32Ret = HI_MPI_VPSS_GetChnFrame(pstChn->s32DevId, pstChn->s32ChnId, pstFrameInfo, s32MilliSec);
        pszModName = "VPSS";
    }
    if (HI_SUCCESS != s32Ret)
    {
        fprintf(stderr, "Get frame from %s failed\n", pszModName);
        return s32Ret;
    }

    printf("get frame from %s, time ref: %d, flags: 0x%x\n", pszModName, 
                pstFrameInfo->stVFrame.u32TimeRef,
                pstFrameInfo->stVFrame.stSupplement.enFlashType);

    return s32Ret;
}

HI_S32 SNAP_ReleaseFrame(MPP_CHN_S *pstChn, VIDEO_FRAME_INFO_S *pstFrameInfo)
{
    if (VB_INVALID_POOLID != pstFrameInfo->u32PoolId)
    {
        if (HI_ID_VIU == pstChn->enModId)
        {
            HI_MPI_VI_ReleaseFrame(pstChn->s32ChnId, pstFrameInfo);
            pstFrameInfo->u32PoolId = VB_INVALID_POOLID;
        }
        else if (HI_ID_VPSS == pstChn->enModId)
        {
            HI_MPI_VPSS_ReleaseChnFrame(pstChn->s32DevId, pstChn->s32ChnId, pstFrameInfo);
            pstFrameInfo->u32PoolId = VB_INVALID_POOLID;
        }  
    }

    return HI_SUCCESS;
}


/*****************************************************************************
* function : Vi chn bind vpss group
*****************************************************************************/
static HI_S32 SNAP_VI_BindVpss(VI_CHN ViChn, VPSS_GRP VpssGrp)
{
    HI_S32 s32Ret;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId  = HI_ID_VIU;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = ViChn;

    stDestChn.enModId  = HI_ID_VPSS;
    stDestChn.s32DevId = VpssGrp;
    stDestChn.s32ChnId = 0;

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }
        
    return HI_SUCCESS;
}

static HI_S32 SNAP_VI_UnBindVpss(VI_CHN ViChn, VPSS_GRP VpssGrp)
{
    HI_S32 s32Ret;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId = HI_ID_VIU;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = ViChn;

    stDestChn.enModId = HI_ID_VPSS;
    stDestChn.s32DevId = VpssGrp;
    stDestChn.s32ChnId = 0;

    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}


static HI_S32 SNAP_StartViIsp(SNAP_PIPE_MODE_E enPipeMode, SAMPLE_VI_CONFIG_S *pstViConfig ,HI_BOOL bDualPipeOnline)
{
    HI_U32 u32SnsId = 0;
    COMBO_DEV MipiDev = 0;
    HI_S32 s32ChnNum = 1;
    HI_S32 i, s32Ret;

    if (SNAP_PIPE_MODE_DUAL == enPipeMode)
    {
        s32ChnNum = 2;
    }
    
    // start MIPI_Rx 0
    s32Ret = SAMPLE_COMM_VI_SetMipiAttr(MipiDev, pstViConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("mipi %d init failed!\n", MipiDev);
        return HI_FAILURE;
    }

    printf("s32ChnNum = %d\n", s32ChnNum);
    for (i = 0; i < s32ChnNum; i++)
    {
        VI_DEV ViDev = i;
        VI_CHN ViChn = i;
        ISP_DEV IspDev = i;
        VI_DEV_ATTR_S  stViDevAttr;
        VI_CHN_ATTR_S  stViChnAttr;
        ISP_PUB_ATTR_S stIspPubAttr;
        HI_U32 u32PreviewWidth, u32PreviewHeight;

        SAMPLE_COMM_VI_GetDevAttrBySns(pstViConfig->enViMode, &stViDevAttr);
        if (1 == ViDev)
        {
            // preview pipe
            u32PreviewWidth  = stViDevAttr.stDevRect.u32Width / 2;
            u32PreviewHeight = stViDevAttr.stDevRect.u32Height / 2;
            stViDevAttr.stBasAttr.stSacleAttr.stBasSize.u32Width  = u32PreviewWidth;
            stViDevAttr.stBasAttr.stSacleAttr.stBasSize.u32Height = u32PreviewHeight;
        }
        else
        {
            u32PreviewWidth  = stViDevAttr.stDevRect.u32Width;
            u32PreviewHeight = stViDevAttr.stDevRect.u32Height;
        }

        s32Ret = SAMPLE_COMM_ISP_Sensor_Regiter_callback(IspDev, u32SnsId);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("register sensor %d to ISP %d failed\n", u32SnsId, IspDev);
            return HI_FAILURE;
        }
        
        s32Ret = SAMPLE_COMM_ISP_Init(IspDev, pstViConfig->enWDRMode, pstViConfig->enFrmRate);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("ISP %d init failed!\n", IspDev);
            return HI_FAILURE;
        }
        if (1 == IspDev)
        {
            HI_MPI_ISP_GetPubAttr(IspDev, &stIspPubAttr);
            stIspPubAttr.stWndRect.u32Width  = u32PreviewWidth;
            stIspPubAttr.stWndRect.u32Height = u32PreviewHeight;
            HI_MPI_ISP_SetPubAttr(IspDev, &stIspPubAttr);
        }

        s32Ret = SAMPLE_COMM_ISP_Run(IspDev);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("ISP %d run failed!\n", IspDev);
            return HI_FAILURE;
        }

        stViChnAttr.stCapRect.s32X = 0;
        stViChnAttr.stCapRect.s32Y = 0;
        stViChnAttr.stCapRect.u32Width   = u32PreviewWidth;
        stViChnAttr.stCapRect.u32Height  = u32PreviewHeight;
        stViChnAttr.stDestSize.u32Width  = stViChnAttr.stCapRect.u32Width;
        stViChnAttr.stDestSize.u32Height = stViChnAttr.stCapRect.u32Height;
        stViChnAttr.enCapSel = VI_CAPSEL_BOTH;
        stViChnAttr.enPixFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
        stViChnAttr.enCompressMode = COMPRESS_MODE_NONE;
        stViChnAttr.bMirror = HI_FALSE;
        stViChnAttr.bFlip = HI_FALSE;
        stViChnAttr.s32SrcFrameRate = -1;
        stViChnAttr.s32DstFrameRate = -1;
        s32Ret = HI_MPI_VI_SetDevAttr(ViDev, &stViDevAttr);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_VI_SetDevAttr failed with 0x%x!\n", s32Ret);
            SAMPLE_COMM_ISP_Stop();
            return HI_FAILURE;
        }
        s32Ret = HI_MPI_VI_SetChnAttr(ViChn, &stViChnAttr);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_VI_SetChnAttr failed with 0x%x!\n", s32Ret);
            SAMPLE_COMM_ISP_Stop();
            return HI_FAILURE;
        }

        // MIPI0 connect to VI1, MIPI1 connect to VI0
        if (SNAP_PIPE_MODE_DUAL == enPipeMode)
        {
            VI_DEV_BIND_ATTR_S stDevBindAttr;
			if(HI_FALSE == bDualPipeOnline)
			{
				stDevBindAttr.s32MipiDev = (1 == ViDev) ? 0 : 1 ;
			}
			else
			{
				stDevBindAttr.s32MipiDev = 0;
			}
            HI_MPI_VI_BindDev(ViDev, &stDevBindAttr);
        }

        s32Ret = HI_MPI_VI_EnableDev(ViDev);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_VI_EnableDev failed with 0x%x!\n", s32Ret);
            SAMPLE_COMM_ISP_Stop();
            return HI_FAILURE;
        }
        s32Ret = HI_MPI_VI_EnableChn(ViChn);
        if (HI_SUCCESS != s32Ret)
        {
            HI_S32 j;
            SAMPLE_PRT("HI_MPI_VI_EnableDev failed with 0x%x!\n", s32Ret);
            for (j = 0; j <= ViDev; j++)
            {
                HI_MPI_VI_DisableDev(j);
            }
            SAMPLE_COMM_ISP_Stop();
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

static HI_S32 SNAP_StopViIsp(SNAP_PIPE_MODE_E enPipeMode)
{
    HI_S32 s32ChnNum = 1;
    HI_S32 i, s32Ret;

    if (SNAP_PIPE_MODE_DUAL == enPipeMode)
    {
        s32ChnNum = 2;
    }

    for (i = 0; i < s32ChnNum; i++)
    {
        VI_CHN ViChn = i;
        s32Ret = HI_MPI_VI_DisableChn(ViChn);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_VI_DisableChn failed with 0x%x\n", s32Ret);
        }
    }

    for (i = 0; i < s32ChnNum; i++)
    {
        VI_DEV ViDev = i;
        s32Ret = HI_MPI_VI_DisableDev(ViDev);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_VI_DisableDev failed with 0x%x\n", s32Ret);
        }
    }

    SAMPLE_COMM_ISP_Stop();

    return HI_SUCCESS;
}

/*
 * Description:
 *  single pipe: sensor0 -> MIPI0 -> VIDev0 -> ISP0 -> VI CHN0
 *      picture pipe: VI CHN0 -> VPSS(group 0, chn 0) -> VO(layer 0, chn 0)
 *      preview pipe: VI CHN0 -> VPSS(group 1, chn 0) -> VO(layer 0, chn 1)
 *  dual pipe:
 *      picture pipe: ISP0 -> VI CHN0 -> VPSS(group 0, chn 0) -> VO(layer 0, chn 0)
 *      preview pipe: sensor0 -> MIPI0 -> VIDev1 -> ISP1 -> VI CHN1 -> VPSS(group 1, chn 0) -> VO(layer 0, chn 1)
 * Parameter:
 *     @enPipeMode:   pipe mode
 */
static HI_S32 SNAP_StartViVpssVo(SNAP_PIPE_MODE_E enPipeMode, SAMPLE_VI_CONFIG_S *pstViConfig, HI_BOOL bPhotoNrEn
								 ,HI_U32 u32PhotoRefNum , HI_BOOL bDualPipeOnline)
{
    VO_DEV VoDev = 0;
    VO_LAYER VoLayer = 0;
    SAMPLE_VO_MODE_E enVoMode = VO_MODE_2MUX;
    HI_U32 u32SnsId = 0;
    ISP_DEV IspDev0 = 0, IspDev1 = 1;
    VI_CHN  ViChn0  = 0, ViChn1  = 1;
    VPSS_GRP VpssGrp0 = 0, VpssGrp1 = 1;
    VPSS_CHN VpssChn0 = 0;
    HI_S32 s32Ret;

    // ISP0 don't control sensor, ISP1 controls sensor 0
    if (SNAP_PIPE_MODE_DUAL == enPipeMode)
    {
        SAMPLE_COMM_ISP_BindSns(IspDev0, u32SnsId, pstViConfig->enViMode, -1);
        SAMPLE_COMM_ISP_BindSns(IspDev1, u32SnsId, pstViConfig->enViMode, 0);
    }
    else
    {
        SAMPLE_COMM_ISP_BindSns(IspDev0, u32SnsId, pstViConfig->enViMode, 0);
        SAMPLE_COMM_ISP_BindSns(IspDev1, u32SnsId, pstViConfig->enViMode, 1);
    }

    // Start VO
    s32Ret = SNAP_StartVO(VoDev, VoLayer, enVoMode, HI_FALSE);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start VO failed with %#x!\n", s32Ret);
    }

    // Start VI
    s32Ret = SNAP_StartViIsp(enPipeMode, pstViConfig, bDualPipeOnline);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start VI failed with %#x!\n", s32Ret);
        SNAP_StopVO(VoDev, VoLayer, enVoMode);
        return s32Ret;
    }

    // Start VPSS
    VI_CHN_ATTR_S stViChnAttr;
    VPSS_GRP_ATTR_S stVpssGrpAttr = s_stVpssAttr.stVpssGrpAttr;
    VPSS_CHN_ATTR_S stVpssChnAttr = s_stVpssAttr.stVpssChnAttr;
    VPSS_CHN_MODE_S stVpssChnMode = s_stVpssAttr.stVpssChnMode;

    /* start group0, process snap picture */
    HI_MPI_VI_GetChnAttr(ViChn0, &stViChnAttr);
    stVpssGrpAttr.u32MaxW = stViChnAttr.stDestSize.u32Width;
    stVpssGrpAttr.u32MaxH = stViChnAttr.stDestSize.u32Height;
    stVpssGrpAttr.bNrEn = bPhotoNrEn;
    stVpssGrpAttr.stNrAttr.enNrType = VPSS_NR_TYPE_SNAP;
    stVpssGrpAttr.stNrAttr.u32RefFrameNum = u32PhotoRefNum;
    s32Ret = SAMPLE_COMM_VPSS_StartGroup(VpssGrp0, &stVpssGrpAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Vpss_StartGroup %d failed with %#x\n", VpssGrp0, s32Ret);
        goto stop_vi;
    }

    stVpssChnMode.u32Width  = stViChnAttr.stDestSize.u32Width;
    stVpssChnMode.u32Height = stViChnAttr.stDestSize.u32Height;
    s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp0, VpssChn0, &stVpssChnAttr, &stVpssChnMode, NULL);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Vpss_Start Group %d, Chn %d failed with %#x\n", VpssGrp0, VpssChn0, s32Ret);
        SAMPLE_COMM_VPSS_StopGroup(VpssGrp0);
        goto stop_vi;
    }

    /* start group1, process video */
    if (SNAP_PIPE_MODE_DUAL == enPipeMode)
    {
        HI_MPI_VI_GetChnAttr(ViChn1, &stViChnAttr);
    }
    stVpssGrpAttr.u32MaxW = stViChnAttr.stDestSize.u32Width;
    stVpssGrpAttr.u32MaxH = stViChnAttr.stDestSize.u32Height;
    stVpssGrpAttr.bNrEn = HI_TRUE;
    stVpssGrpAttr.stNrAttr.enNrType = VPSS_NR_TYPE_VIDEO;
    stVpssGrpAttr.stNrAttr.u32RefFrameNum = 2;
    s32Ret = SAMPLE_COMM_VPSS_StartGroup(VpssGrp1, &stVpssGrpAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Vpss_StartGroup %d failed with %#x\n", VpssGrp1, s32Ret);
        goto stop_vpss0_0;
    }

    stVpssChnMode.u32Width  = stViChnAttr.stDestSize.u32Width;
    stVpssChnMode.u32Height = stViChnAttr.stDestSize.u32Height;
    s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp1, VpssChn0, &stVpssChnAttr, &stVpssChnMode, NULL);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Vpss_Start Group %d, Chn %d failed with %#x\n", VpssGrp1, VpssChn0, s32Ret);
        SAMPLE_COMM_VPSS_StopGroup(VpssGrp1);
        goto stop_vpss0_0;
    }

    if (SNAP_PIPE_MODE_DUAL == enPipeMode)
    {
        SNAP_VI_BindVpss(ViChn0, VpssGrp0);
        SNAP_VI_BindVpss(ViChn1, VpssGrp1);
    }
    else
    {
        SNAP_VI_BindVpss(ViChn0, VpssGrp0);
        SNAP_VI_BindVpss(ViChn0, VpssGrp1);
    }
    //SAMPLE_COMM_VO_BindVpss(VoLayer, 0, VpssGrp0, VpssChn0);
    SAMPLE_COMM_VO_BindVpss(VoLayer, 1, VpssGrp1, VpssChn0);

    return HI_SUCCESS;

stop_vpss0_0:
    SAMPLE_COMM_VPSS_DisableChn(VpssGrp0, VpssChn0);
    SAMPLE_COMM_VPSS_StopGroup(VpssGrp0);

stop_vi:
    SNAP_StopViIsp(enPipeMode);

    SNAP_StopVO(VoDev, VoLayer, enVoMode);

    return s32Ret;
}

static HI_VOID SNAP_StopViVpssVo(SNAP_PIPE_MODE_E enPipeMode)
{
    VI_CHN  ViChn0  = 0, ViChn1  = 1;
    VPSS_GRP VpssGrp0 = 0, VpssGrp1 = 1;
    VPSS_CHN VpssChn0 = 0;
    VO_DEV VoDev = 0;
    VO_LAYER VoLayer = 0;
    SAMPLE_VO_MODE_E enVoMode = VO_MODE_2MUX;

    /* unbind */
    SAMPLE_COMM_VO_UnBindVpss(VoLayer, 1, VpssGrp1, VpssChn0);
    //SAMPLE_COMM_VO_UnBindVpss(VoLayer, 0, VpssGrp0, VpssChn0);

    if (SNAP_PIPE_MODE_DUAL == enPipeMode)
    {
        SNAP_VI_UnBindVpss(ViChn1, VpssGrp1);
        SNAP_VI_UnBindVpss(ViChn0, VpssGrp0);
    }
    else
    {
        SNAP_VI_UnBindVpss(ViChn0, VpssGrp1);
        SNAP_VI_UnBindVpss(ViChn0, VpssGrp0);
    }

    SAMPLE_COMM_VPSS_DisableChn(VpssGrp0,VpssChn0);
    SAMPLE_COMM_VPSS_DisableChn(VpssGrp1,VpssChn0);
    SAMPLE_COMM_VPSS_StopGroup(VpssGrp0);
    SAMPLE_COMM_VPSS_StopGroup(VpssGrp1);

    SNAP_StopViIsp(enPipeMode);

    SNAP_StopVO(VoDev, VoLayer, enVoMode);

}

static inline HI_S32 SNAP_CalcFrameSize(VIDEO_FRAME_INFO_S *pstFrameInfo, HI_U32 *pu32Size, 
                            HI_U32 *pu32LumaSize, HI_U32 *pu32ChrmSize)
{
    PIXEL_FORMAT_E enPixelFormat = pstFrameInfo->stVFrame.enPixelFormat;
    VIDEO_FRAME_S *pstFrame = &pstFrameInfo->stVFrame;
    HI_U32 u32Size, u32LumaSize, u32ChrmSize;
    
    if (PIXEL_FORMAT_YUV_SEMIPLANAR_422 == enPixelFormat)
    {
        u32Size     = (pstFrame->u32Stride[0] * pstFrame->u32Height) * 2;
        u32LumaSize = (pstFrame->u32Stride[0] * pstFrame->u32Height);
        u32ChrmSize = u32LumaSize;
    }
    else if (PIXEL_FORMAT_YUV_SEMIPLANAR_420 == enPixelFormat)
    {
        u32Size = (pstFrame->u32Stride[0] * pstFrame->u32Height) * 3 / 2;
        u32LumaSize = (pstFrame->u32Stride[0] * pstFrame->u32Height);
        u32ChrmSize = u32LumaSize / 2;
    }
    else if (PIXEL_FORMAT_RGB_BAYER == enPixelFormat)
    {
        u32Size     = (pstFrame->u32Stride[0] * pstFrame->u32Height);
        u32LumaSize = (pstFrame->u32Stride[0] * pstFrame->u32Height);
        u32ChrmSize = 0;
    }
    else if (PIXEL_FORMAT_YUV_400 == enPixelFormat)
    {
        u32Size     = (pstFrame->u32Stride[0] * pstFrame->u32Height);
        u32LumaSize = (pstFrame->u32Stride[0] * pstFrame->u32Height);
        u32ChrmSize = 0;
    }
    else
    {
        printf("not support pixelformat: %d\n", enPixelFormat);
        return HI_FAILURE;
    }

    if (NULL != pu32Size)
    {
        *pu32Size = u32Size;
    }
    if (NULL != pu32LumaSize)
    {
        *pu32LumaSize = u32LumaSize;
    }
    if (NULL != pu32ChrmSize)
    {
        *pu32ChrmSize = u32ChrmSize;
    }

    return HI_SUCCESS;
}


/*
 * @hPool       input pool handle
 * @pstFrame
 *              input:  u32Width, u32Height, enPixelFormat
 *              output: u32PhyAddr, pVirAddr, u32Stride
 * @pstVbBlk    output blk
 */
HI_S32 SNAP_GetVB(VB_POOL hPool, VIDEO_FRAME_INFO_S* pstFrameInfo, VB_BLK* pstVbBlk)
{
    VB_BLK VbBlk = VB_INVALID_HANDLE;
    HI_U32 u32PhyAddr, u32Stride;
    HI_U32 u32Size, u32LumaSize, u32ChrmSize;
    HI_S32 s32Ret;

    u32Stride = ((pstFrameInfo->stVFrame.u32Width + 15) / 16) * 16;
    pstFrameInfo->stVFrame.u32Stride[0] = u32Stride;
    pstFrameInfo->stVFrame.u32Stride[1] = u32Stride;

    s32Ret = SNAP_CalcFrameSize(pstFrameInfo, &u32Size, &u32LumaSize, &u32ChrmSize);
    if (HI_SUCCESS != s32Ret)
    {
        return s32Ret;
    }
    printf("%s() size = %d, width x height = %d x %d, stride = %d\n", __FUNCTION__, 
                u32Size, pstFrameInfo->stVFrame.u32Width, pstFrameInfo->stVFrame.u32Height,
                u32Stride);

    VbBlk = HI_MPI_VB_GetBlock(hPool, u32Size, HI_NULL);
    if (VB_INVALID_HANDLE == VbBlk)
    {
        printf("HI_MPI_VB_GetBlock err! size:%d\n", u32Size);
        return HI_FAILURE;
    }
    *pstVbBlk = VbBlk;

    u32PhyAddr = HI_MPI_VB_Handle2PhysAddr(VbBlk);
    if (0 == u32PhyAddr)
    {
        printf("HI_MPI_VB_Handle2PhysAddr err!\n");
        HI_MPI_VB_ReleaseBlock(VbBlk);
        return HI_FAILURE;
    }

    pstFrameInfo->u32PoolId = HI_MPI_VB_Handle2PoolId(VbBlk);
    if (VB_INVALID_POOLID == pstFrameInfo->u32PoolId)
    {
        SAMPLE_PRT("u32PoolId err!\n");
        HI_MPI_VB_ReleaseBlock(VbBlk);
        return HI_FAILURE;
    }

    pstFrameInfo->stVFrame.u32PhyAddr[0] = u32PhyAddr;
    pstFrameInfo->stVFrame.u32PhyAddr[1] = pstFrameInfo->stVFrame.u32PhyAddr[0] + u32LumaSize;
    pstFrameInfo->stVFrame.u32PhyAddr[2] = pstFrameInfo->stVFrame.u32PhyAddr[1] + u32ChrmSize;

    pstFrameInfo->stVFrame.pVirAddr[0] = NULL;
    pstFrameInfo->stVFrame.pVirAddr[1] = NULL;
    pstFrameInfo->stVFrame.pVirAddr[2] = NULL;
    
    pstFrameInfo->stVFrame.u32Field = VIDEO_FIELD_FRAME;
    pstFrameInfo->stVFrame.enCompressMode = COMPRESS_MODE_NONE;

    return HI_SUCCESS;
}

HI_S32 SNAP_PutVB(VIDEO_FRAME_INFO_S *pstFrameInfo, VB_BLK *pstVbBlk)
{
    HI_S32 s32Ret;

    s32Ret = HI_MPI_VB_ReleaseBlock(*pstVbBlk);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("release blk 0x%x failed\n", *pstVbBlk);
    }
    *pstVbBlk = VB_INVALID_HANDLE;
    pstFrameInfo->u32PoolId = VB_INVALID_POOLID;
    pstFrameInfo->stVFrame.u32PhyAddr[0] = 0;
    pstFrameInfo->stVFrame.u32PhyAddr[1] = 0;
    pstFrameInfo->stVFrame.u32PhyAddr[2] = 0;

    return HI_SUCCESS;
}

HI_S32 SNAP_SYS_Init(VB_CONF_S *pstVbConf, HI_U32 u32VbMask)
{
    MPP_SYS_CONF_S stSysConf = {0};
    HI_S32 s32Ret = HI_FAILURE;

    HI_MPI_SYS_Exit();
    HI_MPI_VB_Exit();

    if (NULL == pstVbConf)
    {
        SAMPLE_PRT("input parameter is null, it is invaild!\n");
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VB_SetConf(pstVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VB_SetConf failed!\n");
        return HI_FAILURE;
    }

    if (0 != u32VbMask)
    {
        VB_SUPPLEMENT_CONF_S stSupplementConf;

        stSupplementConf.u32SupplementConf = u32VbMask;
        s32Ret = HI_MPI_VB_SetSupplementConf(&stSupplementConf);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_VB_SetSupplementConf failed!\n");
            return HI_FAILURE;
        }
    }

    s32Ret = HI_MPI_VB_Init();
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VB_Init failed!\n");
        return HI_FAILURE;
    }

    stSysConf.u32AlignWidth = SAMPLE_SYS_ALIGN_WIDTH;
    s32Ret = HI_MPI_SYS_SetConf(&stSysConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SYS_SetConf failed\n");
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_SYS_Init();
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SYS_Init failed!\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

#ifdef HI_SUPPORT_PHOTO_ALGS
typedef struct hiALG_THREAD_PARAM_S
{
    PHOTO_ALG_TYPE_E enAlgType;
    PHOTO_SRC_IMG_S *pstSrcImg;
    PHOTO_DST_IMG_S *pstDstImg;
}ALG_THREAD_PARAM_S;
void *SNAP_Alg_Thread(void *parg)
{
    ALG_THREAD_PARAM_S *pstThreadAlg = (ALG_THREAD_PARAM_S *)parg;
    HI_U32 u32BlkSizeIn, u32BlkSizeOut;
    VIDEO_FRAME_INFO_S *pstFrameInfo;
    PHOTO_ALG_TYPE_E enAlgType;
    PHOTO_SRC_IMG_S *pstSrcImg;
    PHOTO_DST_IMG_S *pstDstImg;
    HI_S32 i, s32Ret = HI_SUCCESS;

    enAlgType = pstThreadAlg->enAlgType;
    pstSrcImg = pstThreadAlg->pstSrcImg;
    pstDstImg = pstThreadAlg->pstDstImg;

    prctl(PR_SET_NAME, (unsigned long)"snap_algorithm", 0, 0, 0);

    if (PIXEL_FORMAT_YUV_SEMIPLANAR_420 != pstSrcImg->astFrm[0].stVFrame.enPixelFormat)
    {
        SAMPLE_PRT("not support input pixel format %d\n", pstSrcImg->astFrm[0].stVFrame.enPixelFormat);
        return NULL;
    }
    if (PIXEL_FORMAT_YUV_SEMIPLANAR_420 != pstDstImg->stFrm.stVFrame.enPixelFormat)
    {
        SAMPLE_PRT("not support output pixel format %d\n", pstDstImg->stFrm.stVFrame.enPixelFormat);
        return NULL;
    }

    SNAP_CalcFrameSize(&pstSrcImg->astFrm[0], &u32BlkSizeIn, NULL, NULL);
    SNAP_CalcFrameSize(&pstDstImg->stFrm, &u32BlkSizeOut, NULL, NULL);

    for (i = 0; i < pstSrcImg->s32FrmNum; i++)
    {
        pstFrameInfo = &pstSrcImg->astFrm[i];
        pstFrameInfo->stVFrame.pVirAddr[0] = HI_MPI_SYS_MmapCache(pstFrameInfo->stVFrame.u32PhyAddr[0], u32BlkSizeIn);
        pstFrameInfo->stVFrame.pVirAddr[1] = pstFrameInfo->stVFrame.pVirAddr[0] + (pstFrameInfo->stVFrame.u32Height * pstFrameInfo->stVFrame.u32Stride[0]);
    }
    pstFrameInfo = &pstDstImg->stFrm;
    pstFrameInfo->stVFrame.pVirAddr[0] = HI_MPI_SYS_MmapCache(pstFrameInfo->stVFrame.u32PhyAddr[0], u32BlkSizeOut);
    pstFrameInfo->stVFrame.pVirAddr[1] = pstFrameInfo->stVFrame.pVirAddr[0] + (pstFrameInfo->stVFrame.u32Height * pstFrameInfo->stVFrame.u32Stride[0]);

    s32Ret = HI_MPI_PHOTO_Proc(enAlgType, pstSrcImg, pstDstImg);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Photo alg %d process failed\n", enAlgType);
    }

    for (i = 0; i < pstSrcImg->s32FrmNum; i++)
    {
        pstFrameInfo = &pstSrcImg->astFrm[i];
        HI_MPI_SYS_Munmap(pstFrameInfo->stVFrame.pVirAddr[0], u32BlkSizeIn);
    }
    
    pstFrameInfo = &pstDstImg->stFrm;
    HI_MPI_SYS_MflushCache(pstFrameInfo->stVFrame.u32PhyAddr[0], pstFrameInfo->stVFrame.pVirAddr[0], u32BlkSizeOut);
    HI_MPI_SYS_Munmap(pstFrameInfo->stVFrame.pVirAddr[0], u32BlkSizeOut);

    return NULL;
}

HI_S32 SNAP_ProcessAlg(PHOTO_ALG_TYPE_E enAlgType, PHOTO_SRC_IMG_S *pstSrcImg, PHOTO_DST_IMG_S *pstDstImg)
{
    HI_S32 s32Ret;
    ALG_THREAD_PARAM_S stThreadAlgParam;
    pthread_attr_t stThreadAttr;
    pthread_t threadId;

    stThreadAlgParam.enAlgType = enAlgType;
    stThreadAlgParam.pstSrcImg = pstSrcImg;
    stThreadAlgParam.pstDstImg = pstDstImg;

    pthread_attr_init(&stThreadAttr);
    pthread_attr_setstacksize(&stThreadAttr, 0x1E00000);
    s32Ret = pthread_create(&threadId, &stThreadAttr, SNAP_Alg_Thread, &stThreadAlgParam);
    if (0 != s32Ret)
    {
        SAMPLE_PRT("create alg thread failed\n");
        pthread_attr_destroy(&stThreadAttr);
        return HI_FAILURE;
    }
    pthread_attr_destroy(&stThreadAttr);

    pthread_join(threadId, NULL);

    return HI_SUCCESS;
}
#endif

HI_S32 SNAP_SetDCFInfo(ISP_DEV IspDev)
{
    ISP_DCF_INFO_S stIspDCF;

    HI_MPI_ISP_GetDCFInfo(IspDev, &stIspDCF);

    //description:
    strncpy((char*)stIspDCF.au8ImageDescription, "HiMpp Snap Test", DCF_DRSCRIPTION_LENGTH);

    //manufacturer: Hisilicon
    strncpy((char *)stIspDCF.au8Make,"Hisilicon", DCF_DRSCRIPTION_LENGTH);

    //device model number: Hisilicon IP Camera
    strncpy((char *)stIspDCF.au8Model,"Hisilicon IP Camera", DCF_DRSCRIPTION_LENGTH);

    //firmware version: v.1.1.0 
    strncpy((char *)stIspDCF.au8Software,"v.1.1.0", DCF_DRSCRIPTION_LENGTH);

    stIspDCF.u16ISOSpeedRatings         = 500;
    stIspDCF.u32FNumber                 = 0x0001000f;
    stIspDCF.u32MaxApertureValue        = 0x00010001;
    stIspDCF.u32ExposureTime            = 0x00010004;
    stIspDCF.u32ExposureBiasValue       = 5;
    stIspDCF.u8ExposureProgram          = 1;

    stIspDCF.u8MeteringMode             = 1;
    stIspDCF.u8LightSource              = 1;

    stIspDCF.u32FocalLength             = 0x00640001;
    stIspDCF.u8SceneType                = 0;
    stIspDCF.u8CustomRendered           = 0;

    stIspDCF.u8ExposureMode             = 0;
    stIspDCF.u8WhiteBalance             = 1;

    stIspDCF.u8FocalLengthIn35mmFilm    = 1;
    stIspDCF.u8SceneCaptureType         = 1;
    stIspDCF.u8GainControl              = 1;

    stIspDCF.u8Contrast                 = 5;
    stIspDCF.u8Saturation               = 1;
    stIspDCF.u8Sharpness                = 5;

    HI_MPI_ISP_SetDCFInfo(IspDev, &stIspDCF);

    return HI_SUCCESS;
}

/*
 * picture:
 *     ISP0 -> VI CHN0 -> VPSS(group 0, channel 0) -> Jpeg (4Kx2K)
 *     ISP0 -> VI CHN0 -> VPSS(group 0, channel 0) -> VO(layer 0, channel 0)
 * preview:
 *     sensor0 -> MIPI Rx0 -> VIDev1 -> ISP1 -> VI CHN1 -> VPSS(group 1, channel 0) -> VO(layer 0, channel 1)
 *     sensor0 -> MIPI Rx0 -> VIDev1 -> ISP1 -> VI CHN1 -> VPSS(group 1, channel 0) -> h265 (1080P)
 */
HI_S32 SAMPLE_SNAP_Normal(SAMPLE_VI_CONFIG_S* pstViConfig, SNAP_PIPE_MODE_E enPipeMode, HI_BOOL bZSL, HI_U32 u32RefFrameNum)
{
    VB_CONF_S stVbConf;
    SIZE_S stSize;
    HI_U32 u32BlkSize;
    VI_DEV  ViDev1 = 1;
    ISP_DEV IspDev0 = 0, IspDev1 = 1;
    VPSS_GRP VpssGrp0 = 0, VpssGrp1 = 1;
    VPSS_CHN VpssChn0 = 0;
    VENC_CHN JpegChn = 0, h265Chn = 1;
    VO_LAYER VoLayer = 0;
    VO_CHN   VoChn = 0;
    VI_SNAP_ATTR_S  stViSnapAttr;
    ISP_SNAP_ATTR_S stIspSnapAttr;
    MEDIA_RECORDER_S hMediaRecorder;
    HI_S32 s32Ret;

    /******************************************
        mpp system init
      ******************************************/
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(s_enNorm, s_enSnsSize, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        return s32Ret;
    }
    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize2(&stSize, PIXEL_FORMAT_YUV_SEMIPLANAR_422, SAMPLE_SYS_ALIGN_WIDTH);
    memset(&stVbConf, 0, sizeof(VB_CONF_S));
    stVbConf.u32MaxPoolCnt = 128;
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt  = 25;

    stSize.u32Width = 160;  // thumbnail
    stSize.u32Height = 120;
    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize2(&stSize, PIXEL_FORMAT_YUV_SEMIPLANAR_420, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.astCommPool[1].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[1].u32BlkCnt  = 6;

    s32Ret = SNAP_SYS_Init(&stVbConf, VB_SUPPLEMENT_JPEG_MASK);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        return s32Ret;
    }

    /******************************************
         start VI VPSS VO
     ******************************************/
    s32Ret = SNAP_StartViVpssVo(enPipeMode, pstViConfig, HI_TRUE, u32RefFrameNum, HI_FALSE);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_COMM_SYS_Exit();
        return s32Ret;
    }
    SAMPLE_COMM_VO_BindVpss(VoLayer, VoChn, VpssGrp0, VpssChn0);

    /******************************************
         start Jpeg and video recorder
     ******************************************/
    SNAP_SetDCFInfo(IspDev0);
    VPSS_CHN_MODE_S stVpssChnMode;
    HI_MPI_VPSS_GetChnMode(VpssGrp0, VpssChn0, &stVpssChnMode);
    stSize.u32Width  = stVpssChnMode.u32Width;
    stSize.u32Height = stVpssChnMode.u32Height;
    s32Ret = SNAP_VencCreateChn(JpegChn, PT_JPEG, &stSize, SAMPLE_RC_CBR, 30, 30);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start jpeg chn: %d failed with %#x\n", JpegChn, s32Ret);
        SNAP_StopViVpssVo(enPipeMode);
        SAMPLE_COMM_SYS_Exit();
        return s32Ret;
    }
    SAMPLE_COMM_VENC_BindVpss(JpegChn, VpssGrp0, VpssChn0);

    MPP_CHN_S stSrcChn;
    stSrcChn.enModId = HI_ID_VPSS;
    stSrcChn.s32DevId = VpssGrp1;
    stSrcChn.s32ChnId = VpssChn0;
    s32Ret = SNAP_CreateVencEncoder(h265Chn, stSrcChn, VIDEO_ENCODER_H265, &hMediaRecorder);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Create media recorder failed\n");
        SAMPLE_COMM_VENC_UnBindVpss(JpegChn, VpssGrp0, VpssChn0);
        SNAP_VencDestroyChn(JpegChn);
        SNAP_StopViVpssVo(enPipeMode);
        SAMPLE_COMM_SYS_Exit();
        return s32Ret;
    }

    /******************************************
         config snap attr
     ******************************************/
    HI_MPI_ISP_GetSnapAttr(IspDev0, &stIspSnapAttr);
    stIspSnapAttr.enSnapType = SNAP_TYPE_NORMAL;
    stIspSnapAttr.enSnapPipeMode = ISP_SNAP_PICTURE;  // ISP0 is picture pipe
    s32Ret = HI_MPI_ISP_SetSnapAttr(IspDev0, &stIspSnapAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("ISP %d set snap attr failed with: %#x\n", IspDev0, s32Ret);
        goto out;
    }
    stIspSnapAttr.enSnapPipeMode = ISP_SNAP_PREVIEW;
    s32Ret = HI_MPI_ISP_SetSnapAttr(IspDev1, &stIspSnapAttr); // ISP0 is preview pipe
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("ISP %d set snap attr failed with: %#x\n", IspDev1, s32Ret);
        goto out;
    }

    stViSnapAttr.enSnapType = SNAP_TYPE_NORMAL;
    stViSnapAttr.IspDev = IspDev0;
    stViSnapAttr.u32RefFrameNum = u32RefFrameNum;
    stViSnapAttr.unAttr.stNormalAttr.u32FrameDepth = 5;
    stViSnapAttr.unAttr.stNormalAttr.s32SrcFrameRate = -1;
    stViSnapAttr.unAttr.stNormalAttr.s32DstFrameRate = -1;
    stViSnapAttr.unAttr.stNormalAttr.bZSL = bZSL;
    stViSnapAttr.unAttr.stNormalAttr.u32RollbackMs = 80;
    stViSnapAttr.unAttr.stNormalAttr.u32Interval = 0;
    stViSnapAttr.unAttr.stNormalAttr.u32FrameCnt = 1;
    s32Ret = HI_MPI_VI_SetSnapAttr(ViDev1, &stViSnapAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("VI set snap attr failed with: %#x\n", s32Ret);
        goto out;
    }

    s32Ret = HI_MPI_VI_EnableSnap(ViDev1);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("VI enable snap failed with: %#x\n", s32Ret);
        goto out;
    }

    HI_U32 u32SnapCnt = 0;
    while(1)
    {
        HI_CHAR szFileName[64] = {0};
        VENC_RECV_PIC_PARAM_S stRecvParam;
        HI_S32 ch, i;

        ch = SNAP_PAUSE("Press Enter key to snap one picture, press 'q' to quit\n");
        if (ch == 'q' || ch == EOF)
        {
            break;
        }

        stRecvParam.s32RecvPicNum = stViSnapAttr.unAttr.stNormalAttr.u32FrameCnt;
        HI_MPI_VENC_StartRecvPicEx(JpegChn, &stRecvParam);

        s32Ret = HI_MPI_VI_TriggerSnap(ViDev1);
        if (s32Ret == HI_ERR_VI_BUSY)
        {
            SAMPLE_PRT("Snap is busy, try again\n");
            continue;
        }
        else if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Trigger snap failed with: %#x\n", s32Ret);
            break;
        }

        for (i = 0; i < stRecvParam.s32RecvPicNum; i++)
        {
            VENC_STREAM_S stStream;
            snprintf(szFileName,sizeof(szFileName), "snap_zsl_%dref_%d.jpg", u32RefFrameNum, u32SnapCnt);

            s32Ret = SNAP_VencGetStream(JpegChn, &stStream, 1000);
            if (HI_SUCCESS != s32Ret)
            {
                continue;
            }
            SNAP_VencSaveStream(&stStream, PT_JPEG, szFileName, "w+");
            SNAP_VencReleaseStream(JpegChn, &stStream);
            u32SnapCnt++;
        }
    }

    HI_MPI_VI_DisableSnap(ViDev1);

out:
    SNAP_DestroyVencEncoder(h265Chn, &hMediaRecorder);

    SAMPLE_COMM_VENC_UnBindVpss(JpegChn, VpssGrp0, VpssChn0);
    SNAP_VencDestroyChn(JpegChn);

    SAMPLE_COMM_VO_UnBindVpss(VoLayer, VoChn, VpssGrp0, VpssChn0);
    SNAP_StopViVpssVo(enPipeMode);
    SAMPLE_COMM_SYS_Exit();
    
    return s32Ret;
}

HI_S32 SAMPLE_SNAP_USER(SAMPLE_VI_CONFIG_S* pstViConfig, SNAP_PIPE_MODE_E enPipeMode,HI_U32 u32RefFrameNum)
{
    VB_CONF_S stVbConf;
    SIZE_S stSize;
    HI_U32 u32BlkSize;
    VI_DEV  ViDev1 = 1, ViDev0 = 0;
    ISP_DEV IspDev0 = 0, IspDev1 = 1;
    VPSS_GRP VpssGrp0 = 0, VpssGrp1 = 1;
    VPSS_CHN VpssChn0 = 0;
    VENC_CHN JpegChn = 0, h265Chn = 1;
    VO_LAYER VoLayer = 0;
    VO_CHN   VoChn = 0;
    VI_SNAP_ATTR_S  stViSnapAttr;
    ISP_SNAP_ATTR_S stIspSnapAttr;
    MEDIA_RECORDER_S hMediaRecorder;
	VI_RAW_FRAME_INFO_S stFrameInfo;
    HI_S32 s32Ret;
	VI_CHN ViChn1 = ViDev1;
	HI_S32 s32MilliSec = 120,phy_addr;
	int  cnt= 0;
	ISP_RAW_STAT_INFO_S  *pstRawStatInfo;	
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(s_enNorm, s_enSnsSize, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        return s32Ret;
    }
    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize2(&stSize, PIXEL_FORMAT_YUV_SEMIPLANAR_422, SAMPLE_SYS_ALIGN_WIDTH);
    memset(&stVbConf, 0, sizeof(VB_CONF_S));
    stVbConf.u32MaxPoolCnt = 128;
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt  = 25;
    s32Ret = SAMPLE_COMM_SYS_Init_With_DCF(&stVbConf, HI_TRUE);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        return s32Ret;
    }
    s32Ret = SNAP_StartViVpssVo(enPipeMode, pstViConfig, HI_TRUE, u32RefFrameNum, HI_FALSE);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_COMM_SYS_Exit();
        return s32Ret;
    }    
    SAMPLE_COMM_VO_BindVpss(VoLayer, VoChn, VpssGrp0, VpssChn0);
    SNAP_SetDCFInfo(IspDev0);
    VPSS_CHN_MODE_S stVpssChnMode;
    HI_MPI_VPSS_GetChnMode(VpssGrp0, VpssChn0, &stVpssChnMode);
    stSize.u32Width  = stVpssChnMode.u32Width;
    stSize.u32Height = stVpssChnMode.u32Height;
    s32Ret = SNAP_VencCreateChn(JpegChn, PT_JPEG, &stSize, SAMPLE_RC_CBR, 30, 30);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start jpeg chn: %d failed with %#x\n", JpegChn, s32Ret);
        SNAP_StopViVpssVo(enPipeMode);
        SAMPLE_COMM_SYS_Exit();
        return s32Ret;
    }
    SAMPLE_COMM_VENC_BindVpss(JpegChn, VpssGrp0, VpssChn0);
    MPP_CHN_S stSrcChn;
    stSrcChn.enModId = HI_ID_VPSS;
    stSrcChn.s32DevId = VpssGrp1;
    stSrcChn.s32ChnId = VpssChn0;
    SNAP_CreateVencEncoder(h265Chn, stSrcChn, VIDEO_ENCODER_H265, &hMediaRecorder);
    HI_MPI_ISP_GetSnapAttr(IspDev0, &stIspSnapAttr);
    stIspSnapAttr.enSnapType = SNAP_TYPE_NORMAL;
    stIspSnapAttr.enSnapPipeMode = ISP_SNAP_PICTURE;  // ISP0 is picture pipe
    s32Ret = HI_MPI_ISP_SetSnapAttr(IspDev0, &stIspSnapAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("ISP %d set snap attr failed with: %#x\n", IspDev0, s32Ret);
        goto out;
    }
    stIspSnapAttr.enSnapPipeMode = ISP_SNAP_PREVIEW;
    s32Ret = HI_MPI_ISP_SetSnapAttr(IspDev1, &stIspSnapAttr); // ISP0 is preview pipe
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("ISP %d set snap attr failed with: %#x\n", IspDev1, s32Ret);
        goto out;
    }
    stViSnapAttr.enSnapType = SNAP_TYPE_USER;
    stViSnapAttr.IspDev = IspDev0;
    stViSnapAttr.u32RefFrameNum = u32RefFrameNum;
    stViSnapAttr.unAttr.stUserAttr.u32FrameDepth = 1;
    stViSnapAttr.unAttr.stUserAttr.s32SrcFrameRate = 30;
    stViSnapAttr.unAttr.stUserAttr.s32DstFrameRate = 30;
    s32Ret = HI_MPI_VI_SetSnapAttr(ViDev1, &stViSnapAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("VI set snap attr failed with: %#x\n", s32Ret);
        goto out;
    }
    s32Ret = HI_MPI_VI_EnableSnap(ViDev1);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("VI enable snap failed with: %#x\n", s32Ret);
        goto out;
    }	
    while(1)
    {
		HI_CHAR szFileName[64] = {0};
		HI_S32 ch;		
		VENC_RECV_PIC_PARAM_S stRecvParam;	
        ch = SNAP_PAUSE("Press Enter key to snap one picture, press 'q' to quit\n");
        if (ch == 'q' || ch == EOF)
        {
            break;
        }
		stRecvParam.s32RecvPicNum = 1;
		HI_MPI_VENC_StartRecvPicEx(JpegChn, &stRecvParam);
		s32Ret = HI_MPI_VI_GetSnapRaw(ViDev1, &stFrameInfo, s32MilliSec);		
		if(s32Ret == HI_SUCCESS)
		{			
			{
				phy_addr = stFrameInfo.stFrame.stVFrame.stSupplement.u32IspStatPhyAddr;
				pstRawStatInfo = (ISP_RAW_STAT_INFO_S*) HI_MPI_SYS_Mmap(phy_addr, sizeof(ISP_RAW_STAT_INFO_S));
				s32Ret = HI_MPI_VI_SendSnapRaw(ViDev0, &stFrameInfo, s32MilliSec);
				HI_MPI_SYS_Munmap(pstRawStatInfo, sizeof(ISP_RAW_STAT_INFO_S));
				cnt++;
				s32Ret = HI_MPI_VI_ReleaseSnapRaw(ViChn1,&stFrameInfo);			
			}
		}
		else
		{		
			printf("videv = %d, HI_MPI_VI_GetBayerFrame timeout\n",ViDev1);
		}		
		VENC_STREAM_S stStream;
		snprintf(szFileName, sizeof(szFileName),"snap_user_%dref_%d.jpg", u32RefFrameNum, cnt);
		s32Ret = SNAP_VencGetStream(JpegChn, &stStream, 1000);
		if (HI_SUCCESS != s32Ret)
		{
			continue;
		}
		SNAP_VencSaveStream(&stStream, PT_JPEG, szFileName, "w+");
		SNAP_VencReleaseStream(JpegChn, &stStream);		
		usleep(10000);
    }
    HI_MPI_VI_DisableSnap(ViDev1);
out:
    SNAP_DestroyVencEncoder(h265Chn, &hMediaRecorder);
    SAMPLE_COMM_VENC_UnBindVpss(JpegChn, VpssGrp0, VpssChn0);
    SNAP_VencDestroyChn(JpegChn);
    SAMPLE_COMM_VO_UnBindVpss(VoLayer, VoChn, VpssGrp0, VpssChn0);
    SNAP_StopViVpssVo(enPipeMode);
    SAMPLE_COMM_SYS_Exit();
    return s32Ret;
}
#ifdef HI_SUPPORT_PHOTO_ALGS
/*
 * picture:
 *     ISP0 -> VI CHN0 -> VPSS(group 0, channel 0) -> HDR Algorithm -> Jpeg (4Kx2K)
 *     HDR algorithm -> VO(layer 0, channel 0)
 * preview:
 *     sensor0 -> MIPI Rx0 -> VIDev1 -> ISP1 -> VICHN 1 -> VPSS(1, 0) -> VO(0, 1)
 *     sensor0 -> MIPI Rx0 -> VIDev1 -> ISP1 -> VICHN 1 -> VPSS(1, 0) -> h265 (1080P)
 */
HI_S32 SAMPLE_SNAP_HDR(SAMPLE_VI_CONFIG_S* pstViConfig, SNAP_PIPE_MODE_E enPipeMode)
{
    VB_CONF_S stVbConf;
    SIZE_S stSize;
    HI_U32 u32BlkSize;
    VI_DEV ViDev1 = 1;
    ISP_DEV IspDev0 = 0, IspDev1 = 1;
    VPSS_GRP VpssGrp0 = 0, VpssGrp1 = 1;
    VPSS_CHN VpssChn0 = 0;
    VENC_CHN JpegChn = 0, h265Chn = 1;
    VO_LAYER VoLayer = 0;
    VO_CHN   VoChn = 0;
    VI_SNAP_ATTR_S  stViSnapAttr;
    ISP_SNAP_ATTR_S stIspSnapAttr;
    MEDIA_RECORDER_S hMediaRecorder;
    HI_U32 u32RefFrameNum = 0;
    HI_S32 s32Ret;

    /******************************************
     mpp system init
    ******************************************/    
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(s_enNorm, s_enSnsSize, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        return s32Ret;
    }
    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize2(&stSize, PIXEL_FORMAT_YUV_SEMIPLANAR_422, SAMPLE_SYS_ALIGN_WIDTH);
    memset(&stVbConf, 0, sizeof(VB_CONF_S));
    stVbConf.u32MaxPoolCnt = 128;
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt  = 25;
    s32Ret = SNAP_SYS_Init(&stVbConf, VB_SUPPLEMENT_JPEG_MASK | VB_SUPPLEMENT_ISPINFO_MASK);
    //s32Ret = SNAP_SYS_Init(&stVbConf, VB_SUPPLEMENT_JPEG_MASK);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        return s32Ret;
    }

    /******************************************
         start VI VPSS VO
     ******************************************/
    s32Ret = SNAP_StartViVpssVo(enPipeMode, pstViConfig, HI_TRUE, u32RefFrameNum, HI_FALSE);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_COMM_SYS_Exit();
        return s32Ret;
    }

    /******************************************
         start Jpeg and video recorder
     ******************************************/
    VPSS_CHN_MODE_S stVpssChnMode;
    SNAP_SetDCFInfo(IspDev0);
    HI_MPI_VPSS_GetChnMode(VpssGrp0, VpssChn0, &stVpssChnMode);
    stSize.u32Width  = stVpssChnMode.u32Width;
    stSize.u32Height = stVpssChnMode.u32Height;
    s32Ret = SNAP_VencCreateChn(JpegChn, PT_JPEG, &stSize, SAMPLE_RC_CBR, 30, 30);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start jpeg chn: %d failed with %#x\n", JpegChn, s32Ret);
        SNAP_StopViVpssVo(enPipeMode);
        SAMPLE_COMM_SYS_Exit();
        return s32Ret;
    }

    MPP_CHN_S stSrcChn;
    stSrcChn.enModId = HI_ID_VPSS;
    stSrcChn.s32DevId = VpssGrp1;
    stSrcChn.s32ChnId = VpssChn0;
    s32Ret = SNAP_CreateVencEncoder(h265Chn, stSrcChn, VIDEO_ENCODER_H265, &hMediaRecorder);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Create media recorder failed\n");
        SNAP_VencDestroyChn(JpegChn);
        SNAP_StopViVpssVo(enPipeMode);
        SAMPLE_COMM_SYS_Exit();
        return s32Ret;
    }

    // config snap attr for ISP
    HI_MPI_ISP_GetSnapAttr(IspDev0, &stIspSnapAttr);
    stIspSnapAttr.enSnapType = SNAP_TYPE_PRO;
    stIspSnapAttr.enSnapPipeMode = ISP_SNAP_PICTURE;  // ISP0 is picture pipe
    #if 1
    stIspSnapAttr.stProParam.enOpType = OP_TYPE_AUTO;
    stIspSnapAttr.stProParam.u8ProFrameNum = 3;
    stIspSnapAttr.stProParam.stAuto.au16ProExpStep[0] = 256;        // 0EV
    stIspSnapAttr.stProParam.stAuto.au16ProExpStep[1] = 256 / 8;    // -3EV
    stIspSnapAttr.stProParam.stAuto.au16ProExpStep[2] = 256 * 8;    // +3EV
    #else

    stIspSnapAttr.stHdrParam.enOpType = OP_TYPE_MANUAL;
    stIspSnapAttr.stHdrParam.u8HDRFrameNum = 3;
    #if 0
    stIspSnapAttr.stHdrParam.stManual.au32ManExpTime[0] = 8000;    // fix exposure time
    stIspSnapAttr.stHdrParam.stManual.au32ManExpTime[1] = 8000;    // 8ms
    stIspSnapAttr.stHdrParam.stManual.au32ManExpTime[2] = 8000;
    stIspSnapAttr.stHdrParam.stManual.au32ManSysgain[0] = 1024;     // ISO 100
    stIspSnapAttr.stHdrParam.stManual.au32ManSysgain[1] = 1024 * 4; // ISO 400
    stIspSnapAttr.stHdrParam.stManual.au32ManSysgain[2] = 1024 * 8; // ISO 800
    #elif 1
    stIspSnapAttr.stHdrParam.stManual.au32ManExpTime[0] = 1000;    // 1ms
    stIspSnapAttr.stHdrParam.stManual.au32ManExpTime[1] = 10000;   // 10ms
    stIspSnapAttr.stHdrParam.stManual.au32ManExpTime[2] = 80000;   // 80ms
    stIspSnapAttr.stHdrParam.stManual.au32ManSysgain[0] = 1024;    // fix gain
    stIspSnapAttr.stHdrParam.stManual.au32ManSysgain[1] = 1024;
    stIspSnapAttr.stHdrParam.stManual.au32ManSysgain[2] = 1024;
    #endif
    
    #endif
    
    s32Ret = HI_MPI_ISP_SetSnapAttr(IspDev0, &stIspSnapAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("ISP %d set snap attr failed with: %#x\n", IspDev0, s32Ret);
        goto out1;
    }
    stIspSnapAttr.enSnapPipeMode = ISP_SNAP_PREVIEW;
    s32Ret = HI_MPI_ISP_SetSnapAttr(IspDev1, &stIspSnapAttr); // ISP0 is preview pipe
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("ISP %d set snap attr failed with: %#x\n", IspDev1, s32Ret);
        goto out1;
    }

    // Config snap attr for VI, and enable snap
    stViSnapAttr.enSnapType = SNAP_TYPE_PRO;
    stViSnapAttr.IspDev = IspDev0;
    stViSnapAttr.u32RefFrameNum = u32RefFrameNum;
    stViSnapAttr.unAttr.stProAttr.u32FrameDepth = 5;
    s32Ret = HI_MPI_VI_SetSnapAttr(ViDev1, &stViSnapAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("VI set snap attr failed with: %#x\n", s32Ret);
        goto out1;
    }

    s32Ret = HI_MPI_VI_EnableSnap(ViDev1);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("VI enable snap failed with: %#x\n", s32Ret);
        goto out1;
    }

    // Init HDR algorithm
    PHOTO_ALG_ATTR_S stAlgAttr;
    stAlgAttr.enAlgType = PHOTO_ALG_TYPE_HDR;
    s32Ret = HI_MPI_PHOTO_Init(stAlgAttr.enAlgType);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("init photo alg hdr failed\n");
        goto out2;
    }

    // Config HDR algorithm parameters
    stAlgAttr.unAttr.stHDRAttr.s32NrLuma = 10;
    stAlgAttr.unAttr.stHDRAttr.s32NrChroma= 0;
    stAlgAttr.unAttr.stHDRAttr.s32Sharpen= 2;
    stAlgAttr.unAttr.stHDRAttr.s32Saturation = 3;
    stAlgAttr.unAttr.stHDRAttr.s32GlobalContrast = 5;
    stAlgAttr.unAttr.stHDRAttr.s32LocalContrast  = 0;
    s32Ret = HI_MPI_PHOTO_SetAlgAttr(&stAlgAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("set photo attr failed\n");
        goto out3;
    }

    // ------------------------------------------------------
    MPP_CHN_S stChn, stRawChn;
    PHOTO_SRC_IMG_S stSrcImg;
    PHOTO_DST_IMG_S stDstImg;
    stChn.enModId = HI_ID_VPSS;
    stChn.s32DevId = VpssGrp0;
    stChn.s32ChnId = VpssChn0;
    stRawChn.enModId = HI_ID_VIU;
    stRawChn.s32DevId = ViDev1;
    VIU_GET_RAW_CHN(ViDev1, stRawChn.s32ChnId);
    HI_MPI_VPSS_SetDepth(VpssGrp0, VpssChn0, 1);
    stSrcImg.s32FrmNum = stIspSnapAttr.stProParam.u8ProFrameNum;
    while(1)
    {
        VB_BLK VbBlk;
        HI_CHAR szFileName[64] = {0};
        VENC_STREAM_S stStream;
        VENC_RECV_PIC_PARAM_S stRecvParam;
        HI_S32 ch, i;

        ch = SNAP_PAUSE("Press Enter key to snap one picture, press 'q' to quit\n");
        if (ch == 'q' || ch == EOF)
        {
            break;
        }
        
        HI_MPI_VI_SetFrameDepth(stRawChn.s32ChnId, stIspSnapAttr.stProParam.u8ProFrameNum);

        s32Ret = HI_MPI_VI_TriggerSnap(ViDev1);
        if (s32Ret == HI_ERR_VI_BUSY)
        {
            SAMPLE_PRT("Snap is busy, try again\n");
            continue;
        }
        else if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Trigger snap failed with: %#x\n", s32Ret);
            break;
        }
        
        for (i = 0; i < stIspSnapAttr.stProParam.u8ProFrameNum; i++)
        {
            s32Ret = SNAP_GetFrame(&stChn, &stSrcImg.astFrm[i], 3000);
            if (HI_SUCCESS != s32Ret)
            {
                goto out3;
            }
        }

        // save the HDR YUV frames
        #if 0
        for (i = 0; i < stSrcImg.s32FrmNum; i++)
        {
            sprintf(szFileName, "snap_hdr_%dx%d_%d.yuv", stSrcImg.astFrm[i].stVFrame.u32Width,
                        stSrcImg.astFrm[i].stVFrame.u32Height, i);
            SNAP_SaveFrame(&stSrcImg.astFrm[i].stVFrame, szFileName);
        }
        #endif

        //  HDR algorithm process
        memcpy(&stDstImg.stFrm, &stSrcImg.astFrm[0], sizeof(VIDEO_FRAME_INFO_S));
        SNAP_GetVB(VB_INVALID_POOLID, &stDstImg.stFrm, &VbBlk);
        SNAP_ProcessAlg(PHOTO_ALG_TYPE_HDR, &stSrcImg, &stDstImg);

        HI_MPI_VO_SendFrame(VoLayer, VoChn, &stDstImg.stFrm, 1000);

        // encode Jpeg
        stRecvParam.s32RecvPicNum = 1;
        HI_MPI_VENC_StartRecvPicEx(JpegChn, &stRecvParam);
        HI_MPI_VENC_SendFrame(JpegChn, &stDstImg.stFrm, 1000);
        sprintf(szFileName, "snap_hdr.jpg");
        s32Ret = SNAP_VencGetStream(JpegChn, &stStream, 1000);
        if (HI_SUCCESS == s32Ret)
        {
            SNAP_VencSaveStream(&stStream, PT_JPEG, szFileName, "w+");
            SNAP_VencReleaseStream(JpegChn, &stStream);
        }

        SNAP_PutVB(&stDstImg.stFrm, &VbBlk);

        // release YUV frames
        for (i = 0; i < stSrcImg.s32FrmNum; i++)
        {
            SNAP_ReleaseFrame(&stChn, &stSrcImg.astFrm[i]);
        }

        // save the HDR Raw frames
        for (i = 0; i < stSrcImg.s32FrmNum; i++)
        {
            s32Ret = SNAP_GetFrame(&stRawChn, &stSrcImg.astFrm[i], 1000);
            if (HI_SUCCESS == s32Ret)
            {
                sprintf(szFileName, "snap_hdr_%dx%d_%d.raw", stSrcImg.astFrm[i].stVFrame.u32Width,
                            stSrcImg.astFrm[i].stVFrame.u32Height, i);
                SNAP_SaveFrame(&stSrcImg.astFrm[i].stVFrame, szFileName);
                SNAP_ReleaseFrame(&stChn, &stSrcImg.astFrm[i]);
            }
        }
    }

out3:
    HI_MPI_PHOTO_DeInit(stAlgAttr.enAlgType);
    
out2:
    HI_MPI_VI_DisableSnap(ViDev1);

out1:
    SNAP_DestroyVencEncoder(h265Chn, &hMediaRecorder);

    SNAP_VencDestroyChn(JpegChn);

    SNAP_StopViVpssVo(enPipeMode);
    SAMPLE_COMM_SYS_Exit();

    return s32Ret;
}


/*
 * picture:
 *     sensor0 -> MIPI Rx0 -> VIDev0 ->ISP0 -> VI CHN0 -> VPSS(group 0, channel 0) -> HDR Algorithm -> Jpeg (4Kx2K)
 *     HDR algorithm -> VO(layer 0, channel 0)
 * preview:
 *     sensor0 -> MIPI Rx0 -> VIDev1 -> ISP1 -> VICHN 1 -> VPSS(1, 0) -> VO(0, 1)
 *     sensor0 -> MIPI Rx0 -> VIDev1 -> ISP1 -> VICHN 1 -> VPSS(1, 0) -> h265 (1080P)
 */
HI_S32 SAMPLE_SNAP_ONLINE_HDR(SAMPLE_VI_CONFIG_S* pstViConfig, SNAP_PIPE_MODE_E enPipeMode)
{
    VB_CONF_S stVbConf;
    SIZE_S stSize;
    HI_U32 u32BlkSize;
    VI_DEV ViDev0 = 0;
    ISP_DEV IspDev0 = 0, IspDev1 = 1;
    VPSS_GRP VpssGrp0 = 0, VpssGrp1 = 1;
    VPSS_CHN VpssChn0 = 0;
    VENC_CHN JpegChn = 0, h265Chn = 1;
    VO_LAYER VoLayer = 0;
    VO_CHN   VoChn = 0;
    VI_SNAP_ATTR_S  stViSnapAttr;
    ISP_SNAP_ATTR_S stIspSnapAttr;
    MEDIA_RECORDER_S hMediaRecorder;
    HI_U32 u32RefFrameNum = 0;
    HI_S32 s32Ret;

    /******************************************
     mpp system init
    ******************************************/    
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(s_enNorm, s_enSnsSize, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        return s32Ret;
    }
    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize2(&stSize, PIXEL_FORMAT_YUV_SEMIPLANAR_422, SAMPLE_SYS_ALIGN_WIDTH);
    memset(&stVbConf, 0, sizeof(VB_CONF_S));
    stVbConf.u32MaxPoolCnt = 128;
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt  = 25;
    s32Ret = SNAP_SYS_Init(&stVbConf, VB_SUPPLEMENT_JPEG_MASK | VB_SUPPLEMENT_ISPINFO_MASK);
    //s32Ret = SNAP_SYS_Init(&stVbConf, VB_SUPPLEMENT_JPEG_MASK);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        return s32Ret;
    }

    /******************************************
         start VI VPSS VO
     ******************************************/
    s32Ret = SNAP_StartViVpssVo(enPipeMode, pstViConfig, HI_TRUE, u32RefFrameNum, HI_TRUE);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_COMM_SYS_Exit();
        return s32Ret;
    }

    /******************************************
         start Jpeg and video recorder
     ******************************************/
    VPSS_CHN_MODE_S stVpssChnMode;
    SNAP_SetDCFInfo(IspDev0);
    HI_MPI_VPSS_GetChnMode(VpssGrp0, VpssChn0, &stVpssChnMode);
    stSize.u32Width  = stVpssChnMode.u32Width;
    stSize.u32Height = stVpssChnMode.u32Height;
    s32Ret = SNAP_VencCreateChn(JpegChn, PT_JPEG, &stSize, SAMPLE_RC_CBR, 30, 30);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start jpeg chn: %d failed with %#x\n", JpegChn, s32Ret);
        SNAP_StopViVpssVo(enPipeMode);
        SAMPLE_COMM_SYS_Exit();
        return s32Ret;
    }

    MPP_CHN_S stSrcChn;
    stSrcChn.enModId = HI_ID_VPSS;
    stSrcChn.s32DevId = VpssGrp1;
    stSrcChn.s32ChnId = VpssChn0;
    s32Ret = SNAP_CreateVencEncoder(h265Chn, stSrcChn, VIDEO_ENCODER_H265, &hMediaRecorder);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Create media recorder failed\n");
        SNAP_VencDestroyChn(JpegChn);
        SNAP_StopViVpssVo(enPipeMode);
        SAMPLE_COMM_SYS_Exit();
        return s32Ret;
    }

    // config snap attr for ISP
    HI_MPI_ISP_GetSnapAttr(IspDev0, &stIspSnapAttr);
    stIspSnapAttr.enSnapType = SNAP_TYPE_SYNC_PRO;
    stIspSnapAttr.enSnapPipeMode = ISP_SNAP_PICTURE;  // ISP0 is picture pipe
    #if 1
    stIspSnapAttr.stProParam.enOpType = OP_TYPE_AUTO;
    stIspSnapAttr.stProParam.u8ProFrameNum = 3;
    stIspSnapAttr.stProParam.stAuto.au16ProExpStep[0] = 256;        // 0EV
    stIspSnapAttr.stProParam.stAuto.au16ProExpStep[1] = 256 / 8;    // -3EV
    stIspSnapAttr.stProParam.stAuto.au16ProExpStep[2] = 256 * 8;    // +3EV
    #else

    stIspSnapAttr.stHdrParam.enOpType = OP_TYPE_MANUAL;
    stIspSnapAttr.stHdrParam.u8HDRFrameNum = 3;
    #if 0
    stIspSnapAttr.stHdrParam.stManual.au32ManExpTime[0] = 8000;    // fix exposure time
    stIspSnapAttr.stHdrParam.stManual.au32ManExpTime[1] = 8000;    // 8ms
    stIspSnapAttr.stHdrParam.stManual.au32ManExpTime[2] = 8000;
    stIspSnapAttr.stHdrParam.stManual.au32ManSysgain[0] = 1024;     // ISO 100
    stIspSnapAttr.stHdrParam.stManual.au32ManSysgain[1] = 1024 * 4; // ISO 400
    stIspSnapAttr.stHdrParam.stManual.au32ManSysgain[2] = 1024 * 8; // ISO 800
    #elif 1
    stIspSnapAttr.stHdrParam.stManual.au32ManExpTime[0] = 1000;    // 1ms
    stIspSnapAttr.stHdrParam.stManual.au32ManExpTime[1] = 10000;   // 10ms
    stIspSnapAttr.stHdrParam.stManual.au32ManExpTime[2] = 80000;   // 80ms
    stIspSnapAttr.stHdrParam.stManual.au32ManSysgain[0] = 1024;    // fix gain
    stIspSnapAttr.stHdrParam.stManual.au32ManSysgain[1] = 1024;
    stIspSnapAttr.stHdrParam.stManual.au32ManSysgain[2] = 1024;
    #endif
    
    #endif
    
    s32Ret = HI_MPI_ISP_SetSnapAttr(IspDev0, &stIspSnapAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("ISP %d set snap attr failed with: %#x\n", IspDev0, s32Ret);
        goto out1;
    }
    stIspSnapAttr.enSnapPipeMode = ISP_SNAP_PREVIEW;
    s32Ret = HI_MPI_ISP_SetSnapAttr(IspDev1, &stIspSnapAttr); // ISP0 is preview pipe
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("ISP %d set snap attr failed with: %#x\n", IspDev1, s32Ret);
        goto out1;
    }

    // Config snap attr for VI, and enable snap
    stViSnapAttr.enSnapType = SNAP_TYPE_SYNC_PRO;
    stViSnapAttr.IspDev = IspDev1;
    stViSnapAttr.u32RefFrameNum = u32RefFrameNum;
 
    s32Ret = HI_MPI_VI_SetSnapAttr(ViDev0, &stViSnapAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("VI set snap attr failed with: %#x\n", s32Ret);
        goto out1;
    }

    s32Ret = HI_MPI_VI_EnableSnap(ViDev0);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("VI enable snap failed with: %#x\n", s32Ret);
        goto out1;
    }

    // Init HDR algorithm
    PHOTO_ALG_ATTR_S stAlgAttr;
    stAlgAttr.enAlgType = PHOTO_ALG_TYPE_HDR;
    s32Ret = HI_MPI_PHOTO_Init(stAlgAttr.enAlgType);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("init photo alg hdr failed\n");
        goto out2;
    }

    // Config HDR algorithm parameters
    stAlgAttr.unAttr.stHDRAttr.s32NrLuma = 10;
    stAlgAttr.unAttr.stHDRAttr.s32NrChroma= 0;
    stAlgAttr.unAttr.stHDRAttr.s32Sharpen= 2;
    stAlgAttr.unAttr.stHDRAttr.s32Saturation = 3;
    stAlgAttr.unAttr.stHDRAttr.s32GlobalContrast = 5;
    stAlgAttr.unAttr.stHDRAttr.s32LocalContrast  = 0;
    s32Ret = HI_MPI_PHOTO_SetAlgAttr(&stAlgAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("set photo attr failed\n");
        goto out3;
    }
	
	s32Ret = HI_MPI_VI_EnableBayerDump(ViDev0);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("VI enable snap failed with: %#x\n", s32Ret);
		goto out1;
	}
    // ------------------------------------------------------
    MPP_CHN_S stChn, stRawChn;
    PHOTO_SRC_IMG_S stSrcImg;
    PHOTO_DST_IMG_S stDstImg;
    stChn.enModId = HI_ID_VPSS;
    stChn.s32DevId = VpssGrp0;
    stChn.s32ChnId = VpssChn0;
    stRawChn.enModId = HI_ID_VIU;
    stRawChn.s32DevId = ViDev0;
    VIU_GET_RAW_CHN(ViDev0, stRawChn.s32ChnId);
    HI_MPI_VPSS_SetDepth(VpssGrp0, VpssChn0, 1);
    stSrcImg.s32FrmNum = stIspSnapAttr.stProParam.u8ProFrameNum;
    while(1)
    {
        VB_BLK VbBlk;
        HI_CHAR szFileName[64] = {0};
        VENC_STREAM_S stStream;
        VENC_RECV_PIC_PARAM_S stRecvParam;
        HI_S32 ch, i;

        ch = SNAP_PAUSE("Press Enter key to snap one picture, press 'q' to quit\n");
        if (ch == 'q' || ch == EOF)
        {
            break;
        }
        
        HI_MPI_VI_SetFrameDepth(stRawChn.s32ChnId, stIspSnapAttr.stProParam.u8ProFrameNum);

        s32Ret = HI_MPI_VI_TriggerSnap(ViDev0);
        if (s32Ret == HI_ERR_VI_BUSY)
        {
            SAMPLE_PRT("Snap is busy, try again\n");
            continue;
        }
        else if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Trigger snap failed with: %#x\n", s32Ret);
            break;
        }
        
        for (i = 0; i < stIspSnapAttr.stProParam.u8ProFrameNum; i++)
        {
            s32Ret = SNAP_GetFrame(&stChn, &stSrcImg.astFrm[i], 3000);
            if (HI_SUCCESS != s32Ret)
            {
                goto out3;
            }
        }

        // save the HDR YUV frames
        #if 1
        for (i = 0; i < stSrcImg.s32FrmNum; i++)
        {
            sprintf(szFileName, "snap_hdr_%dx%d_%d.yuv", stSrcImg.astFrm[i].stVFrame.u32Width,
                        stSrcImg.astFrm[i].stVFrame.u32Height, i);
            SNAP_SaveFrame(&stSrcImg.astFrm[i].stVFrame, szFileName);
        }
        #endif

        //  HDR algorithm process
        memcpy(&stDstImg.stFrm, &stSrcImg.astFrm[0], sizeof(VIDEO_FRAME_INFO_S));
        SNAP_GetVB(VB_INVALID_POOLID, &stDstImg.stFrm, &VbBlk);
        SNAP_ProcessAlg(PHOTO_ALG_TYPE_HDR, &stSrcImg, &stDstImg);

        HI_MPI_VO_SendFrame(VoLayer, VoChn, &stDstImg.stFrm, 1000);

        // encode Jpeg
        stRecvParam.s32RecvPicNum = 1;
        HI_MPI_VENC_StartRecvPicEx(JpegChn, &stRecvParam);
        HI_MPI_VENC_SendFrame(JpegChn, &stDstImg.stFrm, 1000);
        sprintf(szFileName, "snap_hdr.jpg");
        s32Ret = SNAP_VencGetStream(JpegChn, &stStream, 1000);
        if (HI_SUCCESS == s32Ret)
        {
            SNAP_VencSaveStream(&stStream, PT_JPEG, szFileName, "w+");
            SNAP_VencReleaseStream(JpegChn, &stStream);
        }

        SNAP_PutVB(&stDstImg.stFrm, &VbBlk);

        // release YUV frames
        for (i = 0; i < stSrcImg.s32FrmNum; i++)
        {
            SNAP_ReleaseFrame(&stChn, &stSrcImg.astFrm[i]);
        }

        // save the HDR Raw frames
        for (i = 0; i < stSrcImg.s32FrmNum; i++)
        {
            s32Ret = SNAP_GetFrame(&stRawChn, &stSrcImg.astFrm[i], 1000);
            if (HI_SUCCESS == s32Ret)
            {
                sprintf(szFileName, "snap_hdr_%dx%d_%d.raw", stSrcImg.astFrm[i].stVFrame.u32Width,
                            stSrcImg.astFrm[i].stVFrame.u32Height, i);
                SNAP_SaveFrame(&stSrcImg.astFrm[i].stVFrame, szFileName);
                SNAP_ReleaseFrame(&stChn, &stSrcImg.astFrm[i]);
            }
        }
    }

out3:
    HI_MPI_PHOTO_DeInit(stAlgAttr.enAlgType);
    
out2:
    HI_MPI_VI_DisableSnap(ViDev0);

	s32Ret = HI_MPI_VI_DisableBayerDump(ViDev0);
	if(HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("HI_MPI_VI_DisableBayerDump : %#x\n", s32Ret);           
    }

out1:
    SNAP_DestroyVencEncoder(h265Chn, &hMediaRecorder);

    SNAP_VencDestroyChn(JpegChn);

    SNAP_StopViVpssVo(enPipeMode);
    SAMPLE_COMM_SYS_Exit();

    return s32Ret;
}


/*
 * picture:
 *     ISP0 -> VI CHN0 -> VPSS(group 0, channel 0) -> LL Algorithm -> Jpeg (4Kx2K)
 *     LL Algorithm -> VO(layer 0, channel 0)
 * preview:
 *     sensor0 -> MIPI Rx0 -> VIDev1 -> ISP1 -> VI CHN1 -> VPSS(1, 0) -> VO(0, 1)
 *     sensor0 -> MIPI Rx0 -> VIDev1 -> ISP1 -> VI CHN1 -> VPSS(1, 0) -> h265 (1080P)
 */
HI_S32 SAMPLE_SNAP_LL(SAMPLE_VI_CONFIG_S* pstViConfig, SNAP_PIPE_MODE_E enPipeMode)
{
    VB_CONF_S stVbConf;
    SIZE_S stSize;
    HI_U32 u32BlkSize;
    VI_DEV ViDev1 = 1;
    ISP_DEV IspDev0 = 0, IspDev1 = 1;
    VPSS_GRP VpssGrp0 = 0, VpssGrp1 = 1;
    VPSS_CHN VpssChn0 = 0;
    VENC_CHN JpegChn = 0, h265Chn = 1;
    VO_LAYER VoLayer = 0;
    VO_CHN   VoChn = 0;
    VI_SNAP_ATTR_S  stViSnapAttr;
    ISP_SNAP_ATTR_S stIspSnapAttr;
    MEDIA_RECORDER_S hMediaRecorder;
    HI_U32 u32RefFrameNum = 0, u32AlgFrameCnt = 3;
    HI_S32 s32Ret;

    // mpp system init
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(s_enNorm, s_enSnsSize, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        return s32Ret;
    }
    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize2(&stSize, PIXEL_FORMAT_YUV_SEMIPLANAR_422, SAMPLE_SYS_ALIGN_WIDTH);
    memset(&stVbConf, 0, sizeof(VB_CONF_S));
    stVbConf.u32MaxPoolCnt = 128;
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt  = 25;
    s32Ret = SNAP_SYS_Init(&stVbConf, VB_SUPPLEMENT_JPEG_MASK | VB_SUPPLEMENT_ISPINFO_MASK);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        return s32Ret;
    }

    // start VI VPSS VO
    s32Ret = SNAP_StartViVpssVo(enPipeMode, pstViConfig, HI_TRUE, u32RefFrameNum, HI_FALSE);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_COMM_SYS_Exit();
        return s32Ret;
    }

    // start Jpeg and video recorder
    VPSS_CHN_MODE_S stVpssChnMode;
    SNAP_SetDCFInfo(IspDev0);
    HI_MPI_VPSS_GetChnMode(VpssGrp0, VpssChn0, &stVpssChnMode);
    stSize.u32Width  = stVpssChnMode.u32Width;
    stSize.u32Height = stVpssChnMode.u32Height;
    s32Ret = SNAP_VencCreateChn(JpegChn, PT_JPEG, &stSize, SAMPLE_RC_CBR, 30, 30);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start jpeg chn: %d failed with %#x\n", JpegChn, s32Ret);
        SNAP_StopViVpssVo(enPipeMode);
        SAMPLE_COMM_SYS_Exit();
        return s32Ret;
    }

    MPP_CHN_S stSrcChn;
    stSrcChn.enModId = HI_ID_VPSS;
    stSrcChn.s32DevId = VpssGrp1;
    stSrcChn.s32ChnId = VpssChn0;
    s32Ret = SNAP_CreateVencEncoder(h265Chn, stSrcChn, VIDEO_ENCODER_H265, &hMediaRecorder);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Create media recorder failed\n");
        SNAP_VencDestroyChn(JpegChn);
        SNAP_StopViVpssVo(enPipeMode);
        SAMPLE_COMM_SYS_Exit();
        return s32Ret;
    }

    // Config ISP snap attr
    HI_MPI_ISP_GetSnapAttr(IspDev0, &stIspSnapAttr);
    stIspSnapAttr.enSnapType = SNAP_TYPE_NORMAL;
    stIspSnapAttr.enSnapPipeMode = ISP_SNAP_PICTURE;  // ISP0 is picture pipe
    s32Ret = HI_MPI_ISP_SetSnapAttr(IspDev0, &stIspSnapAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("ISP %d set snap attr failed with: %#x\n", IspDev0, s32Ret);
        goto out1;
    }
    stIspSnapAttr.enSnapPipeMode = ISP_SNAP_PREVIEW;
    s32Ret = HI_MPI_ISP_SetSnapAttr(IspDev1, &stIspSnapAttr); // ISP0 is preview pipe
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("ISP %d set snap attr failed with: %#x\n", IspDev1, s32Ret);
        goto out1;
    }

    // Config VI snap attr, and enable snap
    stViSnapAttr.enSnapType = SNAP_TYPE_NORMAL;
    stViSnapAttr.IspDev = IspDev0;
    stViSnapAttr.u32RefFrameNum = u32RefFrameNum;
    stViSnapAttr.unAttr.stNormalAttr.u32FrameDepth = 5;
    stViSnapAttr.unAttr.stNormalAttr.s32SrcFrameRate = -1;
    stViSnapAttr.unAttr.stNormalAttr.s32DstFrameRate = -1;
    stViSnapAttr.unAttr.stNormalAttr.bZSL = HI_FALSE;
    stViSnapAttr.unAttr.stNormalAttr.u32Interval = 0;
    stViSnapAttr.unAttr.stNormalAttr.u32FrameCnt = u32AlgFrameCnt;
    s32Ret = HI_MPI_VI_SetSnapAttr(ViDev1, &stViSnapAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("VI set snap attr failed with: %#x\n", s32Ret);
        goto out1;
    }

    s32Ret = HI_MPI_VI_EnableSnap(ViDev1);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("VI enable snap failed with: %#x\n", s32Ret);
        goto out1;
    }

    // Init LL algorithm 
    PHOTO_ALG_ATTR_S stAlgAttr;
    stAlgAttr.enAlgType = PHOTO_ALG_TYPE_LL;
    s32Ret = HI_MPI_PHOTO_Init(stAlgAttr.enAlgType);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("init photo alg hdr failed\n");
        goto out2;
    }

    // Config LL algorithm parameters
    stAlgAttr.unAttr.stLLAttr.s32Sharpen  = 4;
    stAlgAttr.unAttr.stLLAttr.s32NrLuma   = 4;
    stAlgAttr.unAttr.stLLAttr.s32NrChroma = 4;
    stAlgAttr.unAttr.stLLAttr.s32Saturation = 2;
    s32Ret = HI_MPI_PHOTO_SetAlgAttr(&stAlgAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("set photo attr failed\n");
        goto out3;
    }

    // -------------------------------------------------
    MPP_CHN_S stChn;
    PHOTO_SRC_IMG_S stSrcImg;
    PHOTO_DST_IMG_S stDstImg;
    stChn.enModId = HI_ID_VPSS;
    stChn.s32DevId = VpssGrp0;
    stChn.s32ChnId = VpssChn0;
    stSrcImg.s32FrmNum = u32AlgFrameCnt;
    HI_MPI_VPSS_SetDepth(VpssGrp0, VpssChn0, 1);

    while(1)
    {
        VB_BLK VbBlk;
        HI_CHAR szFileName[64] = {0};
        VENC_STREAM_S stStream;
        VENC_RECV_PIC_PARAM_S stRecvParam;
        HI_S32 ch, i;

        ch = SNAP_PAUSE("Press Enter key to snap one picture, press 'q' to quit\n");
        if (ch == 'q' || ch == EOF)
        {
            break;
        }

        s32Ret = HI_MPI_VI_TriggerSnap(ViDev1);
        if (s32Ret == HI_ERR_VI_BUSY)
        {
            SAMPLE_PRT("Snap is busy, try again\n");
            continue;
        }
        else if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Trigger snap failed with: %#x\n", s32Ret);
            break;
        }

        for (i = 0; i < stSrcImg.s32FrmNum; i++)
        {
            s32Ret = SNAP_GetFrame(&stChn, &stSrcImg.astFrm[i], 2000);
            if (HI_SUCCESS != s32Ret)
            {
                goto out3;
            }
        }

        // save the LL YUV frames
        #if 0
        for (i = 0; i < stSrcImg.s32FrmNum; i++)
        {
            sprintf(szFileName, "snap_ll_%dx%d_%d.yuv", stSrcImg.astFrm[i].stVFrame.u32Width,
                        stSrcImg.astFrm[i].stVFrame.u32Height, i);
            SNAP_SaveFrame(&stSrcImg.astFrm[i].stVFrame, szFileName);
        }
        #endif

        //  LL algorithm process
        memcpy(&stDstImg.stFrm, &stSrcImg.astFrm[0], sizeof(VIDEO_FRAME_INFO_S));
        SNAP_GetVB(VB_INVALID_POOLID, &stDstImg.stFrm, &VbBlk);
        SNAP_ProcessAlg(stAlgAttr.enAlgType, &stSrcImg, &stDstImg);

        HI_MPI_VO_SendFrame(VoLayer, VoChn, &stDstImg.stFrm, 1000);

        // encode Jpeg
        stRecvParam.s32RecvPicNum = 1;
        HI_MPI_VENC_StartRecvPicEx(JpegChn, &stRecvParam);
        HI_MPI_VENC_SendFrame(JpegChn, &stDstImg.stFrm, 1000);
        sprintf(szFileName, "snap_ll.jpg");
        s32Ret = SNAP_VencGetStream(JpegChn, &stStream, 1000);
        if (HI_SUCCESS == s32Ret)
        {
            SNAP_VencSaveStream(&stStream, PT_JPEG, szFileName, "w+");
            SNAP_VencReleaseStream(JpegChn, &stStream);
        }

        SNAP_PutVB(&stDstImg.stFrm, &VbBlk);

        // release frames
        for (i = 0; i < stSrcImg.s32FrmNum; i++)
        {
            SNAP_ReleaseFrame(&stChn, &stSrcImg.astFrm[i]);
        }
    }

out3:
    HI_MPI_PHOTO_DeInit(stAlgAttr.enAlgType);

out2:
    HI_MPI_VI_DisableSnap(ViDev1);

out1:
    SNAP_DestroyVencEncoder(h265Chn, &hMediaRecorder);

    SNAP_VencDestroyChn(JpegChn);

    SNAP_StopViVpssVo(enPipeMode);
    SAMPLE_COMM_SYS_Exit();
    
    return s32Ret;
}

/*
 * picture:
 *     ISP0 -> VI CHN0 -> VPSS(group 0, channel 0) -> SR Algorithm -> Jpeg (4K2K)
 *     SR Algorithm -> VO(layer 0, channel 0)
 * preview:
 *     sensor0 -> MIPI Rx0 -> VIDev1 -> ISP1 -> VI CHN1 -> VPSS(1, 0) -> VO(0, 1)
 *     sensor0 -> MIPI Rx0 -> VIDev1 -> ISP1 -> VI CHN1 -> VPSS(1, 0) -> h265 (1080P)
 */
HI_S32 SAMPLE_SNAP_SR(SAMPLE_VI_CONFIG_S* pstViConfig, SNAP_PIPE_MODE_E enPipeMode)
{
    VB_CONF_S stVbConf;
    SIZE_S stSize;
    HI_U32 u32BlkSize;
    VI_DEV ViDev1 = 1;
    ISP_DEV IspDev0 = 0, IspDev1 = 1;
    VPSS_GRP VpssGrp0 = 0, VpssGrp1 = 1;
    VPSS_CHN VpssChn0 = 0;
    VENC_CHN JpegChn = 0, h265Chn = 1;
    VO_LAYER VoLayer = 0;
    VO_CHN   VoChn = 0;
    VI_SNAP_ATTR_S  stViSnapAttr;
    ISP_SNAP_ATTR_S stIspSnapAttr;
    MEDIA_RECORDER_S hMediaRecorder;
    HI_U32 u32RefFrameNum = 0, u32AlgFrameCnt = 3;
    SIZE_S stSrPicSize;
    VB_POOL hVbPoolPhoto;
    HI_S32 s32Ret;

    // mpp system init
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(s_enNorm, s_enSnsSize, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        return s32Ret;
    }
    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize2(&stSize, PIXEL_FORMAT_YUV_SEMIPLANAR_422, SAMPLE_SYS_ALIGN_WIDTH);
    memset(&stVbConf, 0, sizeof(VB_CONF_S));
    stVbConf.u32MaxPoolCnt = 128;
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt  = 25;
    s32Ret = SNAP_SYS_Init(&stVbConf, VB_SUPPLEMENT_JPEG_MASK | VB_SUPPLEMENT_ISPINFO_MASK);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        return s32Ret;
    }

    stSrPicSize.u32Width  = 4608;
    stSrPicSize.u32Height = (stSize.u32Height * stSrPicSize.u32Width / stSize.u32Width);
    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize2(&stSrPicSize, PIXEL_FORMAT_YUV_SEMIPLANAR_420, SAMPLE_SYS_ALIGN_WIDTH);
    hVbPoolPhoto = HI_MPI_VB_CreatePool(u32BlkSize, 1, NULL);
    if (VB_INVALID_POOLID == hVbPoolPhoto)
    {
        SAMPLE_PRT("create vb pool failed\n");
        SAMPLE_COMM_SYS_Exit();
        return s32Ret;
    }

    // Start VI VPSS VO
    s32Ret = SNAP_StartViVpssVo(enPipeMode, pstViConfig, HI_TRUE, u32RefFrameNum, HI_FALSE);
    if (HI_SUCCESS != s32Ret)
    {
        HI_MPI_VB_DestroyPool(hVbPoolPhoto);
        SAMPLE_COMM_SYS_Exit();
        return s32Ret;
    }

    // Start Jpeg and video recorder
    SNAP_SetDCFInfo(IspDev0);
    s32Ret = SNAP_VencCreateChn(JpegChn, PT_JPEG, &stSrPicSize, SAMPLE_RC_CBR, 30, 30);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start jpeg chn: %d failed with %#x\n", JpegChn, s32Ret);
        SNAP_StopViVpssVo(enPipeMode);
        HI_MPI_VB_DestroyPool(hVbPoolPhoto);
        SAMPLE_COMM_SYS_Exit();
        return s32Ret;
    }

    MPP_CHN_S stSrcChn;
    stSrcChn.enModId = HI_ID_VPSS;
    stSrcChn.s32DevId = VpssGrp1;
    stSrcChn.s32ChnId = VpssChn0;
    s32Ret = SNAP_CreateVencEncoder(h265Chn, stSrcChn, VIDEO_ENCODER_H265, &hMediaRecorder);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Create media recorder failed\n");
        SNAP_VencDestroyChn(JpegChn);
        SNAP_StopViVpssVo(enPipeMode);
        SAMPLE_COMM_SYS_Exit();
        return s32Ret;
    }

    // Config ISP snap attr
    HI_MPI_ISP_GetSnapAttr(IspDev0, &stIspSnapAttr);
    stIspSnapAttr.enSnapType = SNAP_TYPE_NORMAL;
    stIspSnapAttr.enSnapPipeMode = ISP_SNAP_PICTURE;  // ISP0 is picture pipe
    s32Ret = HI_MPI_ISP_SetSnapAttr(IspDev0, &stIspSnapAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("ISP %d set snap attr failed with: %#x\n", IspDev0, s32Ret);
        goto out1;
    }
    stIspSnapAttr.enSnapPipeMode = ISP_SNAP_PREVIEW;
    s32Ret = HI_MPI_ISP_SetSnapAttr(IspDev1, &stIspSnapAttr); // ISP0 is preview pipe
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("ISP %d set snap attr failed with: %#x\n", IspDev1, s32Ret);
        goto out1;
    }

    // Config VI snap attr, and enable snap
    stViSnapAttr.enSnapType = SNAP_TYPE_NORMAL;
    stViSnapAttr.IspDev = IspDev0;
    stViSnapAttr.u32RefFrameNum = u32RefFrameNum;
    stViSnapAttr.unAttr.stNormalAttr.u32FrameDepth = 5;
    stViSnapAttr.unAttr.stNormalAttr.s32SrcFrameRate = -1;
    stViSnapAttr.unAttr.stNormalAttr.s32DstFrameRate = -1;
    stViSnapAttr.unAttr.stNormalAttr.bZSL = HI_FALSE;
    stViSnapAttr.unAttr.stNormalAttr.u32Interval = 0;
    stViSnapAttr.unAttr.stNormalAttr.u32FrameCnt = u32AlgFrameCnt;
    s32Ret = HI_MPI_VI_SetSnapAttr(ViDev1, &stViSnapAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("VI set snap attr failed with: %#x\n", s32Ret);
        goto out1;
    }

    s32Ret = HI_MPI_VI_EnableSnap(ViDev1);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("VI enable snap failed with: %#x\n", s32Ret);
        goto out1;
    }
         
    // Init SR algorithm
    PHOTO_ALG_ATTR_S stAlgAttr;
    stAlgAttr.enAlgType = PHOTO_ALG_TYPE_SR;
    s32Ret = HI_MPI_PHOTO_Init(stAlgAttr.enAlgType);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("init photo alg hdr failed\n");
        goto out2;
    }

    // Config SR algorithm parameters
    stAlgAttr.unAttr.stSRAttr.s32NrLuma   = 2;
    stAlgAttr.unAttr.stSRAttr.s32NrChroma = 1;
    stAlgAttr.unAttr.stSRAttr.s32Sharpen  = 2;
    stAlgAttr.unAttr.stSRAttr.s32Saturation = 0;
    stAlgAttr.unAttr.stSRAttr.s32Iso= -1;
    s32Ret = HI_MPI_PHOTO_SetAlgAttr(&stAlgAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("set photo attr failed\n");
        goto out3;
    }

    // -------------------------------------------------
    MPP_CHN_S stChn;
    PHOTO_SRC_IMG_S stSrcImg;
    PHOTO_DST_IMG_S stDstImg;
    stChn.enModId = HI_ID_VPSS;
    stChn.s32DevId = VpssGrp0;
    stChn.s32ChnId = VpssChn0;
    stSrcImg.s32FrmNum = u32AlgFrameCnt;
    HI_MPI_VPSS_SetDepth(VpssGrp0, VpssChn0, 1);

    while(1)
    {
        VB_BLK VbBlk;
        HI_CHAR szFileName[64] = {0};
        VENC_STREAM_S stStream;
        VENC_RECV_PIC_PARAM_S stRecvParam;
        HI_S32 ch, i;

        ch = SNAP_PAUSE("Press Enter key to snap one picture, press 'q' to quit\n");
        if (ch == 'q' || ch == EOF)
        {
            break;
        }

        s32Ret = HI_MPI_VI_TriggerSnap(ViDev1);
        if (s32Ret == HI_ERR_VI_BUSY)
        {
            SAMPLE_PRT("Snap is busy, try again\n");
            continue;
        }
        else if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Trigger snap failed with: %#x\n", s32Ret);
            break;
        }

        for (i = 0; i < stSrcImg.s32FrmNum; i++)
        {
            s32Ret = SNAP_GetFrame(&stChn, &stSrcImg.astFrm[i], 2000);
            if (HI_SUCCESS != s32Ret)
            {
                goto out3;
            }
        }

        // save the SR YUV frames
        #if 0
        for (i = 0; i < stSrcImg.s32FrmNum; i++)
        {
            sprintf(szFileName, "snap_sr_%dx%d_%d.yuv", stSrcImg.astFrm[i].stVFrame.u32Width,
                        stSrcImg.astFrm[i].stVFrame.u32Height, i);
            SNAP_SaveFrame(&stSrcImg.astFrm[i].stVFrame, szFileName);
        }
        #endif

        //  SR algorithm process
        memcpy(&stDstImg.stFrm, &stSrcImg.astFrm[0], sizeof(VIDEO_FRAME_INFO_S));
        stDstImg.stFrm.stVFrame.u32Width  = stSrPicSize.u32Width;
        stDstImg.stFrm.stVFrame.u32Height = stSrPicSize.u32Height;
        SNAP_GetVB(hVbPoolPhoto, &stDstImg.stFrm, &VbBlk);
        SNAP_ProcessAlg(stAlgAttr.enAlgType, &stSrcImg, &stDstImg);

        HI_MPI_VO_SendFrame(VoLayer, VoChn, &stDstImg.stFrm, 1000);

        // encode Jpeg
        stRecvParam.s32RecvPicNum = 1;
        HI_MPI_VENC_StartRecvPicEx(JpegChn, &stRecvParam);
        HI_MPI_VENC_SendFrame(JpegChn, &stDstImg.stFrm, 1000);
        sprintf(szFileName, "snap_sr.jpg");
        s32Ret = SNAP_VencGetStream(JpegChn, &stStream, 1000);
        if (HI_SUCCESS == s32Ret)
        {
            SNAP_VencSaveStream(&stStream, PT_JPEG, szFileName, "w+");
            SNAP_VencReleaseStream(JpegChn, &stStream);
        }

        SNAP_PutVB(&stDstImg.stFrm, &VbBlk);

        // release frames
        for (i = 0; i < stSrcImg.s32FrmNum; i++)
        {
            SNAP_ReleaseFrame(&stChn, &stSrcImg.astFrm[i]);
        }
    }

out3:
    HI_MPI_PHOTO_DeInit(stAlgAttr.enAlgType);

out2:
    HI_MPI_VI_DisableSnap(ViDev1);

out1:
    SNAP_DestroyVencEncoder(h265Chn, &hMediaRecorder);

    SNAP_VencDestroyChn(JpegChn);

    SNAP_StopViVpssVo(enPipeMode);
    HI_MPI_VB_DestroyPool(hVbPoolPhoto);
    SAMPLE_COMM_SYS_Exit();

    return s32Ret;
}
#endif

int AppMain(int argc, char* argv[])
{
    HI_S32 s32Ret = HI_SUCCESS;

    if ((argc < 2) || (1 != strlen(argv[1])))
    {
        SNAP_Usage();
        return HI_FAILURE;
    }

    signal(SIGINT,  SNAP_HandleSig);
    signal(SIGTERM, SNAP_HandleSig);
    signal(SIGQUIT, SNAP_HandleSig);

    s_stViConfig.enViMode = SENSOR_TYPE;
    SAMPLE_COMM_VI_GetSizeBySensor(&s_enSnsSize);
    if ((argc > 2) && *argv[2] == '1')
    {
        s_enVoIntfType = VO_INTF_BT1120;
    }

    switch (*argv[1])
    {
    case '0':
        s32Ret = SAMPLE_SNAP_Normal(&s_stViConfig, SNAP_PIPE_MODE_DUAL, HI_TRUE, 0);
        break;
    case '1':
        s32Ret = SAMPLE_SNAP_Normal(&s_stViConfig, SNAP_PIPE_MODE_DUAL, HI_TRUE, 2);
        break;

#ifdef HI_SUPPORT_PHOTO_ALGS
    case '2':
        s32Ret = SAMPLE_SNAP_HDR(&s_stViConfig, SNAP_PIPE_MODE_DUAL);
        break;
    case '3':
        s32Ret = SAMPLE_SNAP_LL(&s_stViConfig, SNAP_PIPE_MODE_DUAL);
        break;
    case '4':
        s32Ret = SAMPLE_SNAP_SR(&s_stViConfig, SNAP_PIPE_MODE_DUAL);
        break;
	case '5':
        s32Ret = SAMPLE_SNAP_ONLINE_HDR(&s_stViConfig, SNAP_PIPE_MODE_DUAL);
        break;
#endif

	case '6':
		s32Ret  = SAMPLE_SNAP_USER(&s_stViConfig, SNAP_PIPE_MODE_DUAL,0);
		break;
    default:
        SAMPLE_PRT("the index is invaild!\n");
        SNAP_Usage();
        break;
    }

    if (HI_SUCCESS == s32Ret)
    {
        SAMPLE_PRT("program exit normally!\n");
    }
    else
    {
        SAMPLE_PRT("program exit abnormally!\n");
    }

    return s32Ret;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

