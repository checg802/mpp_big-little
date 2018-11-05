#include "hi_common.h"
#include "hi_comm_video.h"
#include "hi_comm_sys.h"
#include "hi_comm_ive.h"

#include "mpi_vb.h"
#include "mpi_sys.h"
#include "mpi_ive.h"
#include <math.h>

#include "sample_comm_ive.h"

#define PROCESS_NUM     10

typedef struct hiSAMPLE_IVE_CNN_INFO_S
{
	IVE_CNN_MODEL_S  stCnnModel;
    IVE_SRC_IMAGE_S  astSrc[PROCESS_NUM];
   	IVE_CNN_CTRL_S    stCnnCtrl;
    IVE_DST_DATA_S    stFcDst;
    IVE_DST_MEM_INFO_S stRsltDst;

} SAMPLE_IVE_CNN_INFO_S;

static SAMPLE_IVE_CNN_INFO_S s_stCnnInfo;

/******************************************************************************
* function : Cnn uninit
******************************************************************************/
static HI_VOID SAMPLE_IVE_Cnn_Uninit(SAMPLE_IVE_CNN_INFO_S *pstCnnInfo)
{
    HI_U32 i;

    (HI_VOID)HI_MPI_IVE_CNN_UnloadModel(&pstCnnInfo->stCnnModel);

    for (i = 0; i < PROCESS_NUM; i++)
    {
        IVE_MMZ_FREE(pstCnnInfo->astSrc[i].u32PhyAddr[0], pstCnnInfo->astSrc[i].pu8VirAddr[0]);
    }

     IVE_MMZ_FREE(pstCnnInfo->stCnnCtrl.stMem.u32PhyAddr, pstCnnInfo->stCnnCtrl.stMem.pu8VirAddr);
     IVE_MMZ_FREE(pstCnnInfo->stFcDst.u32PhyAddr, pstCnnInfo->stFcDst.pu8VirAddr);
     IVE_MMZ_FREE(pstCnnInfo->stRsltDst.u32PhyAddr, pstCnnInfo->stRsltDst.pu8VirAddr);   

}
/******************************************************************************
* function : Get cnn assist size
******************************************************************************/
static HI_U32 SAMPLE_IVE_Cnn_GetAssistSize(IVE_CNN_MODEL_S* pstCnnModel, HI_U8 u8BlockSize)
{
	HI_U32 u32ArgListBufferSize;
	HI_U32 u32CnnDistBufferSize;
	HI_U8 u8InputChn = 1;
	switch(pstCnnModel->enType)
	{
	case IVE_IMAGE_TYPE_U8C1:
		{
			u8InputChn = 1;
		}
		break;
	case IVE_IMAGE_TYPE_U8C3_PLANAR:
		{
			u8InputChn = 3;
		}
		break;
	default:
		break;
	}
	u32ArgListBufferSize = u8InputChn *u8BlockSize * sizeof(HI_U32);
	u32ArgListBufferSize = SAMPLE_COMM_IVE_CalcStride(u32ArgListBufferSize, IVE_ALIGN);   
	u32CnnDistBufferSize = SAMPLE_COMM_IVE_CalcStride(pstCnnModel->stFullConnect.au16LayerCnt[0] * sizeof(HI_U32), IVE_ALIGN) * u8BlockSize;

	return (u32ArgListBufferSize+u32CnnDistBufferSize);
}

/******************************************************************************
* function : Cnn init
******************************************************************************/
static HI_S32 SAMPLE_IVE_Cnn_Init(SAMPLE_IVE_CNN_INFO_S *pstCnnInfo, HI_CHAR *pchBinFileName)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32Size, i;
    HI_U32 u32CtrlBufferSize = 0;  
    HI_U16 u16ProcNum = PROCESS_NUM;
	HI_U16 u16Width;
	HI_U16 u16Height;
	
    memset(pstCnnInfo, 0, sizeof(SAMPLE_IVE_CNN_INFO_S));
    s32Ret = HI_MPI_IVE_CNN_LoadModel(pchBinFileName, &(pstCnnInfo->stCnnModel));
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("HI_MPI_IVE_CNN_LoadModel fail,Error(%#x)\n", s32Ret);
        goto CNN_INIT_FAIL;
    }

    u16Width = pstCnnInfo->stCnnModel.u16Width;
    u16Height = pstCnnInfo->stCnnModel.u16Height;

	for (i = 0; i < u16ProcNum; i++)
	{
		s32Ret = SAMPLE_COMM_IVE_CreateImage(&(pstCnnInfo->astSrc[i]), IVE_IMAGE_TYPE_U8C1, u16Width, u16Height);
	    if (s32Ret != HI_SUCCESS)
	    {
	        SAMPLE_PRT("SAMPLE_COMM_IVE_CreateImage fail\n");
	        goto CNN_INIT_FAIL;
	    }
	}

    u32CtrlBufferSize = SAMPLE_IVE_Cnn_GetAssistSize(&(pstCnnInfo->stCnnModel), u16ProcNum);
    s32Ret = SAMPLE_COMM_IVE_CreateMemInfo( &(pstCnnInfo->stCnnCtrl.stMem), u32CtrlBufferSize );
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("SAMPLE_COMM_IVE_CreateMemInfo fail\n");
        goto CNN_INIT_FAIL;
    }

    pstCnnInfo->stCnnCtrl.u32Num = u16ProcNum;
    u32Size = pstCnnInfo->stCnnModel.stFullConnect.au16LayerCnt[pstCnnInfo->stCnnModel.stFullConnect.u8LayerNum - 1] * sizeof(HI_S16Q16);
    s32Ret = SAMPLE_COMM_IVE_CreateData(&(pstCnnInfo->stFcDst), u32Size, u16ProcNum);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("SAMPLE_COMM_IVE_CreateData fail\n");
        goto CNN_INIT_FAIL;
    }


    u32Size = u16ProcNum * sizeof(IVE_CNN_RESULT_S);
    s32Ret = SAMPLE_COMM_IVE_CreateMemInfo( &(pstCnnInfo->stRsltDst), u32Size );
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("SAMPLE_COMM_IVE_CreateMemInfo fail\n");
        goto CNN_INIT_FAIL;
    }

