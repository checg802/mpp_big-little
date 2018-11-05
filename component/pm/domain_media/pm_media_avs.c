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
#include "pm_media.h"
#include "pm_media_avs.h"
#include "hal_media_avs.h"
#include "pm_hal.h"

int media_avs_inter = 20;
int media_avs_en = 1;

module_param(media_avs_inter, int, S_IRUGO);
module_param(media_avs_en, int, S_IRUGO);


static int media_get_average_temperature(void)
{
    return hal_media_get_average_temperature();
}

static int media_get_profile(void)
{
    return hal_media_get_profile();
}


static int media_update_cur_avs_info(struct avs_device *avs_dev)
{
    int i;
    int temperature = 0;
    unsigned int temperature_index = 0;
    
    /*Get current temperature and profile*/
    if(avs_dev->avs_temp_num > 1)
    {
        temperature = media_get_average_temperature();
        
        for (i = 0; i < avs_dev->avs_temp_num; i++)
        {
            if(temperature > avs_dev->avs_temp[i])
            {
                temperature_index++;
            }
            else
            {
                break;
            }
        }

        if(temperature_index != avs_dev->temperature_index)
        {
            if ( temperature > avs_dev->temperature)
            {
                if(temperature_index > 0)
                {
                    if((temperature - avs_dev->avs_temp[temperature_index - 1]) >= MEDIA_AVS_TEMP_DELTA_MIN)
                    {
                        avs_dev->temperature        = temperature;
                        avs_dev->temperature_index  = temperature_index;
                    }
                }
            }
            else
            {
                if((avs_dev->avs_temp[temperature_index] - temperature ) >= MEDIA_AVS_TEMP_DELTA_MIN)
                {
                    avs_dev->temperature        = temperature;
                    avs_dev->temperature_index  = temperature_index;
                }
            }
        }

        PM_PRINT("temperature: %d temperature_index: %d\n",temperature,avs_dev->temperature_index);
    }

    avs_dev->cur_freq =  avs_dev->temperature_index*avs_dev->avs_temp_num + avs_dev->avs_profile + 1;
        
    for (i = 0; i < avs_dev->max_hpm_tale_index; i++)
    {
        if (avs_dev->cur_freq  == avs_dev->freq_hpm_table[i].freq)
        {
            avs_dev->cur_hpm      = avs_dev->freq_hpm_table[i].hpmrecord;
            avs_dev->cur_volt_min = avs_dev->freq_hpm_table[i].vmin;
            avs_dev->cur_volt_max = avs_dev->freq_hpm_table[i].vmax;
            avs_dev->div          = avs_dev->freq_hpm_table[i].div; 
            //PM_PRINT("Cur_freq: %d cur_hpm: %d div: %d vmin: %d vmax %d \n",avs_dev->cur_freq,avs_dev->cur_hpm,avs_dev->div,avs_dev->cur_volt_min,avs_dev->cur_volt_max);
            /* media HPM freq */
            hal_media_set_hpm_div(avs_dev->div);
            return 0;
        }
    }
    return -1;
}

static void media_hpm_init(void)
{
    hal_media_hpm_init();
}

static void media_tsensor_init(void)
{
    hal_media_tsensor_init();
}



void media_avs_pause(struct pm_domain_device *pm_device)
{
    struct avs_device *avs_dev = &pm_device->avs_dev;
    
    mutex_lock(&avs_dev->avs_lock);
    avs_dev->avs_enable = 0;
}

void media_avs_resume(struct pm_domain_device *pm_device)
{
    struct avs_device *avs_dev = &pm_device->avs_dev;
    
    avs_dev->avs_enable = media_avs_en;
    media_update_cur_avs_info(avs_dev);
    mutex_unlock(&avs_dev->avs_lock);
}

static int media_get_average_hpm(void)
{
    return hal_media_get_average_hpm();
}




