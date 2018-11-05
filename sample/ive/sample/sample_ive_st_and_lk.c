
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>


#include "hi_common.h"
#include "hi_comm_video.h"
#include "hi_comm_sys.h"
#include "hi_comm_ive.h"
#include "hi_comm_vgs.h"

#include "mpi_vb.h"
#include "mpi_sys.h"
#include "mpi_ive.h"
#include "mpi_vgs.h"

#include "sample_comm_ive.h"

#define MAX_POINT_NUM 500
#define MIN_DIST 5

typedef struct hiSAMPLE_IVE_ST_LK_S
{
    IVE_SRC_IMAGE_S  					astPrevPyr[4];	
    IVE_SRC_IMAGE_S 		 			astNextPyr[4];
	IVE_SRC_MEM_INFO_S 					stPrevPts;
	IVE_MEM_INFO_S 						stNextPts;
	IVE_DST_MEM_INFO_S 					stStatus;
	IVE_DST_MEM_INFO_S 					stErr;
    IVE_LK_OPTICAL_FLOW_PYR_CTRL_S		stLkPyrCtrl;

	IVE_SRC_IMAGE_S  					stStSrc;
	IVE_IMAGE_S 						stStDst;	 
	IVE_DST_MEM_INFO_S 					stCorner;
	IVE_ST_CANDI_CORNER_CTRL_S  		stStCandiCornerCtrl;
	IVE_ST_CORNER_CTRL_S	  			stStCornerCtrl;

	IVE_IMAGE_S 						stPyrTmp;
	IVE_IMAGE_S  						stSrcYuv;
	FILE* 								pFpSrc;
	FILE* 								pFpDst;

    //IVE_DST_IMAGE_S  astDst;
    //IVE_SRC_IMAGE_S  stSrcYUV;
    //IVE_SRC_IMAGE_S	 astPrePyr[3];
    //IVE_SRC_IMAGE_S	 astCurPyr[3];
    //IVE_IMAGE_S stPyrTmp;
    //IVE_DST_MEM_INFO_S stDstCorner;
    //IVE_MEM_INFO_S   stMv;
    //IVE_SRC_MEM_INFO_S astPoint[3];

} SAMPLE_IVE_ST_LK_S;

static SAMPLE_IVE_ST_LK_S s_stStLk;

static HI_S32 SAMPLE_IVE_St_Lk_DMA(IVE_HANDLE *pIveHandle, IVE_SRC_IMAGE_S *pstSrc, 
											IVE_DST_IMAGE_S *pstDst,IVE_DMA_CTRL_S *pstDmaCtrl,HI_BOOL bInstant)
{
	HI_S32 s32Ret = HI_FAILURE;
	IVE_SRC_DATA_S stDataSrc;
	IVE_DST_DATA_S stDataDst;	

	stDataSrc.pu8VirAddr 	= pstSrc->pu8VirAddr[0];
	stDataSrc.u32PhyAddr 	= pstSrc->u32PhyAddr[0];
	stDataSrc.u16Width 		= pstSrc->u16Width;
	stDataSrc.u16Height 	= pstSrc->u16Height;
	stDataSrc.u16Stride 	= pstSrc->u16Stride[0];
	
	stDataDst.pu8VirAddr 	= pstDst->pu8VirAddr[0];
	stDataDst.u32PhyAddr 	= pstDst->u32PhyAddr[0];
	stDataDst.u16Width 		= pstDst->u16Width;
	stDataDst.u16Height 	= pstDst->u16Height;
	stDataDst.u16Stride 	= pstDst->u16Stride[0];
	s32Ret = HI_MPI_IVE_DMA(pIveHandle, &stDataSrc, &stDataDst,pstDmaCtrl,bInstant);
	if (HI_SUCCESS != s32Ret)
	{			
		SAMPLE_PRT("HI_MPI_IVE_DMA fail,Error(%d)\n",s32Ret);
		return s32Ret;
	}

	return s32Ret;
}
/******************************************************************************
* function : Copy pyr
******************************************************************************/
static HI_VOID SAMPLE_IVE_St_Lk_CopyPyr(IVE_SRC_IMAGE_S astPyrSrc[], IVE_DST_IMAGE_S astPyrDst[], HI_U8 u8MaxLevel)
{
    HI_U8 i;
	HI_S32 s32Ret = HI_FAILURE;
	IVE_HANDLE hIveHandle;
	
	IVE_DMA_CTRL_S stCtrl;
	memset(&stCtrl,0,sizeof(stCtrl));
	stCtrl.enMode = IVE_DMA_MODE_DIRECT_COPY;

    for (i = 0; i <= u8MaxLevel; i++)
    {
		s32Ret = SAMPLE_IVE_St_Lk_DMA(&hIveHandle,&astPyrSrc[i],&astPyrDst[i],&stCtrl,HI_FALSE);
		if (HI_SUCCESS != s32Ret)
		{			
	        SAMPLE_PRT("SAMPLE_IVE_St_Lk_DMA fail,Error(%d)\n",s32Ret);
	        break;
		}
    }
}

