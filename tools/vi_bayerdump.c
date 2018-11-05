#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#ifndef __HuaweiLite__
#include <sys/poll.h>
#endif
#include <sys/time.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "hi_common.h"
#include "hi_comm_video.h"
#include "hi_comm_sys.h"
#include "hi_comm_vi.h"
#include "mpi_vb.h"
#include "mpi_sys.h"
#include "mpi_vi.h"
#include "mpi_isp.h"
#include "hi_comm_isp.h"


static HI_BOOL bDumpAllFrame = HI_FALSE;
#define MAX_FRM_CNT     50
#define MAX_FRM_WIDTH   8192      

#define ALIGN_BACK(x, a)              ((a) * (((x) / (a))))

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

void usage(void)
{
    printf(
        "\n"
        "*************************************************\n"
        "Usage: ./vi_bayerdump [DataType] [ViDev] [nbit] [FrmCnt] [dumpsel] [VCNum] \n"       
        "DataType: \n"
        "   -r(R)       dump raw data\n"
        "   -s(S)       dump synchronical two frame raw data(dev0,dev1),not support 'Ctrl+c'\n"
        "ViDev: \n"
        "   0:dev_0,  1:dev_1\n"
        "nbit: \n"
        "   The bit num to be dump\n"
        "FrmCnt: \n"
        "   the count of frame to be dump\n"       
        "DumpSel: \n"
        "0:port,1:bayerscale\n"
        "VCNum: \n"
        "line mode/wdr mode: 0, 1, 2, 3 the frame vc num,long or short frame\n"
        "e.g : ./vi_bayerdump -r 0 16 1 0 0(dump vc0 raw)\n"
        "e.g : ./vi_bayerdump -r 0 16 3 0(dump long and short frame: vc0,vc1, vc2 raw)\n"
        "e.g : ./vi_bayerdump -s 16(dump synchronical two frame raw data(dev0,dev1))\n"
        "*************************************************\n"
        "\n");
    exit(1);
}
#endif
static HI_VOID SigHandler(HI_S32 signo)
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



static inline HI_S32 bitWidth2PixelFormat(HI_U32 u32Nbit, PIXEL_FORMAT_E *penPixelFormat)
{
    PIXEL_FORMAT_E enPixelFormat;
    
    if (8 == u32Nbit)
    {
        enPixelFormat = PIXEL_FORMAT_RGB_BAYER_8BPP;
    }
    else if (10 == u32Nbit)
    {
        enPixelFormat = PIXEL_FORMAT_RGB_BAYER_10BPP;
    }
    else if (12 == u32Nbit)
    {
        enPixelFormat = PIXEL_FORMAT_RGB_BAYER_12BPP;
    }
    else if (14 == u32Nbit)
    {
        enPixelFormat = PIXEL_FORMAT_RGB_BAYER_14BPP;
    }
    else if (16 == u32Nbit)
    {
        enPixelFormat = PIXEL_FORMAT_RGB_BAYER;
    }
    else
    {
        return HI_FAILURE;
    }

    *penPixelFormat = enPixelFormat;
    return HI_SUCCESS;
}
int sample_bayer_dump(VIDEO_FRAME_S* pVBuf, HI_U32 u32Nbit, FILE* pfd)
{
    unsigned int  h;
    HI_U16 *pu16Data;   
    HI_U32 phy_addr, size;
    HI_U8* pUserPageAddr[2];
	HI_U8  *pu8Data;

    size = (pVBuf->u32Stride[0]) * (pVBuf->u32Height) * 2;

    phy_addr = pVBuf->u32PhyAddr[0];

    MEM_DEV_OPEN();

    pUserPageAddr[0] = (HI_U8*) HI_MPI_SYS_Mmap(phy_addr, size);
    if (NULL == pUserPageAddr[0])
    {
        MEM_DEV_CLOSE();
        return -1;
    }
	pu8Data = pUserPageAddr[0];	
	if ((8 != u32Nbit) && (16 != u32Nbit))
	{
		pu16Data = (HI_U16*)malloc(pVBuf->u32Width * 2);
		if (NULL == pu16Data)
		{
			fprintf(stderr, "alloc memory failed\n");
			HI_MPI_SYS_Munmap(pUserPageAddr[0], size);
			pUserPageAddr[0] = NULL;
			return HI_FAILURE;
		}
	}
	
    /* save Y ----------------------------------------------------------------*/
    fprintf(stderr, "saving......dump data......u32Stride[0]: %d, width: %d\n", pVBuf->u32Stride[0], pVBuf->u32Width);
    fflush(stderr);
    for (h = 0; h < pVBuf->u32Height; h++)
    {
		if (8 == u32Nbit)
        {
            fwrite(pu8Data, pVBuf->u32Width, 1, pfd);
        }
        else if (16 == u32Nbit)
        {
            fwrite(pu8Data, pVBuf->u32Width, 2, pfd);
			fflush(pfd);
        }
        else
        {
            convertBitPixel(pu8Data, pVBuf->u32Width, u32Nbit, pu16Data);
            fwrite(pu16Data, pVBuf->u32Width, 2, pfd);
        }
        pu8Data += pVBuf->u32Stride[0];


    }
    fflush(pfd);

    fprintf(stderr, "done u32TimeRef: %d!\n", pVBuf->u32TimeRef);
    fflush(stderr);

    HI_MPI_SYS_Munmap(pUserPageAddr[0], size);

    MEM_DEV_CLOSE();

    return 0;
}

