#include <stdio.h>
#include <math.h>
//#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include "mpi_sys.h"
#include "mpi_vi.h"
#include "mpi_vpss.h"
#include "mpi_vb.h"
#include "mpi_vgs.h"
#include "hi_comm_vb.h"
#include "hi_comm_fisheye.h"

#include "fisheye_calibrate.h"

#define FISHEYE_CALIBRATE_LEVEL LEVEL_1
static volatile HI_BOOL bQuit = HI_FALSE;   /* bQuit may be set in the signal handler */

static void usage(void)
{
    printf(
        "\n"
        "*************************************************\n"
#ifndef __HuaweiLite__
        "Usage: ./fisheye_calibrate [ViPhyChn] [ViExtChn] .\n"
#else
        "Usage: fisheye_calibrate [ViPhyChn] [ViExtChn] .\n"
#endif
        "1)ViPhyChn: \n"
        "   the source physic channel bind the exetend channel\n"
        "2)ViExtChn: \n"
        "   the extend channel to execute fisheye correction\n"
        "*)Example:\n"
#ifndef __HuaweiLite__
        "   e.g : ./fisheye_calibrate 0 2\n"
#else
        "   e.g : fisheye_calibrate 0 2\n"
#endif
        "*************************************************\n"
        "\n");
}

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

HI_S32 FISHEYE_Calibrate_MapVirtAddr( VIDEO_FRAME_INFO_S * pstFrame)
{
	HI_S32 size = 0;
	HI_U32 phy_addr = 0;
	PIXEL_FORMAT_E enPixelFormat;
	HI_U32 u32YSize;
	HI_U32 u32UVInterval = 0;

	enPixelFormat = pstFrame->stVFrame.enPixelFormat;
	phy_addr = pstFrame->stVFrame.u32PhyAddr[0];

	if (PIXEL_FORMAT_YUV_SEMIPLANAR_420 == enPixelFormat)
	{
		size = (pstFrame->stVFrame.u32Stride[0]) * (pstFrame->stVFrame.u32Height) * 3 / 2;
		u32YSize = pstFrame->stVFrame.u32Stride[0]* pstFrame->stVFrame.u32Height;
		u32UVInterval = 0;
	}
	else if (PIXEL_FORMAT_YUV_PLANAR_420 == enPixelFormat)
	{
		
		size = (pstFrame->stVFrame.u32Stride[0]) * (pstFrame->stVFrame.u32Height) * 3 / 2;
		u32YSize = pstFrame->stVFrame.u32Stride[0] * pstFrame->stVFrame.u32Height;
		u32UVInterval = pstFrame->stVFrame.u32Width * pstFrame->stVFrame.u32Height / 2;
	}
	else
	{
		printf("not support pixelformat: %d\n", enPixelFormat);
		return HI_FAILURE;
	}


	pstFrame->stVFrame.pVirAddr[0] = (HI_CHAR*) HI_MPI_SYS_MmapCache(phy_addr, size);
	if (NULL == pstFrame->stVFrame.pVirAddr[0])
	{
		return HI_FAILURE;
	}

	pstFrame->stVFrame.pVirAddr[1] = pstFrame->stVFrame.pVirAddr[0] + u32YSize;
	pstFrame->stVFrame.pVirAddr[2] = pstFrame->stVFrame.pVirAddr[1] + u32UVInterval;

	return HI_SUCCESS;
}


