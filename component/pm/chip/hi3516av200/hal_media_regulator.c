#include <linux/kernel.h>
#include <linux/delay.h>

#include "hal_media_regulator.h"
#include "pm_hal.h"

void hal_media_domain_set_voltage(int step, int regulator_max, int set_volt)
{
    int gap_uV, duty;

    duty = pm_get_pmc3mda0_pwm_duty();
    gap_uV = regulator_max - set_volt;
    do_div(gap_uV, step);
    duty = (gap_uV>0)?(gap_uV - 1):0; 
    pm_set_pmc3mda0_pwm_duty(duty);        
    msleep(DELAY_TIME_MS);
}

int hal_media_domain_get_voltage(int step, int regulator_max)
{
    int gap_uV, duty;

    duty = pm_get_pmc3mda0_pwm_duty();
    gap_uV = step * (duty + 1);
    return regulator_max - gap_uV;
}

int hal_media_domain_get_regulator_step(int min_uV, int max_uV, void *reg)
{
	int step, period;
	period = pm_get_pmc3mda0_pwm_period() + 1;
    
	step = (max_uV - min_uV);
	do_div(step, period);

    return step;
}

int hal_media_domain_check_domain_valid(void)
{
    unsigned int value = 0;
    pm_get_profie_reg_profile(&value);
    
    /*0x0: four Volt-Source domain,VDD,DDR,MEDIA,CPU.*/
    /*0x1: three Volt-Source domain,CORE(VDD,DDR),MEDIA,CPU.*/
    /*0x2: two Volt-Source domain,MEDIA and other*/
    /*0x3: undefined*/
    /*Hi3519V101 now only support 0x1(three Volt-Source domain)*/
    if(0x3 == value)
    {
        printk("Media has no independent Volt-Source.\n");
        return -1;
    }
    else
    {
        return 0;
    }
    
    return 0;
}

