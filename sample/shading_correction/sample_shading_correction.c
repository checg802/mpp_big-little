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

typedef struct
{
	HI_BOOL bGetBakRegValue;
	HI_BOOL bGetBakDumpAttr;
	HI_BOOL bEnableBayerDump;
	HI_BOOL bGetFrame;
	HI_U32 u32BakRegValue;
	VI_DUMP_ATTR_S stBakDumpAttr;
	VI_FRAME_INFO_S stViFrameInfo;
}CAL_STATE_INFO_S;

typedef struct
{
	HI_S32 s32ViDev;
	HI_S32 s32IspDev;
	HI_CHAR aszBinFilePath[64];
}CAL_SHARDING_PARAM_S;

static CAL_STATE_INFO_S g_stStateInfo;

static HI_S32 calRestorEnv(VI_DEV ViDev, VI_CHN ViChn)
{
	HI_S32 s32Ret = HI_SUCCESS;
	HI_U32 u32DesIdAddr = 0x11380310;

	if(0 == ViDev)
	{
		u32DesIdAddr = 0x11380310;	
	}
	else 
	{	
		u32DesIdAddr = 0x11480310;
	}	

	if (HI_TRUE == g_stStateInfo.bGetFrame)
	{
		s32Ret = HI_MPI_VI_ReleaseFrame(ViChn, &g_stStateInfo.stViFrameInfo.stViFrmInfo);
		if (HI_SUCCESS != s32Ret)
		{
			printf("HI_MPI_VI_ReleaseFrame failed(0x%x)!\n", s32Ret);
			return HI_FAILURE;
		}
		g_stStateInfo.bGetFrame = HI_FALSE;
	}

	if (HI_TRUE == g_stStateInfo.bEnableBayerDump)
	{
		s32Ret = HI_MPI_VI_DisableBayerDump(ViDev);
		if (HI_SUCCESS != s32Ret)
	    {
	        printf("HI_MPI_VI_SetDevDumpAttr failed(0x%x)!\n", s32Ret);
	        return HI_FAILURE;
		}
		g_stStateInfo.bEnableBayerDump = HI_FALSE;
	}
	
	if (HI_TRUE == g_stStateInfo.bGetBakDumpAttr)
	{
		s32Ret = HI_MPI_VI_SetDevDumpAttr(ViDev, &g_stStateInfo.stBakDumpAttr);
		if (HI_SUCCESS != s32Ret)
		{
			printf("HI_MPI_VI_SetDevDumpAttr failed(0x%x)!\n", s32Ret);
			return HI_FAILURE;
		}
		g_stStateInfo.bGetBakDumpAttr = HI_FALSE;
	}
		
	if (HI_TRUE == g_stStateInfo.bGetBakRegValue)
	{		
		s32Ret = HI_MPI_SYS_SetReg(u32DesIdAddr, g_stStateInfo.u32BakRegValue);
	    if (HI_SUCCESS != s32Ret)
	    {
	        printf("HI_MPI_SYS_SetReg failed, s32Ret = 0x%x!\n", s32Ret);
	        return HI_FAILURE;
	    }
		g_stStateInfo.bGetBakRegValue = HI_FALSE;
	}
	
	return HI_SUCCESS;
}

