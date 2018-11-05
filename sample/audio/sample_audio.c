/******************************************************************************
  A simple program of Hisilicon mpp audio input/output/encoder/decoder implementation.
  Copyright (C), 2010-2021, Hisilicon Tech. Co., Ltd.
 ******************************************************************************
    Modification:  2013-7 Created
******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>

#include "sample_comm.h"
#include "acodec.h"
//#include "tlv320aic31.h"


static PAYLOAD_TYPE_E gs_enPayloadType = PT_ADPCMA;



static HI_BOOL gs_bAioReSample  = HI_FALSE;
static HI_BOOL gs_bUserGetMode  = HI_FALSE;
static HI_BOOL gs_bAoVolumeCtrl = HI_FALSE;
static AUDIO_SAMPLE_RATE_E enInSampleRate  = AUDIO_SAMPLE_RATE_BUTT;
static AUDIO_SAMPLE_RATE_E enOutSampleRate = AUDIO_SAMPLE_RATE_BUTT;
static HI_U32 u32AencPtNumPerFrm = 0;
/* 0: close, 1: talk, 2: hifi, 3: record*/
static HI_U32 u32AiVqeType = 2;  
/* 0: close, 1: open*/
static HI_U32 u32AoVqeType = 0;  


#define SAMPLE_DBG(s32Ret)\
    do{\
        printf("s32Ret=%#x,fuc:%s,line:%d\n", s32Ret, __FUNCTION__, __LINE__);\
    }while(0)

/******************************************************************************
* function : PT Number to String
******************************************************************************/
static char* SAMPLE_AUDIO_Pt2Str(PAYLOAD_TYPE_E enType)
{
    if (PT_G711A == enType)
    {
        return "g711a";
    }
    else if (PT_G711U == enType)
    {
        return "g711u";
    }
    else if (PT_ADPCMA == enType)
    {
        return "adpcm";
    }
    else if (PT_G726 == enType)
    {
        return "g726";
    }
    else if (PT_LPCM == enType)
    {
        return "pcm";
    }
    else
    {
        return "data";
    }
}

/******************************************************************************
* function : Open Aenc File
******************************************************************************/
static FILE* SAMPLE_AUDIO_OpenAencFile(AENC_CHN AeChn, PAYLOAD_TYPE_E enType)
{
    FILE* pfd;
    HI_CHAR aszFileName[FILE_NAME_LEN];

    /* create file for save stream*/
    snprintf(aszFileName, FILE_NAME_LEN, "audio_chn%d.%s", AeChn, SAMPLE_AUDIO_Pt2Str(enType));
    pfd = fopen(aszFileName, "w+");
    if (NULL == pfd)
    {
        printf("%s: open file %s failed\n", __FUNCTION__, aszFileName);
        return NULL;
    }
    printf("open stream file:\"%s\" for aenc ok\n", aszFileName);
    return pfd;
}

/******************************************************************************
* function : Open Adec File
******************************************************************************/
static FILE* SAMPLE_AUDIO_OpenAdecFile(ADEC_CHN AdChn, PAYLOAD_TYPE_E enType)
{
    FILE* pfd;
    HI_CHAR aszFileName[FILE_NAME_LEN];

    /* create file for save stream*/
    snprintf(aszFileName, FILE_NAME_LEN ,"audio_chn%d.%s", AdChn, SAMPLE_AUDIO_Pt2Str(enType));
    pfd = fopen(aszFileName, "rb");
    if (NULL == pfd)
    {
        printf("%s: open file %s failed\n", __FUNCTION__, aszFileName);
        return NULL;
    }
    printf("open stream file:\"%s\" for adec ok\n", aszFileName);
    return pfd;
}