CNN_INIT_FAIL:

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_IVE_Cnn_Uninit(pstCnnInfo);
    }

    return s32Ret;

}

/******************************************************************************
* function : Cnn predict
******************************************************************************/
HI_VOID SAMPLE_IVE_Cnn_Char_Predict(SAMPLE_IVE_CNN_INFO_S* pstCnnInfo)
{
    IVE_HANDLE iveHandle;
	HI_S32 s32Ret;
	HI_U32 i;
    HI_BOOL bInstant = HI_TRUE;
    HI_BOOL bFinish;
    HI_BOOL bBlock = HI_TRUE;
	HI_BOOL bIsSuccess = HI_TRUE;	

    FILE* pFp = HI_NULL;
	IVE_CNN_RESULT_S *pstRsltDst = NULL;
    HI_CHAR achFileName[IVE_FILE_NAME_LEN];

    for (i = 0; i < pstCnnInfo->stCnnCtrl.u32Num; i++)
    {
        snprintf(achFileName, sizeof(achFileName), "./data/input/cnn/%d.yuv", i);
        pFp = fopen(achFileName, "rb");
        if (NULL == pFp)
        {
            SAMPLE_PRT("Open file %s fail\n", achFileName);
			bIsSuccess = HI_FALSE;
            break;
        }
        s32Ret = SAMPLE_COMM_IVE_ReadFile(&(pstCnnInfo->astSrc[i]), pFp);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("SAMPLE_COMM_IVE_ReadFile fail, file %s\n", achFileName);
            IVE_CLOSE_FILE(pFp);			
			bIsSuccess = HI_FALSE;
            break;
        }
        IVE_CLOSE_FILE(pFp);
    }
	
	if (HI_TRUE == bIsSuccess)
	{
		s32Ret = HI_MPI_IVE_CNN_Predict(&iveHandle, pstCnnInfo->astSrc, &pstCnnInfo->stCnnModel, 
				&pstCnnInfo->stFcDst, &pstCnnInfo->stCnnCtrl, bInstant);		
		if (s32Ret != HI_SUCCESS)
		{
			SAMPLE_PRT("HI_MPI_IVE_CNN_Predict fail,Error(%#x)\n", s32Ret);
			return ;
		}
		s32Ret = HI_MPI_IVE_Query(iveHandle, &bFinish, bBlock);
		while (HI_ERR_IVE_QUERY_TIMEOUT == s32Ret)
		{
			usleep(100);
			s32Ret = HI_MPI_IVE_Query(iveHandle, &bFinish, bBlock);
		}
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("HI_MPI_IVE_Query fail,Error(%#x)\n", s32Ret);
			return;
		}

		s32Ret = HI_MPI_IVE_CNN_GetResult(&pstCnnInfo->stFcDst, &pstCnnInfo->stRsltDst, 
			&pstCnnInfo->stCnnModel, &pstCnnInfo->stCnnCtrl);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("HI_MPI_IVE_CNN_GetResult fail,Error(%#x)\n", s32Ret);
			return;
		}		
	
		pstRsltDst = (IVE_CNN_RESULT_S *)pstCnnInfo->stRsltDst.pu8VirAddr;
		for (i = 0; i < pstCnnInfo->stFcDst.u16Height; i++)
		{
			SAMPLE_PRT("%d.yuv belong to class %d, confidence %d.\n", i, 
				pstRsltDst[i].s32ClassIdx, pstRsltDst[i].s32Confidence);
		}

	}
    
}


/******************************************************************************
* function : show ann sample
******************************************************************************/
HI_VOID SAMPLE_IVE_Cnn(HI_VOID)
{
    HI_S32 s32Ret;
    HI_CHAR* pchBinFileName = "./data/input/cnn/cnn_20x40i1chn10o_v5.bin";

	memset(&s_stCnnInfo,0,sizeof(s_stCnnInfo));
    SAMPLE_COMM_IVE_CheckIveMpiInit();

    //init
    s32Ret = SAMPLE_IVE_Cnn_Init(&s_stCnnInfo, pchBinFileName);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_IVE_Cnn_Init fail\n");
        goto CNN_FAIL;
    }
    //predict
    SAMPLE_IVE_Cnn_Char_Predict(&s_stCnnInfo);
    //uninit
    SAMPLE_IVE_Cnn_Uninit(&s_stCnnInfo);
	memset(&s_stCnnInfo,0,sizeof(s_stCnnInfo));

CNN_FAIL:
    SAMPLE_COMM_IVE_IveMpiExit();
}
/******************************************************************************
* function : Cnn sample signal handle
******************************************************************************/
HI_VOID SAMPLE_IVE_Cnn_HandleSig(HI_VOID)
{
    SAMPLE_IVE_Cnn_Uninit(&s_stCnnInfo);
	memset(&s_stCnnInfo,0,sizeof(s_stCnnInfo));
    SAMPLE_COMM_IVE_IveMpiExit();
}