HI_VOID FISHEYE_Calibrate_SaveSP42XToPlanar(FILE *pfile, VIDEO_FRAME_S *pVBuf)
{
	unsigned int w, h;
	char * pVBufVirt_Y;
	char * pVBufVirt_C;
	char * pMemContent;
	unsigned char *TmpBuff;
	PIXEL_FORMAT_E  enPixelFormat = pVBuf->enPixelFormat;
	HI_U32 u32UvHeight;
    HI_U32 u32LineSize = 2408;
	
	if (PIXEL_FORMAT_YUV_SEMIPLANAR_420 == enPixelFormat)
	{
	    u32UvHeight = pVBuf->u32Height/2;
	}
	else
	{
	    u32UvHeight = pVBuf->u32Height;
	}

	pVBufVirt_Y = pVBuf->pVirAddr[0]; 
	pVBufVirt_C = pVBufVirt_Y + (pVBuf->u32Stride[0])*(pVBuf->u32Height);

    TmpBuff = (unsigned char *)malloc(u32LineSize);
    if(NULL == TmpBuff)
    {
        printf("Func:%s line:%d -- unable alloc %dB memory for tmp buffer\n", 
            __FUNCTION__, __LINE__, u32LineSize);
        return;
    }

	/* save Y ----------------------------------------------------------------*/

	for(h=0; h<pVBuf->u32Height; h++)
	{
	    pMemContent = pVBufVirt_Y + h*pVBuf->u32Stride[0];
	    fwrite(pMemContent, 1,pVBuf->u32Width,pfile);
	}

	/* save U ----------------------------------------------------------------*/
	for(h=0; h<u32UvHeight; h++)
	{
	    pMemContent = pVBufVirt_C + h*pVBuf->u32Stride[1];

	    pMemContent += 1;

	    for(w=0; w<pVBuf->u32Width/2; w++)
	    {
	        TmpBuff[w] = *pMemContent;
	        pMemContent += 2;
	    }
	    fwrite(TmpBuff, 1,pVBuf->u32Width/2,pfile);
	}

	/* save V ----------------------------------------------------------------*/
	for(h=0; h<u32UvHeight; h++)    
	{
	    pMemContent = pVBufVirt_C + h*pVBuf->u32Stride[1];

	    for(w=0; w<pVBuf->u32Width/2; w++)
	    {
	        TmpBuff[w] = *pMemContent;
	        pMemContent += 2;
	    }
	    fwrite(TmpBuff, 1,pVBuf->u32Width/2,pfile);
	}
    
    free(TmpBuff);

	return;
}




