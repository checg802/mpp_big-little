#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/slab.h>

//#include "avs_pm.h"
#include "pm_cpu.h"
#include "pm_cpu_avs.h"
#include "hal_cpu_avs.h"
#include "pm_hal.h"

int cpu_avs_inter = 20;
int cpu_avs_en = 1;

module_param(cpu_avs_inter, int, S_IRUGO);
module_param(cpu_avs_en, int, S_IRUGO);

static int cpu_update_cur_avs_info(struct avs_device *avs_dev, unsigned long cur_freq_volt)
{
    int i;
    if(NULL == avs_dev->freq_hpm_table)
        return -1;
    /* get default hpm record */
    for (i = 0; i < avs_dev->max_hpm_tale_index; i++)
    {
        if (avs_dev->cur_freq/1000 == avs_dev->freq_hpm_table[i].freq)
        {
            avs_dev->cur_hpm      = avs_dev->freq_hpm_table[i].hpmrecord;
            avs_dev->cur_volt_min = avs_dev->freq_hpm_table[i].vmin;            
            avs_dev->div          = avs_dev->freq_hpm_table[i].div; 
            avs_dev->cur_volt_max = cur_freq_volt;
            
            //PM_PRINT("Cur_freq: %d cur_hpm: %d div: %d\n",avs_dev->cur_freq,avs_dev->cur_hpm,avs_dev->div);
            /* cpu HPM freq */
            hal_cpu_set_hpm_div(avs_dev->div);
            return 0;
        }
    }
    return -1;
}

static void cpu_hpm_init(void)
{
    hal_cpu_hpm_init();
}

void cpu_avs_pause(struct pm_domain_device *pm_device)
{
    struct avs_device *avs_dev = &pm_device->avs_dev;
    
    mutex_lock(&avs_dev->avs_lock);
    avs_dev->avs_enable = 0;
}

void cpu_avs_resume(struct pm_domain_device *pm_device)
{
    struct avs_device *avs_dev = &pm_device->avs_dev;
    
    avs_dev->cur_freq   = clk_get_rate(cpu_pm_device.manager_clk->clk);
    avs_dev->avs_enable = cpu_avs_en;
    cpu_update_cur_avs_info(avs_dev, pm_device->cur_freq_volt);
    mutex_unlock(&avs_dev->avs_lock);

    return;
}

static int cpu_get_average_hpm(void)
{
    return hal_cpu_get_average_hpm();
}

static void cpu_set_volt_accord_hpm(struct pm_domain_device *pm_device, int hpm_delta, unsigned int volt_min, unsigned int volt_max)
{
    int ret;
    struct regulator *regulator = pm_device->regulator->regulator;
    
    if (hpm_delta < CPU_AVS_HPM_DELTA_MIN)
    {            
        //PM_PRINT("Hpm_delta: %d  cur_volt:%d\n",hpm_delta,pm_device->cur_volt);
        /* up 10mv */
        if (pm_device->cur_volt +  CPU_AVS_VOLT_STEP <= volt_max)
        {
            ret = regulator_set_voltage(regulator, pm_device->cur_volt + CPU_AVS_VOLT_STEP, pm_device->cur_volt + CPU_AVS_VOLT_STEP);  
            if (ret)
            {
                PM_PRINT("Scale volt to %u fail\n", pm_device->cur_volt + CPU_AVS_VOLT_STEP);
            }
        }
    }
    else if (hpm_delta >= CPU_AVS_HPM_DELTA_MAX)
    {            
        //PM_PRINT("Hpm_delta: %d  cur_volt:%d\n",hpm_delta,pm_device->cur_volt);
        /*down 10mv */   
        if (pm_device->cur_volt - CPU_AVS_VOLT_STEP >= volt_min)
        {
            ret = regulator_set_voltage(regulator, pm_device->cur_volt - CPU_AVS_VOLT_STEP, pm_device->cur_volt - CPU_AVS_VOLT_STEP);
            if (ret)
            {
                PM_PRINT("Scale volt to %u fail\n",pm_device->cur_volt - CPU_AVS_VOLT_STEP);
            }
        }
        
    }
}

int cpu_avs_thread(void *Arg)
{
    int cur_average_hpm = 0,delta = 0;
    struct pm_domain_device *pm_device = (struct pm_domain_device *)Arg;
    struct avs_device *avs_dev = &(pm_device->avs_dev);

    while (!kthread_should_stop())
    {
        msleep(cpu_avs_inter);

        if (!avs_dev->avs_enable)
        {
            continue;
        }
        mutex_lock(&avs_dev->avs_lock);
        
        cur_average_hpm = cpu_get_average_hpm();
        delta = cur_average_hpm - avs_dev->cur_hpm;
        
        //PM_PRINT("Cur_average_hpm: %d cur_hpm :%d cur_volt:%d\n",cur_average_hpm,avs_dev->cur_hpm,pm_device->cur_volt);
        cpu_set_volt_accord_hpm(pm_device, delta, avs_dev->cur_volt_min, avs_dev->cur_volt_max);

        pm_device->cur_volt = regulator_get_voltage(pm_device->regulator->regulator);
        mutex_unlock(&avs_dev->avs_lock);
    }

    return 0;
}

static struct avs_ops cpu_avs_ops = 
{
    .avs_pause = cpu_avs_pause,
    .avs_resume = cpu_avs_resume,
};


