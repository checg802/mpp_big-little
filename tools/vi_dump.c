#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <errno.h>


#include "hi_common.h"
#include "hi_comm_video.h"
#include "hi_comm_sys.h"
#include "mpi_sys.h"
#include "hi_comm_vb.h"
#include "mpi_vb.h"
#include "hi_comm_vi.h"
#include "mpi_vi.h"
#include "hi_comm_vgs.h"
#include "mpi_vgs.h"

//////////////////////////////////////////////////////////////
//#define DIS_DATA_DEBUG

#define MAX_FRM_CNT     25
#define MAX_FRM_WIDTH   8192

VI_CHN_ATTR_S stChnAttrBackup;
static volatile HI_BOOL bQuit = HI_FALSE;   /* bQuit may be set in the signal handler */
#if 1
static HI_S32 s_s32MemDev = -1;

#define MEM_DEV_OPEN() \
    do {\
        if (s_s32MemDev <= 0)\
        {\
            s_s32MemDev = open("/dev/mem", O_CREAT|O_RDWR|O_SYNC);\
            if (s_s32MemDev < 0)\
            {\
                perror("Open dev/mem error");\
                return -1;\
            }\
        }\
    }while(0)

#define MEM_DEV_CLOSE() \
    do {\
        HI_S32 s32Ret;\
        if (s_s32MemDev > 0)\
        {\
            s32Ret = close(s_s32MemDev);\
            if(HI_SUCCESS != s32Ret)\
            {\
                perror("Close mem/dev Fail");\
                return s32Ret;\
            }\
            s_s32MemDev = -1;\
        }\
    }while(0)

static void usage(void)
{
    printf(
        "\n"
        "*************************************************\n"
        "Usage: ./vi_dump [DataType] [ViChn] [FrmCnt]\n"
        "DataType: \n"
        "	-y(YUV) 	dump YUV\n"
        "	-r(RAW) 	dump RAW\n"
		"	-s(YUV) 	dump synchronal two frame(vichn0,vichn1),do not support 'Ctrl+c'.\n"
        "ViChn: \n"
        "	which chn to be dump\n"
        "FrmCnt: \n"
        "	the count of frame to be dump\n"
        "e.g : ./vi_dump -y 0 1 (dump one YUV)\n"
        "e.g : ./vi_dump -r 0 1 (dump one RAW)\n"
		"e.g : ./vi_dump -s     (dump synchronal two frame(vichn0,vichn1)\n"
        "*************************************************\n"
        "\n");
}
#endif
static void SigHandler(HI_S32 signo)
{
    if (HI_TRUE == bQuit)
    {
        return;
    }
    if (SIGINT == signo || SIGTERM == signo)
    {
        bQuit = HI_TRUE;
        fclose(stdin);  /* close stdin, so getchar will return EOF */
    }
}


#ifdef DIS_DATA_DEBUG
#define DIS_STATS_NUM   9
typedef struct hiVI_DIS_STATS_S
{
    HI_S32 as32HDelta[DIS_STATS_NUM];
    HI_S32 as32HSad[DIS_STATS_NUM];
    HI_S32 as32HMv[DIS_STATS_NUM];
    HI_S32 as32VDelta[DIS_STATS_NUM];
    HI_S32 as32VSad[DIS_STATS_NUM];
    HI_S32 as32VMv[DIS_STATS_NUM];
    HI_U32 u32HMotion;
    HI_U32 u32VMotion;
    HI_U32 u32HOffset;
    HI_U32 u32VOffset;
} VI_DIS_STATS_S;

HI_S32 vi_dump_save_one_dis(VI_CHN ViChn, VIDEO_FRAME_S* pVBuf)
{
    char g_strDisBuff[256] = {'\0'};
    char szDisFileName[128];
    static FILE* pstDisFd = NULL;
    HI_U32 u32DisBufOffset;
    VI_DIS_STATS_S* pstDisStats;
    int j;
    HI_U32 u32BufSize;
    int iBufSize = 0;
    int size = 256;

    if (pstDisFd == NULL)
    {
        sprintf(szDisFileName, "./vi_%dp_01_dis_result.txt", pVBuf->u32Width);

        pstDisFd = fopen(szDisFileName, "wb");
        if (NULL == pstDisFd)
        {
            printf("open file failed:%s!\n", strerror(errno));
            return HI_FAILURE;
        }
    }

    u32BufSize = pVBuf->u32Stride[0] * pVBuf->u32Height;
    if (PIXEL_FORMAT_YUV_SEMIPLANAR_422 == pVBuf->enPixelFormat)
    {
        u32BufSize *= 2;
    }
    else
    {
        u32BufSize = u32BufSize * 3 >> 1;
    }

    u32DisBufOffset = pVBuf->u32PhyAddr[0] + u32BufSize;
    pstDisStats = (VI_DIS_STATS_S*) HI_MPI_SYS_Mmap(u32DisBufOffset, size);

    sprintf(g_strDisBuff, "%s\n", \
            "v_delta[i], v_sad[i], v_mv[i], h_delta[i], h_sad[i], h_mv[i]");
    iBufSize = strlen(g_strDisBuff) + 1;
    fwrite(g_strDisBuff, iBufSize, 1, pstDisFd);


    for (j = 0; j < 9; j++)
    {
        sprintf(g_strDisBuff, "%8d, %8d, %8d, %8d, %8d, %8d\n", \
                pstDisStats->as32VDelta[j], pstDisStats->as32VSad[j], pstDisStats->as32VMv[j], \
                pstDisStats->as32HDelta[j], pstDisStats->as32HSad[j], pstDisStats->as32HMv[j]);
        iBufSize = strlen(g_strDisBuff) + 1;
        fwrite(g_strDisBuff, iBufSize, 1, pstDisFd);
    }

    sprintf(g_strDisBuff, "%s\n", "H_Motion, V_Motion, *H_Offset, *V_Offset");
    iBufSize = strlen(g_strDisBuff) + 1;
    fwrite(g_strDisBuff, iBufSize, 1, pstDisFd);

    sprintf(g_strDisBuff, "%8d  %8d  %8d  %8d\n\n", \
            pstDisStats->u32HMotion, pstDisStats->u32VMotion,
            pstDisStats->u32HOffset, pstDisStats->u32VOffset);
    iBufSize = strlen(g_strDisBuff) + 1;
    fwrite(g_strDisBuff, iBufSize, 1, pstDisFd);
    fflush(pstDisFd);

    HI_MPI_SYS_Munmap((HI_VOID*)pstDisStats, size);

    return HI_SUCCESS;
}
#endif


