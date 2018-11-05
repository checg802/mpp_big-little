

/******************************************************************************

  Copyright (C), 2001-2011, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : hi_sys.h
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       :
  Description   :
  History       :
  1.Date        :
    Author      :
    Modification:



******************************************************************************/
#include "hi_type.h"
#include "mod_ext.h"

#include "hi_comm_vpss.h"
#include "mkp_sys.h"

#ifndef __VPSS_EXT_H__
#define __VPSS_EXT_H__

typedef struct HI_VPSS_LOW_DELAY_CFG_S
{
    HI_BOOL bTunlFrame;
    HI_U32  u32TunlPhyAddr;
    HI_VOID* pTunlVirAddr;
    HI_VOID* pbFrameOk;
} VPSS_LOW_DELAY_CFG_S;

/* 48 bytes low delay information attached to the frame buffer */
/* [4       |12      |4         |12      |4        |12      ]*/
/*  Yheight |reserve |Cheight   |reserve |BFrameOk |reserve*/
typedef struct HI_VPSS_PIC_INFO_S
{
    VIDEO_FRAME_INFO_S stVideoFrame;
    MOD_ID_E        enModId;
    //VPSS_PRESCALE_E                enVpssPresclType;          
    //    HI_BOOL bFlashed;               /* Flashed Video frame or not. */
    HI_BOOL bBlock;                 /* Flashed Video frame or not. */
    HI_BOOL bVGS;
    HI_BOOL bLDC;
    VPSS_LOW_DELAY_CFG_S stLowDelayCfg;             /* low delay Video info */
} VPSS_PIC_INFO_S;


typedef struct HI_VPSS_QUERY_INFO_S
{
    VPSS_PIC_INFO_S* pstSrcPicInfo;     /*information of source pic*/
    VPSS_PIC_INFO_S* pstOldPicInfo;     /*information of backup pic*/
    HI_BOOL          bScaleCap;         /*whether scaling*/
    HI_BOOL          bTransCap;         /*whether the frame rate is doubled*/
    HI_BOOL          bMalloc;           /*whether malloc frame buffer*/
} VPSS_QUERY_INFO_S;

typedef struct HI_VPSS_INST_INFO_S
{
    HI_BOOL          bNew;               /*whether use new pic to query*/
    HI_BOOL          bVpss;              /*whether vpss need to process*/
    HI_BOOL          bDouble;            /*whether the frame rate is doubled */
    HI_BOOL          bUpdate;            /*whether update backup pic*/
    COMPRESS_MODE_E enCompressMode;      /*compress mode*/
    VPSS_PIC_INFO_S  astDestPicInfo[2];  /*destation pic,0:Top field 1:Bottom field*/
} VPSS_INST_INFO_S;


typedef struct HI_VPSS_SEND_INFO_S
{
    HI_BOOL           bSuc;                  /*Whether successful completion*/
    VPSS_GRP          VpssGrp;
    VPSS_CHN          VpssChn;
    VPSS_PIC_INFO_S*   pstDestPicInfo[2];    /*pic processed by vpss.0:Top field 1:Bottom field*/
} VPSS_SEND_INFO_S;


typedef struct HI_VPSS_REGISTER_INFO_S
{
    MOD_ID_E    enModId;
    HI_S32      (*pVpssQuery)(HI_S32 s32DevId, HI_S32 s32ChnId, VPSS_QUERY_INFO_S*  pstQueryInfo, VPSS_INST_INFO_S* pstInstInfo);
    HI_S32      (*pVpssSend)(HI_S32 s32DevId, HI_S32 s32ChnId, VPSS_SEND_INFO_S*  pstSendInfo);
    HI_S32      (*pResetCallBack)(HI_S32 s32DevId, HI_S32 s32ChnId, HI_VOID* pvData);

} VPSS_REGISTER_INFO_S;


typedef struct hiVPSS_EXPORT_FUNC_S
{
    HI_S32  (*pfnVpssRegister)(VPSS_REGISTER_INFO_S* pstInfo);
    HI_S32  (*pfnVpssUnRegister)(MOD_ID_E enModId);

} VPSS_EXPORT_FUNC_S;

#define CKFN_VPSS_ENTRY()  CHECK_FUNC_ENTRY(HI_ID_VPSS)

#define CKFN_VPSS_Register() \
    (NULL != FUNC_ENTRY(VPSS_EXPORT_FUNC_S, HI_ID_VPSS)->pfnVpssRegister)
#define CALL_VPSS_Register(pstInfo) \
    FUNC_ENTRY(VPSS_EXPORT_FUNC_S, HI_ID_VPSS)->pfnVpssRegister(pstInfo)

#define CKFN_VPSS_UnRegister() \
    (NULL != FUNC_ENTRY(VPSS_EXPORT_FUNC_S, HI_ID_VPSS)->pfnVpssUnRegister)
#define CALL_VPSS_UnRegister(enModId) \
    FUNC_ENTRY(VPSS_EXPORT_FUNC_S, HI_ID_VPSS)->pfnVpssUnRegister(enModId)

#endif /* __VPSS_EXT_H__ */




