#ifndef  __HI_PM_MEDIA_REGULATOR_H__
#define  __HI_PM_MEDIA_REGULATOR_H__

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


int media_domain_regulator_register(struct platform_device *pdev, struct pm_domain_device *pm_device);
void media_domain_regulator_unregister(struct platform_device *pdev, struct pm_domain_device *pm_device);
void media_regulator_restore(struct pm_domain_device *pm_domain_device);
int media_check_domain_valid(void);

#ifdef __cplusplus
#if __cplusplus
    }
#endif

#endif /* End of #ifdef __cplusplus */
#endif/* End of #ifndef __HI_PM_MEDIA_REGULATOR_H__*/