/* sp420 -> p420 ;  sp422 -> p422  */
HI_S32 vi_dump_save_one_frame(VIDEO_FRAME_S* pVBuf, FILE* pfd)
{
    unsigned int w, h;
    char* pVBufVirt_Y;
    char* pVBufVirt_C;
    char* pMemContent;
    unsigned char TmpBuff[MAX_FRM_WIDTH];                
    HI_U32 phy_addr, size;
    HI_CHAR* pUserPageAddr[2];
    PIXEL_FORMAT_E  enPixelFormat = pVBuf->enPixelFormat;
    HI_U32 u32UvHeight;/* UV height for planar format */

    if (pVBuf->u32Width > MAX_FRM_WIDTH)
    {
        printf("Over max frame width: %d, can't support.\n", MAX_FRM_WIDTH);
        return HI_FAILURE;
    }

    if (PIXEL_FORMAT_YUV_SEMIPLANAR_420 == enPixelFormat)
    {
        size = (pVBuf->u32Stride[0]) * (pVBuf->u32Height) * 3 / 2;
        u32UvHeight = pVBuf->u32Height / 2;
    }
    else if (PIXEL_FORMAT_YUV_SEMIPLANAR_422 == enPixelFormat)
    {
        size = (pVBuf->u32Stride[0]) * (pVBuf->u32Height) * 2;
        u32UvHeight = pVBuf->u32Height;
    }
    else if (PIXEL_FORMAT_YUV_400 == enPixelFormat)
    {
        size = (pVBuf->u32Stride[0]) * (pVBuf->u32Height);
        u32UvHeight = 0;
    }
    else
    {
        printf("not support pixelformat: %d\n", enPixelFormat);
        return HI_FAILURE;
    }

    phy_addr = pVBuf->u32PhyAddr[0];

    pUserPageAddr[0] = (HI_CHAR*) HI_MPI_SYS_Mmap(phy_addr, size);
    if (NULL == pUserPageAddr[0])
    {
        return HI_FAILURE;
    }
    printf("stride: %d,%d\n", pVBuf->u32Stride[0], pVBuf->u32Stride[1] );

    pVBufVirt_Y = pUserPageAddr[0];
    pVBufVirt_C = pVBufVirt_Y + (pVBuf->u32Stride[0]) * (pVBuf->u32Height);

    /* save Y ----------------------------------------------------------------*/
    fprintf(stderr, "saving......Y......");
    fflush(stderr);
    for (h = 0; h < pVBuf->u32Height; h++)
    {
        pMemContent = pVBufVirt_Y + h * pVBuf->u32Stride[0];
        fwrite(pMemContent, pVBuf->u32Width, 1, pfd);
    }
    fflush(pfd);

    /* save U ----------------------------------------------------------------*/
    fprintf(stderr, "U......");
    fflush(stderr);
    for (h = 0; h < u32UvHeight; h++)
    {
        pMemContent = pVBufVirt_C + h * pVBuf->u32Stride[1];

        pMemContent += 1;

        for (w = 0; w < pVBuf->u32Width / 2; w++)
        {
            TmpBuff[w] = *pMemContent;
            pMemContent += 2;
        }
        fwrite(TmpBuff, pVBuf->u32Width / 2, 1, pfd);
    }
    fflush(pfd);

    /* save V ----------------------------------------------------------------*/
    fprintf(stderr, "V......");
    fflush(stderr);
    for (h = 0; h < u32UvHeight; h++)
    {
        pMemContent = pVBufVirt_C + h * pVBuf->u32Stride[1];

        for (w = 0; w < pVBuf->u32Width / 2; w++)
        {
            TmpBuff[w] = *pMemContent;
            pMemContent += 2;
        }
        fwrite(TmpBuff, pVBuf->u32Width / 2, 1, pfd);
    }
    fflush(pfd);

    fprintf(stderr, "done %d!\n", pVBuf->u32TimeRef);
    fflush(stderr);

    HI_MPI_SYS_Munmap(pUserPageAddr[0], size);

    return HI_SUCCESS;
}

HI_S32 vi_dump_save_one_raw(VIDEO_FRAME_S* pVBuf, FILE* pfd)
{
    unsigned int w, h;
    HI_U16* pVBufVirt_Y;
    static HI_U16 au16Data[MAX_FRM_WIDTH];
    HI_U32 phy_addr, size;
    HI_U8* pUserPageAddr[2];

    size = (pVBuf->u32Stride[0]) * (pVBuf->u32Height);

    phy_addr = pVBuf->u32PhyAddr[0];

    pUserPageAddr[0] = (HI_U8*) HI_MPI_SYS_Mmap(phy_addr, size);
    if (NULL == pUserPageAddr[0])
    {
        return HI_FAILURE;
    }

    pVBufVirt_Y = (HI_U16*)pUserPageAddr[0];

    /* save Y ----------------------------------------------------------------*/
    fprintf(stderr, "saving......Raw data......u32Stride[0]: %d, width: %d\n", pVBuf->u32Stride[0], pVBuf->u32Width);
    fflush(stderr);
    for (h = 0; h < pVBuf->u32Height; h++)
    {
        HI_U16 u16Data;
        for (w = 0; w < pVBuf->u32Width; w++)
        {
            u16Data = pVBufVirt_Y[h * pVBuf->u32Width + w];
            au16Data[w] = u16Data;
        }

        fwrite(au16Data, pVBuf->u32Width, 2, pfd);
    }
    fflush(pfd);

    fprintf(stderr, "done u32TimeRef: %d!\n", pVBuf->u32TimeRef);
    fflush(stderr);

    HI_MPI_SYS_Munmap(pUserPageAddr[0], size);

    return HI_SUCCESS;
}


HI_S32 SAMPLE_VI_BackupAttr(VI_CHN ViChn)
{
    VI_CHN_ATTR_S stChnAttr;

    MEM_DEV_OPEN();

    if (ViChn != 0)
    {
        return 0;
    }

    if (HI_MPI_VI_GetChnAttr(ViChn, &stChnAttrBackup))
    {
        printf("HI_MPI_VI_GetChnAttr err, vi chn %d \n", ViChn);
        return -1;
    }

    /* clear user list */
    if(HI_MPI_VI_SetFrameDepth(ViChn, 0))
	{
		printf("HI_MPI_VI_SetFrameDepth err, vi chn %d \n", ViChn);
		return -1;
	}
		
    sleep(1);

    printf("compress mode: %d -> %d. \n", stChnAttrBackup.enCompressMode, COMPRESS_MODE_NONE);
    memcpy(&stChnAttr, &stChnAttrBackup, sizeof(VI_CHN_ATTR_S));

    stChnAttr.enCompressMode = COMPRESS_MODE_NONE;


    if (HI_MPI_VI_SetChnAttr(ViChn, &stChnAttr))
    {
        printf("HI_MPI_VI_SetChnAttr err, vi chn %d \n", ViChn);
        return -1;
    }

    return 0;
}

