/******************************************************************************

  Copyright (C), 2001-2020, Huawei Tech. Co., Ltd.

 ******************************************************************************
  File Name     : cpu.c
  Version       : Initial Draft
  Author        : l00165163
  Created       : 2014/6/25
  Last Modified :
  Description   : cpu.c
  Function List :
              cpu_pm_exit
              cpu_pm_init
  History       :
  1.Date        : 2014/6/25
    Author      : l00165163
    Modification: Created file

******************************************************************************/
#ifdef __cplusplus
#if __cplusplus
    extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/cpufreq.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/of_regulator.h>
#include <linux/regulator/machine.h>

#include "pm_device.h"
#include "pm_cpu.h"
#include "pm_cpu_regulator.h"
#include "pm_cpu_freq.h"
#include "pm_cpu_avs.h"

static int  cpu_domain_regulator_init(struct platform_device *pdev, struct pm_domain_device *pm_device);
static void cpu_domain_regulator_exit(struct platform_device *pdev, struct pm_domain_device *pm_device);

static int  cpu_domain_avs_init(struct platform_device *pdev, struct pm_domain_device *pm_device);
static void cpu_domain_avs_exit(struct platform_device *pdev, struct pm_domain_device *pm_device);

static int  cpu_domain_cpufreq_init(struct pm_domain_device *pm_device, void *data);
static void cpu_domain_cpufreq_exit(struct pm_domain_device *pm_device, void *data);

static void cpu_domain_freeze(struct pm_domain_device *pm_domain_device);
static void cpu_domain_restore(struct pm_domain_device *pm_domain_device);

struct pm_domain_device cpu_pm_device = {
    .domain_regulator_init  = cpu_domain_regulator_init,
    .domain_regulator_exit  = cpu_domain_regulator_exit,
    .domain_avs_init        = cpu_domain_avs_init,
    .domain_avs_exit        = cpu_domain_avs_exit,
    .domain_cpufreq_init    = cpu_domain_cpufreq_init,
    .domain_cpufreq_exit    = cpu_domain_cpufreq_exit,
    .domain_freeze          = cpu_domain_freeze,
    .domain_restore         = cpu_domain_restore,
};

static int cpu_domain_regulator_init(struct platform_device *pdev, struct pm_domain_device *pm_device)
{
    int ret = 0;

    ret = cpu_domain_regulator_register(pdev, pm_device);
    if(ret)
    {
		PM_PRINT("Failed to register  cpu regulator driver\n");
    }
    
    return ret;
}

static void cpu_domain_regulator_exit(struct platform_device *pdev, struct pm_domain_device *pm_device)
{
    cpu_domain_regutator_unregister(pdev, pm_device);
}

static int cpu_domain_avs_init(struct platform_device *pdev, struct pm_domain_device *pm_device)
{
    return cpu_avs_init(pdev, pm_device);
}

static void cpu_domain_avs_exit(struct platform_device *pdev, struct pm_domain_device *pm_device)
{
    cpu_avs_exit(pdev, pm_device);
}

static int cpu_domain_cpufreq_init(struct pm_domain_device *pm_device, void *data)
{
    return cpu_domain_freq_init(pm_device);
}

static void cpu_domain_cpufreq_exit(struct pm_domain_device *pm_device, void *data)
{
    cpu_domain_freq_exit(pm_device);
}

void cpu_domain_avs_pause(struct pm_domain_device *pm_device)
{
    if(pm_device->avs_dev.avs_ops && pm_device->avs_dev.avs_ops->avs_pause)
    {
        pm_device->avs_dev.avs_ops->avs_pause(pm_device);
    }
}

void cpu_domain_avs_resume(struct pm_domain_device *pm_device)
{
    if(pm_device->avs_dev.avs_ops && pm_device->avs_dev.avs_ops->avs_resume)
    {
        pm_device->avs_dev.avs_ops->avs_resume(pm_device);
    }
}

static void cpu_domain_freeze(struct pm_domain_device *pm_domain_device)
{
    cpu_freq_freeze(pm_domain_device);
    cpu_avs_freeze(pm_domain_device);
}

static void cpu_domain_restore(struct pm_domain_device *pm_domain_device)
{
    cpu_regulator_restore(pm_domain_device);
    cpu_freq_restore(pm_domain_device);
    cpu_avs_restore(pm_domain_device);
}

int cpu_domain_check_domain_valid(void)
{
    return cpu_check_domain_valid();
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

