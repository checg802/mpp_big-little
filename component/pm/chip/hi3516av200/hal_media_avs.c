#include "hal_media_avs.h"
#include "pm_hal.h"
#include "pm_device.h"
void hal_media_set_hpm_div(unsigned int div)
{
    pm_set_pmc22mda0_hpm_div(div);
}

void hal_media_hpm_init(void)
{
    /* media HPM reset */
    pm_set_pmc22mda0_hpm_srst_req(0x1);    
    pm_set_pmc22mda0_hpm_srst_req(0x0);
    /* media HPM limit*/
    pm_set_pmc25mda0_hpm_uplimit(276);    
    pm_set_pmc25mda0_hpm_lowlimit(180);
    pm_set_pmc25mda0_hpm_monitor_period(0x1);
    pm_set_pmc30mda0_hpm_monitor_period_fine(0);
    pm_set_pmc22mda0_hpm_shift(0);
    pm_set_pmc22mda0_hpm_offset(0);
    /* set circularly check */
    pm_set_pmc22mda0_hpm_monitor_en(0x1);
    /* enable */
    pm_set_pmc22mda0_hpm_en(0x1);
}


void hal_media_tsensor_init(void)
{
    /* media HPM reset */
    /* media HPM limit*/
    pm_set_pmc69tsensor_temp_uplimit(931);
    pm_set_pmc69tsensor_temp_lowlimit(125);
    pm_set_pmc68tsensor_monitor_period(0x1);
    /* set circularly check */
    pm_set_pmc68tsensor_monitor_en(0x1);
    /* enable */
    pm_set_pmc68tsensor_en(0x1);
}


int hal_media_get_average_hpm(void)
{
    int total_hpm;
    
    total_hpm = pm_get_pmc23mda0_hpm_pc_records();
    total_hpm += pm_get_pmc24mda0_hpm_pc_records();
    
    return total_hpm/4;
}

int hal_media_get_average_temperature(void)
{
    int total_temp = 0;
    
    total_temp = pm_get_pmc70tsensor_temp_records();
    total_temp += pm_get_pmc71tsensor_temp_records();
    total_temp += pm_get_pmc72tsensor_temp_records();
    total_temp += pm_get_pmc73tsensor_temp_records();
    
    return (((((total_temp>>3) -125)*210)>>10)-40);
}


extern volatile unsigned int  *u32VEDUReg;
extern volatile unsigned int  *u32DDRreg;
extern volatile unsigned int  *u32DPLLreg;

int hal_media_get_profile(void)
{
    int profile = 0;
    
    if((((*u32VEDUReg)>>10)&0x7) == 0x4)
    {
        profile = 1;
        PM_PRINT("==========%s %d  vedu hz %d==============\n",__func__,__LINE__,(((*u32VEDUReg)>>10)&0x7));
        return profile;
    }

    if(((*u32DDRreg)&0x7) == 0x001)
    {
        int dpll_refdiv = ((*u32DPLLreg)>>12)&0x3f;
        int dpll_fbdiv  = (*u32DPLLreg)&0xfff;

        if(24/dpll_refdiv*dpll_fbdiv*2>1800)
        {
            profile = 1;
            PM_PRINT("==========%s %d dpll_refdiv %d dpll_fbdiv %d==============\n",__func__,__LINE__,dpll_refdiv,dpll_fbdiv);
            return profile;
        }
    }
    
    return profile;
}