HI_S32 SAMPLE_VI_RestoreAttr(VI_CHN ViChn)
{
    MEM_DEV_CLOSE();

    if (ViChn != 0)
    {
        return 0;
    }

    printf("restore compress mode: %d\n", stChnAttrBackup.enCompressMode);
    if (HI_MPI_VI_SetChnAttr(ViChn, &stChnAttrBackup))
    {
        printf("HI_MPI_VI_SetChnAttr err, vi chn %d \n", ViChn);
        return -1;
    }

    return 0;
}

HI_S32 SAMPLE_MISC_GETVB(VIDEO_FRAME_INFO_S* pstOutFrame, VIDEO_FRAME_INFO_S* pstInFrame,
                         VB_BLK* pstVbBlk, VB_POOL pool)
{
    HI_U32 u32Size;
    VB_BLK VbBlk = VB_INVALID_HANDLE;
    HI_U32 u32PhyAddr;
    HI_VOID* pVirAddr;
    HI_U32 u32LumaSize, u32ChrmSize;
    HI_U32 u32LStride, u32CStride;
    HI_U32 u32Width, u32Height;


    u32Width = pstInFrame->stVFrame.u32Width;
    u32Height = pstInFrame->stVFrame.u32Height;
    u32LStride = pstInFrame->stVFrame.u32Stride[0];
    u32CStride = pstInFrame->stVFrame.u32Stride[1];
    if (PIXEL_FORMAT_YUV_SEMIPLANAR_422 == pstInFrame->stVFrame.enPixelFormat)
    {
        u32Size =  u32LStride * u32Height << 1;
        u32LumaSize = u32LStride * u32Height;
        u32ChrmSize = u32LumaSize;
    }
    else if (PIXEL_FORMAT_YUV_SEMIPLANAR_420 == pstInFrame->stVFrame.enPixelFormat)
    {
        u32Size = (3 * u32LStride * u32Height) >> 1;
        u32LumaSize = u32LStride * u32Height;
        u32ChrmSize = u32LumaSize >> 1;
    }
    else if (PIXEL_FORMAT_RGB_BAYER == pstInFrame->stVFrame.enPixelFormat)
    {
        u32Size = u32LStride * u32Height;
        u32LumaSize = u32LStride * u32Height;
        u32ChrmSize = 0;
    }
    else if (PIXEL_FORMAT_YUV_400 == pstInFrame->stVFrame.enPixelFormat)
    {
        u32Size     = u32LStride * u32Height;
        u32LumaSize = u32LStride * u32Height;
        u32ChrmSize = 0;
    }
    else
    {
        printf("Error!!!, not support PixelFormat: %d\n", pstInFrame->stVFrame.enPixelFormat);
        return HI_FAILURE;
    }

    VbBlk = HI_MPI_VB_GetBlock(pool, u32Size, HI_NULL);
    *pstVbBlk = VbBlk;

    if (VB_INVALID_HANDLE == VbBlk)
    {
        printf("HI_MPI_VB_GetBlock err! size:%d\n", u32Size);
        return HI_FAILURE;
    }

    u32PhyAddr = HI_MPI_VB_Handle2PhysAddr(VbBlk);
    if (0 == u32PhyAddr)
    {
        printf("HI_MPI_VB_Handle2PhysAddr err!\n");
        return HI_FAILURE;
    }

    pVirAddr = (HI_U8*) HI_MPI_SYS_Mmap(u32PhyAddr, u32Size);
    if (NULL == pVirAddr)
    {
        printf("HI_MPI_SYS_Mmap err!\n");
        return HI_FAILURE;
    }

    pstOutFrame->u32PoolId = HI_MPI_VB_Handle2PoolId(VbBlk);
    if (VB_INVALID_POOLID == pstOutFrame->u32PoolId)
    {
        printf("u32PoolId err!\n");
        return HI_FAILURE;
    }

    pstOutFrame->stVFrame.u32PhyAddr[0] = u32PhyAddr;

    //printf("\nuser u32phyaddr = 0x%x\n", pstOutFrame->stVFrame.u32PhyAddr[0]);
    pstOutFrame->stVFrame.u32PhyAddr[1] = pstOutFrame->stVFrame.u32PhyAddr[0] + u32LumaSize;
    pstOutFrame->stVFrame.u32PhyAddr[2] = pstOutFrame->stVFrame.u32PhyAddr[1] + u32ChrmSize;

    pstOutFrame->stVFrame.pVirAddr[0] = pVirAddr;
    pstOutFrame->stVFrame.pVirAddr[1] = pstOutFrame->stVFrame.pVirAddr[0] + u32LumaSize;
    pstOutFrame->stVFrame.pVirAddr[2] = pstOutFrame->stVFrame.pVirAddr[1] + u32ChrmSize;

    pstOutFrame->stVFrame.u32Width  = u32Width;
    pstOutFrame->stVFrame.u32Height = u32Height;
    pstOutFrame->stVFrame.u32Stride[0] = u32LStride;
    pstOutFrame->stVFrame.u32Stride[1] = u32CStride;
    pstOutFrame->stVFrame.u32Stride[2] = u32CStride;
    pstOutFrame->stVFrame.u32Field = VIDEO_FIELD_FRAME;
    pstOutFrame->stVFrame.enCompressMode = COMPRESS_MODE_NONE;
    pstOutFrame->stVFrame.enPixelFormat = pstInFrame->stVFrame.enPixelFormat;

    return HI_SUCCESS;
}


