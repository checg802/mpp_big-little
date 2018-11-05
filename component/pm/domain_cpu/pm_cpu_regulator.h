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
#ifndef  __HI_PM_CPU_REGULATOR_H__
#define  __HI_PM_CPU_REGULATOR_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* Begin of #ifdef __cplusplus */


#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/of_regulator.h>
#include <linux/regulator/machine.h>


int cpu_domain_regulator_register(struct platform_device *pdev, struct pm_domain_device *pm_device);
void cpu_domain_regutator_unregister(struct platform_device *pdev, struct pm_domain_device *pm_device);
void cpu_regulator_restore(struct pm_domain_device *pm_domain_device);
int cpu_check_domain_valid(void);

#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif /* End of #ifdef __cplusplus */
    
#endif/* End of #ifndef __HI_PM_REGULATOR_H__*/