HI_S32 FISHEYE_CALIBRATE_MISC_GETVB(VIDEO_FRAME_INFO_S* pstOutFrame, VIDEO_FRAME_INFO_S* pstInFrame,
                         VB_BLK* pstVbBlk, VB_POOL pool)
{
    HI_U32 u32Size;
    VB_BLK VbBlk = VB_INVALID_HANDLE;
    HI_U32 u32PhyAddr;
    HI_VOID* pVirAddr;
    HI_U32 u32LumaSize, u32UVInterval;
    HI_U32 u32LStride, u32CStride;
    HI_U32 u32Width, u32Height;


    u32Width = pstInFrame->stVFrame.u32Width;
    u32Height = pstInFrame->stVFrame.u32Height;
    u32LStride = pstInFrame->stVFrame.u32Stride[0];
    u32CStride = pstInFrame->stVFrame.u32Stride[1];

	if (PIXEL_FORMAT_YUV_SEMIPLANAR_420 == pstInFrame->stVFrame.enPixelFormat)
    {
        u32Size = (3 * u32LStride * u32Height) >> 1;
        u32LumaSize = u32LStride * u32Height;
        u32UVInterval = 0;
    }
	else if (PIXEL_FORMAT_YUV_PLANAR_420 == pstInFrame->stVFrame.enPixelFormat)
    {
        u32Size = (3 * u32LStride * u32Height) >> 1;
        u32LumaSize = u32LStride * u32Height;
        u32UVInterval = u32LumaSize >> 2;
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
    pstOutFrame->stVFrame.u32PhyAddr[2] = pstOutFrame->stVFrame.u32PhyAddr[1] + u32UVInterval;

    pstOutFrame->stVFrame.pVirAddr[0] = pVirAddr;
    pstOutFrame->stVFrame.pVirAddr[1] = pstOutFrame->stVFrame.pVirAddr[0] + u32LumaSize;
    pstOutFrame->stVFrame.pVirAddr[2] = pstOutFrame->stVFrame.pVirAddr[1] + u32UVInterval;

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




#ifdef __HuaweiLite__
HI_S32 fisheye_calibrate(int argc, char* argv[])
#else
HI_S32 main(int argc, char* argv[])
#endif
{
	HI_U32 u32Width =0;
	HI_U32  u32Height =0;
	VB_POOL hPool  = VB_INVALID_POOLID;
	HI_U32  u32BlkSize = 0;
	HI_S32	s32Ret = HI_SUCCESS;
	
	VB_BLK VbBlk = VB_INVALID_HANDLE;
	HI_U32 u32OldDepth = 0;
	VIDEO_FRAME_INFO_S stFrame;
	VIDEO_FRAME_INFO_S *pstVFramIn = HI_NULL;
	VIDEO_FRAME_INFO_S stVFramOut;
	HI_S32 s32MilliSec = 2000;

	HI_CHAR* pcPixFrm = NULL;
	HI_CHAR szYuvName[128] = {0};

	CALIBTATE_OUTPUT_S stOutCalibrate;
	FISHEYE_ATTR_S stFisheyeAttr = {0};
	VI_CHN ViChn = 0;
	VI_CHN ViExtChn = 2;
	HI_S32 i = 0;
	HI_U32 u32OutRadious = 0;
	FILE *pfdOut = NULL;

	printf("\nNOTICE: This tool only can be used for TESTING !!!\n");
	printf("NOTICE: This tool only only suport PIXEL_FORMAT_YUV_SEMIPLANAR_420\n");

    if (argc > 1)
    {
        if (!strncmp(argv[1], "-h", 2))
        {
            usage();
            exit(HI_SUCCESS);
        }
        ViChn = atoi(argv[1]);
    }

    if (argc > 2)
    {
        ViExtChn = atoi(argv[2]);
    }

    if (argc < 3)
    {
		printf("err para\n");        
		usage();  
		return HI_FAILURE;
    }

	/* register signal handler */
    signal(SIGINT, SigHandler);
    signal(SIGTERM, SigHandler);

	memset(&stOutCalibrate,0,sizeof(CALIBTATE_OUTPUT_S));
	memset(&stFisheyeAttr,0,sizeof(FISHEYE_ATTR_S));

	if((ViChn < 0) ||(ViChn >= VIU_MAX_PHYCHN_NUM))
	{
		printf("Not Correct Physic Channel\n");
		return HI_FAILURE;
	}

	if((ViExtChn < 2) ||(ViExtChn >= VIU_MAX_EXT_CHN_NUM))
	{
		printf("Not Correct Extend Channel\n");
		return HI_FAILURE;
	}

    if (HI_MPI_VI_GetFrameDepth(ViChn, &u32OldDepth))
    {
        printf("HI_MPI_VI_GetFrameDepth err, vi chn %d \n", ViChn);
       return HI_FAILURE;
    }
    if (HI_MPI_VI_SetFrameDepth(ViChn, 1))
    {
        printf("HI_MPI_VI_SetFrameDepth err, vi chn %d \n", ViChn);
        goto END1;
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

	pstVFramIn = &stFrame;

	u32Width = pstVFramIn->stVFrame.u32Width;
	u32Height= pstVFramIn->stVFrame.u32Height;

    pcPixFrm = (PIXEL_FORMAT_YUV_400 == stFrame.stVFrame.enPixelFormat ) ? "single" :
               ((PIXEL_FORMAT_YUV_SEMIPLANAR_420 == stFrame.stVFrame.enPixelFormat) ? "p420" : "p422");

	snprintf(szYuvName, sizeof(szYuvName)-1, "./fisheye_calibrate_out_%d_%d_%d_%s.yuv", ViChn,u32Width, u32Height, pcPixFrm);

	pfdOut = fopen(szYuvName, "wb");
    if (NULL == pfdOut)
    {
        printf("open file %s err\n",szYuvName);
        goto END2;
    }

	//Save VI PIC For UserMode
	u32BlkSize = u32Width*u32Height* 3/2;
    /*create comm vb pool*/
    hPool   = HI_MPI_VB_CreatePool(u32BlkSize, 4 , NULL);
    if (hPool == VB_INVALID_POOLID)
    {
        printf("HI_MPI_VB_CreatePool failed! \n");
        s32Ret = HI_FAILURE;
		goto END2;
    }	

	memcpy(&stVFramOut,pstVFramIn,sizeof(VIDEO_FRAME_INFO_S));
 	if (HI_SUCCESS != FISHEYE_CALIBRATE_MISC_GETVB(&stVFramOut,pstVFramIn,&VbBlk,hPool))
	{
		printf("FAILURE--fun:%s line:%d\n",__FUNCTION__,__LINE__);
		goto END3;
	}

	s32Ret = FISHEYE_Calibrate_MapVirtAddr(pstVFramIn);
	if (HI_SUCCESS != s32Ret)
	{
		printf("Map Virt Addr Failed!\n");
		goto END4;
	}

	if (HI_TRUE == bQuit)
    {
        goto END5;
    }

	printf("Compute Calibrate Result.....\n");
	s32Ret = HI_FISHEYE_ComputeCalibrateResult(&pstVFramIn->stVFrame,FISHEYE_CALIBRATE_LEVEL,&stOutCalibrate);
	if(HI_SUCCESS != s32Ret)
	{
		printf("Mark Result Failed!\n");
		goto END5;
	}

	if (HI_TRUE == bQuit)
    {
        goto END5;
    }

	printf(" Radius_X=%d,\n Radius_Y=%d,\n Radius=%d,\n OffsetH=%d,\n OffsetV=%d. \n",
			stOutCalibrate.stCalibrateResult.s32Radius_X,stOutCalibrate.stCalibrateResult.s32Radius_Y,stOutCalibrate.stCalibrateResult.u32Radius,
			stOutCalibrate.stCalibrateResult.s32OffsetH,stOutCalibrate.stCalibrateResult.s32OffsetV);

	printf("Mark Calibrate Result.....\n");
	s32Ret = HI_FISHEYE_MarkCalibrateResult(&pstVFramIn->stVFrame,&stVFramOut.stVFrame,&stOutCalibrate.stCalibrateResult);
	if(HI_SUCCESS != s32Ret)
	{
		printf("Mark Result Failed!\n");
		goto END5;
	}

	s32Ret = HI_MPI_VI_GetFisheyeAttr(ViExtChn,&stFisheyeAttr);
	if(HI_SUCCESS != s32Ret)
	{
		printf("Get Fisheye Attr Failed!\n");
		goto END5;
	}

	if(stOutCalibrate.stCalibrateResult.s32OffsetH >127)
	{
		stFisheyeAttr.s32HorOffset = 127;
	}
	else if(stOutCalibrate.stCalibrateResult.s32OffsetH < -127)
	{
		stFisheyeAttr.s32HorOffset = -127;
	}
	else
	{
		stFisheyeAttr.s32HorOffset = stOutCalibrate.stCalibrateResult.s32OffsetH;
	}

	if(stOutCalibrate.stCalibrateResult.s32OffsetV >127)
	{
		stFisheyeAttr.s32VerOffset = 127;
	}
	else if(stOutCalibrate.stCalibrateResult.s32OffsetV < -127)
	{
		stFisheyeAttr.s32VerOffset = -127;
	}
	else
	{
		stFisheyeAttr.s32VerOffset = stOutCalibrate.stCalibrateResult.s32OffsetV;
	}

	if(stOutCalibrate.stCalibrateResult.u32Radius > 2304)
	{
		u32OutRadious = 2304;
	}
	else
	{
		u32OutRadious = stOutCalibrate.stCalibrateResult.u32Radius;
	}

	//printf("u32RegionNum:%d u32OutRadious:%d \n",stFisheyeAttr.u32RegionNum,u32OutRadious);
	for(i = 0;i < stFisheyeAttr.u32RegionNum; i ++)
	{
		stFisheyeAttr.astFisheyeRegionAttr[i].u32OutRadius = u32OutRadious;
	}

	
	s32Ret = HI_MPI_VI_SetFisheyeAttr(ViExtChn,&stFisheyeAttr);
	if(HI_SUCCESS != s32Ret)
	{
		printf("Set Fisheye Attr Failed!\n");
		goto END5;
	}
	
	FISHEYE_Calibrate_SaveSP42XToPlanar(pfdOut, &stVFramOut.stVFrame);

END5:

	HI_MPI_SYS_Munmap(stVFramOut.stVFrame.pVirAddr[0],u32BlkSize);

END4:
	
	if (VbBlk != VB_INVALID_HANDLE)
    {
		HI_MPI_VB_ReleaseBlock(VbBlk);
	}
	
END3:	
	
	if (hPool != VB_INVALID_POOLID)
    {
        HI_MPI_VB_DestroyPool(hPool);
    }

END2:

	HI_MPI_VI_ReleaseFrame(ViChn, pstVFramIn);

END1:

	if (-1U != u32OldDepth)
    {
        HI_MPI_VI_SetFrameDepth(ViChn, u32OldDepth);
    }

	if(NULL != pfdOut)
	{
		fclose(pfdOut);
	}

	printf("Calibrate Finished.....\n");
	
    return s32Ret;
}