HI_S32 VI_DumpBayer(VI_DEV ViDev, VI_DUMP_ATTR_S* pstViDumpAttr, HI_U32 u32Nbit, HI_U32 u32Cnt, HI_S32 s32VcNum)
{
    int i, j;
    VI_FRAME_INFO_S stFrame;
    VI_FRAME_INFO_S astFrame[MAX_FRM_CNT];
    HI_CHAR szYuvName[128] = {0};
    FILE* pfd = NULL;
    HI_S32 s32MilliSec = 2000;
    VI_CHN ViChn;
    HI_U32 u32CapCnt = 0;

    VIU_GET_RAW_CHN(ViDev, ViChn);

	/*dump from des1*/
	ViChn = ViChn+1;

    if (HI_MPI_VI_SetFrameDepth(ViChn, 1))
    {
        printf("HI_MPI_VI_SetFrameDepth err, vi chn %d \n", ViChn);
        return -1;
    }

    usleep(5000);
	if (HI_TRUE == bQuit)
	{
		return -1;
	}

    if (HI_MPI_VI_GetFrame(ViChn, &stFrame.stViFrmInfo, s32MilliSec))
    {
        printf("HI_MPI_VI_GetFrame err, vi chn %d \n", ViChn);
        return -1;
    }

    /* get VI frame  */
    for (i = 0; i < u32Cnt; i++)
    {
	   astFrame[i].stViFrmInfo.stVFrame.u32PhyAddr[0] = 0;
    }
    for (i = 0; i < u32Cnt; i++)
    {
	    if (HI_TRUE == bQuit)
		{
			break;
		}
		
        if (HI_MPI_VI_GetFrame(ViChn, &astFrame[i].stViFrmInfo, s32MilliSec) < 0)
        {
            printf("get vi chn %d frame err\n", ViChn);
            printf("only get %d frame\n", i);
            break;
        }

		
		if (bDumpAllFrame)
		{
			if (i==0 && astFrame[i].stViFrmInfo.stVFrame.u32PrivateData != 0)
			{
				printf("note: the first frame vc must be 0 when the scene is frame WDR mode.\n");
				HI_MPI_VI_ReleaseFrame(ViChn, &astFrame[i].stViFrmInfo);
				i--;
			}
		}		
    }
    u32CapCnt = i;
    if ((0 == u32CapCnt)||(HI_TRUE == bQuit))
    {
        return -1;
    }

    /* make file name */
    if (VI_DUMP_TYPE_RAW == pstViDumpAttr->enDumpType)
    {
    	
        if ((-1 == s32VcNum)||(HI_TRUE == bDumpAllFrame))
        {
            sprintf(szYuvName, "./vi_dev_%d_%d_%d_%d_%dbits.raw", ViDev,
                    stFrame.stViFrmInfo.stVFrame.u32Width, stFrame.stViFrmInfo.stVFrame.u32Height,
                    u32CapCnt, u32Nbit);
        }
        else
        {
            sprintf(szYuvName, "./vi_dev_%d_%d_%d_%d_%dbits_vc%d.raw", ViDev,
                    stFrame.stViFrmInfo.stVFrame.u32Width, stFrame.stViFrmInfo.stVFrame.u32Height,
                    u32CapCnt, u32Nbit, s32VcNum);
        }

        printf("Dump raw frame of vi chn %d  to file: \"%s\"\n", ViChn, szYuvName);
    }
    else if (VI_DUMP_TYPE_IR == pstViDumpAttr->enDumpType)
    {
        u32Nbit = 16;
        sprintf(szYuvName, "./vi_dev_%d_%d_%d_%d_%dbits.ir", ViDev,
                stFrame.stViFrmInfo.stVFrame.u32Width, stFrame.stViFrmInfo.stVFrame.u32Height,
                u32CapCnt, u32Nbit);
        printf("Dump ir frame of vi chn %d  to file: \"%s\"\n", ViChn, szYuvName);
    }
    else
    {
        printf("Invalid dump type %d\n", pstViDumpAttr->enDumpType);
    }
    HI_MPI_VI_ReleaseFrame(ViChn, &stFrame.stViFrmInfo);

    /* open file */
    pfd = fopen(szYuvName, "wb");
    if (NULL == pfd)
    {
        printf("open file failed:%s!\n", strerror(errno));
        for (j = 0; j < u32CapCnt; j++)
        {
            HI_MPI_VI_ReleaseFrame(ViChn, &astFrame[j].stViFrmInfo);
			astFrame[j].stViFrmInfo.stVFrame.u32PhyAddr[0] = 0;
        }
        return -1;
    }

    for (j = 0; j < u32CapCnt; j++)
    {
        /* save VI frame to file */
		if (HI_TRUE == bQuit)
		{
			goto END1;
		}	
		if (HI_TRUE != bDumpAllFrame)
		{		
			FRAME_SUPPLEMENT_INFO_S *pstSupplmentInfo = NULL;
			HI_U32 phy_addr = 0;
			phy_addr = astFrame[j].stViFrmInfo.stVFrame.stSupplement.u32FrameSupplementPhyAddr;
			pstSupplmentInfo = (FRAME_SUPPLEMENT_INFO_S*) HI_MPI_SYS_Mmap(phy_addr, sizeof(FRAME_SUPPLEMENT_INFO_S));
			if(HI_NULL != pstSupplmentInfo)
			{
				pstSupplmentInfo->u32VcNum = s32VcNum;
			}
			HI_MPI_SYS_Munmap(pstSupplmentInfo, sizeof(FRAME_SUPPLEMENT_INFO_S));
		}
        sample_bayer_dump(&astFrame[j].stViFrmInfo.stVFrame, u32Nbit, pfd);

        /* release frame after using */
        HI_MPI_VI_ReleaseFrame(ViChn, &astFrame[j].stViFrmInfo);		
		astFrame[j].stViFrmInfo.stVFrame.u32PhyAddr[0] = 0;
    }

    fclose(pfd);

	return 0;
	END1:
	for (j = 0; j < u32CapCnt; j++)
    {
       if(0 != astFrame[j].stViFrmInfo.stVFrame.u32PhyAddr[0])
		{
			HI_MPI_VI_ReleaseFrame(ViChn, &astFrame[j].stViFrmInfo);
			astFrame[j].stViFrmInfo.stVFrame.u32PhyAddr[0] = 0;
		}
    }	
	fclose(pfd);
    return 0;
}