/******************************************************************************
* function : St lk uninit
******************************************************************************/
static HI_VOID SAMPLE_IVE_St_Lk_Uninit(SAMPLE_IVE_ST_LK_S* pstStLk)
{
    HI_U16 i;
	for (i = 0; i <= pstStLk->stLkPyrCtrl.u8MaxLevel; i++)
	{			
		IVE_MMZ_FREE(pstStLk->astPrevPyr[i].u32PhyAddr[0], pstStLk->astPrevPyr[i].pu8VirAddr[0]);		
		IVE_MMZ_FREE(pstStLk->astNextPyr[i].u32PhyAddr[0], pstStLk->astNextPyr[i].pu8VirAddr[0]);
	}
	
    IVE_MMZ_FREE(pstStLk->stPrevPts.u32PhyAddr, pstStLk->stPrevPts.pu8VirAddr);
    IVE_MMZ_FREE(pstStLk->stNextPts.u32PhyAddr, pstStLk->stNextPts.pu8VirAddr);
    IVE_MMZ_FREE(pstStLk->stStatus.u32PhyAddr, pstStLk->stStatus.pu8VirAddr);
    IVE_MMZ_FREE(pstStLk->stErr.u32PhyAddr, pstStLk->stErr.pu8VirAddr);

    IVE_MMZ_FREE(pstStLk->stStSrc.u32PhyAddr[0], pstStLk->stStSrc.pu8VirAddr[0]);
    IVE_MMZ_FREE(pstStLk->stStDst.u32PhyAddr[0], pstStLk->stStDst.pu8VirAddr[0]);
    IVE_MMZ_FREE(pstStLk->stCorner.u32PhyAddr, pstStLk->stCorner.pu8VirAddr);
	
    IVE_MMZ_FREE(pstStLk->stStCandiCornerCtrl.stMem.u32PhyAddr, pstStLk->stStCandiCornerCtrl.stMem.pu8VirAddr);

	IVE_MMZ_FREE(pstStLk->stPyrTmp.u32PhyAddr[0], pstStLk->stPyrTmp.pu8VirAddr[0]);	
	IVE_MMZ_FREE(pstStLk->stSrcYuv.u32PhyAddr[0], pstStLk->stSrcYuv.pu8VirAddr[0]);

    IVE_CLOSE_FILE(pstStLk->pFpSrc);
    IVE_CLOSE_FILE(pstStLk->pFpDst);
}

