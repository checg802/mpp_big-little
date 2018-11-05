#include <linux/delay.h>

#include "pm_device.h"
#include "pm_cpu.h"

extern struct pm_device hisi_pm_device;

static int regulator_probe(struct platform_device *pdev);
static int regulator_remove(struct platform_device *pdev);

int every_domain_regulator_init(struct platform_device *pdev, struct pm_device *pm_device, PM_DOMAIN_E pm_domain)
{    
    if(PM_DOMAIN_CPU == pm_domain && pm_device->cpu_domain_device && pm_device->cpu_domain_device->domain_regulator_init)
    {
        return pm_device->cpu_domain_device->domain_regulator_init(pdev, pm_device->cpu_domain_device);
    }
    else if(PM_DOMAIN_MEDIA == pm_domain && pm_device->media_domain_device && pm_device->media_domain_device->domain_regulator_init)
    {
        return pm_device->media_domain_device->domain_regulator_init(pdev, pm_device->media_domain_device);
    }
    else
    {
        PM_PRINT("\n");
        return -1;
    }
}

void every_domain_regulator_exit(struct platform_device *pdev, struct pm_device *pm_device, PM_DOMAIN_E pm_domain)
{    
    if(PM_DOMAIN_CPU == pm_domain && pm_device->cpu_domain_device && pm_device->cpu_domain_device->domain_regulator_exit)
    {
        pm_device->cpu_domain_device->domain_regulator_exit(pdev, pm_device->cpu_domain_device);
    }
    else if(PM_DOMAIN_MEDIA == pm_domain && pm_device->media_domain_device && pm_device->media_domain_device->domain_regulator_exit)
    {
        pm_device->media_domain_device->domain_regulator_exit(pdev, pm_device->media_domain_device);
    }
    else
    {
        PM_PRINT("\n");
    }
}


static const struct of_device_id of_pm_regulator_match[] = {
	{ .compatible = "hi35xx,regulators", },
	{},
};

static struct platform_driver regulator_driver = {
	.driver = {
		.name = "hi3519-regulator",
		.owner = THIS_MODULE,
		.of_match_table = of_pm_regulator_match,
	},
	.probe = regulator_probe,
	.remove = regulator_remove,
};

PM_DOMAIN_E regulator_name_to_domain(const char *regulator_name)
{
    PM_DOMAIN_E pm_domain = PM_DOMAIN_BUTT;
    
    if(!strcmp(regulator_name, "regulator-a17"))
    {
        pm_domain = PM_DOMAIN_CPU;
    }
    else if(!strcmp(regulator_name, "regulator-media"))
    {
        pm_domain = PM_DOMAIN_MEDIA;
    }
    else if(!strcmp(regulator_name, "regulator-ddr"))
    {
        pm_domain = PM_DOMAIN_DDR;
    }
    else if(!strcmp(regulator_name, "regulator-core"))
    {
        pm_domain = PM_DOMAIN_CORE;
    }
    else
    {
        PM_PRINT("Wrong regulator name : %s\n", regulator_name);
    }
    
    return pm_domain;
}

static int regulator_probe(struct platform_device *pdev)
{
    int ret = 0,i = 0;
    PM_DOMAIN_E pm_domain = PM_DOMAIN_BUTT;
    unsigned int regulator_num = 0;
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
    const char *regulator_name = NULL;

    ret = of_property_read_u32(np, "regulator-num", &regulator_num);
    if(ret)
    {
        PM_PRINT("Get regulator-num fail\n");
        return ret;
    }
    
    for (i = 0; i < regulator_num; i++) 
    {
		ret = of_property_read_string_index(np, "regulator-name-array", i, &regulator_name);
		if (ret) 
        {
            PM_PRINT("Get regulator-name-array fail\n");
            return ret;
		}
        
        pm_domain = regulator_name_to_domain(regulator_name);
        
        if(pm_domain != PM_DOMAIN_BUTT)
        {
            every_domain_regulator_init(pdev, &hisi_pm_device, pm_domain);
        }
        else
        {
            PM_PRINT("Wrong regulator name : %s\n", regulator_name);
        }
    }
    return 0;
}

static int regulator_remove(struct platform_device *pdev)
{
    int ret = 0,i = 0;
    PM_DOMAIN_E pm_domain = PM_DOMAIN_BUTT;
    unsigned int regulator_num = 0;
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
    const char *regulator_name = NULL;

    ret = of_property_read_u32(np, "regulator-num", &regulator_num);
    if(ret)
    {
        PM_PRINT("Get regulator-num fail\n");
        return ret;
    }

    for (i = 0; i < regulator_num; i++) 
    {
		ret = of_property_read_string_index(np, "regulator-name-array", i, &regulator_name);
		if (ret) 
        {
            PM_PRINT("Get regulator-name-array fail\n");
            return ret;
		}
        
        pm_domain = regulator_name_to_domain(regulator_name);
        if(pm_domain != PM_DOMAIN_BUTT)
        {
            every_domain_regulator_exit(pdev, &hisi_pm_device, pm_domain);
        }
        else
        {
            PM_PRINT("Wrong regulator name : %s\n", regulator_name);
        }
    }
    return 0;

}

int pm_regulator_init(struct pm_device *pm_device)
{
    return  platform_driver_register(&regulator_driver);
}


void pm_regulator_exit(struct pm_device *pm_device)
{
    platform_driver_unregister(&regulator_driver);
}