HI_S32 VI_SYNC_DumpBayer(HI_U32 u32Nbit)
{
    int i, j;
    VI_FRAME_INFO_S stFrame[2];
    VI_FRAME_INFO_S astFrame[2][MAX_FRM_CNT];
    HI_CHAR szYuvName[2][128] = {{0}};
    FILE* pfd[2] = {NULL};
    HI_S32 s32MilliSec = 2000;
    VI_CHN ViChn[2];
    VI_DEV ViDev;
    HI_U32 oldDepth[2];
    HI_U32 VI_MAX_DEV_NUM = 2;
    HI_S32 s32FrmCnt, s32Cnt = 3, s32MidViChn1 = -1;
    HI_S32 s32MidViChn0 = s32Cnt/2;
    HI_U64 u64PtsTap[20], u64temp;
	HI_S32 s32Ret;

    for(i = 0;i<VI_MAX_DEV_NUM;i++)
    {
        ViDev = i;
        VIU_GET_RAW_CHN(ViDev, ViChn[i]);
		/*dump from des1 */
		ViChn[i] =  ViChn[i]+1;

        s32Ret = HI_MPI_VI_GetFrameDepth(ViChn[i], &oldDepth[i]);
		if (HI_SUCCESS != s32Ret)
		{
			printf("HI_MPI_VI_GetFrameDepth err, vi chn %d \n", ViChn[i]);
			return -1;
		}
        
        if (HI_MPI_VI_SetFrameDepth(ViChn[i], s32Cnt))
        {
            printf("HI_MPI_VI_SetFrameDepth err, vi chn %d \n", ViChn[i]);
            return -1;
        }
    }

    usleep(5000);
    
    for(i = 0;i<VI_MAX_DEV_NUM;i++)
    {
        if (HI_MPI_VI_GetFrame(ViChn[i], &stFrame[i].stViFrmInfo, s32MilliSec))
        {
            printf("HI_MPI_VI_GetFrame err, vi chn %d \n", ViChn[i]);
            return -1;
        }  
    }

    /* get VI frame  */
    
    for(ViDev = 0;ViDev < VI_MAX_DEV_NUM;ViDev++)
    {
        for (i = 0; i < s32Cnt; i++)
        {
            if (HI_MPI_VI_GetFrame(ViChn[ViDev], &astFrame[ViDev][i].stViFrmInfo, s32MilliSec) < 0)
            {
                printf("get vi chn %d frame err\n", ViChn[ViDev]);
                printf("only get %d frame\n", i);
                break;
            }
        }
    }  

    /* make file name */
    
    for(ViDev = 0;ViDev < VI_MAX_DEV_NUM;ViDev++)
    {       
        sprintf(szYuvName[ViDev], "./vi_dev_%d_%d_%d_%dbits.raw", ViDev,
                stFrame[ViDev].stViFrmInfo.stVFrame.u32Width, stFrame[ViDev].stViFrmInfo.stVFrame.u32Height, u32Nbit);      

        printf("Dump raw frame of vi chn %d  to file: \"%s\"\n", ViChn[ViDev], szYuvName[ViDev]);
    }

   
    for(i = 0;i<VI_MAX_DEV_NUM;i++)
	{
		HI_MPI_VI_ReleaseFrame(ViChn[i], &stFrame[i].stViFrmInfo);
	}

    /* open file */
    
    for(ViDev = 0;ViDev < VI_MAX_DEV_NUM; ViDev++)
    {
        pfd[ViDev] = fopen(szYuvName[ViDev], "wb");
    }
    
    if (NULL == pfd[0]||NULL == pfd[1])
    {
        printf("open file failed:%s!\n", strerror(errno));
      
        for(ViDev = 0;ViDev < VI_MAX_DEV_NUM;ViDev++) 
        {
            for (j = 0; j < s32Cnt; j++)
            {
                HI_MPI_VI_ReleaseFrame(ViChn[ViDev], &astFrame[ViDev][j].stViFrmInfo);
            }
        }
        return -1;
    }    

	
	for (s32FrmCnt= 0; s32FrmCnt < s32Cnt; s32FrmCnt++)
	{
		if(s32FrmCnt != s32MidViChn0)
		{	
			HI_MPI_VI_ReleaseFrame(ViChn[0], &astFrame[0][s32FrmCnt].stViFrmInfo);	
		}

		if(astFrame[0][s32MidViChn0].stViFrmInfo.stVFrame.u64pts >= astFrame[1][s32FrmCnt].stViFrmInfo.stVFrame.u64pts)
		{
			u64PtsTap[s32FrmCnt] = astFrame[0][s32MidViChn0].stViFrmInfo.stVFrame.u64pts - astFrame[1][s32FrmCnt].stViFrmInfo.stVFrame.u64pts;		
		}
		else
		{
			u64PtsTap[s32FrmCnt] = astFrame[1][s32FrmCnt].stViFrmInfo.stVFrame.u64pts- astFrame[0][s32MidViChn0].stViFrmInfo.stVFrame.u64pts;	
		}
	}

    s32FrmCnt= 0;

	u64temp = u64PtsTap[s32FrmCnt];
	
	for (s32FrmCnt= 0; s32FrmCnt < s32Cnt; s32FrmCnt++)
	{		
		if(u64temp >= u64PtsTap[s32FrmCnt])
		{
			u64temp = u64PtsTap[s32FrmCnt];
			s32MidViChn1 = s32FrmCnt;	
		}

        printf("\nvichn1 frame num  = %d, u64PtsTap = %llu\n",
            s32FrmCnt, u64PtsTap[s32FrmCnt]);
	}


    if(s32MidViChn1 != -1)
    {
        /* save VI frame to file */
         printf("\nsave videv0 frame num = %d\n",s32MidViChn0);
        sample_bayer_dump(&astFrame[0][s32MidViChn0].stViFrmInfo.stVFrame, u32Nbit, pfd[0]);

          /* release frame after using */
        HI_MPI_VI_ReleaseFrame(ViChn[0], &astFrame[0][s32MidViChn0].stViFrmInfo);

        printf("\nsave videv1 frame num = %d\n",s32MidViChn1);
        /* save VI frame to file */
        sample_bayer_dump(&astFrame[1][s32MidViChn1].stViFrmInfo.stVFrame, u32Nbit, pfd[1]);
    }
    else
    {          /* release frame after using */
        HI_MPI_VI_ReleaseFrame(ViChn[0], &astFrame[0][s32MidViChn0].stViFrmInfo);
    }

    for (j = 0; j < s32Cnt; j++)
    {       
        /* release frame after using */
        HI_MPI_VI_ReleaseFrame(ViChn[1], &astFrame[1][j].stViFrmInfo);
    }

    for(i = 0;i<VI_MAX_DEV_NUM;i++)
    {       
        if (HI_MPI_VI_SetFrameDepth(ViChn[i], oldDepth[i]))
        {
            printf("HI_MPI_VI_SetFrameDepth err, vi chn %d \n", ViChn[i]);
            return -1;
        }
        fclose(pfd[i]);
    }

    return 0;
}