static void media_set_volt_accord_hpm(struct pm_domain_device *pm_device, int hpm_delta, unsigned int volt_min, unsigned int volt_max)
{
    int ret;
    struct regulator *regulator = pm_device->regulator->regulator;
    
    if(pm_device->cur_volt > volt_max)
    {
        ret = regulator_set_voltage(regulator, pm_device->cur_volt - MEDIA_AVS_VOLT_STEP, pm_device->cur_volt - MEDIA_AVS_VOLT_STEP);
        if (ret)
        {
            PM_PRINT("Scale volt to %u falt\n", pm_device->cur_volt - MEDIA_AVS_VOLT_STEP);
        }
    }
    else if (pm_device->cur_volt < volt_min)
    {
        ret = regulator_set_voltage(regulator, pm_device->cur_volt + MEDIA_AVS_VOLT_STEP, pm_device->cur_volt + MEDIA_AVS_VOLT_STEP);  
        if (ret)
        {
            PM_PRINT("Scale volt to %u falt\n", pm_device->cur_volt + MEDIA_AVS_VOLT_STEP);
        }
    }
    else if (hpm_delta < MEDIA_AVS_HPM_DELTA_MIN)
    {       
        //PM_PRINT(">>>>>>Hpm_delta: %d  cur_volt:%d\n",hpm_delta,pm_device->cur_volt);
        /* up 10mv */
        if (pm_device->cur_volt +  MEDIA_AVS_VOLT_STEP <= volt_max)
        {
            ret = regulator_set_voltage(regulator, pm_device->cur_volt + MEDIA_AVS_VOLT_STEP, pm_device->cur_volt + MEDIA_AVS_VOLT_STEP);  
            if (ret)
            {
                PM_PRINT("Scale volt to %u falt\n", pm_device->cur_volt + MEDIA_AVS_VOLT_STEP);
            }
        }
    }
    else if (hpm_delta >= MEDIA_AVS_HPM_DELTA_MAX)
    {            
        //PM_PRINT(">>>>>>Hpm_delta: %d  cur_volt:%d\n",hpm_delta,pm_device->cur_volt);
        /*down 10mv */   
        if (pm_device->cur_volt - MEDIA_AVS_VOLT_STEP >= volt_min)
        {
            ret = regulator_set_voltage(regulator, pm_device->cur_volt - MEDIA_AVS_VOLT_STEP, pm_device->cur_volt - MEDIA_AVS_VOLT_STEP);
            if (ret)
            {
                PM_PRINT("Scale volt to %u falt\n", pm_device->cur_volt - MEDIA_AVS_VOLT_STEP);
            }
        }
        
    }
}

int media_avs_thread(void *Arg)
{
    int cur_average_hpm = 0,delta = 0;
    struct pm_domain_device *pm_device = (struct pm_domain_device *)Arg;
    struct avs_device *avs_dev = &(pm_device->avs_dev);
    
    while (!kthread_should_stop())
    {
        msleep(media_avs_inter);

        if (!avs_dev->avs_enable)
        {
            continue;
        }

        mutex_lock(&avs_dev->avs_lock);
        
        media_update_cur_avs_info(avs_dev);

        cur_average_hpm = media_get_average_hpm();
        delta = cur_average_hpm - avs_dev->cur_hpm;

        media_set_volt_accord_hpm(pm_device,delta, avs_dev->cur_volt_min, avs_dev->cur_volt_max);

        pm_device->cur_volt = regulator_get_voltage(pm_device->regulator->regulator);
        
        PM_PRINT("Cur_average_hpm: %d cur_hpm :%d cur_volt:%d\n",cur_average_hpm,avs_dev->cur_hpm,pm_device->cur_volt);
        
        mutex_unlock(&avs_dev->avs_lock);
    }

    return 0;
}

static struct avs_ops media_avs_ops = 
{
    .avs_pause = media_avs_pause,
    .avs_resume = media_avs_resume,
};