/******************************************************************************
* function : file -> Adec -> Ao
******************************************************************************/
HI_S32 SAMPLE_AUDIO_AdecAo(HI_VOID)
{
    HI_S32      s32Ret;
    AUDIO_DEV   AoDev = SAMPLE_AUDIO_AO_DEV;
    AO_CHN      AoChn = 0;
    ADEC_CHN    AdChn = 0;
    HI_S32      s32AoChnCnt;
    FILE*        pfd = NULL;
    AIO_ATTR_S stAioAttr;

#ifdef HI_ACODEC_TYPE_TLV320AIC31
    stAioAttr.enSamplerate   = AUDIO_SAMPLE_RATE_8000;
    stAioAttr.enBitwidth     = AUDIO_BIT_WIDTH_16;
    stAioAttr.enWorkmode     = AIO_MODE_I2S_MASTER;
    stAioAttr.enSoundmode    = AUDIO_SOUND_MODE_MONO;
    stAioAttr.u32EXFlag      = 0;
    stAioAttr.u32FrmNum      = 30;
    stAioAttr.u32PtNumPerFrm = SAMPLE_AUDIO_PTNUMPERFRM;
    stAioAttr.u32ChnCnt      = 1;
    stAioAttr.u32ClkSel      = 1;
#else
    stAioAttr.enSamplerate   = AUDIO_SAMPLE_RATE_8000;
    stAioAttr.enBitwidth     = AUDIO_BIT_WIDTH_16;
    stAioAttr.enWorkmode     = AIO_MODE_I2S_MASTER;
    stAioAttr.enSoundmode    = AUDIO_SOUND_MODE_MONO;
    stAioAttr.u32EXFlag      = 0;
    stAioAttr.u32FrmNum      = 30;
    stAioAttr.u32PtNumPerFrm = SAMPLE_AUDIO_PTNUMPERFRM;
    stAioAttr.u32ChnCnt      = 1;
    stAioAttr.u32ClkSel      = 0;
#endif

    gs_bAioReSample = HI_FALSE;
    enInSampleRate  = AUDIO_SAMPLE_RATE_BUTT;
    enOutSampleRate = AUDIO_SAMPLE_RATE_BUTT;
	
    s32Ret = SAMPLE_COMM_AUDIO_CfgAcodec(&stAioAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_DBG(s32Ret);
        goto ADECAO_ERR3;
    }

    s32Ret = SAMPLE_COMM_AUDIO_StartAdec(AdChn, gs_enPayloadType);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
        goto ADECAO_ERR3;
    }

    s32AoChnCnt = stAioAttr.u32ChnCnt;
    s32Ret = SAMPLE_COMM_AUDIO_StartAo(AoDev, s32AoChnCnt, &stAioAttr, enInSampleRate, gs_bAioReSample, NULL, 0);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
        goto ADECAO_ERR2;
    }

    s32Ret = SAMPLE_COMM_AUDIO_AoBindAdec(AoDev, AoChn, AdChn);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
        goto ADECAO_ERR1;
    }

    pfd = SAMPLE_AUDIO_OpenAdecFile(AdChn, gs_enPayloadType);
    if (!pfd)
    {
        SAMPLE_DBG(HI_FAILURE);
        goto ADECAO_ERR0;
    }
    s32Ret = SAMPLE_COMM_AUDIO_CreatTrdFileAdec(AdChn, pfd);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
        goto ADECAO_ERR0;
    }

    printf("bind adec:%d to ao(%d,%d) ok \n", AdChn, AoDev, AoChn);

    printf("\nplease press twice ENTER to exit this sample\n");
    getchar();
    getchar();

    s32Ret = SAMPLE_COMM_AUDIO_DestoryTrdFileAdec(AdChn);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
        return HI_FAILURE;
    }

ADECAO_ERR0:
	s32Ret = SAMPLE_COMM_AUDIO_AoUnbindAdec(AoDev, AoChn, AdChn);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
    }
    
ADECAO_ERR1:
    s32Ret |= SAMPLE_COMM_AUDIO_StopAo(AoDev, s32AoChnCnt, gs_bAioReSample, HI_FALSE);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
    }
ADECAO_ERR2:
    s32Ret |= SAMPLE_COMM_AUDIO_StopAdec(AdChn);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
    }
	
ADECAO_ERR3:
    return s32Ret;
}

