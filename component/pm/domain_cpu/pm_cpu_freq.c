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

#include "pm_cpu_freq.h"
#include "hal_cpu_freq.h"
#include "pm_cpu.h"

static unsigned int locking_frequency;

static int cpu_freq_resume(struct cpufreq_policy *policy)
{
	int ret;

	if (!policy->restore_freq) 
    {
		PM_PRINT("restore_freq can't be zero\n");
		return -EINVAL;
	}

	PM_PRINT("Setting restore-freq: %u\n", policy->restore_freq);

	ret = __cpufreq_driver_target(policy, policy->restore_freq, CPUFREQ_RELATION_H);
	if (ret)
    {   
		PM_PRINT("Unable to set suspend-freq: %u. err: %d\n", policy->restore_freq, ret);
    }

	return ret;
}

static int cpu_freq_suspend(struct cpufreq_policy *policy)
{
	int ret;
    struct clk_info *manager_clk = NULL;

    if(MANAGER_CPU == policy->cpu)
    {
        manager_clk = cpu_pm_device.manager_clk;
    }
    else 
    {
        manager_clk = cpu_pm_device.manager_clk_ex;
    }
    
	if (!policy->suspend_freq) 
    {
		PM_PRINT("suspend_freq can't be zero\n");
		return -EINVAL;
	}

	PM_PRINT("Setting suspend-freq: %u\n", policy->suspend_freq);
    
    policy->restore_freq = clk_get_rate(manager_clk->clk) / 1000;

	ret = __cpufreq_driver_target(policy, policy->suspend_freq, CPUFREQ_RELATION_H);
	if (ret)
    {   
		PM_PRINT("Unable to set suspend-freq: %u. err: %d\n", policy->suspend_freq, ret);
    }

	return ret;
}

static struct cpufreq_driver cpufreq_driver = {
	.flags = CPUFREQ_STICKY | CPUFREQ_NEED_INITIAL_FREQ_CHECK,
	.verify = cpufreq_generic_frequency_table_verify,
	.target_index = cpu_freq_target_index,
	.get = cpufreq_generic_get,
	.init = cpufreq_init,
	.suspend = cpu_freq_suspend,
	.resume = cpu_freq_resume,
	.name = CPUFREQ_NAME,
	.attr = cpufreq_generic_attr,
};

struct clk *cpu_freq_get_clk(unsigned int index, unsigned long rate)
{
    struct clk *clk,*parent;
    clk = hal_cpu_freq_get_clk(index,rate);
    if(!clk)
    {
        parent = clk_get_parent(cpu_pm_device.manager_clk->clk);
        if(parent == cpu_pm_device.var_clks->clk)
        {
            clk_set_parent(cpu_pm_device.manager_clk->clk, hal_cpu_freq_get_lowest_fixed_clk());
        }
        clk_set_rate(cpu_pm_device.var_clks->clk, rate * 1000);
        mdelay(1);
        if(parent == cpu_pm_device.var_clks->clk)
        {
            clk_set_parent(cpu_pm_device.manager_clk->clk, cpu_pm_device.var_clks->clk);
        }
        return cpu_pm_device.var_clks->clk;
    }
    else
    {
        return clk;
    }
}

int cpufreq_init(struct cpufreq_policy *policy)
{
    int ret;
    unsigned int suspend_freq;
    struct clk_info *manager_clk = NULL;
    struct cpufreq_frequency_table *freq_table = NULL;

    if(MANAGER_CPU == MANAGER_CPU_EX)
    {
        if (policy->cpu != MANAGER_CPU)
            return -EINVAL; 
    }
    
    if(MANAGER_CPU == policy->cpu)
    {
        manager_clk = cpu_pm_device.manager_clk;
        suspend_freq = locking_frequency;
        freq_table = cpu_pm_device.freq_table;
    }
    else
    {
        manager_clk = cpu_pm_device.manager_clk_ex;
        suspend_freq = clk_get_rate(manager_clk->clk);
        freq_table = cpu_pm_device.freq_table_ex;
    }

    policy->clk = manager_clk->clk;
    policy->suspend_freq = suspend_freq;
    
    cpumask_copy(policy->cpus, cpumask_of(policy->cpu));
    policy->cpuinfo.transition_latency = cpu_pm_device.latency;

    ret = cpufreq_table_validate_and_show(policy, freq_table);
    if (ret) 
    {
        PM_PRINT("Invalid frequency table: %d\n", ret);
        return ret;
    }
    
    return 0;
}