HI_S32 get_raw_pts(VI_FRAME_INFO_S **pastFrame, HI_U64 u64RawPts, HI_U32 u32RowCnt, 
						 HI_U32 u32ColCnt, HI_U32 u32Col,  HI_U32* pu32Index)
{
    HI_S32 i;

    for (i = 0; i < u32RowCnt; i++)
    {		
        if (u64RawPts == pastFrame[i][u32Col].stViFrmInfo.stVFrame.stSupplement.u64RawPts)
        {
			*pu32Index = i;
			return 0;
        }
    }

    return -1;
}

HI_S32 GetWdrVcMaxNum(WDR_MODE_E enWDRMode)
{
	HI_S32 VcMaxNum = 0;
	if((WDR_MODE_4To1_LINE == enWDRMode)
		||(WDR_MODE_4To1_FRAME == enWDRMode)
		||(WDR_MODE_4To1_FRAME_FULL_RATE == enWDRMode))
	{
		VcMaxNum = 3;
	}
	else if((WDR_MODE_3To1_LINE == enWDRMode)
			||(WDR_MODE_3To1_FRAME == enWDRMode)
			||(WDR_MODE_3To1_FRAME_FULL_RATE == enWDRMode))
	{
		VcMaxNum = 2;
	}
	else if((WDR_MODE_2To1_LINE == enWDRMode)
			||(WDR_MODE_2To1_FRAME == enWDRMode)
			||(WDR_MODE_2To1_FRAME_FULL_RATE == enWDRMode))
	{
		VcMaxNum = 1;
	}
	else
	{
		VcMaxNum = 0;
	}
	return VcMaxNum;
}

