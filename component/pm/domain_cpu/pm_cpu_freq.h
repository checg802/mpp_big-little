#ifndef  __HI_PM_CPU_FREQ_H__
#define  __HI_PM_CPU_FREQ_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* Begin of #ifdef __cplusplus */

#include <linux/clk.h>
#include <linux/cpu.h>
#include <linux/cpufreq.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/pm_opp.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/delay.h>

#include "pm_device.h"



int cpu_domain_freq_init(struct pm_domain_device *pm_device);
void cpu_domain_freq_exit(struct pm_domain_device *pm_device);

int cpu_freq_target_index(struct cpufreq_policy *policy, unsigned int index);

int cpufreq_init(struct cpufreq_policy *policy);

void cpu_freq_freeze(struct pm_domain_device *pm_domain_device);
void cpu_freq_restore(struct pm_domain_device *pm_domain_device);




#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif /* End of #ifdef __cplusplus */
    
#endif/* End of #ifndef __PM_H__*/