int cpu_freq_target_index(struct cpufreq_policy *policy, unsigned int index)
{
	int ret = 0;
	struct clk *clk;
	struct device *cpu_dev;
    struct dev_pm_opp *opp;
	unsigned long new_freq, old_freq, freq_hz,volt;

    mutex_lock(&cpu_pm_device.freq_table_lock);
    mutex_lock(&cpu_pm_device.dvfs_lock);
    rcu_read_lock();

    if(cpu_pm_device.cpufreq_en == 0)
    {
        ret= -EAGAIN;
        goto done0;
    }

	new_freq = cpu_pm_device.freq_table[index].frequency;
	old_freq = clk_get_rate(cpu_pm_device.manager_clk->clk);
	freq_hz = new_freq * 1000;

    cpu_domain_avs_pause(&cpu_pm_device);

    cpu_dev = get_cpu_device(MANAGER_CPU);
    if (!cpu_dev) 
    {
        PM_PRINT("Failed to get hisi cpufreq device\n");
        goto done1;
    }
    
	opp = dev_pm_opp_find_freq_ceil(cpu_dev, &freq_hz);
	if (IS_ERR(opp)) 
    {
		dev_err(cpu_dev, "failed to find OPP for %ld\n", freq_hz);
        goto done1;
	}
    
	volt = dev_pm_opp_get_voltage(opp);
    
    PM_PRINT("CPU: %d  old_freq %lu   new_freq: %lu  \n", policy->cpu, old_freq, new_freq);
    
	if (freq_hz >= old_freq) 
    {
		ret = regulator_set_voltage(cpu_pm_device.regulator->regulator, volt, volt);
		if (ret) 
        {
			PM_PRINT("Failed to set cpu voltage to %lu\n", volt);
            goto done1;
		}
        
        clk = cpu_freq_get_clk(index, new_freq);        
        ret = clk_set_parent(cpu_pm_device.manager_clk->clk, clk);
    	if (ret) 
        {
    		PM_PRINT("Failed to set clock rate: %d\n", ret);
            goto done1;
    	}
	} 
    else if (freq_hz < old_freq) 
    {
        clk = cpu_freq_get_clk(index, new_freq);

        ret = clk_set_parent(cpu_pm_device.manager_clk->clk, clk);
        if (ret) 
        {
            PM_PRINT("Failed to set clock rate: %d\n", ret);
            goto done1;
        }

        
        PM_PRINT("regulator: %p,volt %ld \n", cpu_pm_device.regulator,volt);
        ret = regulator_set_voltage(cpu_pm_device.regulator->regulator, volt, volt);
        PM_PRINT("regulator: %p,volt %ld \n", cpu_pm_device.regulator,volt);
		if (ret) 
        {
			PM_PRINT("Failed to set cpu voltage to %lu\n", volt);
            goto done1;
		}
    }
    
    cpu_pm_device.cur_volt = regulator_get_voltage(cpu_pm_device.regulator->regulator);
    cpu_pm_device.cur_freq_volt = volt;

done1:
    
    cpu_domain_avs_resume(&cpu_pm_device);

done0:    
    rcu_read_unlock();
    mutex_unlock(&cpu_pm_device.dvfs_lock);
    mutex_unlock(&cpu_pm_device.freq_table_lock);
		
	return ret;
}