int cpu_avs_init(struct platform_device *pdev, struct pm_domain_device *pm_device)
{
    int i,found = 0;
    int ret = -1;
    const char *avs_name;
    unsigned int opp_num = 0;
    unsigned int opp_volt_min[10],opp_div[10];
    unsigned int opp_hpm[10],opp_freq[10];
    
    struct device *dev = &pdev->dev;
	struct device_node *tmp_np,*np = dev->of_node;
    struct avs_device *avs_dev = &pm_device->avs_dev;

    for_each_child_of_node(np, tmp_np)
    {
        of_property_read_string(tmp_np, "avs-name", &avs_name);
        if(strcmp(avs_name, "cpu-avs"))
        {
            continue;
        }
        else
        {
            PM_PRINT("Avs_name : %s\n", avs_name);
            found = 1;
            break;
        }
    }

    if(!found)
    {
        PM_PRINT("Get cpu avs device fail\n");
        return -1;
    }

    ret = of_property_read_u32(tmp_np, "opp-num", &opp_num);
    if (ret)
    {
        PM_PRINT("Read opp-num err.\n");
        return -1;
    } 
    
    ret = of_property_read_u32_array(tmp_np, "opp-volt-min", opp_volt_min, opp_num);
    if (ret)
    {
        PM_PRINT("Read opp-volt-min err.\n");
        return -1;
    } 
    
    ret = of_property_read_u32_array(tmp_np, "opp-freq", opp_freq, opp_num);
    if (ret)
    {
        PM_PRINT("Read opp-freq err.\n");
        return -1;
    } 
    
    ret = of_property_read_u32_array(tmp_np, "opp-hpm", opp_hpm, opp_num);
    if (ret)
    {
        PM_PRINT("Read opp-hpm err.\n");
        return -1;
    } 
    
    ret = of_property_read_u32_array(tmp_np, "opp-div", opp_div, opp_num);
    if (ret)
    {
        PM_PRINT("Read opp-div err.\n");
        return -1;
    } 

    avs_dev->freq_hpm_table = kmalloc(sizeof(struct hpm_opp) * opp_num, GFP_KERNEL);
    if (!avs_dev->freq_hpm_table)
    {
        PM_PRINT("Malloc hpm table err.\n");
        return -1;
    }   
    
    for(i = 0;i <opp_num;i++ )
    {
        avs_dev->freq_hpm_table[i].div = opp_div[i];
        avs_dev->freq_hpm_table[i].freq = opp_freq[i];
        avs_dev->freq_hpm_table[i].hpmrecord = opp_hpm[i];
        avs_dev->freq_hpm_table[i].vmin = opp_volt_min[i];
        //PM_PRINT("Volt : %u    freq : %u    hpm : %u     div : %d\n",opp_volt_min[i],opp_freq[i],opp_hpm[i],opp_div[i]);
    }

    avs_dev->max_hpm_tale_index = opp_num;
    avs_dev->cur_freq           = clk_get_rate(cpu_pm_device.manager_clk->clk);
    avs_dev->avs_ops            = &cpu_avs_ops;    

    cpu_update_cur_avs_info(avs_dev, pm_device->cur_freq_volt);

    cpu_hpm_init();

    avs_dev->avs_enable = true;
    
    mutex_init(&avs_dev->avs_lock);

    avs_dev->avs_thread = kthread_create(cpu_avs_thread, (void*)pm_device, "avs_cpu");
    if (IS_ERR(avs_dev->avs_thread))
    {
        PM_PRINT("Create avs thread failed.\n");
        kfree(avs_dev->freq_hpm_table);
        return -1;
    }
    if(1 == cpu_avs_en)
    {
       PM_PRINT("Cpu avs  enable.\n");
       wake_up_process(avs_dev->avs_thread);
    }
    return 0;
}

void cpu_avs_exit(struct platform_device *pdev, struct pm_domain_device *pm_device)
{
    if (cpu_avs_en && pm_device->avs_dev.avs_thread)
    {
        kthread_stop(pm_device->avs_dev.avs_thread);
        pm_device->avs_dev.avs_thread = NULL;
    }
    
    if (pm_device->avs_dev.freq_hpm_table)
    {
        kfree(pm_device->avs_dev.freq_hpm_table);
        pm_device->avs_dev.freq_hpm_table = NULL;
    }
    return;
}

void cpu_avs_freeze(struct pm_domain_device *pm_domain_device)
{
    struct avs_device *avs_dev = &pm_domain_device->avs_dev;
    
    mutex_lock(&avs_dev->avs_lock);
    avs_dev->avs_enable = 0;
    mutex_unlock(&avs_dev->avs_lock);
}

void cpu_avs_restore(struct pm_domain_device *pm_domain_device)
{
    struct avs_device *avs_dev = &pm_domain_device->avs_dev;
    
    cpu_hpm_init();
    
    mutex_lock(&avs_dev->avs_lock);
    avs_dev->avs_enable = cpu_avs_en;
    avs_dev->cur_freq   = clk_get_rate(pm_domain_device->manager_clk->clk);
    cpu_update_cur_avs_info(avs_dev, pm_domain_device->cur_freq_volt);
    mutex_unlock(&avs_dev->avs_lock);
}

