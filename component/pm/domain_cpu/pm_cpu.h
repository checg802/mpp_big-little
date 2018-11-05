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
#ifndef  __HI_PM_CPU_H__
#define  __HI_PM_CPU_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* Begin of #ifdef __cplusplus */

#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/clk.h>
#include <mach/io.h>

#include "pm_device.h"
//#include "avs_pm.h"

#define PM_MANAGER_CPU   1

extern struct pm_domain_device cpu_pm_device;
extern struct platform_device *regulator_platform_dev;

void cpu_domain_avs_pause(struct pm_domain_device *pm_device);
void cpu_domain_avs_resume(struct pm_domain_device *pm_device);
int cpu_domain_check_domain_valid(void);

#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif /* End of #ifdef __cplusplus */
    
#endif/* End of #ifndef __PM_H__*/