/******************************************************************************
* function : Ai -> Aenc -> file
*                       -> Adec -> Ao
******************************************************************************/
HI_S32 SAMPLE_AUDIO_AiAenc(HI_VOID)
{
    HI_S32 i, j, s32Ret;
    AUDIO_DEV   AiDev = SAMPLE_AUDIO_AI_DEV;
    AI_CHN      AiChn;
    AUDIO_DEV   AoDev = SAMPLE_AUDIO_AO_DEV;
    AO_CHN      AoChn = 0;
    ADEC_CHN    AdChn = 0;
    HI_S32      s32AiChnCnt;
    HI_S32      s32AoChnCnt;
    HI_S32      s32AencChnCnt;
    AENC_CHN    AeChn;
    HI_BOOL     bSendAdec = HI_TRUE;
    FILE*        pfd = NULL;
    AIO_ATTR_S stAioAttr;

#ifdef HI_ACODEC_TYPE_TLV320AIC31
    stAioAttr.enSamplerate   = AUDIO_SAMPLE_RATE_8000;
    stAioAttr.enBitwidth     = AUDIO_BIT_WIDTH_16;
    stAioAttr.enWorkmode     = AIO_MODE_I2S_MASTER;
    stAioAttr.enSoundmode    = AUDIO_SOUND_MODE_MONO;
    stAioAttr.u32EXFlag      = 0;
    stAioAttr.u32FrmNum      = 30;
    stAioAttr.u32PtNumPerFrm = SAMPLE_AUDIO_PTNUMPERFRM;
    stAioAttr.u32ChnCnt      = 1;
    stAioAttr.u32ClkSel      = 1;
#else
    stAioAttr.enSamplerate   = AUDIO_SAMPLE_RATE_8000;
    stAioAttr.enBitwidth     = AUDIO_BIT_WIDTH_16;
    stAioAttr.enWorkmode     = AIO_MODE_I2S_MASTER;
    stAioAttr.enSoundmode    = AUDIO_SOUND_MODE_MONO;
    stAioAttr.u32EXFlag      = 0;
    stAioAttr.u32FrmNum      = 30;
    stAioAttr.u32PtNumPerFrm = SAMPLE_AUDIO_PTNUMPERFRM;
    stAioAttr.u32ChnCnt      = 1;
    stAioAttr.u32ClkSel      = 0;
#endif
    gs_bAioReSample = HI_FALSE;
    enInSampleRate  = AUDIO_SAMPLE_RATE_BUTT;
    enOutSampleRate = AUDIO_SAMPLE_RATE_BUTT;
    u32AencPtNumPerFrm = stAioAttr.u32PtNumPerFrm;

    /********************************************
      step 1: config audio codec
    ********************************************/
    s32Ret = SAMPLE_COMM_AUDIO_CfgAcodec(&stAioAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_DBG(s32Ret);
        goto AIAENC_ERR6;
    }

    /********************************************
      step 2: start Ai
    ********************************************/
    s32AiChnCnt = stAioAttr.u32ChnCnt;
    s32Ret = SAMPLE_COMM_AUDIO_StartAi(AiDev, s32AiChnCnt, &stAioAttr, enOutSampleRate, gs_bAioReSample, NULL, 0);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
        goto AIAENC_ERR6;
    }

    /********************************************
      step 3: start Aenc
    ********************************************/
    s32AencChnCnt = stAioAttr.u32ChnCnt;
    s32Ret = SAMPLE_COMM_AUDIO_StartAenc(s32AencChnCnt, u32AencPtNumPerFrm, gs_enPayloadType);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
        goto AIAENC_ERR5;
    }

    /********************************************
      step 4: Aenc bind Ai Chn
    ********************************************/
    for (i = 0; i < s32AencChnCnt; i++)
    {
        AeChn = i;
        AiChn = i;

        if (HI_TRUE == gs_bUserGetMode)
        {
            s32Ret = SAMPLE_COMM_AUDIO_CreatTrdAiAenc(AiDev, AiChn, AeChn);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_DBG(s32Ret);
				for (j=0; j<i; j++)
				{
					SAMPLE_COMM_AUDIO_DestoryTrdAi(AiDev, j);
				}
                goto AIAENC_ERR4;
            }
        }
        else
        {
            s32Ret = SAMPLE_COMM_AUDIO_AencBindAi(AiDev, AiChn, AeChn);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_DBG(s32Ret);
				for (j=0; j<i; j++)
				{
					SAMPLE_COMM_AUDIO_AencUnbindAi(AiDev, j, j);
				}
                goto AIAENC_ERR4;
            }
        }
        printf("Ai(%d,%d) bind to AencChn:%d ok!\n", AiDev , AiChn, AeChn);
    }

    /********************************************
      step 5: start Adec & Ao. ( if you want )
    ********************************************/
    if (HI_TRUE == bSendAdec)
    {
        s32Ret = SAMPLE_COMM_AUDIO_StartAdec(AdChn, gs_enPayloadType);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_DBG(s32Ret);
            goto AIAENC_ERR3;
        }

        s32AoChnCnt = stAioAttr.u32ChnCnt;
        s32Ret = SAMPLE_COMM_AUDIO_StartAo(AoDev, s32AoChnCnt, &stAioAttr, enInSampleRate, gs_bAioReSample, NULL , 0);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_DBG(s32Ret);
            goto AIAENC_ERR2;
        }

        pfd = SAMPLE_AUDIO_OpenAencFile(AdChn, gs_enPayloadType);
        if (!pfd)
        {
            SAMPLE_DBG(HI_FAILURE);
            goto AIAENC_ERR1;
        }
        s32Ret = SAMPLE_COMM_AUDIO_CreatTrdAencAdec(AeChn, AdChn, pfd);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_DBG(s32Ret);
            goto AIAENC_ERR1;
        }

        s32Ret = SAMPLE_COMM_AUDIO_AoBindAdec(AoDev, AoChn, AdChn);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_DBG(s32Ret);
            goto AIAENC_ERR0;
        }

        printf("bind adec:%d to ao(%d,%d) ok \n", AdChn, AoDev, AoChn);
    }

    printf("\nplease press twice ENTER to exit this sample\n");
    getchar();
    getchar();

    /********************************************
      step 6: exit the process
    ********************************************/
    if (HI_TRUE == bSendAdec)
    {
    	s32Ret = SAMPLE_COMM_AUDIO_AoUnbindAdec(AoDev, AoChn, AdChn);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_DBG(s32Ret);
            return HI_FAILURE;
        }
		