HI_S32 SAMPLE_MISC_ViDump(VI_CHN ViChn, HI_U32 u32Cnt)
{
    HI_S32 s32FrmCnt, j, s32Ret = HI_FAILURE;
    VIDEO_FRAME_INFO_S stFrame;
    VIDEO_FRAME_INFO_S* pstOutFrame;
    VIDEO_FRAME_INFO_S astFrame[MAX_FRM_CNT];
    HI_CHAR szYuvName[128] = {0};
    FILE* pfd = NULL;
    HI_CHAR szPixFrm[10] = {0};
    HI_S32 s32MilliSec = 2000;
    VGS_TASK_ATTR_S stTask;
    memset(&stTask, 0, sizeof(stTask));
    VGS_HANDLE hHandle;
    VB_BLK VbBlk = VB_INVALID_HANDLE;
    VB_POOL hPool  = VB_INVALID_POOLID;
    HI_U32  u32BlkSize = 0;
    HI_U32 u32OldDepth = -1U;

    if (HI_MPI_VI_GetFrameDepth(ViChn, &u32OldDepth))
    {
        printf("HI_MPI_VI_GetFrameDepth err, vi chn %d \n", ViChn);
        return HI_FAILURE;
    }
    if (HI_MPI_VI_SetFrameDepth(ViChn, 1))
    {
        printf("HI_MPI_VI_SetFrameDepth err, vi chn %d \n", ViChn);
        return HI_FAILURE;
    }

    usleep(90000);
	if (HI_TRUE == bQuit)
	{
		HI_MPI_VI_SetFrameDepth(ViChn, u32OldDepth);
		return HI_FAILURE;
	}

    if (HI_MPI_VI_GetFrame(ViChn, &stFrame, s32MilliSec))
    {
        printf("HI_MPI_VI_GetFrame err, vi chn %d \n", ViChn);
        goto END1;
    }

    /* make file name */
    strcpy(szPixFrm,
           ( PIXEL_FORMAT_YUV_400 == stFrame.stVFrame.enPixelFormat ) ? "single" :
           ( (PIXEL_FORMAT_YUV_SEMIPLANAR_420 == stFrame.stVFrame.enPixelFormat) ? "p420" : "p422" )
          );
    sprintf(szYuvName, "./vi_chn_%d_%d_%d_%s_%d.yuv", ViChn,
            stFrame.stVFrame.u32Width, stFrame.stVFrame.u32Height, szPixFrm, u32Cnt);

    s32Ret = HI_MPI_VI_ReleaseFrame(ViChn, &stFrame);
    if (s32Ret != HI_SUCCESS)
    {
        printf("release frame failed\n");
        goto END1;
    }

    if (COMPRESS_MODE_NONE != stFrame.stVFrame.enCompressMode)
    {
        if (PIXEL_FORMAT_YUV_400 == stFrame.stVFrame.enPixelFormat)
        {
            u32BlkSize = stFrame.stVFrame.u32Stride[0] * stFrame.stVFrame.u32Height;
        }
        else if (PIXEL_FORMAT_YUV_SEMIPLANAR_420 == stFrame.stVFrame.enPixelFormat)
        {
            u32BlkSize = stFrame.stVFrame.u32Stride[0] * stFrame.stVFrame.u32Height * 3 >> 1;
        }
        else if (PIXEL_FORMAT_YUV_SEMIPLANAR_422 == stFrame.stVFrame.enPixelFormat)
        {
            u32BlkSize = stFrame.stVFrame.u32Stride[0] * stFrame.stVFrame.u32Height * 2;
        }
        else
        {
            printf("Not support this pix format %d!\n", stFrame.stVFrame.enPixelFormat);
            s32Ret = HI_FAILURE;
            goto END1;
        }

        /*create comm vb pool*/
        hPool   = HI_MPI_VB_CreatePool(u32BlkSize, 2 , NULL);
        if (hPool == VB_INVALID_POOLID)
        {
            printf("HI_MPI_VB_CreatePool failed! \n");
            s32Ret = HI_FAILURE;
            goto END1;
        }
    }

    /* get VI frame  */
	for (j = 0; j < u32Cnt; j++)
	{
		astFrame[j].stVFrame.u32PhyAddr[0] = 0;
	}
    for (s32FrmCnt= 0; s32FrmCnt < u32Cnt; s32FrmCnt++)
    {
        if (HI_TRUE == bQuit)
        {
            break;
        }
        if (HI_MPI_VI_GetFrame(ViChn, &astFrame[s32FrmCnt], s32MilliSec) < 0)
        {
            printf("get vi chn %d frame err\n", ViChn);
            printf("only get %d frame\n", s32FrmCnt);
            break;
        }
    }
	if ((HI_TRUE == bQuit) || (0 >= s32FrmCnt))
    {
        goto END2;
    }
    /* open file */
    pfd = fopen(szYuvName, "w+b");
    if (NULL == pfd)
    {
        printf("open file failed:%s!\n", strerror(errno));
		s32Ret = HI_FAILURE;
        goto END2;
    }
    printf("Dump YUV frame of vi chn %d to file: \"%s\"\n", ViChn, szYuvName);

    for (j = 0; j < s32FrmCnt; j++)
    {
        if (HI_TRUE == bQuit)
        {
            break;
        }
        if (COMPRESS_MODE_NONE != astFrame[j].stVFrame.enCompressMode)
        {
            pstOutFrame = &stTask.stImgOut;
            memcpy(&stTask.stImgIn.stVFrame, &astFrame[j].stVFrame, sizeof(VIDEO_FRAME_S));
            stTask.stImgIn.u32PoolId = astFrame[j].u32PoolId;
            if (HI_SUCCESS != SAMPLE_MISC_GETVB(pstOutFrame, &astFrame[j], &VbBlk, hPool))
            {
                HI_MPI_VI_ReleaseFrame(ViChn, &astFrame[j]);
				astFrame[j].stVFrame.u32PhyAddr[0] = 0;

                if (VB_INVALID_HANDLE != VbBlk)
                {
                    HI_MPI_VB_ReleaseBlock(VbBlk);
                }
                continue;
            }

            s32Ret = HI_MPI_VGS_BeginJob(&hHandle);
            if (s32Ret != HI_SUCCESS)
            {
                printf("HI_MPI_VGS_BeginJob failed\n");
                HI_MPI_VI_ReleaseFrame(ViChn, &astFrame[j]);
				astFrame[j].stVFrame.u32PhyAddr[0] = 0;
                HI_MPI_VB_ReleaseBlock(VbBlk);
                goto END2;
            }

            s32Ret =  HI_MPI_VGS_AddScaleTask(hHandle, &stTask);

            if (s32Ret != HI_SUCCESS)
            {
                printf("HI_MPI_VGS_AddScaleTask failed\n");
                HI_MPI_VGS_CancelJob(hHandle);
                HI_MPI_VI_ReleaseFrame(ViChn, &astFrame[j]);
				astFrame[j].stVFrame.u32PhyAddr[0] = 0;
                HI_MPI_VB_ReleaseBlock(VbBlk);
                goto END2;
            }


            s32Ret = HI_MPI_VGS_EndJob(hHandle);
            if (s32Ret != HI_SUCCESS)
            {
                printf("HI_MPI_VGS_EndJob failed\n");
                HI_MPI_VGS_CancelJob(hHandle);
                HI_MPI_VI_ReleaseFrame(ViChn, &astFrame[j]);
				astFrame[j].stVFrame.u32PhyAddr[0] = 0;
                HI_MPI_VB_ReleaseBlock(VbBlk);
                goto END2;
            }
            /* save VI frame to file */
            s32Ret = vi_dump_save_one_frame(&pstOutFrame->stVFrame, pfd);
        }
        else
        {
            s32Ret = vi_dump_save_one_frame(&astFrame[j].stVFrame, pfd);
        }

#ifdef DIS_DATA_DEBUG
        s32Ret = vi_dump_save_one_dis(ViChn, &astFrame[j].stVFrame);
#endif

        /* release frame after using */
        HI_MPI_VI_ReleaseFrame(ViChn, &astFrame[j]);
		astFrame[j].stVFrame.u32PhyAddr[0] = 0;
        if (VB_INVALID_HANDLE != VbBlk)
        {
            HI_MPI_VB_ReleaseBlock(VbBlk);
        }
    }

END2:
    if (HI_SUCCESS != s32Ret)
    {
        remove(szYuvName);
    }
    if (hPool != VB_INVALID_POOLID)
    {
        HI_MPI_VB_DestroyPool(hPool);
    }
    
	for (j = 0; j < s32FrmCnt; j++)
	{
	    if (0 != astFrame[j].stVFrame.u32PhyAddr[0])
	    {
	        HI_MPI_VI_ReleaseFrame(ViChn, &astFrame[j]);
	    }
	}    
END1:
    if (NULL != pfd)
    {
        fclose(pfd);
    }
    if (-1U != u32OldDepth)
    {
        HI_MPI_VI_SetFrameDepth(ViChn, u32OldDepth);
    }
    
    return s32Ret;
}

