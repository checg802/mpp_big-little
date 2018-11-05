#include "hal_cpu_avs.h"
#include "pm_hal.h"

void hal_cpu_set_hpm_div(unsigned int div)
{
    pm_set_pmc14cpu_hpm_div(div);
}

void hal_cpu_hpm_init(void)
{
    /* cpu HPM reset */
    pm_set_pmc14cpu_hpm_srst_req(0x1);    
    pm_set_pmc14cpu_hpm_srst_req(0x0);
    /* cpu HPM limit*/
    pm_set_pmc17cpu_hpm_uplimit(320);    
    pm_set_pmc17cpu_hpm_lowlimit(190);
    pm_set_pmc17cpu_hpm_monitor_period(0x1);
    pm_set_pmc30cpu_hpm_monitor_period_fine(0);

    pm_set_pmc14cpu_hpm_shift(0);
    pm_set_pmc14cpu_hpm_offset(0);
    /* set circularly check */
    pm_set_pmc14cpu_hpm_monitor_en(0x1);
    /* enable */
    pm_set_pmc14cpu_hpm_en(0x1);
}

int hal_cpu_get_average_hpm(void)
{
    int total_hpm;
    
    total_hpm = pm_get_pmc15cpu_hpm_pc_records();
    total_hpm += pm_get_pmc16cpu_hpm_pc_records();
    
    return total_hpm/4;
}