AIAENC_ERR0:
		s32Ret = SAMPLE_COMM_AUDIO_DestoryTrdAencAdec(AdChn);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_DBG(s32Ret);
        }        

AIAENC_ERR1:
		s32Ret |= SAMPLE_COMM_AUDIO_StopAo(AoDev, s32AoChnCnt, gs_bAioReSample, HI_FALSE);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_DBG(s32Ret);
        }        

AIAENC_ERR2:
        s32Ret |= SAMPLE_COMM_AUDIO_StopAdec(AdChn);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_DBG(s32Ret);
        }

    }

AIAENC_ERR3:
    for (i = 0; i < s32AencChnCnt; i++)
    {
        AeChn = i;
        AiChn = i;

        if (HI_TRUE == gs_bUserGetMode)
        {
            s32Ret |= SAMPLE_COMM_AUDIO_DestoryTrdAi(AiDev, AiChn);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_DBG(s32Ret);
            }
        }
        else
        {
            s32Ret |= SAMPLE_COMM_AUDIO_AencUnbindAi(AiDev, AiChn, AeChn);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_DBG(s32Ret);
            }
        }
    }

AIAENC_ERR4:
    s32Ret |= SAMPLE_COMM_AUDIO_StopAenc(s32AencChnCnt);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
    }

AIAENC_ERR5:
    s32Ret |= SAMPLE_COMM_AUDIO_StopAi(AiDev, s32AiChnCnt, gs_bAioReSample, HI_FALSE);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
    }

AIAENC_ERR6:

    return s32Ret;
}

