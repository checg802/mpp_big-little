/******************************************************************************

  Copyright (C), 2001-2020, Huawei Tech. Co., Ltd.

 ******************************************************************************
  File Name     : pm.h
  Version       : Initial Draft
  Author        : c00321909
  Created       : 2016/01/06
  Last Modified :
  Description   : pm.h
  Function List :
  History       :
  1.Date        : 2016/01/06
    Author      : c00321909
    Modification: Created file

******************************************************************************/
#ifndef  __HI_PM_AVS_H__
#define  __HI_PM_AVS_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* Begin of #ifdef __cplusplus */


#include "pm_device.h"

int pm_avs_init(struct pm_device *pm_device);
void pm_avs_exit(struct pm_device *pm_device);

#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif /* End of #ifdef __cplusplus */
    
#endif/* End of #ifndef __HI_PM_REGULATOR_H__*/