/******************************************************************************
* function : St lk init
******************************************************************************/
static HI_S32 SAMPLE_IVE_St_Lk_Init(SAMPLE_IVE_ST_LK_S* pstStLk,
                                    HI_CHAR* pchSrcFileName, 
                                    HI_CHAR* pchDstFileName, 
                                    HI_U16 u16Width, 
                                    HI_U16 u16Height,	
                                    HI_U16 u16PyrWidth,
									HI_U16 u16PyrHeight,
                                    HI_U8 u8MaxLevel)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32Size = 0;
    HI_U16 i;

    memset(pstStLk, 0, sizeof(SAMPLE_IVE_ST_LK_S));
	
	pstStLk->stLkPyrCtrl.enOutMode = IVE_LK_OPTICAL_FLOW_PYR_OUT_MODE_BOTH;
	pstStLk->stLkPyrCtrl.bUseInitFlow = HI_TRUE;
    pstStLk->stLkPyrCtrl.u16PtsNum= MAX_POINT_NUM;
	pstStLk->stLkPyrCtrl.u8MaxLevel = u8MaxLevel;
    pstStLk->stLkPyrCtrl.u0q8MinEigThr = 100;
	pstStLk->stLkPyrCtrl.u8IterCnt = 10;
	pstStLk->stLkPyrCtrl.u0q8Eps = 2;
	//Init Pyr
	for(i=0; i<=u8MaxLevel; i++)
	{
		s32Ret = SAMPLE_COMM_IVE_CreateImage(&pstStLk->astPrevPyr[i], IVE_IMAGE_TYPE_U8C1, 
			u16Width >> i, u16Height >> i);
	    if (s32Ret != HI_SUCCESS)
	    {
	        SAMPLE_PRT("SAMPLE_COMM_IVE_CreateImage fail\n");
	        goto ST_LK_INIT_FAIL;
	    }
		s32Ret = SAMPLE_COMM_IVE_CreateImage(&pstStLk->astNextPyr[i], IVE_IMAGE_TYPE_U8C1, 
			pstStLk->astPrevPyr[i].u16Width, pstStLk->astPrevPyr[i].u16Height);
	    if (s32Ret != HI_SUCCESS)
	    {
	        SAMPLE_PRT("SAMPLE_COMM_IVE_CreateImage fail\n");
	        goto ST_LK_INIT_FAIL;
	    }		
	}
	//Init prev pts
	u32Size = sizeof(IVE_POINT_S25Q7_S) * MAX_POINT_NUM;
	u32Size = SAMPLE_COMM_IVE_CalcStride(u32Size, IVE_ALIGN);
	s32Ret = SAMPLE_COMM_IVE_CreateMemInfo(&(pstStLk->stPrevPts), u32Size);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("SAMPLE_COMM_IVE_CreateMemInfo fail\n");
        goto ST_LK_INIT_FAIL;
    }
	//Init next pts
	s32Ret = SAMPLE_COMM_IVE_CreateMemInfo(&(pstStLk->stNextPts), u32Size);
	if (s32Ret != HI_SUCCESS)
	{
		SAMPLE_PRT("SAMPLE_COMM_IVE_CreateMemInfo fail\n");
		goto ST_LK_INIT_FAIL;
	}
	//Init status
	u32Size = sizeof(HI_U8) * MAX_POINT_NUM;
	u32Size = SAMPLE_COMM_IVE_CalcStride(u32Size, IVE_ALIGN);
	s32Ret = SAMPLE_COMM_IVE_CreateMemInfo(&(pstStLk->stStatus), u32Size);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("SAMPLE_COMM_IVE_CreateMemInfo fail\n");
        goto ST_LK_INIT_FAIL;
    }
	//Init err
	u32Size = sizeof(HI_U9Q7) * MAX_POINT_NUM;
	u32Size = SAMPLE_COMM_IVE_CalcStride(u32Size, IVE_ALIGN);
	s32Ret = SAMPLE_COMM_IVE_CreateMemInfo(&(pstStLk->stErr), u32Size);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("SAMPLE_COMM_IVE_CreateMemInfo fail\n");
        goto ST_LK_INIT_FAIL;
    }

	//Init St 	
    s32Ret = SAMPLE_COMM_IVE_CreateImage(&pstStLk->stStSrc, IVE_IMAGE_TYPE_U8C1, u16Width, u16Height);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("SAMPLE_COMM_IVE_CreateImage fail\n");
        goto ST_LK_INIT_FAIL;
    }
    s32Ret = SAMPLE_COMM_IVE_CreateImage(&pstStLk->stStDst, IVE_IMAGE_TYPE_U8C1, u16Width, u16Height);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("SAMPLE_COMM_IVE_CreateImage fail\n");
        goto ST_LK_INIT_FAIL;
    }
    
    pstStLk->stStCandiCornerCtrl.u0q8QualityLevel = 25;
    u32Size = 4 * SAMPLE_COMM_IVE_CalcStride(u16Width, IVE_ALIGN) * u16Height + sizeof(IVE_ST_MAX_EIG_S);
    s32Ret = SAMPLE_COMM_IVE_CreateMemInfo(&(pstStLk->stStCandiCornerCtrl.stMem), u32Size);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("SAMPLE_COMM_IVE_CreateMemInfo fail\n");
        goto ST_LK_INIT_FAIL;
    }
	
	u32Size = sizeof(IVE_ST_CORNER_INFO_S);
    s32Ret = SAMPLE_COMM_IVE_CreateMemInfo(&(pstStLk->stCorner), u32Size);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("SAMPLE_COMM_IVE_CreateMemInfo fail\n");
        goto ST_LK_INIT_FAIL;
    }
	
    pstStLk->stStCornerCtrl.u16MaxCornerNum = MAX_POINT_NUM;
    pstStLk->stStCornerCtrl.u16MinDist = MIN_DIST;    

	s32Ret = SAMPLE_COMM_IVE_CreateImage(&pstStLk->stPyrTmp, IVE_IMAGE_TYPE_U8C1, u16PyrWidth, u16PyrHeight);
	 if (s32Ret != HI_SUCCESS)
	 {
		 SAMPLE_PRT("SAMPLE_COMM_IVE_CreateImage fail\n");
		 goto ST_LK_INIT_FAIL;
	 }
	 s32Ret = SAMPLE_COMM_IVE_CreateImage(&pstStLk->stSrcYuv, IVE_IMAGE_TYPE_YUV420SP, u16Width, u16Height);
	 if (s32Ret != HI_SUCCESS)
	 {
		 SAMPLE_PRT("SAMPLE_COMM_IVE_CreateImage fail\n");
		 goto ST_LK_INIT_FAIL;
	 }
	 

    pstStLk->pFpSrc = fopen(pchSrcFileName, "rb");
    if (HI_NULL == pstStLk->pFpSrc)
    {
        SAMPLE_PRT("Open file %s fail\n", pchSrcFileName);
        s32Ret = HI_FAILURE;
        goto ST_LK_INIT_FAIL;
    }

    pstStLk->pFpDst = fopen(pchDstFileName, "wb");
    if (HI_NULL == pstStLk->pFpDst)
    {
        SAMPLE_PRT("Open file %s fail\n", pchDstFileName);
        s32Ret = HI_FAILURE;
        goto ST_LK_INIT_FAIL;
    }

