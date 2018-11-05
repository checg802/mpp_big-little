#ifndef  __HI_PM_CPU_AVS_H__
#define  __HI_PM_CPU_AVS_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* Begin of #ifdef __cplusplus */

#include "pm_device.h"

int cpu_avs_init(struct platform_device *pdev, struct pm_domain_device *pm_device);
void cpu_avs_exit(struct platform_device *pdev, struct pm_domain_device *pm_device);
void cpu_avs_freeze(struct pm_domain_device *pm_domain_device);
void cpu_avs_restore(struct pm_domain_device *pm_domain_device);

#ifdef __cplusplus
#if __cplusplus
    }
#endif

#endif /* End of #ifdef __cplusplus */
#endif/* End of #ifndef __HI_PM_CPU_AVS_H__*/