static HI_S32 calCalibrateShading(ISP_DEV IspDev, ISP_MESH_SHADING_TABLE_S *pstMeshShadingTable)
{
	HI_S32 s32Ret = HI_SUCCESS;
	HI_S32 s32MemDev = -1;
	HI_U8 *pu8UserPageAddr;
	HI_U32 u32Size;	
	HI_U16 *pu16RawData;	
	ISP_LSC_CALIBRATION_CFG_S stLSCCaliCfg;
	ISP_PUB_ATTR_S stIspPubAttr = {{0},{0},0,0};
	ISP_BLACK_LEVEL_S stBlackLevel = {{0}};

	/*sys map*/
	s32MemDev = open("/dev/mem", O_CREAT|O_RDWR|O_SYNC);
	if (s32MemDev < 0)
	{
		printf("Open dev/mem error\n");
		return HI_FAILURE;
	}

	u32Size = (g_stStateInfo.stViFrameInfo.stViFrmInfo.stVFrame.u32Stride[0]) * (g_stStateInfo.stViFrameInfo.stViFrmInfo.stVFrame.u32Height) *2;
	pu8UserPageAddr = (HI_U8*) HI_MPI_SYS_Mmap(g_stStateInfo.stViFrameInfo.stViFrmInfo.stVFrame.u32PhyAddr[0], u32Size);
	if (NULL == pu8UserPageAddr)
    {
        close(s32MemDev);
		s32MemDev = -1;
        return HI_FAILURE;
    }
	pu16RawData = (HI_U16 *)pu8UserPageAddr;

#if 0
	/*
		save raw data for test
	*/
	HI_U8 *pu8Data;
	pu8Data = pu8UserPageAddr;
	FILE *pfd = NULL;
	pfd = fopen("calibrate.raw", "wb");
	if (NULL == pfd)
	{
		printf("open file failed!\n");
		close(s32MemDev);
		s32MemDev = -1;
		return HI_FAILURE;
	}
	int h;
	fprintf(stderr, "saving......dump data......u32Stride[0]: %d, width: %d\n", g_stStateInfo.stViFrameInfo.stViFrmInfo.stVFrame.u32Stride[0], g_stStateInfo.stViFrameInfo.stViFrmInfo.stVFrame.u32Width);
    fflush(stderr);
    for (h = 0; h < g_stStateInfo.stViFrameInfo.stViFrmInfo.stVFrame.u32Height; h++)
    {
       	fwrite(pu8Data, g_stStateInfo.stViFrameInfo.stViFrmInfo.stVFrame.u32Width, 2, pfd);
		fflush(pfd);
        pu8Data += g_stStateInfo.stViFrameInfo.stViFrmInfo.stVFrame.u32Stride[0];
    }
    fflush(pfd);
    fprintf(stderr, "done u32TimeRef: %d!\n", g_stStateInfo.stViFrameInfo.stViFrmInfo.stVFrame.u32TimeRef);
    fflush(stderr);

	fclose(pfd);
#endif

	/*calibrate*/
	stLSCCaliCfg.bOffsetInEn = HI_TRUE;  
	stLSCCaliCfg.enRawBit = BAYER_RAW_16BIT;           
	stLSCCaliCfg.u8MeshScale = 1;
	stLSCCaliCfg.u8EliminatePer = 10;
	stLSCCaliCfg.u8GridSizeX = 65;    //Mesh_Shading horizontal Block Num+1
	stLSCCaliCfg.u8GridSizeY = 65;    //Mesh_Shading vertical Block Num+1
	s32Ret = HI_MPI_ISP_GetPubAttr(IspDev,&stIspPubAttr);
	stLSCCaliCfg.enBayer = stIspPubAttr.enBayer;
	stLSCCaliCfg.u16ImageWidth = g_stStateInfo.stViFrameInfo.stViFrmInfo.stVFrame.u32Width;	//image width 
	stLSCCaliCfg.u16ImageHeight = g_stStateInfo.stViFrameInfo.stViFrmInfo.stVFrame.u32Height; //image height 
	s32Ret = HI_MPI_ISP_GetBlackLevelAttr(IspDev,&stBlackLevel);
	stLSCCaliCfg.u16BLCOffsetR = stBlackLevel.au16BlackLevel[0]; 	//Black level for R channel of 12bit
	stLSCCaliCfg.u16BLCOffsetGr = stBlackLevel.au16BlackLevel[1];	//Black level for Gr channel of 12bit
	stLSCCaliCfg.u16BLCOffsetGb = stBlackLevel.au16BlackLevel[2];	//Black level for Gb channel of 12bit
	stLSCCaliCfg.u16BLCOffsetB = stBlackLevel.au16BlackLevel[3];	//Black level for B channel of 12bit

	s32Ret = HI_MPI_ISP_MeshShadingCalibration(pu16RawData , &stLSCCaliCfg, *pstMeshShadingTable);
	if(HI_SUCCESS != s32Ret)
    {
		HI_MPI_SYS_Munmap(pu8UserPageAddr, u32Size);
		close(s32MemDev);
		s32MemDev = -1;
        return HI_FAILURE;
    }

	HI_MPI_SYS_Munmap(pu8UserPageAddr, u32Size);
	close(s32MemDev);
	s32MemDev = -1;
	return HI_SUCCESS;
}