HI_S32 VI_SAME_ViDump()
{
    HI_S32 s32FrmCnt, j, s32Ret = HI_FAILURE;
    VIDEO_FRAME_INFO_S stFrame[2];
    VIDEO_FRAME_INFO_S* pstOutFrame;
    VIDEO_FRAME_INFO_S astFrame[2][MAX_FRM_CNT];
    HI_CHAR szYuvName[2][128] = {{0}};
    FILE* pfd[2] = {NULL};
    HI_CHAR szPixFrm[2][10] = {{0}};
    HI_S32 s32MilliSec = 2000;
    VGS_TASK_ATTR_S stTask;
    VGS_HANDLE hHandle;
    VB_BLK VbBlk = VB_INVALID_HANDLE;
	VB_BLK VbBlk1 = VB_INVALID_HANDLE;
    VB_POOL hPool  = VB_INVALID_POOLID;
    HI_U32  u32BlkSize = 0,u32BlkSize1,u32tmpBlkSize;
    HI_U32 u32OldDepth[2] = {0,0};
	HI_S32 ViChn[2] = {0,1};
	HI_S32 i = 0;
	HI_U32	u32Cnt = 3,u32MidChn0 = 0,s32Midchn1 = -1;
	HI_U64 u64PtsTap[10] = {0}, u64temp;

	for(i = 0;i<2;i++)
	{
		if (HI_MPI_VI_GetFrameDepth(ViChn[i], &u32OldDepth[i]))
		{
			printf("HI_MPI_VI_GetFrameDepth err, vi chn %d \n", ViChn[i]);
			return HI_FAILURE;
		}
		if (HI_MPI_VI_SetFrameDepth(ViChn[i], u32Cnt))
		{
			printf("HI_MPI_VI_SetFrameDepth err, vi chn %d \n", ViChn[i]);
			return HI_FAILURE;
		}
	}

    usleep(90000);    

    if (HI_MPI_VI_GetFrame(ViChn[0], &stFrame[0], s32MilliSec))
    {
        printf("HI_MPI_VI_GetFrame err, vi chn %d \n", ViChn[0]);
        goto END1;
    }	

    if (HI_MPI_VI_GetFrame(ViChn[1], &stFrame[1], s32MilliSec))
    {
        HI_MPI_VI_ReleaseFrame(ViChn[0], &stFrame[0]);
        printf("HI_MPI_VI_GetFrame err, vi chn %d \n", ViChn[0]);
        goto END1;
    }
    
	for(i = 0;i<2;i++)
	{
        /* make file name */
        strcpy(szPixFrm[i],
               ( PIXEL_FORMAT_YUV_400 == stFrame[i].stVFrame.enPixelFormat ) ? "single" :
               ( (PIXEL_FORMAT_YUV_SEMIPLANAR_420 == stFrame[i].stVFrame.enPixelFormat) ? "p420" : "p422" )
              ); 

        sprintf(szYuvName[i], "./vi_chn_%d_%d_%d_%s.yuv", ViChn[i],
                stFrame[i].stVFrame.u32Width, stFrame[i].stVFrame.u32Height, szPixFrm[i]); 
     
        s32Ret = HI_MPI_VI_ReleaseFrame(ViChn[i], &stFrame[i]);
        if (s32Ret != HI_SUCCESS)
        {
            printf("release frame failed\n");
            goto END1;
        }
	}  

    if (COMPRESS_MODE_NONE != stFrame[0].stVFrame.enCompressMode)
    {
        if (PIXEL_FORMAT_YUV_400 == stFrame[0].stVFrame.enPixelFormat)
        {
            u32BlkSize = stFrame[0].stVFrame.u32Stride[0] * stFrame[0].stVFrame.u32Height;
			u32BlkSize1 = stFrame[1].stVFrame.u32Stride[0] * stFrame[1].stVFrame.u32Height;
        }
        else if (PIXEL_FORMAT_YUV_SEMIPLANAR_420 == stFrame[0].stVFrame.enPixelFormat)
        {
            u32BlkSize = stFrame[0].stVFrame.u32Stride[0] * stFrame[0].stVFrame.u32Height * 3 >> 1;
			u32BlkSize1 = stFrame[1].stVFrame.u32Stride[0] * stFrame[1].stVFrame.u32Height * 3 >> 1;
        }
        else if (PIXEL_FORMAT_YUV_SEMIPLANAR_422 == stFrame[0].stVFrame.enPixelFormat)
        {
            u32BlkSize = stFrame[0].stVFrame.u32Stride[0] * stFrame[0].stVFrame.u32Height * 2;
			u32BlkSize1 = stFrame[1].stVFrame.u32Stride[0] * stFrame[1].stVFrame.u32Height * 2;
        }
        else
        {
            printf("Not support this pix format %d!\n", stFrame[0].stVFrame.enPixelFormat);
            s32Ret = HI_FAILURE;
            goto END1;
        }

		if(u32BlkSize > u32BlkSize1)
		{
			u32tmpBlkSize = u32BlkSize;
		}
		else
		{
			u32tmpBlkSize = u32BlkSize1;
		}
        /*create comm vb pool*/
        hPool   = HI_MPI_VB_CreatePool(u32tmpBlkSize, 3*u32Cnt , NULL);
        if (hPool == VB_INVALID_POOLID)
        {
            printf("HI_MPI_VB_CreatePool failed! \n");
            s32Ret = HI_FAILURE;
            goto END1;
        }
    }

    /* get VI frame  */
	for(i = 0; i<2; i++)
	{
		for (s32FrmCnt= 0; s32FrmCnt < u32Cnt; s32FrmCnt++)
		{
			if (HI_MPI_VI_GetFrame(ViChn[i], &astFrame[i][s32FrmCnt], s32MilliSec) < 0)
			{
			    printf("get vi chn %d frame err\n", ViChn[i]);
			    printf("only get %d frame\n", s32FrmCnt);
			    break;
			}
		}
	}
    
    if (0 >= s32FrmCnt)
    {
        goto END2;
    }	
    /* open file */
    pfd[0] = fopen(szYuvName[0], "w+b");
    pfd[1] = fopen(szYuvName[1], "w+b");
    if ((NULL == pfd[0])||(NULL == pfd[1]))
    {
        printf("open file failed:%s!\n", strerror(errno));

        for(i = 0; i<2; i++)
        {
            for (s32FrmCnt= 0; s32FrmCnt < u32Cnt; s32FrmCnt++)
            {
            	  HI_MPI_VI_ReleaseFrame(ViChn[i], &astFrame[i][s32FrmCnt]);	
            }
        }       
		s32Ret = HI_FAILURE;
        goto END2;
    }
	
    printf("Dump YUV frame of vi chn %d to file: \"%s\"\n", ViChn[0], szYuvName[0]);

    
    printf("Dump YUV frame of vi chn %d to file: \"%s\"\n", ViChn[1], szYuvName[1]);


	u32MidChn0 = u32Cnt/2;
	
	for (s32FrmCnt= 0; s32FrmCnt < u32Cnt; s32FrmCnt++)
	{
		if(u32MidChn0 != s32FrmCnt)
		{	
			HI_MPI_VI_ReleaseFrame(ViChn[0], &astFrame[0][s32FrmCnt]);	
		}

		if(astFrame[0][u32MidChn0].stVFrame.u64pts >= astFrame[1][s32FrmCnt].stVFrame.u64pts)
		{
			u64PtsTap[s32FrmCnt] = astFrame[0][u32MidChn0].stVFrame.u64pts - astFrame[1][s32FrmCnt].stVFrame.u64pts;		
		}
		else
		{
			u64PtsTap[s32FrmCnt] = astFrame[1][s32FrmCnt].stVFrame.u64pts- astFrame[0][u32MidChn0].stVFrame.u64pts;	
		}
	}

    s32FrmCnt= 0;

	u64temp = u64PtsTap[s32FrmCnt];
	
	for (s32FrmCnt= 0; s32FrmCnt < u32Cnt; s32FrmCnt++)
	{		
		if(u64temp >= u64PtsTap[s32FrmCnt])
		{
			u64temp = u64PtsTap[s32FrmCnt];
			s32Midchn1 = s32FrmCnt;	
		}
        printf("vichn1 frame num = %d, u64PtsTap = %llu\n",s32FrmCnt, u64PtsTap[s32FrmCnt]);
	}

    for (j = 0; j < s32FrmCnt; j++)
    {
	    if(j == u32MidChn0)
	    {
	         printf("\nsave vichn0 frame num = %d, pts = %llu\n",
	            j, astFrame[0][j].stVFrame.u64pts);
        if (COMPRESS_MODE_NONE != astFrame[0][j].stVFrame.enCompressMode)
        {

            pstOutFrame = &stTask.stImgOut;
			    memcpy(&stTask.stImgIn.stVFrame, &astFrame[0][j].stVFrame, sizeof(VIDEO_FRAME_S));
			    stTask.stImgIn.u32PoolId = astFrame[0][j].u32PoolId;
			    if (HI_SUCCESS != SAMPLE_MISC_GETVB(pstOutFrame, &astFrame[0][j], &VbBlk, hPool))
            {
			        HI_MPI_VI_ReleaseFrame(ViChn[0], &astFrame[0][j]);

                if (VB_INVALID_HANDLE != VbBlk)
                {
                    HI_MPI_VB_ReleaseBlock(VbBlk);
                }
			         goto END3;
            }

            s32Ret = HI_MPI_VGS_BeginJob(&hHandle);
            if (s32Ret != HI_SUCCESS)
            {
                printf("HI_MPI_VGS_BeginJob failed\n");
			        HI_MPI_VI_ReleaseFrame(ViChn[0], &astFrame[0][j]);
                HI_MPI_VB_ReleaseBlock(VbBlk);
			        goto END3;
            }

            s32Ret =  HI_MPI_VGS_AddScaleTask(hHandle, &stTask);

            if (s32Ret != HI_SUCCESS)
            {
                printf("HI_MPI_VGS_AddScaleTask failed\n");
                HI_MPI_VGS_CancelJob(hHandle);
			        HI_MPI_VI_ReleaseFrame(ViChn[0], &astFrame[0][j]);
                HI_MPI_VB_ReleaseBlock(VbBlk);
			        goto END3;
            }


            s32Ret = HI_MPI_VGS_EndJob(hHandle);
            if (s32Ret != HI_SUCCESS)
            {
                printf("HI_MPI_VGS_EndJob failed\n");
                HI_MPI_VGS_CancelJob(hHandle);
			        HI_MPI_VI_ReleaseFrame(ViChn[0], &astFrame[0][j]);
                HI_MPI_VB_ReleaseBlock(VbBlk);
			        goto END3;
            }
            /* save VI frame to file */
			    s32Ret = vi_dump_save_one_frame(&pstOutFrame->stVFrame, pfd[0]);
				
        }
        else  
        {

                s32Ret = vi_dump_save_one_frame(&astFrame[0][j].stVFrame, pfd[0]);
			}
                /* release frame after using */
                HI_MPI_VI_ReleaseFrame(ViChn[0], &astFrame[0][j]);
		if (VB_INVALID_HANDLE != VbBlk)
		{
			HI_MPI_VB_ReleaseBlock(VbBlk);
		}
            }
            
            if(j == s32Midchn1)
            {
                printf("save vichn1 frame num  = %d, pts = %llu\n",
                    j, astFrame[1][j].stVFrame.u64pts);
			if(COMPRESS_MODE_NONE != astFrame[1][j].stVFrame.enCompressMode)
			{
				pstOutFrame = &stTask.stImgOut;
			    memcpy(&stTask.stImgIn.stVFrame, &astFrame[1][j].stVFrame, sizeof(VIDEO_FRAME_S));
			    stTask.stImgIn.u32PoolId = astFrame[1][j].u32PoolId;
			    if (HI_SUCCESS != SAMPLE_MISC_GETVB(pstOutFrame, &astFrame[1][j], &VbBlk1, hPool))
			    {			       
			         goto END3;
			    }
			    s32Ret = HI_MPI_VGS_BeginJob(&hHandle);
			    if (s32Ret != HI_SUCCESS)
			    {
			        printf("HI_MPI_VGS_BeginJob failed\n");			       
			        goto END3;
			    }
			    s32Ret =  HI_MPI_VGS_AddScaleTask(hHandle, &stTask);
			    if (s32Ret != HI_SUCCESS)
			    {
			        printf("HI_MPI_VGS_AddScaleTask failed\n");
			        HI_MPI_VGS_CancelJob(hHandle);			      
			        goto END3;
			    }
			    s32Ret = HI_MPI_VGS_EndJob(hHandle);
			    if (s32Ret != HI_SUCCESS)
			    {
			        printf("HI_MPI_VGS_EndJob failed\n");
			        HI_MPI_VGS_CancelJob(hHandle);
			        goto END3;
			    }
			    s32Ret = vi_dump_save_one_frame(&pstOutFrame->stVFrame, pfd[1]);
			}
			else
			{
                s32Ret = vi_dump_save_one_frame(&astFrame[1][j].stVFrame, pfd[1]);
            }
        }
	}
	
END3:
    for (j = 0; j < s32FrmCnt; j++)
	{
        /* release frame after using */
        HI_MPI_VI_ReleaseFrame(ViChn[1], &astFrame[1][j]);
	}
    if (VB_INVALID_HANDLE != VbBlk1)
        {
        HI_MPI_VB_ReleaseBlock(VbBlk1);
        }


END2:
    if (HI_SUCCESS != s32Ret)
    {
        remove(szYuvName[0]);
        remove(szYuvName[1]);
    }
    if (hPool != VB_INVALID_POOLID)
    {
        HI_MPI_VB_DestroyPool(hPool);
    }
    
END1:

    for(i  = 0;i< 2;i++)
    {
        if (NULL != pfd[i])
        {
            fclose(pfd[i]);
        }
    }

    for(i = 0;i<2;i++)
    {
        if (-1U != u32OldDepth[i])
        {
            if(HI_MPI_VI_SetFrameDepth(i, u32OldDepth[i]))
			{
				printf("Set  vi chn%d's Depth failed\n", i);
			}
        }
    }
    
    return s32Ret;
}

