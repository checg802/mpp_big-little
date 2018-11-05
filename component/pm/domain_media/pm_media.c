/******************************************************************************

  Copyright (C), 2001-2020, Huawei Tech. Co., Ltd.

 ******************************************************************************
  File Name     : media.c
  Version       : Initial Draft
  Author        : l00165163
  Created       : 2014/6/25
  Last Modified :
  Description   : media.c
  Function List :
                  media_pm_exit
                  media_pm_init
  History       :
  1.Date        : 2014/6/25
    Author      : l00165163
    Modification: Created file

******************************************************************************/
#ifdef __cplusplus
#if __cplusplus
    extern "C"{
#endif
#endif /* Begin of #ifdef __cplusplus */

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/clk.h>
#include <linux/io.h>

#include "pm_device.h"
#include "pm_media.h"
#include "pm_media_regulator.h"
#include "pm_media_avs.h"

static int  media_domain_regulator_init(struct platform_device *pdev, struct pm_domain_device *pm_device);
static void media_domain_regulator_exit(struct platform_device *pdev, struct pm_domain_device *pm_device);

static int  media_domain_avs_init(struct platform_device *pdev, struct pm_domain_device *pm_device);
static void media_domain_avs_exit(struct platform_device *pdev, struct pm_domain_device *pm_device);

static void media_domain_freeze(struct pm_domain_device *pm_domain_device);
static void media_domain_restore(struct pm_domain_device *pm_domain_device);

struct pm_domain_device media_pm_device = {
    .domain_regulator_init  = media_domain_regulator_init,
    .domain_regulator_exit  = media_domain_regulator_exit,
    .domain_avs_init        = media_domain_avs_init,
    .domain_avs_exit        = media_domain_avs_exit,
    .domain_freeze          = media_domain_freeze,
    .domain_restore         = media_domain_restore,    
};

static int media_domain_regulator_init(struct platform_device *pdev, struct pm_domain_device *pm_device)
{
    return media_domain_regulator_register(pdev, pm_device);
}

static void media_domain_regulator_exit(struct platform_device *pdev, struct pm_domain_device *pm_device)
{
    media_domain_regulator_unregister(pdev, pm_device);
}

static int media_domain_avs_init(struct platform_device *pdev, struct pm_domain_device *pm_device)
{
    return media_avs_init(pdev, pm_device);
}

static void media_domain_avs_exit(struct platform_device *pdev, struct pm_domain_device *pm_device)
{
    media_avs_exit(pdev, pm_device);
}

void media_domain_avs_pause(struct pm_domain_device *pm_device)
{
    if(pm_device->avs_dev.avs_ops && pm_device->avs_dev.avs_ops->avs_pause)
    {
        pm_device->avs_dev.avs_ops->avs_pause(pm_device);
    }
}

void media_domain_avs_resume(struct pm_domain_device *pm_device)
{
    if(pm_device->avs_dev.avs_ops && pm_device->avs_dev.avs_ops->avs_resume)
    {
        pm_device->avs_dev.avs_ops->avs_resume(pm_device);
    }
}

static void media_domain_freeze(struct pm_domain_device *pm_domain_device)
{
    media_avs_freeze(pm_domain_device);
}

static void media_domain_restore(struct pm_domain_device *pm_domain_device)
{
    media_regulator_restore(pm_domain_device);
    media_avs_restore(pm_domain_device);
}

int media_domain_check_domain_valid(void)
{
    return media_check_domain_valid();
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