static HI_S32 calCaptureRaw(VI_DEV ViDev, VI_CHN ViChn)
{
	HI_S32 s32Ret = HI_SUCCESS;
	HI_S32 s32MilliSec = 2000;	
	VI_DUMP_ATTR_S stDumpAttr;
	HI_U32 u32DesIdAddr = 0x11380310;
    printf("ViDev = %d\n",ViDev);
	
	if(0 == ViDev)
	{
		u32DesIdAddr = 0x11380310;	
	}
	else 
	{	
		u32DesIdAddr = 0x11480310;
	}

	/*save reg value and dump attr. when calibration is over set them back*/
	s32Ret = HI_MPI_SYS_GetReg(u32DesIdAddr, &g_stStateInfo.u32BakRegValue);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_SYS_GetReg failed, s32Ret = %d!\n", s32Ret);
        return HI_FAILURE;
    }
	g_stStateInfo.bGetBakRegValue = HI_TRUE;
	
	s32Ret = HI_MPI_VI_GetDevDumpAttr(ViDev, &g_stStateInfo.stBakDumpAttr);
	if (HI_SUCCESS != s32Ret)
	{
		printf("HI_MPI_VI_GetDevDumpAttr failed(0x%x)\n", s32Ret);
		return HI_FAILURE;
	}
	g_stStateInfo.bGetBakDumpAttr = HI_TRUE;

	/*disable dump*/
	s32Ret = HI_MPI_VI_DisableBayerDump(ViDev);
	if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VI_SetDevDumpAttr failed(0x%x)!\n", s32Ret);
        return HI_FAILURE;
	}

	/*set dump attr*/
	stDumpAttr.enDumpType = VI_DUMP_TYPE_RAW;
	stDumpAttr.enPixelFormat = PIXEL_FORMAT_RGB_BAYER;
	stDumpAttr.stCropInfo.bEnable = HI_FALSE;
	stDumpAttr.enDumpSel = VI_DUMP_SEL_PORT;
	s32Ret =  HI_MPI_VI_SetDevDumpAttr(ViDev, &stDumpAttr);
	if (HI_SUCCESS != s32Ret)
	{
		printf("ViDev%d, HI_MPI_VI_SetDevDumpAttr failed(0x%x)!\n", ViDev, s32Ret);
		return HI_FAILURE;
	}

	/*start dump raw*/
	s32Ret = HI_MPI_VI_EnableBayerDump(ViDev);
	if (HI_SUCCESS != s32Ret)
	{
		printf("HI_MPI_VI_EnableBayerDump failed(0x%x)!\n", s32Ret);
		return HI_FAILURE;
	}
	g_stStateInfo.bEnableBayerDump = HI_TRUE;
	
	s32Ret = HI_MPI_SYS_SetReg(u32DesIdAddr, 0x1);
	if (HI_SUCCESS != s32Ret)
	{
		printf("HI_MPI_SYS_SetReg failed, s32Ret = 0x%x!\n", s32Ret);
		return HI_FAILURE;
	}	
	s32Ret = HI_MPI_VI_SetFrameDepth(ViChn, 1);
	if (HI_SUCCESS != s32Ret)
	{
		printf("HI_MPI_VI_SetFrameDepth err, vi chn %d \n", ViChn);
        return HI_FAILURE;
	}
	usleep(5000);

	g_stStateInfo.stViFrameInfo.stViFrmInfo.stVFrame.u32PhyAddr[0] = 0;
	s32Ret = HI_MPI_VI_GetFrame(ViChn, &g_stStateInfo.stViFrameInfo.stViFrmInfo, s32MilliSec);
	if (HI_SUCCESS != s32Ret)
	{
		printf("HI_MPI_VI_GetFrame err, vi chn %d \n", ViChn);
        return HI_FAILURE;
	}
	g_stStateInfo.bGetFrame = HI_TRUE;
	
	return HI_SUCCESS;
}