HI_S32 VI_DumpBayer_Line_WDR(VI_DEV ViDev, HI_U32 u32DataNum, HI_U32 u32Nbit, HI_U32 u32Cnt)
{
    HI_S32 j,ViChn = 0;
    VI_FRAME_INFO_S stFrame[3];
    VI_FRAME_INFO_S *pstSaveFrame = {0};
    VI_FRAME_INFO_S *pastFrame[MAX_FRM_CNT] =  {0};
    HI_CHAR szYuvName[128] = {0};
    FILE* pfd = NULL; 
    HI_S32 s32MilliSec = 2000;
    VI_CHN RawStarChn;
    HI_S32 s32CapCnt = 0;
    HI_S32 s32SaveCnt = 0;
    HI_U32 u32Width, u32Height;
    HI_S32 i,k; 
    HI_U32 u32SaveNum;
	
    VIU_GET_RAW_CHN(ViDev, RawStarChn); 

    printf("u32DataNum: %d,u32Cnt: %d\n", u32DataNum,u32Cnt);
    
    for (i = 0; i < u32DataNum; i++)
    {
        if (HI_MPI_VI_SetFrameDepth(RawStarChn+i, 1))
        {
            printf("HI_MPI_VI_SetFrameDepth err, vi chn %d \n", RawStarChn+i);
            return HI_FAILURE;
        }
    }

    pstSaveFrame = (VI_FRAME_INFO_S*)malloc((u32Cnt/u32DataNum) * u32DataNum * sizeof(VI_FRAME_INFO_S));

    for (i = 0; i < u32Cnt/u32DataNum; i++)
    {
        pastFrame[i] = (VI_FRAME_INFO_S*)malloc(u32DataNum * sizeof(VI_FRAME_INFO_S));
        if (HI_NULL == pastFrame[i])
        {
            printf("malloc %d fail\n", (u32DataNum * sizeof(VI_FRAME_INFO_S)));
            for (i = i -1; i >= 0; i--)
            {
                free(pastFrame[i]);
            }
            return HI_FAILURE;
		}
    }
    
    usleep(5000);
    for (i = 0; i < u32DataNum; i++)
    {       
		if (HI_TRUE == bQuit)
		{
			break;
		}
		if (HI_MPI_VI_GetFrame(RawStarChn+i, &stFrame[i].stViFrmInfo, s32MilliSec))
		{
			printf("HI_MPI_VI_GetFrame err, vi chn %d \n", RawStarChn+i);
			break;
		}          
		HI_MPI_VI_ReleaseFrame(RawStarChn+i, &stFrame[i].stViFrmInfo);
           
    }
	
    u32Width  = stFrame[0].stViFrmInfo.stVFrame.u32Width;
    u32Height = stFrame[0].stViFrmInfo.stVFrame.u32Height;
        
    /* get VI frame  */
    for (i = 0; i < (u32Cnt/u32DataNum); i++)
    {
        if (HI_TRUE == bQuit)
        {
            break;
        }
        
        for (j = 0; j < u32DataNum; j++)
        {
            if (HI_MPI_VI_GetFrame(RawStarChn+j, &pastFrame[i][j].stViFrmInfo, s32MilliSec) < 0)
            {
                printf("get vi chn %d frame err\n", RawStarChn+j);
                printf("only get %d frame\n", j);
                break;
            }
        }
    }
    
    s32CapCnt = i;
    if ((HI_TRUE == bQuit) || (0 >= s32CapCnt))
    {
        goto exit;
    }	

    for (i = 0; i < u32Cnt/u32DataNum; i++)
    {
        HI_U64 u64RawPts = pastFrame[i][0].stViFrmInfo.stVFrame.stSupplement.u64RawPts;
        HI_U32 index[3];
        
        for (j = 1; j < u32DataNum; j++)
        {
            if (get_raw_pts(pastFrame, u64RawPts, u32Cnt/u32DataNum, u32DataNum, j, &index[j]))
            {
                break;
            }
        }

        if (j == u32DataNum)
        {           
            for (k = 0, j = u32DataNum-1; j >= 1; j--, k++)
            {
                memcpy(&pstSaveFrame[s32SaveCnt+k], &pastFrame[index[j]][j], sizeof(VI_FRAME_INFO_S));
            }

			s32SaveCnt += u32DataNum;
			
			memcpy(&pstSaveFrame[s32SaveCnt-1], &pastFrame[i][0], sizeof(VI_FRAME_INFO_S));			
            
        }
        
    }

	  printf("s32SaveCnt: %d,u32Cnt = %d\n", s32SaveCnt,u32Cnt);

    /* make file name */
    snprintf(szYuvName, 128, "./vi_dev_%d_%d_%d_%d_%dbits_lineWDR.raw", ViDev, u32Width, u32Height, 
                ((s32SaveCnt > u32Cnt)?u32Cnt:s32SaveCnt), u32Nbit);  

    printf("Dump raw frame of vi chn %d to file: \"%s\"\n", ViChn, szYuvName);

    /* open file */
    pfd = fopen(szYuvName, "wb");
    if (NULL == pfd)
    {
        printf("open file failed:%s!\n", strerror(errno));
        goto exit;
    }  

    u32SaveNum = ((s32SaveCnt > u32Cnt)?u32Cnt:s32SaveCnt);
    for (j = 0; j < u32SaveNum; j++)
    {
        if (HI_TRUE == bQuit)
        {
            break;
        }
         printf("save : %d.\n", j);
        /* save VI frame to file */
        sample_bayer_dump(&pstSaveFrame[j].stViFrmInfo.stVFrame, u32Nbit, pfd);
    }

exit:
    for (i = 0; i < u32Cnt/u32DataNum; i++)
    {
		for (j = 0; j < u32DataNum; j++)
		/* release frame after using */
		{
			if(HI_MPI_VI_ReleaseFrame(RawStarChn+j, &pastFrame[i][j].stViFrmInfo))
			{
				printf("HI_MPI_VI_ReleaseFrame rawchn: %d error.\n", RawStarChn+j);
			}
		}
    }
    
    if (NULL != pfd)
    {
        fclose(pfd);
    }   
    
    for (i = 0; i < u32Cnt/u32DataNum; i++)
    {
        if (HI_NULL != pastFrame[i])
        {
            free(pastFrame[i]);
            pastFrame[i] =  HI_NULL;
        }
    }

    if (HI_NULL != pstSaveFrame)
    {
        free(pstSaveFrame);
        pstSaveFrame =  HI_NULL;
    }
    
    return 0;
}