/******************************************************************************
* function : Ai -> Ao(with fade in/out and volume adjust)
******************************************************************************/
HI_S32 SAMPLE_AUDIO_AiAo(HI_VOID)
{
    HI_S32 s32Ret;
    HI_S32 s32AiChnCnt;
    HI_S32 s32AoChnCnt;
    AUDIO_DEV   AiDev = SAMPLE_AUDIO_AI_DEV;
    AI_CHN      AiChn = 0;
    AUDIO_DEV   AoDev = SAMPLE_AUDIO_AO_DEV;
    AO_CHN      AoChn = 0;
    AIO_ATTR_S stAioAttr;

#ifdef HI_ACODEC_TYPE_TLV320AIC31
    stAioAttr.enSamplerate   = AUDIO_SAMPLE_RATE_8000;
    stAioAttr.enBitwidth     = AUDIO_BIT_WIDTH_16;
    stAioAttr.enWorkmode     = AIO_MODE_I2S_MASTER;
    stAioAttr.enSoundmode    = AUDIO_SOUND_MODE_MONO;
    stAioAttr.u32EXFlag      = 0;
    stAioAttr.u32FrmNum      = 30;
    stAioAttr.u32PtNumPerFrm = SAMPLE_AUDIO_PTNUMPERFRM;
    stAioAttr.u32ChnCnt      = 1;
    stAioAttr.u32ClkSel      = 1;
#else //  inner acodec
    stAioAttr.enSamplerate   = AUDIO_SAMPLE_RATE_8000;
    stAioAttr.enBitwidth     = AUDIO_BIT_WIDTH_16;
    stAioAttr.enWorkmode     = AIO_MODE_I2S_MASTER;
    stAioAttr.enSoundmode    = AUDIO_SOUND_MODE_MONO;
    stAioAttr.u32EXFlag      = 0;
    stAioAttr.u32FrmNum      = 30;
    stAioAttr.u32PtNumPerFrm = SAMPLE_AUDIO_PTNUMPERFRM;
    stAioAttr.u32ChnCnt      = 1;
    stAioAttr.u32ClkSel      = 0;
#endif
    /* config ao resample attr if needed */
    if (HI_TRUE == gs_bAioReSample)
    {
        stAioAttr.enSamplerate   = AUDIO_SAMPLE_RATE_32000;
        stAioAttr.u32PtNumPerFrm = SAMPLE_AUDIO_PTNUMPERFRM * 4;

        /* ai 32k -> 8k */
        enOutSampleRate = AUDIO_SAMPLE_RATE_8000;

        /* ao 8k -> 32k */
        enInSampleRate  = AUDIO_SAMPLE_RATE_8000;
    }
    else
    {
        enInSampleRate  = AUDIO_SAMPLE_RATE_BUTT;
        enOutSampleRate = AUDIO_SAMPLE_RATE_BUTT;
    }

    /* resample and anr should be user get mode */
    gs_bUserGetMode = (HI_TRUE == gs_bAioReSample) ? HI_TRUE : HI_FALSE;
    
    /* config internal audio codec */
    s32Ret = SAMPLE_COMM_AUDIO_CfgAcodec(&stAioAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_DBG(s32Ret);
        goto AIAO_ERR3;
    }
    
    /* enable AI channle */
    s32AiChnCnt = stAioAttr.u32ChnCnt;
    s32Ret = SAMPLE_COMM_AUDIO_StartAi(AiDev, s32AiChnCnt, &stAioAttr, enOutSampleRate, gs_bAioReSample, NULL, 0);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
        goto AIAO_ERR3;
    }

    /* enable AO channle */
    s32AoChnCnt = stAioAttr.u32ChnCnt;
    s32Ret = SAMPLE_COMM_AUDIO_StartAo(AoDev, s32AoChnCnt, &stAioAttr, enInSampleRate, gs_bAioReSample, NULL, 0);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
         goto AIAO_ERR2;
    }

    /* bind AI to AO channle */
    if (HI_TRUE == gs_bUserGetMode)
    {
        s32Ret = SAMPLE_COMM_AUDIO_CreatTrdAiAo(AiDev, AiChn, AoDev, AoChn);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_DBG(s32Ret);
             goto AIAO_ERR1;
        }
    }
    else
    {
        s32Ret = SAMPLE_COMM_AUDIO_AoBindAi(AiDev, AiChn, AoDev, AoChn);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_DBG(s32Ret);
            goto AIAO_ERR1;
        }
    }
    printf("ai(%d,%d) bind to ao(%d,%d) ok\n", AiDev, AiChn, AoDev, AoChn);

    if (HI_TRUE == gs_bAoVolumeCtrl)
    {
        s32Ret = SAMPLE_COMM_AUDIO_CreatTrdAoVolCtrl(AoDev);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_DBG(s32Ret);
            goto AIAO_ERR0;
        }
    }

    printf("\nplease press twice ENTER to exit this sample\n");
    getchar();
    getchar();

    if (HI_TRUE == gs_bAoVolumeCtrl)
    {
        s32Ret = SAMPLE_COMM_AUDIO_DestoryTrdAoVolCtrl(AoDev);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_DBG(s32Ret);
            return HI_FAILURE;
        }
	}
	
AIAO_ERR0:

    if (HI_TRUE == gs_bUserGetMode)
    {
        s32Ret = SAMPLE_COMM_AUDIO_DestoryTrdAi(AiDev, AiChn);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_DBG(s32Ret);
        }
    }
    else
    {
        s32Ret = SAMPLE_COMM_AUDIO_AoUnbindAi(AiDev, AiChn, AoDev, AoChn);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_DBG(s32Ret);
        }
    }
	
AIAO_ERR1:
    
    s32Ret |= SAMPLE_COMM_AUDIO_StopAo(AoDev, s32AoChnCnt, gs_bAioReSample, HI_FALSE);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
    }
	
AIAO_ERR2:
	s32Ret |= SAMPLE_COMM_AUDIO_StopAi(AiDev, s32AiChnCnt, gs_bAioReSample, HI_FALSE);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
    }
	
AIAO_ERR3:

    return s32Ret;
}