HI_S32 SAMPLE_MISC_ViDumpRaw(VI_CHN ViChn, HI_U32 u32Cnt)
{
    HI_S32 i, j, s32Ret;
    VIDEO_FRAME_INFO_S stFrame;
    VIDEO_FRAME_INFO_S* pstOutFrame;
    VIDEO_FRAME_INFO_S astFrame[MAX_FRM_CNT];
    HI_CHAR szYuvName[128] = {0};
    FILE* pfd = NULL;
    HI_S32 s32MilliSec = 2000;
    VGS_TASK_ATTR_S stTask;
    VGS_HANDLE hHandle;
    VB_BLK VbBlk = VB_INVALID_HANDLE;
    VB_POOL hPool  = VB_INVALID_POOLID;
    HI_U32  u32BlkSize = 0;
    HI_S32  s32Tries = 10;
    HI_BOOL bGetFrame = HI_FAILURE;
    HI_U32  u32OldDepth = -1U;
    ROTATE_E enRotate = ROTATE_NONE;
    VI_LDC_ATTR_S stLdcAttr = {0};
    VI_CHN_ATTR_S stChnAttr = {{0}};
    VI_CHN_ATTR_S stChnAttrBackup = {{0}};

    HI_MPI_VI_GetRotate(ViChn, &enRotate);
    if (ROTATE_NONE != enRotate)
    {
        printf("not support when Rotate is set!\n");
        return HI_FAILURE;
    }
    HI_MPI_VI_GetLDCAttr(ViChn, &stLdcAttr);
    if (HI_TRUE == stLdcAttr.bEnable)
    {
        printf("not support when LDC is enable\n");
        return HI_FAILURE;
    }
    
    HI_MPI_VI_GetChnAttr(ViChn, &stChnAttr);
    memcpy(&stChnAttrBackup, &stChnAttr, sizeof(stChnAttrBackup));
    stChnAttr.enPixFormat = PIXEL_FORMAT_RGB_BAYER;
    stChnAttr.enCompressMode = COMPRESS_MODE_NONE;
    HI_MPI_VI_SetChnAttr(ViChn, &stChnAttr);

    if (HI_MPI_VI_GetFrameDepth(ViChn, &u32OldDepth))
    {
        printf("HI_MPI_VI_GetFrameDepth err, vi chn %d \n", ViChn);
        goto END1;
    }
    if (HI_MPI_VI_SetFrameDepth(ViChn, 1))
    {
        printf("HI_MPI_VI_SetFrameDepth err, vi chn %d \n", ViChn);
        goto END1;
    }

    while (s32Tries > 0)
    {
        usleep(90000);
        s32Tries--;
		if (HI_TRUE == bQuit)
        {
            break;
        }
        if (HI_MPI_VI_GetFrame(ViChn, &stFrame, s32MilliSec))
        {
            printf("HI_MPI_VI_GetFrame err, vi chn %d \n", ViChn);
            goto END1;
        }

        if (PIXEL_FORMAT_RGB_BAYER == stFrame.stVFrame.enPixelFormat)
        {
            bGetFrame = HI_SUCCESS;
            /* make file name */
            sprintf(szYuvName, "./vi_chn_%d_%d_%d_%d_16bit.raw", ViChn,
                    stFrame.stVFrame.u32Width, stFrame.stVFrame.u32Height, u32Cnt);
        }

        s32Ret = HI_MPI_VI_ReleaseFrame(ViChn, &stFrame);
        if (s32Ret != HI_SUCCESS)
        {
            printf("release frame failed\n");
            goto END1;
        }

        if (HI_SUCCESS == bGetFrame)
        {
            break;
        }
    }
    if (HI_SUCCESS != bGetFrame)
    {
        printf("GetFrame timeout\n");
        goto END1;
    }

    if (COMPRESS_MODE_NONE != stFrame.stVFrame.enCompressMode)
    {
        u32BlkSize = (stFrame.stVFrame.u32Stride[0] * stFrame.stVFrame.u32Height * 2);

        /*create comm vb pool*/
        hPool   = HI_MPI_VB_CreatePool(u32BlkSize, 2 , NULL);
        if (hPool == VB_INVALID_POOLID)
        {
            printf("HI_MPI_VB_CreatePool failed! \n");
            goto END1;
        }
    }

    /* get VI frame  */
	for (j = 0; j < u32Cnt; j++)
    {
        astFrame[j].stVFrame.u32PhyAddr[0] = 0;
    }
    for (i = 0; i < u32Cnt; i++)
    {
	    if (HI_TRUE == bQuit)
        {
            break;
        }
        if (HI_MPI_VI_GetFrame(ViChn, &astFrame[i], s32MilliSec) < 0)
        {
            printf("get vi chn %d frame err\n", ViChn);
            printf("only get %d frame\n", i);
            break;
        }
    }
    if ((0 >= i)||(HI_TRUE == bQuit))
    {
        goto END2;
    }
    /* open file */
    pfd = fopen(szYuvName, "w+b");
    if (NULL == pfd)
    {
        printf("open file failed:%s!\n", strerror(errno));
        goto END2;
    }
    printf("Dump raw frame of vi chn %d  to file: \"%s\"\n", ViChn, szYuvName);

    for (j = 0; j < i; j++)
    {
		if (HI_TRUE == bQuit)
        {
            break;
        }
        if (COMPRESS_MODE_NONE != astFrame[j].stVFrame.enCompressMode)
        {
            pstOutFrame = &stTask.stImgOut;
            memcpy(&stTask.stImgIn.stVFrame, &astFrame[j].stVFrame, sizeof(VIDEO_FRAME_S));
            stTask.stImgIn.u32PoolId = astFrame[j].u32PoolId;
            if (HI_SUCCESS != SAMPLE_MISC_GETVB(pstOutFrame, &astFrame[j], &VbBlk, hPool))
            {
                HI_MPI_VI_ReleaseFrame(ViChn, &astFrame[j]);
				astFrame[j].stVFrame.u32PhyAddr[0] = 0;

                if (VB_INVALID_HANDLE != VbBlk)
                {
                    HI_MPI_VB_ReleaseBlock(VbBlk);
                }
                continue;
            }

            s32Ret = HI_MPI_VGS_BeginJob(&hHandle);
            if (s32Ret != HI_SUCCESS)
            {
                printf("HI_MPI_VGS_BeginJob failed\n");
                HI_MPI_VI_ReleaseFrame(ViChn, &astFrame[j]);
				astFrame[j].stVFrame.u32PhyAddr[0] = 0;
                HI_MPI_VB_ReleaseBlock(VbBlk);
                goto END2;
            }

            s32Ret =  HI_MPI_VGS_AddScaleTask(hHandle, &stTask);

            if (s32Ret != HI_SUCCESS)
            {
                printf("HI_MPI_VGS_AddScaleTask failed\n");
                HI_MPI_VGS_CancelJob(hHandle);
                HI_MPI_VI_ReleaseFrame(ViChn, &astFrame[j]);
				astFrame[j].stVFrame.u32PhyAddr[0] = 0;
                HI_MPI_VB_ReleaseBlock(VbBlk);
                goto END2;
            }


            s32Ret = HI_MPI_VGS_EndJob(hHandle);
            if (s32Ret != HI_SUCCESS)
            {
                printf("HI_MPI_VGS_EndJob failed\n");
                HI_MPI_VGS_CancelJob(hHandle);
                HI_MPI_VI_ReleaseFrame(ViChn, &astFrame[j]);
				astFrame[j].stVFrame.u32PhyAddr[0] = 0;
                HI_MPI_VB_ReleaseBlock(VbBlk);
                goto END2;
            }
            /* save VI frame to file */
            s32Ret = vi_dump_save_one_raw(&pstOutFrame->stVFrame, pfd);
        }
        else
        {
            s32Ret = vi_dump_save_one_raw(&astFrame[j].stVFrame, pfd);
        }

#ifdef DIS_DATA_DEBUG
        s32Ret = vi_dump_save_one_dis(ViChn, &astFrame[j].stVFrame);
#endif

        /* release frame after using */
        HI_MPI_VI_ReleaseFrame(ViChn, &astFrame[j]);
		astFrame[j].stVFrame.u32PhyAddr[0] = 0;
        if (VB_INVALID_HANDLE != VbBlk)
        {
            HI_MPI_VB_ReleaseBlock(VbBlk);
        }
    }

END2:
    if (HI_SUCCESS != s32Ret)
    {
        remove(szYuvName);
    }
    if (hPool != VB_INVALID_POOLID)
    {
        HI_MPI_VB_DestroyPool(hPool);
    }
	for (j = 0; j < i; j++)
    {
        if (0 != astFrame[j].stVFrame.u32PhyAddr[0])
        {
            HI_MPI_VI_ReleaseFrame(ViChn, &astFrame[j]);
        }
    }
END1:
    if (NULL != pfd)
    {
        fclose(pfd);
    }

    if (-1U != u32OldDepth)
    {
        HI_MPI_VI_SetFrameDepth(ViChn, u32OldDepth);
    }

    HI_MPI_VI_SetChnAttr(ViChn, &stChnAttrBackup);

    return 0;
}