#ifdef __HuaweiLite__
HI_S32 vi_bayerdump(int argc, char* argv[])
#else
HI_S32 main(int argc, char* argv[])
#endif
{
    VI_DEV ViDev = 0;
	HI_S32 s32VcNumMax = 0;
    HI_S32 s32Ret = 0;
    HI_U32 u32Nbit = 16;
    HI_U32 u32FrmCnt = 4;
	HI_S32 s32VcNum = -1;
    HI_CHAR au8Type[20] = "-r";
    VI_DUMP_ATTR_S stViDumpAttr = {0}, stDumpBkAttr[2];
	HI_U32 u32DesIdAddr = 0x11380310,u32BasIdAddr;
	HI_U32 u32BackupBasValue = 0x1;
    HI_U32 s32Dev  = 0;
	HI_S32 s32DumpSel = 0;
	VI_DUMP_ATTR_S stDumpAttr;
	PIXEL_FORMAT_E stPixelFrt;
	VI_WDR_ATTR_S stWDRAttr;
	HI_U32 u32DataNum;

    if (argc > 1)
    {
        if (!strncmp(argv[1], "-h", 2))
        {
            usage();
            exit(HI_SUCCESS);
        }
        else
        {
            strcpy(au8Type, argv[1]);  /*bayer dump mode :-r: single, -s : dump synchronical two frame raw data(dev0,dev1)*/
        }                          
    }
    
    printf("\nNOTICE: This tool only can be used for TESTING , argc = %d!!!\n", argc);

    if ((!strcmp("-r", au8Type) || !strcmp("-R", au8Type))&& argc < 3)
    {
      usage();
      return s32Ret;
    }

	if(!strcmp("-r", au8Type) || !strcmp("-R", au8Type))
	{
		if (argc > 2)
		{       
			ViDev = atoi(argv[2]);       /* videv:0,1 */
		}
		if (argc > 3)
		{
			u32Nbit = atoi(argv[3]);    /* nbit of Raw data:8bit;10bit;12bit,14bit,16bit */
		}
		if (argc > 4)
		{
			u32FrmCnt = atoi(argv[4]);  /* the frame number */
		}
		if (argc > 5)
		{
			s32DumpSel = atoi(argv[5]);   /* port or bas */
		}
		if (argc > 6)
		{
			s32VcNum = atoi(argv[6]);  	/* long or short frame */
		}
		if ((8 > u32Nbit || 16 < u32Nbit) || (u32Nbit % 2))
		{
			printf("can't not support %d bits, only support 8bits,10bits,12bits,14bits,16bits\n", u32Nbit);
			exit(HI_FAILURE);
		}
		if (1 > u32FrmCnt||u32FrmCnt > MAX_FRM_CNT)
		{
			printf("invalid FrmCnt %d,only dump frame count:[1,%d]\n", u32FrmCnt, MAX_FRM_CNT);
			exit(HI_FAILURE);
		}
		if ((1 != s32DumpSel)&& (0 != s32DumpSel))
		{
			printf("invalid dumpsel %d\n", s32DumpSel);
			exit(HI_FAILURE);
		}
		if ((0 != ViDev)&&(1 != ViDev))
		{
			printf("invalid ViDev %d\n", ViDev);
			exit(HI_FAILURE);
		}

		if (argc > 6)
		{

			if(0 == ViDev)
			{
				if ((0 > s32VcNum)||(3 < s32VcNum))
				{
					printf("invalid s32VcNum %d\n", s32VcNum);
					exit(HI_FAILURE);
				}
			}
			else
			{
				if ((0 > s32VcNum)||(1 < s32VcNum))
				{
					printf("invalid s32VcNum %d\n", s32VcNum);
					exit(HI_FAILURE);
				}
			}
		}

		if(0 == ViDev)
		{
			u32DesIdAddr = 0x11380310;	
			u32BasIdAddr = 0x11382010;	
		}
		else 
		{	
			u32DesIdAddr = 0x11480310;
			u32BasIdAddr = 0x11482010;	
		}	    
		
		s32Ret = HI_MPI_VI_GetDevDumpAttr(ViDev, &stDumpBkAttr[ViDev]);
		if (HI_SUCCESS != s32Ret)
		{
			printf("HI_MPI_VI_GetDevDumpAttr failed(0x%x)\n", s32Ret);
			return s32Ret;
		}
		stDumpAttr.enDumpType = VI_DUMP_TYPE_RAW;
		bitWidth2PixelFormat(u32Nbit, &stPixelFrt);
		stDumpAttr.enPixelFormat = stPixelFrt;
		stDumpAttr.stCropInfo.bEnable = HI_FALSE;
		if(0 == ViDev)
		{
			stDumpAttr.enDumpSel = VI_DUMP_SEL_PORT;
		}
		else
		{
			if(0 == s32DumpSel)
			{		
				stDumpAttr.enDumpSel = VI_DUMP_SEL_PORT;
			}
			else
			{
				stDumpAttr.enDumpSel = VI_DUMP_SEL_BAS;
			}

			s32Ret = HI_MPI_SYS_GetReg(u32BasIdAddr, &u32BackupBasValue);
			if (HI_SUCCESS != s32Ret)
			{
				printf("HI_MPI_SYS_GetReg failed, s32Ret = %d!\n", s32Ret);
				return s32Ret;
			}			
		}	
		s32Ret =  HI_MPI_VI_SetDevDumpAttr(ViDev, &stDumpAttr);
		if (HI_SUCCESS != s32Ret)
		{
			printf("ViDev%d, HI_MPI_VI_SetDevDumpAttr failed(0x%x)!\n", ViDev, s32Ret);
			return s32Ret;
		}
	}
	else if(!strcmp("-s", au8Type) || !strcmp("-S", au8Type))
	{
		if (argc > 2)
		{       
			u32Nbit = atoi(argv[2]);       /* videv:0,1 */
		}		

		if ((8 > u32Nbit || 16 < u32Nbit) || (u32Nbit % 2))
		{
			printf("can't not support %d bits, only support 8bits,10bits,12bits,14bits,16bits\n", u32Nbit);
			exit(HI_FAILURE);
		}		
	}
	else
	{
		usage();
	}

	bDumpAllFrame = HI_FALSE;

	s32Ret = HI_MPI_VI_GetWDRAttr(ViDev, &stWDRAttr);
	if (HI_SUCCESS != s32Ret)   
	{      
		printf("HI_MPI_VI_GetWDRAttr failed(0x%x)\n", s32Ret);       
		return s32Ret;   
	}

	if (stWDRAttr.enWDRMode == WDR_MODE_2To1_LINE)  
	{  
		u32DataNum = 2;   
	}   
	else if (stWDRAttr.enWDRMode == WDR_MODE_3To1_LINE)   
	{   
		u32DataNum = 3;  
	}    
	else  
	{   
		u32DataNum = 1;  
	}
	s32VcNumMax = GetWdrVcMaxNum(stWDRAttr.enWDRMode);
	if ((argc > 6)&&(s32VcNum > s32VcNumMax))
	{		
		printf("invalid s32VcNum %d\n", s32VcNum);
		exit(HI_FAILURE);	
	}

    if (!strcmp("-s", au8Type) || !strcmp("-S", au8Type))
    {
		stDumpAttr.enDumpType = VI_DUMP_TYPE_RAW;
		bitWidth2PixelFormat(u32Nbit, &stPixelFrt);
		stDumpAttr.enPixelFormat = stPixelFrt;
		stDumpAttr.stCropInfo.bEnable = HI_FALSE;
		stDumpAttr.enDumpSel = VI_DUMP_SEL_PORT;
		for(s32Dev = 0 ;s32Dev < 2; s32Dev++)
		{	
			ISP_DEV IspDev = s32Dev;
			ISP_FMW_STATE_E enState= ISP_FMW_STATE_FREEZE;
			s32Ret = HI_MPI_ISP_SetFMWState(IspDev, enState);
			if (HI_SUCCESS != s32Ret)
			{
				printf("HI_MPI_ISP_SetFMWState failed(0x%x)\n", s32Ret);
				return s32Ret;
			}
			s32Ret = HI_MPI_VI_GetDevDumpAttr(s32Dev, &stDumpBkAttr[s32Dev]);
			if (HI_SUCCESS != s32Ret)
			{
				printf("HI_MPI_VI_GetDevDumpAttr failed(0x%x)\n", s32Ret);
				return s32Ret;
			}
			s32Ret =  HI_MPI_VI_SetDevDumpAttr(s32Dev, &stDumpAttr);
			if (HI_SUCCESS != s32Ret)
			{
				printf("ViDev%d, HI_MPI_VI_SetDevDumpAttr failed(0x%x)!\n", s32Dev, s32Ret);
				return s32Ret;
			}
		}
        for(s32Dev = 0 ;s32Dev<2; s32Dev++)
        {
            s32Ret = HI_MPI_VI_EnableBayerDump(s32Dev);         
            if (HI_SUCCESS != s32Ret)
            {
                printf("HI_MPI_VI_EnableBayerDump failed(0x%x)!\n", s32Ret);
                return s32Ret;
            }
        }
    }
    else
    {
		ISP_DEV IspDev = ViDev;
		ISP_FMW_STATE_E enState= ISP_FMW_STATE_FREEZE;
		s32Ret = HI_MPI_ISP_SetFMWState(IspDev, enState);
		if (HI_SUCCESS != s32Ret)
		{
			printf("HI_MPI_ISP_SetFMWState failed(0x%x)\n", s32Ret);
			return s32Ret;
		}
        s32Ret = HI_MPI_VI_EnableBayerDump(ViDev);
    }    
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VI_EnableBayerDump failed(0x%x)!\n", s32Ret);
        return s32Ret;
    }

	if(argc >= 7)
	{		
		if (0 == s32VcNum)
		{
		    s32Ret = HI_MPI_SYS_SetReg(u32DesIdAddr, 0x1);
		    if (HI_SUCCESS != s32Ret)
		    {
		        printf("HI_MPI_SYS_SetReg failed, s32Ret = 0x%x!\n", s32Ret);
		        return s32Ret;
		    }
			if((1 == ViDev)&&(1 == s32DumpSel))
			{
				s32Ret = HI_MPI_SYS_SetReg(u32BasIdAddr, 0x1);
				if (HI_SUCCESS != s32Ret)
				{
					printf("HI_MPI_SYS_SetReg failed, s32Ret = 0x%x!\n", s32Ret);
					return s32Ret;
				}
			}
		}
		else if (1 == s32VcNum)
		{
		    s32Ret = HI_MPI_SYS_SetReg(u32DesIdAddr, 0x2);
		    if (HI_SUCCESS != s32Ret)
		    {
		        printf("HI_MPI_SYS_SetReg failed, s32Ret = 0x%x!\n", s32Ret);
		        return s32Ret;
		    }
			if((1 == ViDev)&&(1 == s32DumpSel))
			{
				s32Ret = HI_MPI_SYS_SetReg(u32BasIdAddr, 0x2);
				if (HI_SUCCESS != s32Ret)
				{
					printf("HI_MPI_SYS_SetReg failed, s32Ret = 0x%x!\n", s32Ret);
					return s32Ret;
				}
			}
		}
		else if (2 == s32VcNum)
		{
		    s32Ret = HI_MPI_SYS_SetReg(u32DesIdAddr, 0x4);
		    if (HI_SUCCESS != s32Ret)
		    {
		        printf("HI_MPI_SYS_SetReg failed, s32Ret = 0x%x!\n", s32Ret);
		        return s32Ret;
		    }
			if((1 == ViDev)&&(1 == s32DumpSel))
			{
				s32Ret = HI_MPI_SYS_SetReg(u32BasIdAddr, 0x4);
				if (HI_SUCCESS != s32Ret)
				{
					printf("HI_MPI_SYS_SetReg failed, s32Ret = 0x%x!\n", s32Ret);
					return s32Ret;
				}
			}
		}
		else if (3 == s32VcNum)
		{
		    s32Ret = HI_MPI_SYS_SetReg(u32DesIdAddr, 0x8);
		    if (HI_SUCCESS != s32Ret)
		    {
		        printf("HI_MPI_SYS_SetReg failed, s32Ret = 0x%x!\n", s32Ret);
		        return s32Ret;
		    }
			
			if((1 == ViDev)&&(1 == s32DumpSel))
			{
				s32Ret = HI_MPI_SYS_SetReg(u32BasIdAddr, 0x8);
				if (HI_SUCCESS != s32Ret)
				{
					printf("HI_MPI_SYS_SetReg failed, s32Ret = 0x%x!\n", s32Ret);
					return s32Ret;
				}
			}		
		}
		else
		{
		    printf("Invalid VC num!\n");
		    goto EXIT;
		}
	}
	else
	{
		/* dump all vc data */
		if (stWDRAttr.enWDRMode == WDR_MODE_2To1_FRAME || stWDRAttr.enWDRMode == WDR_MODE_2To1_FRAME_FULL_RATE)      
		{           
			s32Ret = HI_MPI_SYS_SetReg(u32DesIdAddr, 0x3);
			if((1 == ViDev)&&(1 == s32DumpSel))
			{
				s32Ret = HI_MPI_SYS_SetReg(u32BasIdAddr, 0x3);
				if (HI_SUCCESS != s32Ret)
				{
					printf("HI_MPI_SYS_SetReg failed, s32Ret = 0x%x!\n", s32Ret);
					return s32Ret;
				}
			}
			bDumpAllFrame = HI_TRUE;
		}	
		else if (stWDRAttr.enWDRMode == WDR_MODE_3To1_FRAME || stWDRAttr.enWDRMode == WDR_MODE_3To1_FRAME_FULL_RATE)	
		{			
			s32Ret = HI_MPI_SYS_SetReg(u32DesIdAddr, 0x7);
			if((1 == ViDev)&&(1 == s32DumpSel))
			{
				s32Ret = HI_MPI_SYS_SetReg(u32BasIdAddr, 0x7);
				if (HI_SUCCESS != s32Ret)
				{
					printf("HI_MPI_SYS_SetReg failed, s32Ret = 0x%x!\n", s32Ret);
					return s32Ret;
				}
			}
			bDumpAllFrame = HI_TRUE;
		}	
		else if (stWDRAttr.enWDRMode == WDR_MODE_4To1_FRAME || stWDRAttr.enWDRMode == WDR_MODE_4To1_FRAME_FULL_RATE)	
		{			
			s32Ret = HI_MPI_SYS_SetReg(u32DesIdAddr, 0xf);
			if((1 == ViDev)&&(1 == s32DumpSel))
			{
				s32Ret = HI_MPI_SYS_SetReg(u32BasIdAddr, 0xf);
				if (HI_SUCCESS != s32Ret)
				{
					printf("HI_MPI_SYS_SetReg failed, s32Ret = 0x%x!\n", s32Ret);
					return s32Ret;
				}
			}
			bDumpAllFrame = HI_TRUE;
		}   
		else if(stWDRAttr.enWDRMode == WDR_MODE_NONE)     
		{            
			s32Ret = HI_MPI_SYS_SetReg(u32DesIdAddr, 0x1);    
		}	

	}

    signal(SIGINT, SigHandler);
    signal(SIGTERM, SigHandler);
    printf("===press any key to continue.\n");
    getchar();

	if (!strcmp("-s", au8Type) || !strcmp("-S", au8Type))
	{
		s32Ret = VI_SYNC_DumpBayer(u32Nbit);
        if (HI_SUCCESS != s32Ret)
        {
            printf("VI_SYNC_DumpBayer failed!\n");
            goto EXIT;
        }
	}
	else if ((u32DataNum > 1) && (argc <= 6))
	{
		s32Ret = VI_DumpBayer_Line_WDR(ViDev, u32DataNum, u32Nbit, u32FrmCnt);
		if (HI_SUCCESS != s32Ret)
		{
			printf("VI_DumpBayer_Line_WDR failed!\n");
			goto EXIT;
		}
	}
    else
    {
        s32Ret = VI_DumpBayer(ViDev, &stViDumpAttr, u32Nbit, u32FrmCnt, s32VcNum);
        if (HI_SUCCESS != s32Ret)
        {
            printf("VI_DumpBayer failed!\n");
            goto EXIT;
        }
    }   