/******************************************************************************
* function : Ai -> Ao
******************************************************************************/
HI_S32 SAMPLE_AUDIO_AiVqeProcessAo(HI_VOID)
{
    HI_S32 i, j, s32Ret;
    AUDIO_DEV   AiDev = SAMPLE_AUDIO_AI_DEV;
    AUDIO_DEV   AoDev = SAMPLE_AUDIO_AO_DEV;
    HI_S32      s32AiChnCnt;
    HI_S32      s32AoChnCnt;
    AIO_ATTR_S  stAioAttr;
    AI_TALKVQE_CONFIG_S stAiVqeTalkAttr;
	AI_HIFIVQE_CONFIG_S stAiVqeHifiAttr;
	AI_RECORDVQE_CONFIG_S stAiVqeRecordAttr;
	HI_VOID     *pAiVqeAttr = NULL;
	AO_VQE_CONFIG_S stAoVqeAttr;
	HI_VOID     *pAoVqeAttr = NULL;

#ifdef HI_ACODEC_TYPE_TLV320AIC31
    stAioAttr.enSamplerate   = AUDIO_SAMPLE_RATE_8000;
    stAioAttr.enBitwidth     = AUDIO_BIT_WIDTH_16;
    stAioAttr.enWorkmode     = AIO_MODE_I2S_MASTER;
    stAioAttr.enSoundmode    = AUDIO_SOUND_MODE_MONO;
    stAioAttr.u32EXFlag      = 0;
    stAioAttr.u32FrmNum      = 30;
    stAioAttr.u32PtNumPerFrm = SAMPLE_AUDIO_PTNUMPERFRM;
    stAioAttr.u32ChnCnt      = 1;
    stAioAttr.u32ClkSel      = 1;
#else
    stAioAttr.enSamplerate   = AUDIO_SAMPLE_RATE_8000;
    stAioAttr.enBitwidth     = AUDIO_BIT_WIDTH_16;
    stAioAttr.enWorkmode     = AIO_MODE_I2S_MASTER;
    stAioAttr.enSoundmode    = AUDIO_SOUND_MODE_MONO;
    stAioAttr.u32EXFlag      = 0;
    stAioAttr.u32FrmNum      = 30;
    stAioAttr.u32PtNumPerFrm = SAMPLE_AUDIO_PTNUMPERFRM;
    stAioAttr.u32ChnCnt      = 1;
    stAioAttr.u32ClkSel      = 0;
#endif
    gs_bAioReSample = HI_FALSE;
    enInSampleRate  = AUDIO_SAMPLE_RATE_BUTT;
    enOutSampleRate = AUDIO_SAMPLE_RATE_BUTT;

	if (1 == u32AiVqeType)
    {
        memset(&stAiVqeTalkAttr, 0, sizeof(AI_TALKVQE_CONFIG_S));
	    stAiVqeTalkAttr.s32WorkSampleRate    = AUDIO_SAMPLE_RATE_8000;
	    stAiVqeTalkAttr.s32FrameSample       = SAMPLE_AUDIO_PTNUMPERFRM;
	    stAiVqeTalkAttr.enWorkstate          = VQE_WORKSTATE_COMMON;
	    stAiVqeTalkAttr.stAecCfg.bUsrMode    = HI_FALSE;
	    stAiVqeTalkAttr.stAecCfg.s8CngMode   = 0;
	    stAiVqeTalkAttr.stAgcCfg.bUsrMode    = HI_FALSE;
	    stAiVqeTalkAttr.stAnrCfg.bUsrMode    = HI_FALSE;
		stAiVqeTalkAttr.stHpfCfg.bUsrMode    = HI_TRUE;
	    stAiVqeTalkAttr.stHpfCfg.enHpfFreq   = AUDIO_HPF_FREQ_150;
		stAiVqeTalkAttr.stHdrCfg.bUsrMode    = HI_FALSE;

		stAiVqeTalkAttr.u32OpenMask = AI_TALKVQE_MASK_AEC | AI_TALKVQE_MASK_AGC | AI_TALKVQE_MASK_ANR | AI_TALKVQE_MASK_HPF | AI_TALKVQE_MASK_HDR;

		pAiVqeAttr = (HI_VOID *)&stAiVqeTalkAttr;
    }
	else if (2 == u32AiVqeType)
    {
        memset(&stAiVqeHifiAttr, 0, sizeof(AI_HIFIVQE_CONFIG_S));
	    stAiVqeHifiAttr.s32WorkSampleRate    = AUDIO_SAMPLE_RATE_48000;
	    stAiVqeHifiAttr.s32FrameSample       = SAMPLE_AUDIO_PTNUMPERFRM;
	    stAiVqeHifiAttr.enWorkstate          = VQE_WORKSTATE_COMMON;
	    stAiVqeHifiAttr.stDrcCfg.bUsrMode    = HI_FALSE;
	    stAiVqeHifiAttr.stPeqCfg.bUsrMode    = HI_FALSE;
	    stAiVqeHifiAttr.stRnrCfg.bUsrMode    = HI_FALSE;
	    stAiVqeHifiAttr.stHdrCfg.bUsrMode    = HI_FALSE;
		stAiVqeHifiAttr.stHpfCfg.bUsrMode    = HI_TRUE;
	    stAiVqeHifiAttr.stHpfCfg.enHpfFreq   = AUDIO_HPF_FREQ_150;

		stAiVqeHifiAttr.u32OpenMask = AI_HIFIVQE_MASK_DRC | AI_HIFIVQE_MASK_PEQ | AI_HIFIVQE_MASK_HDR | AI_HIFIVQE_MASK_HPF;

		pAiVqeAttr = (HI_VOID *)&stAiVqeHifiAttr;
    }
	else if (3 == u32AiVqeType)
    {
        memset(&stAiVqeRecordAttr, 0, sizeof(AI_RECORDVQE_CONFIG_S));
	    stAiVqeRecordAttr.s32WorkSampleRate    = AUDIO_SAMPLE_RATE_48000;
	    stAiVqeRecordAttr.s32FrameSample       = SAMPLE_AUDIO_PTNUMPERFRM;
	    stAiVqeRecordAttr.enWorkstate          = VQE_WORKSTATE_COMMON;
		stAiVqeRecordAttr.s32InChNum		   = 1;
		stAiVqeRecordAttr.s32OutChNum		   = 1;
		stAiVqeRecordAttr.enRecordType		   = VQE_RECORD_NORMAL;
	    stAiVqeRecordAttr.stDrcCfg.bUsrMode    = HI_FALSE;
	    stAiVqeRecordAttr.stRnrCfg.bUsrMode    = HI_FALSE;
	    stAiVqeRecordAttr.stHdrCfg.bUsrMode    = HI_FALSE;
		stAiVqeRecordAttr.stHpfCfg.bUsrMode    = HI_TRUE;
	    stAiVqeRecordAttr.stHpfCfg.enHpfFreq   = AUDIO_HPF_FREQ_80;

		stAiVqeRecordAttr.u32OpenMask = AI_RECORDVQE_MASK_DRC | AI_RECORDVQE_MASK_HDR | AI_RECORDVQE_MASK_HPF | AI_RECORDVQE_MASK_RNR;

		pAiVqeAttr = (HI_VOID *)&stAiVqeRecordAttr;
    }
	else
	{
		pAiVqeAttr = HI_NULL;
	}

	if (1 == u32AoVqeType)
    {
        memset(&stAoVqeAttr, 0, sizeof(AO_VQE_CONFIG_S));
	    stAoVqeAttr.s32WorkSampleRate    = AUDIO_SAMPLE_RATE_8000;
	    stAoVqeAttr.s32FrameSample       = SAMPLE_AUDIO_PTNUMPERFRM;
	    stAoVqeAttr.enWorkstate          = VQE_WORKSTATE_COMMON;
	    stAoVqeAttr.stAgcCfg.bUsrMode    = HI_FALSE;
	    stAoVqeAttr.stAnrCfg.bUsrMode    = HI_FALSE;
		stAoVqeAttr.stHpfCfg.bUsrMode    = HI_TRUE;
	    stAoVqeAttr.stHpfCfg.enHpfFreq   = AUDIO_HPF_FREQ_150;

		stAoVqeAttr.u32OpenMask = AO_VQE_MASK_AGC | AO_VQE_MASK_ANR | AO_VQE_MASK_EQ | AO_VQE_MASK_HPF;

		pAoVqeAttr = (HI_VOID *)&stAoVqeAttr;
    }

    
    /********************************************
      step 1: config audio codec
    ********************************************/
    s32Ret = SAMPLE_COMM_AUDIO_CfgAcodec(&stAioAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_DBG(s32Ret);
        goto VQE_ERR2;
    }

    /********************************************
      step 2: start Ai
    ********************************************/
    s32AiChnCnt = stAioAttr.u32ChnCnt;
    s32Ret = SAMPLE_COMM_AUDIO_StartAi(AiDev, s32AiChnCnt, &stAioAttr, enOutSampleRate, gs_bAioReSample, pAiVqeAttr, u32AiVqeType);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
        goto VQE_ERR2;
    }

    /********************************************
      step 3: start Ao
    ********************************************/
    s32AoChnCnt = stAioAttr.u32ChnCnt;
    s32Ret = SAMPLE_COMM_AUDIO_StartAo(AoDev, s32AoChnCnt, &stAioAttr, enInSampleRate, gs_bAioReSample, pAoVqeAttr, u32AoVqeType);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
        goto VQE_ERR1;
    }

    /********************************************
      step 4: Ao bind Ai Chn
    ********************************************/
    for (i = 0; i < s32AiChnCnt; i++)
    {
        s32Ret = SAMPLE_COMM_AUDIO_CreatTrdAiAo(AiDev, i, AoDev, i);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_DBG(s32Ret);
			for (j=0; j<i; j++)
			{
				SAMPLE_COMM_AUDIO_DestoryTrdAi(AiDev, j);
			}
            goto VQE_ERR0;
        }
		
		printf("bind ai(%d,%d) to ao(%d,%d) ok \n", AiDev, i, AoDev, i);
    }

    printf("\nplease press twice ENTER to exit this sample\n");
    getchar();
    getchar();

    /********************************************
      step 6: exit the process
    ********************************************/
    for (i = 0; i < s32AiChnCnt; i++)
    {
        s32Ret = SAMPLE_COMM_AUDIO_DestoryTrdAi(AiDev, i);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_DBG(s32Ret);
        }
    }
	
