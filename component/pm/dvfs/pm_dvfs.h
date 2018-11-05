#ifndef  __HI_PM_DVFS_H__
#define  __HI_PM_DVFS_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* Begin of #ifdef __cplusplus */

#include "pm_device.h"


int pm_dvfs_init(struct pm_device *pm_device);
void pm_dvfs_exit(struct pm_device *pm_device);

#ifdef __cplusplus
#if __cplusplus
    }
#endif

#endif /* End of #ifdef __cplusplus */
#endif/* End of #ifndef __HI_PM_DVFS_H__*/


