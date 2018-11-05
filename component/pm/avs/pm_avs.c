#include <linux/delay.h>

#include "pm_device.h"
#include "pm_avs.h"

extern struct pm_device hisi_pm_device;

static int avs_probe(struct platform_device *pdev);
static int avs_remove(struct platform_device *pdev);

int every_domain_avs_init(struct platform_device *pdev,struct pm_device *pm_device, PM_DOMAIN_E pm_domain)
{
    if(PM_DOMAIN_CPU == pm_domain && pm_device->cpu_domain_device && pm_device->cpu_domain_device->domain_avs_init)
    {
        return pm_device->cpu_domain_device->domain_avs_init(pdev, pm_device->cpu_domain_device);
    }
    else if(PM_DOMAIN_MEDIA == pm_domain && pm_device->media_domain_device && pm_device->media_domain_device->domain_avs_init)
    {
        return pm_device->media_domain_device->domain_avs_init(pdev, pm_device->media_domain_device);
    }
    else
    {
        return -1;
    }
}

void every_domain_avs_exit(struct platform_device *pdev,struct pm_device *pm_device,PM_DOMAIN_E pm_domain)
{    
    if(PM_DOMAIN_CPU == pm_domain && pm_device->cpu_domain_device && pm_device->cpu_domain_device->domain_avs_exit)
    {
        pm_device->cpu_domain_device->domain_avs_exit(pdev, pm_device->cpu_domain_device);
    }
    else if(PM_DOMAIN_MEDIA == pm_domain && pm_device->media_domain_device && pm_device->media_domain_device->domain_avs_exit)
    {
        pm_device->media_domain_device->domain_avs_exit(pdev, pm_device->media_domain_device);
    }
    else
    {
    }
}

static const struct of_device_id of_pm_avs_match[] = {
	{ .compatible = "hi3519,avs", },
	{},
};

static struct platform_driver avs_driver = {
	.driver = {
		.name = "hi3519-avs",
		.owner = THIS_MODULE,
		.of_match_table = of_pm_avs_match,
	},
	.probe = avs_probe,
	.remove = avs_remove,
};

PM_DOMAIN_E avs_name_to_domain(const char *avs_name)
{
    PM_DOMAIN_E pm_domain = PM_DOMAIN_BUTT;
    
    if(!strcmp(avs_name, "cpu-avs"))
    {
        pm_domain = PM_DOMAIN_CPU;
    }
    else if(!strcmp(avs_name, "media-avs"))
    {
        pm_domain = PM_DOMAIN_MEDIA;
    }
    else if(!strcmp(avs_name, "avs-ddr"))
    {
        pm_domain = PM_DOMAIN_DDR;
    }
    else if(!strcmp(avs_name, "avs-core"))
    {
        pm_domain = PM_DOMAIN_CORE;
    }
    else
    {
        PM_PRINT("Wrong avs name : %s\n", avs_name);
    }
    return pm_domain;
}

static int avs_probe(struct platform_device *pdev)
{
    int ret = 0,i = 0;
    unsigned int avs_num = 0;
    const char *avs_name = NULL;
    PM_DOMAIN_E pm_domain = PM_DOMAIN_BUTT;
    
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;

    ret = of_property_read_u32(np, "avs-num", &avs_num);
    if(ret)
    {
        PM_PRINT("Get avs-num fail\n");
        return ret;
    }

    for (i = 0; i < avs_num; i++) 
    {
		ret = of_property_read_string_index(np, "avs-name-array", i, &avs_name);
		if (ret) 
        {
            PM_PRINT("Get avs-name-array fail\n");
            return ret;
		}
        pm_domain = avs_name_to_domain(avs_name);
        if(pm_domain != PM_DOMAIN_BUTT)
        {
            every_domain_avs_init(pdev, &hisi_pm_device, pm_domain);
        }
        else
        {
            PM_PRINT("Wrong avs name : %s\n", avs_name);
        }
    }
    return 0;
}

static int avs_remove(struct platform_device *pdev)
{
    int ret = 0,i = 0;
    unsigned int avs_num = 0;
    const char *avs_name = NULL;
    PM_DOMAIN_E pm_domain = PM_DOMAIN_BUTT;
    
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;

    ret = of_property_read_u32(np, "avs-num", &avs_num);
    if(ret)
    {
        PM_PRINT("Get avs-num fail\n");
        return ret;
    }

    for (i = 0; i < avs_num; i++) 
    {
		ret = of_property_read_string_index(np, "avs-name-array", i, &avs_name);
		if (ret) 
        {
            PM_PRINT("Get avs-name-array fail\n");
            return ret;
		}
        
        pm_domain = avs_name_to_domain(avs_name);
        if(pm_domain != PM_DOMAIN_BUTT)
        {
            every_domain_avs_exit(pdev, &hisi_pm_device, pm_domain);
        }
        else
        {
            PM_PRINT("Wrong avs name : %s\n", avs_name);
        }
    }
    return 0;
}

int pm_avs_init(struct pm_device *pm_device)
{
    return platform_driver_register(&avs_driver);
}

void pm_avs_exit(struct pm_device *pm_device)
{
    platform_driver_unregister(&avs_driver);
    
}

