/******************************************************************************

  Copyright (C), 2013-2023, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : vgs_ext.h
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2013/04/23
  Description   :
  History       :
  1.Date        : 2013/04/23
    Author      : z00183560
    Modification: Created file
******************************************************************************/

#ifndef __VGS_EXT_H__
#define __VGS_EXT_H__

#include "hi_comm_video.h"
#include "hi_comm_vgs.h"
#include "hi_errno.h"
#include "vb_ext.h"

#define HI_TRACE_VGS(level, fmt...)	\
do{\
	HI_TRACE(level, HI_ID_VGS, "[%s]: %d,", __FUNCTION__, __LINE__);\
	HI_TRACE(level, HI_ID_VGS, ##fmt);\
} while(0);

#define VGS_INVALD_HANDLE          (-1UL) /*invalid job handle*/

#define VGS_MAX_JOB_NUM             400  /*max job number*/
#define VGS_MIN_JOB_NUM             20  /*min job number*/

#define VGS_MAX_TASK_NUM            800  /*max task number*/
#define VGS_MIN_TASK_NUM            20  /*min task number*/

#define VGS_MAX_NODE_NUM            800  /*max node number*/
#define VGS_MIN_NODE_NUM            20  /*min node number*/

#define VGS_MAX_WEIGHT_THRESHOLD    100  /*max weight threshold*/
#define VGS_MIN_WEIGHT_THRESHOLD    1  /*min weight threshold*/

#define MAX_VGS_COVER               1
#define MAX_VGS_OSD                 1


/*The type of job*/
typedef enum hiVGS_JOB_TYPE_E
{
    VGS_JOB_TYPE_BYPASS = 0,        /*BYPASS job,can only contain bypass task*/
    VGS_JOB_TYPE_NORMAL = 1,         /*normal job,can contain any task except bypass task and lumastat task*/
    VGS_JOB_TYPE_BUTT
}VGS_JOB_TYPE_E;


/*The completion status of task*/
typedef enum hiVGS_TASK_FNSH_STAT_E
{
    VGS_TASK_FNSH_STAT_OK = 1,      /*task has been completed correctly */
    VGS_TASK_FNSH_STAT_FAIL = 2,    /*task failed because of exception or not completed */
    VGS_TASK_FNSH_STAT_CANCELED= 3, /*task has been cancelled */
    VGS_TASK_FNSH_STAT_BUTT
}VGS_TASK_FNSH_STAT_E;

/* The struct of vgs task ,include input and output frame buffer,caller,and callback function .
After complete the task,VGS call the callback function to notify the caller with this struct as an parameter.
*/
typedef struct hiVGS_TASK_DATA_S
{
    VIDEO_FRAME_INFO_S      stImgIn;        /* input picture */
    VIDEO_FRAME_INFO_S      stImgOut;       /* output picture */
    HI_U32                  au32privateData[VGS_PRIVATE_DATA_LEN];    /* task's private data */
    void                    (*pCallBack)(MOD_ID_E enCallModId, HI_U32 u32CallDevId, const struct hiVGS_TASK_DATA_S* pTask); /* callback */
    MOD_ID_E	            enCallModId;    /* module ID */
    HI_U32                  u32CallChnId;    /*chnnal ID */
    VGS_TASK_FNSH_STAT_E    enFinishStat;     /* output param:task finish status(ok or nok)*/
    HI_U32                  reserved;       /* save current picture's state while debug */
} VGS_TASK_DATA_S;

/* the priority of VGS task */
typedef enum hiVGS_JOB_PRI_E
{
    VGS_JOB_PRI_HIGH = 0,      /*high priority*/
    VGS_JOB_PRI_NORMAL =1,     /*normal priority*/
    VGS_JOB_PRI_LOW =2,        /*low priority*/
    VGS_JOB_PRI_BUTT
}VGS_JOB_PRI_E;

/* The status after VGS cancle job */
typedef struct hiVGS_CANCEL_STAT_S
{
    HI_U32  u32JobsCanceled;    /*The count of cancelled job*/
    HI_U32  u32JobsLeft;        /*The count of the rest job*/
}VGS_CANCEL_STAT_S;

/*The completion status of job*/
typedef enum hiVGS_JOB_FNSH_STAT_E
{
    VGS_JOB_FNSH_STAT_OK = 1,       /*job has been completed correctly*/
    VGS_JOB_FNSH_STAT_FAIL = 2,     /*job failed because of exception or not completed*/
    VGS_JOB_FNSH_STAT_CANCELED = 3, /*job has been cancelled*/
    VGS_JOB_FNSH_STAT_BUTT
}VGS_JOB_FNSH_STAT_E;

/* The struct of vgs job.
After complete the job,VGS call the callback function to notify the caller with this struct as an parameter.
*/
typedef struct hiVGS_JOB_DATA_S
{
    HI_U32              au32PrivateData[VGS_PRIVATE_DATA_LEN];  /* private data of job */
    VGS_JOB_FNSH_STAT_E enJobFinishStat;                        /* output param:job finish status(ok or nok)*/
    VGS_JOB_TYPE_E      enJobType;
    void                (*pJobCallBack)(MOD_ID_E enCallModId,HI_U32 u32CallDevId,struct hiVGS_JOB_DATA_S *pJobData); /* callback */
}VGS_JOB_DATA_S;

/* The information of OSD */
typedef struct hiVGS_OSD_S
{
    /* first address of bitmap */
    HI_U32 u32PhyAddr;

    PIXEL_FORMAT_E enPixelFormat;

    HI_U32 u32Stride;

    /* Alpha bit should be extended by setting Alpha0 and Alpha1, when enPixelFormat is PIXEL_FORMAT_RGB_1555 */
    HI_BOOL bAlphaExt1555; /* whether Alpha bit should be extended */
    HI_U8 u8Alpha0;        /* u8Alpha0 will be valid where alpha bit is 0 */
    HI_U8 u8Alpha1;        /* u8Alpha1 will be valid where alpha bit is 1 */
} VGS_OSD_S;

/*The struct of OSD operation*/
typedef struct hiVGS_OSD_OPT_S
{
    HI_BOOL         bOsdEn;

    HI_U8           u32GlobalAlpha;
    VGS_OSD_S       stVgsOsd;
    RECT_S          stOsdRect;
} VGS_OSD_OPT_S;

typedef struct hiVGS_LUMAINFO_OPT_S
{
    RECT_S      stRect;             /*the regions to get lumainfo*/
    HI_U32 *    pu32VirtAddrForResult;
    HI_U32      u32PhysAddrForResult;
}VGS_LUMASTAT_OPT_S;

/*------------------discard 2016.01.21 begin---------------------*/
/*Define 4 video frame*/
typedef enum HI_VGS_BORDER_WORK_E
{
    VGS_BORDER_WORK_LEFT     =  0,
    VGS_BORDER_WORK_RIGHT    =  1,
    VGS_BORDER_WORK_BOTTOM   =  2,
    VGS_BORDER_WORK_TOP      =  3,
    VGS_BORDER_WORK_BUTT
}VGS_BORDER_WORK_E;

/*Define attributes of video frame*/
typedef struct HI_VGS_FRAME_OPT_S
{
    RECT_S  stRect;
    HI_U32  u32Width; /*Width of 4 frames,0:L,1:R,2:B,3:T*/
    HI_U32  u32Color; /*Color of 4 frames,R/G/B*/
}VGS_FRAME_OPT_S;

/*Define attributes of video frame*/
typedef struct HI_VGS_BORDER_OPT_S
{
    HI_U32  u32Width[VGS_BORDER_WORK_BUTT]; /*Width of 4 frames,0:L,1:R,2:B,3:T*/
    HI_U32  u32Color; /*Color of 4 frames,R/G/B*/
}VGS_BORDER_OPT_S;

/*------------------discard 2016.01.21 end---------------------*/

/* The type of rotation, include Chroma rotation and luminance rotation*/
typedef enum hiVGS_ROTATE_TYPE_E
{
    VGS_ROTATE_LUMA_TYPE = 0,            /*luminance rotation*/
    VGS_ROTATE_CHRO_TYPE = 1,            /*Chroma rotation*/
    VGS_ROTATE_TYPE_BUTT
} VGS_ROTATE_TYPE_E;

typedef struct hiVGS_ROTATE_OPT_S
{
    VGS_ROTATE_TYPE_E eRotateType;
    ROTATE_E enRotateAngle;
}VGS_ROTATE_OPT_S;

/* The type of LDC operations */
typedef enum hiVGS_LDC_TASK_TYPE_E
{
    VGS_LDC_TASK_HOR_TYPE      = 0,       /*horizontal LDC operation*/
    VGS_LDC_TASK_VER_LUMA_TYPE = 1,       /*vertical LDC of luminance operation*/
    VGS_LDC_TASK_VER_CHRO_TYPE = 2,       /*vertical LDC of Chroma operation*/
    VGS_LDC_TASK_BUTT
} VGS_LDC_TASK_TYPE_E;

typedef LDC_ATTR_S VGS_LDC_OPT_S;

typedef struct hiVGS_COVER_OPT_S
{
    HI_BOOL     				bCoverEn;
    VGS_COVER_TYPE_E   			enCoverType;
    union
    {
        RECT_S              	stDstRect;      /* rectangle */
        VGS_QUADRANGLE_COVER_S  stQuadRangle;   /* arbitrary quadrilateral */
    };
    HI_U32      				u32CoverColor;
} VGS_COVER_OPT_S;

typedef struct hiVGS_CROP_OPT_S
{
    RECT_S stDestRect;
}VGS_CROP_OPT_S;

typedef enum hiVGS_SCLCOEF_MODE_E
{
    VGS_SCLCOEF_NORMAL      = 0,    /* normal scale coefficient*/
    VGS_SCLCOEF_IVETAP4     = 1,    /* scale coefficient of 4 tap for IVE */
    VGS_SCLCOEF_IVETAP6     = 2,    /* scale coefficient of 6 tap for IVE*/
    VGS_SCLCOEF_IVETAP8     = 3,    /* scale coefficient of 8 tap for IVE */
    VGS_SCLCOEF_IVEBILINEAR = 4,    /* scale coefficient of 2 tap double-line for IVE */
    VGS_SCLCOEF_BUTT
}VGS_SCLCOEF_MODE_E;

typedef struct hiVGS_ONLINE_OPT_S
{
    HI_BOOL                 bCrop;              /*if enable crop*/
    VGS_CROP_OPT_S          stCropOpt;
    HI_BOOL                 bCover;             /*if enable cover*/
    VGS_COVER_OPT_S         stCoverOpt[MAX_VGS_COVER];
    HI_BOOL                 bOsd;               /*if enable osd*/
    VGS_OSD_OPT_S           stOsdOpt[MAX_VGS_OSD];
    HI_BOOL                 bBorder;            /*if enable Border*/
    VGS_BORDER_OPT_S        stBorderOpt;
    HI_BOOL                 bMirror;            /*if enable mirror*/
    HI_BOOL                 bFlip;              /*if enable flip*/
    HI_BOOL                 bForceHFilt;        /*Whether to force the horizontal direction filtering,it can be
                                                set while input and output pic are same size at horizontal direction*/
    HI_BOOL                 bForceVFilt;        /*Whether to force the vertical direction filtering,it can be 
                                                set while input and output pic are same size at vertical direction*/
    HI_BOOL                 bDeflicker;         /*Whether decrease flicker*/
    VGS_SCLCOEF_MODE_E      enVGSSclCoefMode;

    HI_BOOL                 bFisheye;          	/*The operation is belong to fisheye*/
    RECT_S                  stFisheyeRect;
} VGS_ONLINE_OPT_S;

typedef struct hiVGS_ONLINE2_OPT_S
{
    HI_BOOL                 bLDC;              /*if enable LDC*/
    VGS_LDC_OPT_S          stLdcOpt;
    HI_BOOL                 bRTT;             /*if enable RTT*/
    VGS_ROTATE_OPT_S         stRotateOpt;
} VGS_ONLINE2_OPT_S;

/* vertical scanning direction */
typedef enum hiVGS_DRV_VSCAN_E
{
    VGS_SCAN_UP_DOWN = 0,	/* form up to down */
    VGS_SCAN_DOWN_UP = 1 	/* form down to up */
} VGS_DRV_VSCAN_E;

/* horizontal scanning direction */
typedef enum hiVGS_DRV_HSCAN_E
{
    VGS_SCAN_LEFT_RIGHT = 0,	/* form left to right */
    VGS_SCAN_RIGHT_LEFT = 1 	/* form right to left */
} VGS_DRV_HSCAN_E;

/* Definition on scanning direction */
typedef struct hiVGS_SCANDIRECTION_S
{
    /* vertical scanning direction */
    VGS_DRV_VSCAN_E enVScan;

    /* horizontal scanning direction */
    VGS_DRV_HSCAN_E enHScan;
} VGS_SCANDIRECTION_S;

typedef struct hiVGS_OSD_QUICKCOPY_OPT_S
{
    RECT_S stSrcRect;  
	RECT_S stDestRect;
}VGS_OSD_QUICKCOPY_OPT_S;



typedef HI_S32 	    FN_VGS_Init(HI_VOID* pVrgs);

typedef HI_VOID     FN_VGS_Exit(HI_VOID);

typedef HI_S32      FN_VGS_BeginJob(VGS_HANDLE *pVgsHanle,VGS_JOB_PRI_E enPriLevel,
                                    MOD_ID_E enCallModId,HI_U32 u32CallChnId,VGS_JOB_DATA_S *pJobData);

typedef HI_S32      FN_VGS_EndJob(VGS_HANDLE VgsHanle,HI_BOOL bSort,VGS_JOB_DATA_S *pJobData);

typedef HI_S32      FN_VGS_EndJobBlock(VGS_HANDLE VgsHanle);

typedef HI_S32      FN_VGS_CancelJob(VGS_HANDLE hHandle);

typedef HI_S32      FN_VGS_CancelJobByModDev(MOD_ID_E	enCallModId,HI_U32 u32CallDevId,VGS_CANCEL_STAT_S*  pstVgsCancelStat);

typedef HI_S32      FN_VGS_AddFrameTask(VGS_HANDLE VgsHanle,VGS_TASK_DATA_S *pTask,VGS_FRAME_OPT_S pstVgsFrameOpt);

typedef HI_S32      FN_VGS_AddCoverTask(VGS_HANDLE VgsHanle,VGS_TASK_DATA_S *pTask,VGS_COVER_OPT_S* pstCoverOpt);

typedef HI_S32      FN_VGS_AddOSDTask(VGS_HANDLE VgsHanle,VGS_TASK_DATA_S *pTask,VGS_OSD_OPT_S* pstOsdOpt);

typedef HI_S32      FN_VGS_AddOnlineTask(VGS_HANDLE VgsHanle,VGS_TASK_DATA_S *pTask,VGS_ONLINE_OPT_S *pstOnlineOpt); 

typedef HI_S32      FN_VGS_AddLDCTask(VGS_HANDLE hHandle, VGS_TASK_DATA_S *pTask, VGS_LDC_OPT_S *pstLdcOpt);

typedef HI_S32      FN_VGS_AddRotateTask(VGS_HANDLE VgsHanle,VGS_TASK_DATA_S *pTask,VGS_ROTATE_OPT_S* pstRotateOpt);

typedef HI_S32      FN_VGS_AddOnline2Task(VGS_HANDLE VgsHanle,VGS_TASK_DATA_S *pTask,VGS_ONLINE2_OPT_S *pstOnline2Opt); 

typedef HI_S32      FN_VGS_Add2ScaleTask(VGS_HANDLE VgsHandle, VGS_TASK_DATA_S* pTask);

typedef HI_S32      FN_VGS_AddGetLumaStatTask(VGS_HANDLE VgsHanle,VGS_TASK_DATA_S *pTask,VGS_LUMASTAT_OPT_S* pstLumaInfoOpt);

typedef HI_S32      FN_VGS_AddQuickCopyTask(VGS_HANDLE VgsHandle, VGS_TASK_DATA_S* pTask, VGS_OSD_QUICKCOPY_OPT_S *pstQuickCopyOpt);

typedef HI_S32      FN_VGS_AddBypassTask(VGS_HANDLE VgsHanle,VGS_TASK_DATA_S *pTask);

/*only for test*/
typedef HI_VOID     FN_VGS_GetMaxJobNum(HI_S32* s32MaxJobNum);
typedef HI_VOID     FN_VGS_GetMaxTaskNum(HI_S32* s32MaxTaskNum);


typedef struct hiVGS_EXPORT_FUNC_S
{
    FN_VGS_BeginJob         *pfnVgsBeginJob;
    FN_VGS_CancelJob        *pfnVgsCancelJob;
    FN_VGS_CancelJobByModDev    *pfnVgsCancelJobByModDev;
    FN_VGS_EndJob           *pfnVgsEndJob;
    FN_VGS_AddFrameTask     *pfnVgsAddFrameTask;
    FN_VGS_AddCoverTask     *pfnVgsAddCoverTask;
    FN_VGS_AddRotateTask    *pfnVgsAddRotateTask;
    FN_VGS_AddRotateTask    *pfnVgsAddRGBRotateTask;
    FN_VGS_AddOSDTask       *pfnVgsAddOSDTask;
    FN_VGS_AddBypassTask    *pfnVgsAddBypassTask;
    FN_VGS_AddGetLumaStatTask   *pfnVgsGetLumaStatTask;
    FN_VGS_AddOnlineTask    *pfnVgsAddOnlineTask;
    FN_VGS_AddLDCTask       *pfnVgsAddLDCTask;
	/* for jpeg */
    FN_VGS_Add2ScaleTask    *pfnVgsAdd2ScaleTask;
	/* for region */
	FN_VGS_AddQuickCopyTask    *pfnVgsAddQuickCopyTask;

	FN_VGS_AddOnline2Task    *pfnVgsAddOnline2Task;

    /* for ive */
    FN_VGS_EndJobBlock      *pfnVgsEndJobBlock;

    /*only for test*/
    FN_VGS_GetMaxJobNum     *pfnVgsGetMaxJobNum;
    FN_VGS_GetMaxTaskNum    *pfnVgsGetMaxTaskNum;
}VGS_EXPORT_FUNC_S;



#endif /* __VGS_H__ */