EXIT:

    if (!strcmp("-s", au8Type) || !strcmp("-S", au8Type))
    {
        for(s32Dev = 0 ;s32Dev < 2; s32Dev++)
        {
			ISP_DEV IspDev = s32Dev;
			ISP_FMW_STATE_E enState= ISP_FMW_STATE_RUN;
            s32Ret = HI_MPI_VI_DisableBayerDump(s32Dev);		
            if (HI_SUCCESS != s32Ret)
            {
                printf("HI_MPI_ISP_SetFMWState failed(0x%x)!\n", s32Ret);
                return s32Ret;
            }
			s32Ret = HI_MPI_VI_SetDevDumpAttr(s32Dev, &stDumpBkAttr[s32Dev]);
			if (HI_SUCCESS != s32Ret)
            {
                printf("HI_MPI_VI_SetDevDumpAttr failed(0x%x)!\n", s32Ret);
                return s32Ret;
            }			
			s32Ret = HI_MPI_ISP_SetFMWState(IspDev, enState);
			if (HI_SUCCESS != s32Ret)
			{
				printf("HI_MPI_ISP_SetFMWState failed(0x%x)\n", s32Ret);
				return s32Ret;
			}
        }
    }
    else
    {	
		ISP_DEV IspDev = ViDev;
		ISP_FMW_STATE_E enState= ISP_FMW_STATE_RUN;
		s32Ret = HI_MPI_ISP_SetFMWState(IspDev, enState);
		if (HI_SUCCESS != s32Ret)
		{
			printf("HI_MPI_ISP_SetFMWState failed(0x%x)\n", s32Ret);
			return s32Ret;
		}
			if((1 == ViDev)&&(1 == s32DumpSel))
		{
	    	s32Ret = HI_MPI_SYS_SetReg(u32BasIdAddr, u32BackupBasValue);
		    if (HI_SUCCESS != s32Ret)
		    {
		        printf("HI_MPI_SYS_SetReg failed, s32Ret = 0x%x!\n", s32Ret);
		        return s32Ret;
		    }
		}
		s32Ret = HI_MPI_VI_DisableBayerDump(ViDev);
		if (HI_SUCCESS != s32Ret)
	    {
	        printf("HI_MPI_VI_DisableBayerDump failed!\n");
	        return s32Ret;
	    }    
		s32Ret = HI_MPI_VI_SetDevDumpAttr(ViDev, &stDumpBkAttr[ViDev]);
	    if (HI_SUCCESS != s32Ret)
	    {
			printf("HI_MPI_VI_SetDevDumpAttr failed(0x%x)!\n", s32Ret);
			return s32Ret;
	    }	   
    }     
	return HI_SUCCESS;
}