ST_LK_INIT_FAIL:

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_IVE_St_Lk_Uninit(pstStLk);
    }
    return s32Ret;
}
/******************************************************************************
* function : Pyr down
******************************************************************************/
static HI_S32 SAMPLE_IVE_St_Lk_PyrDown(SAMPLE_IVE_ST_LK_S* pstStLk, IVE_SRC_IMAGE_S* pstSrc, IVE_DST_IMAGE_S* pstDst)
{
    HI_S32 s32Ret = HI_SUCCESS;
    IVE_HANDLE hIveHandle;
    IVE_DMA_CTRL_S  stDmaCtrl = {IVE_DMA_MODE_INTERVAL_COPY,
                                 0, 2, 1, 2
                                };
    IVE_FILTER_CTRL_S stFltCtrl = {{
            1, 2, 3, 2, 1,
            2, 5, 6, 5, 2,
            3, 6, 8, 6, 3,
            2, 5, 6, 5, 2,
            1, 2, 3, 2, 1
        }, 7
    };

    pstStLk->stPyrTmp.u16Width = pstSrc->u16Width;
    pstStLk->stPyrTmp.u16Height = pstSrc->u16Height;

    s32Ret = HI_MPI_IVE_Filter(&hIveHandle, pstSrc, &pstStLk->stPyrTmp, &stFltCtrl,  HI_FALSE);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_IVE_Filter fail,Error(%#x)\n", s32Ret);
        return s32Ret;
    }

	s32Ret = SAMPLE_IVE_St_Lk_DMA(&hIveHandle,&pstStLk->stPyrTmp,pstDst,&stDmaCtrl,HI_FALSE);
	if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("HI_MPI_IVE_DMA fail,Error(%#x)", s32Ret);
        return s32Ret;
    }  		
    
    return s32Ret;
}