int media_avs_init(struct platform_device *pdev, struct pm_domain_device *pm_device)
{
    int i,found = 0;
    int ret = -1;
    const char *avs_name;
    unsigned int opp_num = 0;
    unsigned int opp_prof_num = 0;
    unsigned int opp_temp_num = 0;
    unsigned int opp_volt_min[10],opp_volt_max[10],opp_div[10];
    unsigned int opp_hpm[10],opp_freq[10];
    int opp_temp[10];
    struct device *dev = &pdev->dev;
	struct device_node *tmp_np,*np = dev->of_node;
    struct avs_device *avs_dev = &pm_device->avs_dev;

    for_each_child_of_node(np, tmp_np)
    {
        of_property_read_string(tmp_np, "avs-name", &avs_name);
        if(strcmp(avs_name, "media-avs"))
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
    if(ret)
    {
        PM_PRINT("Read opp-num err.\n");
        return -1;
    }

    ret = of_property_read_u32(tmp_np, "opp-prof-num", &opp_prof_num);
    if(ret)
    {
        PM_PRINT("Read opp-prof-num err.\n");
        return -1;
    }

    ret = of_property_read_u32(tmp_np, "opp-temp-num", &opp_temp_num);
    if(ret)
    {
        PM_PRINT("Read opp-temp-num err.\n");
        return -1;
    }

    if(opp_temp_num*opp_prof_num != opp_num)
    {
        PM_PRINT("opp_temp_num(%d)*opp_prof_num(%d) != opp_num(%d).\n",opp_temp_num,opp_prof_num,opp_num);
        return -1;
    }

    ret = of_property_read_u32_array(tmp_np, "opp-temp", opp_temp, opp_temp_num);
    if(ret)
    {
        PM_PRINT("Read opp-temp err.\n");
        return -1;
    }
    
    ret = of_property_read_u32_array(tmp_np, "opp-volt-min", opp_volt_min, opp_num);
    if(ret)
    {
        PM_PRINT("Read opp-volt-min err.\n");
        return -1;
    }

    ret = of_property_read_u32_array(tmp_np, "opp-volt-max",opp_volt_max, opp_num);
    if(ret)
    {
        PM_PRINT("Read opp-volt-max err.\n");
        return -1;
    }
    
    ret = of_property_read_u32_array(tmp_np, "opp-freq", opp_freq, opp_num);
    if(ret)
    {
        PM_PRINT("Read opp-freq err.\n");
        return -1;
    }
    ret = of_property_read_u32_array(tmp_np, "opp-hpm", opp_hpm, opp_num);
    if(ret)
    {
        PM_PRINT("Read opp-hpm err.\n");
        return -1;
    }
    ret = of_property_read_u32_array(tmp_np, "opp-div", opp_div, opp_num);
    if(ret)
    {
        PM_PRINT("Read opp-div err.\n");
        return -1;
    }

    avs_dev->freq_hpm_table = kmalloc(sizeof(struct hpm_opp)*opp_num, GFP_KERNEL);
    if (!avs_dev->freq_hpm_table)
    {
        PM_PRINT("Malloc hpm table err.\n");
        return -1;
    }   
    
    for(i = 0;i <opp_num;i++ )
    {
        avs_dev->freq_hpm_table[i].div          = opp_div[i];
        avs_dev->freq_hpm_table[i].freq         = opp_freq[i];
        avs_dev->freq_hpm_table[i].hpmrecord    = opp_hpm[i];
        avs_dev->freq_hpm_table[i].vmin         = opp_volt_min[i];
        avs_dev->freq_hpm_table[i].vmax         = opp_volt_max[i];
        //PM_PRINT("Volt : %u    freq : %u    hpm : %u     div : %d\n",opp_volt_min[i],opp_freq[i],opp_hpm[i],opp_div[i]);
    }

    avs_dev->max_hpm_tale_index = opp_num;

    avs_dev->cur_freq           = 1;
    avs_dev->temperature_index  = 0;
    avs_dev->temperature        = media_get_average_temperature();
    avs_dev->avs_ops            = &media_avs_ops;  
    
    
    avs_dev->avs_temp_num       = opp_temp_num;
    
    for(i = 0;i <opp_temp_num;i++ )
    {
        avs_dev->avs_temp[i]          = opp_temp[i];
    }

    
    avs_dev->avs_prof_num       = opp_prof_num;

    if(avs_dev->avs_prof_num > 1)
    {
        avs_dev->avs_profile = media_get_profile();
        PM_PRINT("avs_profile: %d\n",avs_dev->avs_profile);
    }
    else
    {
        avs_dev->avs_profile = 0;
    }

    
    media_update_cur_avs_info(avs_dev);
    
    media_hpm_init();
    
    media_tsensor_init();
    
    avs_dev->avs_enable = true;

    mutex_init(&avs_dev->avs_lock);

    avs_dev->avs_thread = kthread_create(media_avs_thread, (void*)pm_device,"avs-media");
    if (IS_ERR(avs_dev->avs_thread))
    {
        PM_PRINT("Create avs thread failed.\n");
        kfree(avs_dev->freq_hpm_table);
        return -1;
    }
    if(1 == media_avs_en)
    {
       PM_PRINT("Media avs  enable.\n");
       wake_up_process(avs_dev->avs_thread);
    }
    return 0;
}

void media_avs_exit(struct platform_device *pdev,struct pm_domain_device *pm_device)
{
    if (media_avs_en && pm_device->avs_dev.avs_thread)
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

void media_avs_freeze(struct pm_domain_device *pm_domain_device)
{
    struct avs_device *avs_dev = &pm_domain_device->avs_dev;
    
    mutex_lock(&avs_dev->avs_lock);
    avs_dev->avs_enable = 0;
    mutex_unlock(&avs_dev->avs_lock);

}

void media_avs_restore(struct pm_domain_device *pm_domain_device)
{
    struct avs_device *avs_dev = &pm_domain_device->avs_dev;

    media_hpm_init();
    media_tsensor_init();
    
    mutex_lock(&avs_dev->avs_lock);
    avs_dev->avs_enable = media_avs_en;
    media_update_cur_avs_info(avs_dev);
    mutex_unlock(&avs_dev->avs_lock);
}