int cpu_domain_freq_init_ex(struct pm_domain_device *pm_device)
{
    int num, ret;
    struct device *cpu_dev;
    
    cpu_dev = get_cpu_device(MANAGER_CPU_EX);
    if (!cpu_dev) 
    {
        PM_PRINT("Failed to get hisi cpufreq device\n");
        return -ENODEV;
    }

    
    num = dev_pm_opp_get_opp_count(cpu_dev);
    if (num < 0) 
    {
        ret = of_init_opp_table(cpu_dev);
        if (ret < 0) 
        {
            PM_PRINT("Failed to init OPP table: %d\n", ret);
            goto fail0;
        }
        num = dev_pm_opp_get_opp_count(cpu_dev);
    }
        
    mutex_lock(&pm_device->freq_table_lock);
    ret = dev_pm_opp_init_cpufreq_table(cpu_dev, &pm_device->freq_table_ex);
    mutex_unlock(&pm_device->freq_table_lock);
    if (ret) 
    {
        PM_PRINT("Failed to init cpufreq table: %d\n", ret);
        goto fail0;
    }
    
    pm_device->manager_clk_ex = kzalloc(sizeof(struct clk_info), GFP_KERNEL);
    if (!pm_device->manager_clk_ex) 
    {
        PM_PRINT("Failed to allocate mem for clk\n");
        goto fail1;
    }
    
    strncpy(pm_device->manager_clk_ex->name, MANAGER_CLK_EX, MAX_CLK_NAME_LEN);
    
    pm_device->manager_clk_ex->clk = clk_get(cpu_dev, pm_device->manager_clk_ex->name);
    if(IS_ERR(pm_device->manager_clk_ex->clk))
    {
        PM_PRINT("Failed to get clocks\n");
        ret = -ENOENT;
        goto fail2;
    } 

    return 0;

fail2:
    kfree(pm_device->manager_clk->clk);
    
fail1:
    dev_pm_opp_free_cpufreq_table(cpu_dev, &pm_device->freq_table_ex);

fail0:
	return ret;
}

void cpu_domain_freq_exit_ex(struct pm_domain_device *pm_device)
{
    struct device *cpu_dev;
    
    cpu_dev = get_cpu_device(MANAGER_CPU_EX);
    if (!cpu_dev) 
    {
        PM_PRINT("Failed to get hisi cpufreq device\n");
        return ;
    }

    clk_put(pm_device->manager_clk_ex->clk);
        
    kfree(pm_device->manager_clk_ex);
    
    mutex_lock(&pm_device->freq_table_lock);
	dev_pm_opp_free_cpufreq_table(cpu_dev, &pm_device->freq_table_ex);
    mutex_unlock(&pm_device->freq_table_lock);
}

int cpu_domain_freq_init(struct pm_domain_device *pm_device)
{
    struct device *cpu_dev;
	struct device_node *np;
	int num, ret, i;
    unsigned long freq,volt;
    struct dev_pm_opp *opp;

    mutex_init(&pm_device->freq_table_lock);
    mutex_init(&pm_device->dvfs_lock);
    
    if(!of_machine_is_compatible(COMPATIBLE))
    {
        PM_PRINT("Find no device compatible with %s \n", COMPATIBLE);
        return -ENODEV;
    }

    cpu_dev = get_cpu_device(MANAGER_CPU);
	if (!cpu_dev) 
    {
		PM_PRINT("Failed to get hisi cpufreq device\n");
		return -ENODEV;
	}

    np = of_node_get(cpu_dev->of_node);
	if (!np)
    {
		PM_PRINT("Failed to find cpufreq node\n");
		return -ENOENT;
	}
    
    if (of_property_read_u32(np, "clock-latency", &pm_device->latency))
    {
	    pm_device->latency = DEF_TRANS_LATENCY;
    }
    
	of_node_put(np);

    /*"operating-points" int hisi-hi3519v101-hmp-demb.dts*/
    num = dev_pm_opp_get_opp_count(cpu_dev);
    if (num < 0) 
    {
        ret = of_init_opp_table(cpu_dev);
        if (ret < 0) 
        {
            PM_PRINT("Failed to init OPP table: %d\n", ret);
            goto fail0;
        }
        num = dev_pm_opp_get_opp_count(cpu_dev);
    }
    
    mutex_lock(&pm_device->freq_table_lock);
    ret = dev_pm_opp_init_cpufreq_table(cpu_dev, &pm_device->freq_table);
    mutex_unlock(&pm_device->freq_table_lock);
    if (ret) 
    {
        PM_PRINT("Failed to init cpufreq table: %d\n", ret);
        goto fail0;
    }
    
    pm_device->manager_clk = kzalloc(sizeof(struct clk_info), GFP_KERNEL);
    if (!pm_device->manager_clk) 
    {
		PM_PRINT("Failed to allocate mem for dvfs_info\n");
		goto fail1;
	}
    
    pm_device->var_clks = kzalloc(sizeof(struct clk_info), GFP_KERNEL);
    if (!pm_device->var_clks) 
    {
        PM_PRINT("Failed to allocate mem for dvfs_info\n");
        goto fail2;
    }

    strncpy(pm_device->manager_clk->name, MANAGER_CLK, MAX_CLK_NAME_LEN);
    strncpy(pm_device->var_clks->name, VARIABLE_CLK, MAX_CLK_NAME_LEN);

    
    pm_device->manager_clk->clk = clk_get(cpu_dev, pm_device->manager_clk->name);
    pm_device->var_clks->clk = clk_get(cpu_dev, pm_device->var_clks->name);
    if(IS_ERR(pm_device->manager_clk->clk) || IS_ERR(pm_device->var_clks->clk))
    {
        PM_PRINT("Failed to get clocks\n");
		ret = -ENOENT;
		goto fail3;
    } 

    ret = hal_cpu_freq_init_fixed_clk(cpu_dev);
    if(ret)
    {
        goto fail4;
    }

    locking_frequency = clk_get_rate(pm_device->manager_clk->clk) / 1000;
    PM_PRINT("locking_frequency = %d\n",locking_frequency);
    
    mutex_lock(&pm_device->dvfs_lock);
    pm_device->cpufreq_en = 1;
    mutex_unlock(&pm_device->dvfs_lock);

    pm_device->cur_freq_volt = 0;
    for(i = 0;i < num;i++)
    {
        freq = pm_device->freq_table[i].frequency * 1000;
        opp = dev_pm_opp_find_freq_ceil(cpu_dev, &freq);
        volt = dev_pm_opp_get_voltage(opp);
        if(pm_device->cur_freq_volt < volt)
        {
            pm_device->cur_freq_volt = volt;
        }
    }

    if(MANAGER_CPU != MANAGER_CPU_EX)
    {
        ret = cpu_domain_freq_init_ex(pm_device);
        if (ret) 
        {
        	PM_PRINT("\n");
        	goto fail5;
        }
    }

    ret = cpufreq_register_driver(&cpufreq_driver);
	if (ret) 
    {
		PM_PRINT("Failed to register cpufreq driver\n");
		goto fail5;
	}

    pm_device->cur_volt = regulator_get_voltage(cpu_pm_device.regulator->regulator);
    
	return 0;
    
fail5:
    hal_cpu_freq_release_fixed_clk();
    
fail4:
    clk_put(pm_device->manager_clk->clk);
    clk_put(pm_device->var_clks->clk);
    
fail3:
    kfree(pm_device->var_clks->clk);
    
fail2:
    kfree(pm_device->manager_clk->clk);
    
fail1:
    dev_pm_opp_free_cpufreq_table(cpu_dev, &pm_device->freq_table);

fail0:
	return ret;
}