VQE_ERR0:
	s32Ret = SAMPLE_COMM_AUDIO_StopAo(AoDev, s32AoChnCnt, gs_bAioReSample, HI_TRUE);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
    }
    
VQE_ERR1:
    s32Ret |= SAMPLE_COMM_AUDIO_StopAi(AiDev, s32AiChnCnt, gs_bAioReSample, HI_TRUE);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
    }

VQE_ERR2:

    return s32Ret;
}


HI_VOID SAMPLE_AUDIO_Usage(HI_VOID)
{
    printf("\n\n/Usage:./sample_audio <index>/\n");
	printf("\tindex and its function list below\n");
    printf("\t0:  start AI to AO loop\n");
    printf("\t1:  send audio frame to AENC channel from AI, save them\n");
    printf("\t2:  read audio stream from file, decode and send AO\n");
    printf("\t3:  start AI(AEC/ANR/ALC process), then send to AO\n");
}

/******************************************************************************
* function : to process abnormal case
******************************************************************************/
void SAMPLE_AUDIO_HandleSig(HI_S32 signo)
{
    signal(SIGINT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
    
    if (SIGINT == signo || SIGTERM == signo)
    {

        SAMPLE_COMM_AUDIO_DestoryAllTrd();
        SAMPLE_COMM_SYS_Exit();
        printf("\033[0;31mprogram exit abnormally!\033[0;39m\n");
    }

    exit(0);
}

/******************************************************************************
* function : main
******************************************************************************/
#ifdef __HuaweiLite__
HI_S32 app_main(int argc, char* argv[])
#else
HI_S32 main(int argc, char* argv[])
#endif
{
    HI_S32 s32Ret = HI_SUCCESS;
    VB_CONF_S stVbConf;
	HI_U32 u32Index = 0;

	if (2 != argc)
	{
		SAMPLE_AUDIO_Usage();
		return HI_FAILURE;
	}

	u32Index = atoi(argv[1]);

	if (u32Index > 3)
	{
		SAMPLE_AUDIO_Usage();
		return HI_FAILURE;
	}
    
    signal(SIGINT, SAMPLE_AUDIO_HandleSig);
    signal(SIGTERM, SAMPLE_AUDIO_HandleSig);

    memset(&stVbConf, 0, sizeof(VB_CONF_S));
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        printf("%s: system init failed with %d!\n", __FUNCTION__, s32Ret);
        return HI_FAILURE;
    }
    
    switch (u32Index)
    {
        case 0:
        {
            SAMPLE_AUDIO_AiAo();
            break;
        }
        case 1:
        {
            SAMPLE_AUDIO_AiAenc();
            break;
        }
        case 2:
        {
            SAMPLE_AUDIO_AdecAo();
            break;
        }
        case 3:
        {
            SAMPLE_AUDIO_AiVqeProcessAo();
            break;
        }
        default:
        {
            break;
        }
    }

    SAMPLE_COMM_SYS_Exit();

    return s32Ret;
}


