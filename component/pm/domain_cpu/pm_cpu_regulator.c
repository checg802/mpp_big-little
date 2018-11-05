#include <linux/delay.h>

#include "pm_device.h"
#include "pm_cpu.h"
#include "pm_cpu_regulator.h"
#include "hal_cpu_regulator.h"
#include "pm_hal.h"

static int cpu_domain_set_voltage(struct regulator_dev *reg, int min_uV, int max_uV, unsigned *selector);
static int cpu_domain_get_voltage(struct regulator_dev *reg);
static int cpu_domain_get_regulator_step(int min_uV, int max_uV, void __iomem *reg);  
static int cpu_domain_get_regulator_dt_data(struct platform_device *pdev, struct pm_regulator *regulator);

static struct regulator_ops regulator_ops_cpu = {
    .set_voltage    = cpu_domain_set_voltage,
    .get_voltage    = cpu_domain_get_voltage,
};

static struct of_regulator_match cpu_domain_regulator_matches = {
	.name = CPU_REGULATOR_MATCH_NAME,
};

static const struct of_device_id of_cpu_domain_regulator_match[] = {
	{ .compatible = CPU_REGULATOR_compatible, },
	{},
};

static int cpu_domain_set_voltage(struct regulator_dev *reg, int min_uV, int max_uV, unsigned *selector)
{
    struct regulator_config_data *reg_config = rdev_get_drvdata(reg);

    if(reg_config)
    {
        hal_cpu_domain_set_voltage(reg_config->step_uV, reg_config->reg_init->constraints.max_uV, min_uV);
        PM_PRINT("Cur_volt:%d\n",max_uV);
        return 0;
    }
    else
    {
        return -1;
    }
}

static int cpu_domain_get_voltage(struct regulator_dev *reg)
{
    struct regulator_config_data *reg_config = rdev_get_drvdata(reg);
    if(reg_config)
    {
        return hal_cpu_domain_get_voltage(reg_config->step_uV, reg_config->reg_init->constraints.max_uV);
    }
    else
    {
        return -1;
    }
}

static int cpu_domain_get_regulator_step(int min_uV, int max_uV, void __iomem *reg)
{
	return hal_cpu_domain_get_regulator_step(min_uV, max_uV, reg);
}

static int cpu_domain_get_regulator_dt_data(struct platform_device *pdev, struct pm_regulator *regulator)
{
    struct device_node *np = pdev->dev.of_node;
    struct regulator_config_data *reg_config;
    struct device_node *reg_node;
    u32 offset = 0;
    int ret;

    ret = of_regulator_match(&pdev->dev, np, &cpu_domain_regulator_matches, 1);
    of_node_put(np);
    if (ret < 0) 
    {
        PM_PRINT("Parsing of regulator node failed: %d\n",ret);
        return ret;
    }

    reg_config = &regulator->config_data;
    reg_config->reg_init = cpu_domain_regulator_matches.init_data;

    reg_node = cpu_domain_regulator_matches.of_node;

    if (!reg_config->reg_init || !reg_node)
    {
        PM_PRINT("\n");
        return -1;
    }

    ret = of_property_read_u32(reg_node, "reg_offset", &offset);
    if (!ret)
    {
        reg_config->reg = offset + regulator->reg_base;
    }
    else 
    {
        dev_warn(&pdev->dev,"no reg_offset option, use 0 as default\n");
        reg_config->reg = regulator->reg_base;
    }   
    
    return 0;
}

int cpu_domain_regulator_register(struct platform_device *pdev, struct pm_domain_device *pm_device)
{
    int ret;
    
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct regulator_config config = { };	
	
	pm_device->regulator = devm_kzalloc(&pdev->dev, sizeof(*pm_device->regulator), GFP_KERNEL);
	if (!pm_device->regulator)
    {   
        PM_PRINT("devm_kzalloc cpu regulator fail\n");
		return -ENOMEM;
    }
    
	pm_device->regulator->dev = &pdev->dev;
	pm_device->regulator->reg_base = of_iomap(np, 0);
	WARN_ON(!pm_device->regulator->reg_base);
    
    ret = cpu_domain_get_regulator_dt_data(pdev, pm_device->regulator);
    if (ret < 0)
    {   
        goto fail0;
    }
    
    pm_device->regulator->desc.name = pm_device->regulator->config_data.reg_init->constraints.name;
    pm_device->regulator->desc.type = REGULATOR_VOLTAGE;
    pm_device->regulator->desc.ops = &regulator_ops_cpu;
    
    config.dev = &pdev->dev;
    config.init_data = pm_device->regulator->config_data.reg_init;
    config.of_node = cpu_domain_regulator_matches.of_node;
    config.driver_data = &pm_device->regulator->config_data;

    pm_device->regulator->rdev = devm_regulator_register(&pdev->dev, &pm_device->regulator->desc, &config);
    if (IS_ERR(pm_device->regulator->rdev)) 
    {
        goto fail0;
    }

    pm_device->regulator->regulator = devm_regulator_get(&pdev->dev, CPU_REGULATOR_NAME);
    if(!pm_device->regulator->regulator)
    {   
        PM_PRINT("\n");
        goto fail1;
    }
    
    pm_device->regulator->config_data.step_uV = 
            cpu_domain_get_regulator_step(pm_device->regulator->config_data.reg_init->constraints.min_uV,
                   pm_device->regulator->config_data.reg_init->constraints.max_uV,
                   pm_device->regulator->config_data.reg);

    return ret;
    
fail1:
    devm_regulator_unregister(&pdev->dev, pm_device->regulator->rdev);
fail0:
    devm_kfree(&pdev->dev, pm_device->regulator);
    return ret;
}

void cpu_domain_regutator_unregister(struct platform_device *pdev, struct pm_domain_device *pm_device)
{
    devm_regulator_put(pm_device->regulator->regulator);
    devm_regulator_unregister(&pdev->dev, pm_device->regulator->rdev);
    devm_kfree(&pdev->dev, pm_device->regulator);
}

void cpu_regulator_restore(struct pm_domain_device *pm_domain_device)
{
    int cur_volt = pm_domain_device->cur_volt;
    cpu_domain_set_voltage(pm_domain_device->regulator->rdev, cur_volt, cur_volt, NULL);
}

int cpu_check_domain_valid(void)
{
    return hal_cpu_domain_check_domain_valid();
}




