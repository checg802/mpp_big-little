#include "pm_dvfs.h"

int pm_dvfs_init(struct pm_device *pm_device)
{
    if(pm_device->cpu_domain_device && pm_device->cpu_domain_device->domain_cpufreq_init)
        pm_device->cpu_domain_device->domain_cpufreq_init(pm_device->cpu_domain_device, NULL);
        
    if(pm_device->media_domain_device && pm_device->media_domain_device->domain_cpufreq_init)
        pm_device->media_domain_device->domain_cpufreq_init(pm_device->media_domain_device, NULL);
        
    return 0;
}

void pm_dvfs_exit(struct pm_device *pm_device)
{
    if(pm_device->cpu_domain_device && pm_device->cpu_domain_device->domain_cpufreq_exit)
        pm_device->cpu_domain_device->domain_cpufreq_exit(pm_device->cpu_domain_device, NULL);
        
    if(pm_device->media_domain_device && pm_device->media_domain_device->domain_cpufreq_exit)
        pm_device->media_domain_device->domain_cpufreq_exit(pm_device->media_domain_device, NULL);
}

