#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/cpufreq.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/cpu.h>
#include <linux/module.h>
#include <linux/regulator/consumer.h>
#include <asm/stat.h>
#include <asm/smp_plat.h>
#include <asm/cpu.h>
#include <mach/io.h>

#include "pm_cpu.h"
#include "pm_media.h"
#include "pm_regulator.h"
#include "pm_avs.h"
#include "pm_dvfs.h"

#define to_pm_device(x) container_of((x), struct pm_device, device)
#define to_pm_device_ex(x) to_pm_device(to_platform_device(x))

#define HISI_PM_NAME    "hisi_pm"

static void pm_platform_device_release(struct device* dev)
{
}

static int  pm_platform_device_probe(struct platform_device *device)
{
	return 0;
}

static int  pm_platform_device_remove(struct platform_device *device)
{
	return 0;
}

static void freeze_one_domain(struct pm_domain_device *pm_domain_device)
{
    if(pm_domain_device && pm_domain_device->domain_freeze)
    {
        pm_domain_device->domain_freeze(pm_domain_device);
    }
}

static void restore_one_domain(struct pm_domain_device *pm_domain_device)
{
    if(pm_domain_device && pm_domain_device->domain_restore)
    {
        pm_domain_device->domain_restore(pm_domain_device);
    }
}


static int pm_freeze(struct device *dev)
{
    struct pm_device *pm_device;
	pm_device = to_pm_device_ex(dev);
    PM_PRINT("\n");

    freeze_one_domain(pm_device->cpu_domain_device);
    
    freeze_one_domain(pm_device->media_domain_device);
    
    return 0;
}

static int pm_restore(struct device *dev)
{    
    struct pm_device *pm_device;
	pm_device = to_pm_device_ex(dev);
    PM_PRINT("\n");
    
    restore_one_domain(pm_device->media_domain_device);

    restore_one_domain(pm_device->cpu_domain_device);
    
	return 0;
}

static struct dev_pm_ops pm_ops = {
	.freeze         = pm_freeze,
	.restore        = pm_restore,
};

struct pm_device hisi_pm_device = {
    .device = {
            .name = HISI_PM_NAME,
            .dev  = {
                .platform_data = NULL,                
                .release = pm_platform_device_release,
            },      
        },
    .driver = {
            .driver = {
                    .name  = HISI_PM_NAME,
                    .owner = THIS_MODULE,
                    .pm = &pm_ops,
                },
            .probe = pm_platform_device_probe,
            .remove = pm_platform_device_remove,
     },
};

static int pm_check_condition_valid(void)
{
    int ret;
    int cpu_domain_valid = 0;
    int media_domain_valid = 0;
    
    /*check if the CPU domain is independent*/
    ret = cpu_domain_check_domain_valid();
    if(0 == ret)
    {
        cpu_domain_valid = 1;
    }
    
    /*check if the media domain is independent*/
    ret = media_domain_check_domain_valid();
    if(0 == ret)
    {
        media_domain_valid = 1;
    }

    /*if CPU power management is enabled*/
#ifdef CPU_PM
    PM_PRINT(" cpu_domain_valid %d \n",cpu_domain_valid);
    if(cpu_domain_valid)
    {
        hisi_pm_device.cpu_domain_device = &cpu_pm_device;
    }
    else
    {
        hisi_pm_device.cpu_domain_device = NULL;
    }
#else

    hisi_pm_device.cpu_domain_device = NULL;
#endif

    /*if MEDIA power management is enabled*/
#ifdef MEDIA_PM
    PM_PRINT(" media_domain_valid %d \n",media_domain_valid);
    if(media_domain_valid)
    {
        hisi_pm_device.media_domain_device = &media_pm_device;
    }
    else
    {
        hisi_pm_device.media_domain_device = NULL;
    }
#else

    hisi_pm_device.media_domain_device = NULL;
#endif

    /*CPU domain or MEDIA domain must be valid */
    if(!hisi_pm_device.media_domain_device && !hisi_pm_device.cpu_domain_device)
    {
        return -1;
    }

    return 0;
}

static int __init pm_init(void)
{
    int ret;

    ret = pm_check_condition_valid();
    if(ret)
    {
        PM_PRINT(" Init pm module fail because of invalid condition!\n");
        return ret;
    }
        
    ret = platform_device_register(&hisi_pm_device.device);
    if (ret)
    {
        PM_PRINT("Register pm_device fail. \n");
        return ret;
    }

    ret = platform_driver_register(&hisi_pm_device.driver);
    if (ret)
    {
        PM_PRINT("Register pm_driver fail. \n");
        goto fail0;
    }

    ret = pm_regulator_init(&hisi_pm_device);
    if (ret)
    {
        PM_PRINT("Init  regulator fail. \n");
        goto fail1;
    }
    ret = pm_dvfs_init(&hisi_pm_device);
    if (ret)
    {
        PM_PRINT("Init  dvfs  fail. \n");
        goto fail2;
    }
    ret = pm_avs_init(&hisi_pm_device);
    if (ret)
    {
        PM_PRINT("Init  avs fail. \n");
        goto fail3;
    }

    printk("Load pm.ko ok!.\n");
    
    return ret;

fail3:
    pm_dvfs_exit(&hisi_pm_device);
    
fail2:
    pm_regulator_init(&hisi_pm_device);
fail1:
    platform_driver_unregister(&hisi_pm_device.driver);

fail0:
    platform_device_unregister(&hisi_pm_device.device);
    return ret;
    
}

static void pm_exit(void)
{
    pm_avs_exit(&hisi_pm_device);

    pm_dvfs_exit(&hisi_pm_device);
    
    pm_regulator_exit(&hisi_pm_device);
    
    platform_driver_unregister(&hisi_pm_device.driver);
    
    platform_device_unregister(&hisi_pm_device.device);
    
    printk("Unload pm.ko ok!.\n");

}

MODULE_DESCRIPTION("pm driver for hi3519 SoCs");
MODULE_LICENSE("GPL");
module_init(pm_init);
module_exit(pm_exit);