HI_BOOL IsViVpssOnline(void)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32OnlineMode;
    
    s32Ret = HI_MPI_SYS_GetViVpssMode(&u32OnlineMode);

    if (s32Ret != HI_SUCCESS)
    {
        printf("get vi-vpss online mode failed\n");
        return HI_TRUE;
    }

    return (u32OnlineMode ? HI_TRUE : HI_FALSE);
}

#ifdef __HuaweiLite__
HI_S32 vi_dump(int argc, char* argv[])
#else
HI_S32 main(int argc, char* argv[])
#endif
{
    VI_CHN ViChn = 0;
    HI_U32 u32FrmCnt = 1;
    HI_S32 s32Ret = HI_FAILURE;
    HI_CHAR au8Type[20] = "-y";

    printf("\nNOTICE: This tool only can be used for TESTING !!!\n");
    printf("Usage:  ./vi_dump [DataType] [ViChn] [FrmCnt]. sample: ./vi_dump -y 0 5\n");
    printf("\t To see more usage, please enter: ./vi_dump -h\n\n");
    
    if (argc > 1)
    {
        strcpy(au8Type, argv[1]); /* DataType */
    }

    if (argc > 2)
    {
        ViChn = atoi(argv[2]);
    }

    if (argc > 3)
    {
        u32FrmCnt = atoi(argv[3]);/* FrmCnt */
    }

    if (!strncmp(au8Type, "-h", 2))
    {
        usage();
        exit(HI_SUCCESS);
    }
    if (1 > u32FrmCnt)
    {
        printf("invalid FrmCnt %d\n", u32FrmCnt);
        exit(HI_FAILURE);
    }

	if (u32FrmCnt > MAX_FRM_CNT)
    {
        printf("invalid FrmCnt, max frame count is %d.\n", MAX_FRM_CNT);
        exit(HI_FAILURE);
    }

    if (HI_TRUE == IsViVpssOnline())
    {
        printf("vi_dump not support online mode\n");
        exit(s32Ret);
    }

    MEM_DEV_OPEN();
    signal(SIGINT, SigHandler);
    signal(SIGTERM, SigHandler);

    if (!strcmp("-y", au8Type) || !strcmp("-Y", au8Type))
    {
        s32Ret = SAMPLE_MISC_ViDump(ViChn, u32FrmCnt);
    }
    else if (!strcmp("-r", au8Type) || !strcmp("-R", au8Type))
    {
        s32Ret = SAMPLE_MISC_ViDumpRaw(ViChn, u32FrmCnt);
    }
    else if(!strcmp("-s", au8Type) || !strcmp("-S", au8Type))
    {
       
       s32Ret = VI_SAME_ViDump();       
    }
    else
    {
        usage();
    }
   
    MEM_DEV_CLOSE();

    return s32Ret;
}