/******************************************************************************
* function : St lk proc
******************************************************************************/
static HI_VOID SAMPLE_IVE_St_LkProc(SAMPLE_IVE_ST_LK_S* pstStLk)
{
    HI_U32 u32FrameNum = 10;
    HI_U32 i, k;
    HI_U16 u16RectNum;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_BOOL bBlock = HI_TRUE;
    HI_BOOL bFinish = HI_FALSE;
    VGS_HANDLE hVgsHandle;
    IVE_HANDLE hIveHandle;
    RECT_S  astRect[MAX_POINT_NUM];
	IVE_DMA_CTRL_S stDmaCtrl;
    IVE_ST_CORNER_INFO_S* pstCornerInfo = (IVE_ST_CORNER_INFO_S*)pstStLk->stCorner.pu8VirAddr;
	IVE_POINT_S25Q7_S *psts25q7NextPts = (IVE_POINT_S25Q7_S *) pstStLk->stNextPts.pu8VirAddr;

	memset(&stDmaCtrl,0,sizeof(stDmaCtrl));
	stDmaCtrl.enMode = IVE_DMA_MODE_DIRECT_COPY;

    for (i = 0; i < u32FrameNum; i++)
    {
        SAMPLE_PRT("Proc frame %d\n", i);
        s32Ret = SAMPLE_COMM_IVE_ReadFile(&pstStLk->stSrcYuv, pstStLk->pFpSrc);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("SAMPLE_COMM_IVE_ReadFile fail\n");
            break;
        }
		s32Ret = SAMPLE_IVE_St_Lk_DMA(&hIveHandle,&pstStLk->stSrcYuv,&pstStLk->astNextPyr[0],&stDmaCtrl,HI_FALSE);
		if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("SAMPLE_IVE_St_Lk_DMA fail\n");
            break;
        }
		//buid pyr
		for (k = 1; k <= pstStLk->stLkPyrCtrl.u8MaxLevel; k++)
		{
			s32Ret = SAMPLE_IVE_St_Lk_PyrDown(pstStLk, &pstStLk->astNextPyr[k - 1], &pstStLk->astNextPyr[k]);
			if (s32Ret != HI_SUCCESS)
			{
				SAMPLE_PRT("SAMPLE_IVE_St_Lk_PyrDown fail\n");
				return;
			}
		
		}

        if (0 == i)
        {
            s32Ret = HI_MPI_IVE_STCandiCorner(&hIveHandle, &pstStLk->astNextPyr[0], &pstStLk->stStDst,
                                              &pstStLk->stStCandiCornerCtrl, HI_TRUE);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("HI_MPI_IVE_STCandiCorner fail,Error(%#x)\n", s32Ret);
                break;
            }

            s32Ret = HI_MPI_IVE_Query(hIveHandle, &bFinish, bBlock);
            while (HI_ERR_IVE_QUERY_TIMEOUT == s32Ret)
            {
                usleep(100);
                s32Ret = HI_MPI_IVE_Query(hIveHandle, &bFinish, bBlock);
            }
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("HI_MPI_IVE_Query fail,Error(%#x)\n", s32Ret);
                break;
            }

            s32Ret = HI_MPI_IVE_STCorner(&pstStLk->stStDst, &pstStLk->stCorner, &pstStLk->stStCornerCtrl);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("HI_MPI_IVE_STCorner fail,Error(%#x)\n", s32Ret);
                break;
            }

            pstStLk->stLkPyrCtrl.u16PtsNum = pstCornerInfo->u16CornerNum;
            for (k = 0; k < pstStLk->stLkPyrCtrl.u16PtsNum; k++)
            {
                psts25q7NextPts[k].s25q7X = (HI_S32)(pstCornerInfo->astCorner[k].u16X << 7);
                psts25q7NextPts[k].s25q7Y = (HI_S32)(pstCornerInfo->astCorner[k].u16Y << 7);
                //psts25q7NextPts[k].s25q7X = psts25q7PrevPts[k].s25q7X;
                //psts25q7NextPts[k].s25q7Y = psts25q7PrevPts[k].s25q7Y;
            }
			

        }
		else
		{
			s32Ret = HI_MPI_IVE_LKOpticalFlowPyr(&hIveHandle, 
					pstStLk->astPrevPyr, pstStLk->astNextPyr,
					&pstStLk->stPrevPts, &pstStLk->stNextPts, 
					&pstStLk->stStatus, &pstStLk->stErr,
					&pstStLk->stLkPyrCtrl,HI_TRUE);
			if (s32Ret != HI_SUCCESS)
			{
				SAMPLE_PRT("HI_MPI_IVE_STCandiCorner fail,Error(%#x)\n", s32Ret);
				break;
			}

			s32Ret = HI_MPI_IVE_Query(hIveHandle, &bFinish, bBlock);
			while (HI_ERR_IVE_QUERY_TIMEOUT == s32Ret)
			{
				usleep(100);
				s32Ret = HI_MPI_IVE_Query(hIveHandle, &bFinish, bBlock);
			}
			if (s32Ret != HI_SUCCESS)
			{
				SAMPLE_PRT("HI_MPI_IVE_Query fail,Error(%#x)\n", s32Ret);
				break;
			}
			u16RectNum = 0;
			for (k = 0; k < pstStLk->stLkPyrCtrl.u16PtsNum; k++)
			{				
				if(! pstStLk->stStatus.pu8VirAddr[k])
				{
					continue;
				}
				psts25q7NextPts[u16RectNum].s25q7X = psts25q7NextPts[k].s25q7X;				
				psts25q7NextPts[u16RectNum].s25q7Y = psts25q7NextPts[k].s25q7Y;				
				astRect[u16RectNum].s32X = psts25q7NextPts[k].s25q7X / 128 - 1;
                astRect[u16RectNum].s32Y = psts25q7NextPts[k].s25q7Y / 128 - 1;
                astRect[u16RectNum].u32Width = 4;
                astRect[u16RectNum].u32Height = 4;
				u16RectNum++;				
			}

			if (u16RectNum > 0)
            {
                s32Ret =  HI_MPI_VGS_BeginJob(&hVgsHandle);
                if (s32Ret != HI_SUCCESS)
                {
                    SAMPLE_PRT("HI_MPI_VGS_BeginJob fail,Error(%#x)\n", s32Ret);
                    break;
                }
                s32Ret = SAMPLE_COMM_VGS_AddDrawRectJob(hVgsHandle, &pstStLk->stSrcYuv, &pstStLk->stSrcYuv, astRect, u16RectNum);
                if (s32Ret != HI_SUCCESS)
                {
                    SAMPLE_PRT("SAMPLE_COMM_VGS_AddDrawRectJob fail\n");
                    break;
                }
                s32Ret = HI_MPI_VGS_EndJob(hVgsHandle);
                if (s32Ret != HI_SUCCESS)
                {
                    HI_MPI_VGS_CancelJob(hVgsHandle);
                    SAMPLE_PRT("HI_MPI_VGS_EndJob fail,Error(%#x)\n", s32Ret);
                    break;
                }
            }

			pstStLk->stLkPyrCtrl.u16PtsNum = u16RectNum;		
			
			SAMPLE_COMM_IVE_WriteFile(&pstStLk->stSrcYuv, pstStLk->pFpDst);

		} 
		
	    memcpy(pstStLk->stPrevPts.pu8VirAddr,pstStLk->stNextPts.pu8VirAddr,
			sizeof(IVE_POINT_S25Q7_S) * pstStLk->stLkPyrCtrl.u16PtsNum);

        SAMPLE_IVE_St_Lk_CopyPyr(pstStLk->astNextPyr, pstStLk->astPrevPyr, pstStLk->stLkPyrCtrl.u8MaxLevel);
    }
    return;

}
/******************************************************************************
* function : show St Lk sample
******************************************************************************/
HI_VOID SAMPLE_IVE_St_Lk(HI_VOID)
{
    HI_CHAR* pchSrcFileName = "./data/input/stlk/st_lk_720x576_420sp.yuv";
    HI_CHAR* pchDstFileName = "./data/output/stlk/st_lk_720x576_420sp_result.yuv";
    HI_U16 u16Width = 720;
    HI_U16 u16Height = 576;
	HI_U16 u16PyrWidth = 720;
	HI_U16 u16PyrHeight = 576;
	
    HI_S32 s32Ret;
	HI_U8 u8MaxLevel = 3;

	memset(&s_stStLk,0,sizeof(s_stStLk));
    SAMPLE_COMM_IVE_CheckIveMpiInit();

    s32Ret =  SAMPLE_IVE_St_Lk_Init(&s_stStLk,pchSrcFileName, pchDstFileName,
                                    u16Width, u16Height,u16PyrWidth,u16PyrHeight, u8MaxLevel);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_IVE_St_Lk_Init fail\n");
        goto ST_LK_FAIL;
    }

    SAMPLE_IVE_St_LkProc(&s_stStLk);

    SAMPLE_IVE_St_Lk_Uninit(&s_stStLk);
	memset(&s_stStLk,0,sizeof(s_stStLk));

ST_LK_FAIL:
    SAMPLE_COMM_IVE_IveMpiExit();
}

/******************************************************************************
* function : St_Lk sample signal handle
******************************************************************************/
HI_VOID SAMPLE_IVE_St_Lk_HandleSig(HI_VOID)
{
    SAMPLE_IVE_St_Lk_Uninit(&s_stStLk);
	memset(&s_stStLk,0,sizeof(s_stStLk));
    SAMPLE_COMM_IVE_IveMpiExit();
}