HI_S32 shading_correction(void)
{	
	HI_S32 s32Ret = HI_SUCCESS;
	//HI_U32 u32Count = 0;
	ISP_DEV IspDev = 0;		//this value must be isp dev for video mode
	VI_DEV ViDev = 0;		//this value must be vi dev for video mode
	VI_CHN ViChn = 19+ViDev; //VIU_MAX_PHYCHN_NUM(2) + VIU_MAX_EXT_CHN_NUM(16) dev0 channel is 19
	ISP_MESH_SHADING_TABLE_S stMeshShadingTable = {0};
	ISP_MESH_SHADING_ATTR_S stIspMeshShardingAttr;	

	/*step1. dump raw data*/
	s32Ret = calCaptureRaw(ViDev, ViChn);
	if (HI_SUCCESS != s32Ret)
	{
		printf("calCaptureRaw failed!\n");
		calRestorEnv(ViDev, ViChn);
		return HI_FAILURE;
	}
	
	/*step2. calibrate shading*/
	stMeshShadingTable.pu8Rgain = (HI_U8*)malloc(sizeof(HI_U8)*4096);	//64 * 64
	if (HI_NULL == stMeshShadingTable.pu8Rgain)
	{
		printf("stMeshShadingTable malloc failure !\n");
		calRestorEnv(ViDev, ViChn);
        return HI_FAILURE;
	}
	stMeshShadingTable.pu8Ggain = (HI_U8*)malloc(sizeof(HI_U8)*4096);	//64 * 64
	if (HI_NULL == stMeshShadingTable.pu8Ggain)
	{
		printf("stMeshShadingTable malloc failure !\n");
		free(stMeshShadingTable.pu8Rgain);
		calRestorEnv(ViDev, ViChn);
        return HI_FAILURE;
	}
	stMeshShadingTable.pu8Bgain = (HI_U8*)malloc(sizeof(HI_U8)*4096);	//64 * 64
	if (HI_NULL == stMeshShadingTable.pu8Bgain)
    {
        printf("stMeshShadingTable malloc failure !\n");
		free(stMeshShadingTable.pu8Rgain);
		free(stMeshShadingTable.pu8Ggain);
		calRestorEnv(ViDev, ViChn);
        return HI_FAILURE;
    }
	s32Ret = calCalibrateShading(IspDev, &stMeshShadingTable);
	if (HI_SUCCESS != s32Ret)
	{
		printf("calCalibrateSharding failed!\n");
		free(stMeshShadingTable.pu8Rgain);
		free(stMeshShadingTable.pu8Ggain);
		free(stMeshShadingTable.pu8Bgain);
		calRestorEnv(ViDev, ViChn);
		return HI_FAILURE;
	}
	
#if 0
	/*
		save sharding table for test
	*/
	FILE *pFileR = fopen("Rgain.txt", "wb");
	FILE *pFileG = fopen("Ggain.txt", "wb");
	FILE *pFileB = fopen("Bgain.txt", "wb");
	int i,j;
	for (j = 0;j < 64;j++)
	{
		for (i = 0;i < 64;i++)
		{
            fprintf(pFileR , "%d, ",stMeshShadingTable.pu8Rgain[j*64+i]);
			fprintf(pFileG , "%d, ",stMeshShadingTable.pu8Ggain[j*64+i]);
			fprintf(pFileB , "%d, ",stMeshShadingTable.pu8Bgain[j*64+i]);
		}
        fprintf(pFileR,"\n");
        fprintf(pFileG,"\n");
        fprintf(pFileB,"\n");
	}
	fclose(pFileR);
    fclose(pFileG);
    fclose(pFileB);
#endif
	
	/*restore environment*/
	calRestorEnv(ViDev, ViChn);
	
	stIspMeshShardingAttr.bEnable = HI_TRUE;
	stIspMeshShardingAttr.enMeshLSMode = ISP_MESH_MODE_TYPE_1LS;	//single light source D50
	stIspMeshShardingAttr.u8MeshScale = 1;
	stIspMeshShardingAttr.u16MeshStrength = 4096;
	stIspMeshShardingAttr.au32MeshColorTemp[0] = 5000;	//D50
	stIspMeshShardingAttr.au32MeshColorTemp[1] = 5000;	//D50
	stIspMeshShardingAttr.au32MeshColorTemp[2] = 5000;	//D50
	stIspMeshShardingAttr.au32MeshColorTemp[3] = 5000;	//D50
	memcpy(stIspMeshShardingAttr.au8MeshTable[0], stMeshShadingTable.pu8Rgain, 4096);
	memcpy(stIspMeshShardingAttr.au8MeshTable[1], stMeshShadingTable.pu8Ggain, 4096);
	memcpy(stIspMeshShardingAttr.au8MeshTable[2], stMeshShadingTable.pu8Bgain, 4096);	
	free(stMeshShadingTable.pu8Rgain);	
	free(stMeshShadingTable.pu8Ggain);	
	free(stMeshShadingTable.pu8Bgain);
	
	/*step3. set sharding tablet*/	
    s32Ret = HI_MPI_ISP_SetMeshShadingAttr(IspDev, &stIspMeshShardingAttr);
    if (HI_SUCCESS != s32Ret)
    {
    	printf("HI_MPI_ISP_SetMeshShadingAttr failed(0x%x)!\n", s32Ret);			
    	return HI_FAILURE;
    }
	
	return HI_SUCCESS;
}

int main(int argc, char *argv[])
{
    ISP_DEV IspDev = 0;
    ISP_EXPOSURE_ATTR_S stExpAttr;

    HI_MPI_ISP_GetExposureAttr(IspDev, &stExpAttr);
    
    printf("set antiflicker enable and the value of frequency\n");
    stExpAttr.stAuto.stAntiflicker.bEnable = HI_TRUE;
    stExpAttr.stAuto.stAntiflicker.u8Frequency = 50;
    HI_MPI_ISP_SetExposureAttr(IspDev, &stExpAttr);

    shading_correction();
    return 0;
}