void cpu_domain_freq_exit(struct pm_domain_device *pm_device)
{
    struct device *cpu_dev;
    
    cpu_dev = get_cpu_device(MANAGER_CPU);
    if (!cpu_dev) 
    {
        PM_PRINT("Failed to get hisi cpufreq device\n");
        return ;
    }
    
    mutex_lock(&pm_device->dvfs_lock);
    pm_device->cpufreq_en = 0;
    mutex_unlock(&pm_device->dvfs_lock);

    cpufreq_unregister_driver(&cpufreq_driver);
    clk_put(pm_device->manager_clk->clk);
    clk_put(pm_device->var_clks->clk);
    
    hal_cpu_freq_release_fixed_clk();
    
    kfree(pm_device->manager_clk);
    kfree(pm_device->var_clks);
    
    mutex_lock(&pm_device->freq_table_lock);
	dev_pm_opp_free_cpufreq_table(cpu_dev, &pm_device->freq_table);
    mutex_unlock(&pm_device->freq_table_lock);

    if(MANAGER_CPU != MANAGER_CPU_EX)
    {
        cpu_domain_freq_exit_ex(pm_device);
    }
}

void cpu_freq_freeze(struct pm_domain_device *pm_domain_device)
{
    if(!pm_domain_device)
    {
       return ;
    }

    mutex_lock(&pm_domain_device->dvfs_lock);
    pm_domain_device->cpufreq_en = 0;
    mutex_unlock(&pm_domain_device->dvfs_lock);
}

void cpu_freq_restore(struct pm_domain_device *pm_domain_device)
{
    if(!pm_domain_device)
    {
        return ;
    }
    
    mutex_lock(&pm_domain_device->dvfs_lock);
    pm_domain_device->cpufreq_en = 1;
    mutex_unlock(&pm_domain_device->dvfs_lock);
}


