
/******************************************************************************

  Copyright (C), 2001-2020, Huawei Tech. Co., Ltd.

 ******************************************************************************
  File Name     : pm.h
  Version       : Initial Draft
  Author        : c00321909
  Created       : 2016/01/06
  Last Modified :
  Description   : pm.h
  Function List :
  History       :
  1.Date        : 2016/01/06
    Author      : c00321909
    Modification: Created file

******************************************************************************/
#ifndef  __HI_PM_DEVICE_H__
#define  __HI_PM_DEVICE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* Begin of #ifdef __cplusplus */

#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/clk.h>
#include <mach/io.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/of_regulator.h>
#include <linux/regulator/machine.h>


#define PM_CPU      (0x01L<<0)
#define PM_MEDIA    (0x01L<<0)

struct avs_device;
struct pm_domain_device;

#define MAX_CLK_NAME_LEN    10

typedef enum PM_DOMAIN
{
    PM_DOMAIN_CPU   = 0,
    PM_DOMAIN_MEDIA = 1,
    PM_DOMAIN_DDR   = 2,
    PM_DOMAIN_CORE  = 3,
    PM_DOMAIN_BUTT
}PM_DOMAIN_E;

#ifdef PM_DEBUG
#define PM_PRINT(format,args...) printk("[%s][%d]"format,__FUNCTION__, __LINE__,## args)
#else
#define PM_PRINT(format,args...)
#endif

typedef int  domain_regulator_init(struct platform_device *, struct pm_domain_device *);
typedef void domain_regulator_exit(struct platform_device *, struct pm_domain_device *);
typedef int  domain_cpufreq_init(struct pm_domain_device *, void *);
typedef void domain_cpufreq_exit(struct pm_domain_device *, void *);
typedef int  domain_avs_init(struct platform_device *, struct pm_domain_device *);
typedef void domain_avs_exit(struct platform_device *, struct pm_domain_device *);
typedef void domain_avs_pause(struct pm_domain_device *);
typedef void domain_avs_resume(struct pm_domain_device *);

typedef void domain_freeze(struct pm_domain_device *);
typedef void domain_restore(struct pm_domain_device *);

struct avs_ops
{
    domain_avs_pause  *avs_pause;
    domain_avs_resume *avs_resume;
};

struct hpm_opp
{
    unsigned int freq;          /* unit: kHz */
    unsigned int vmin;
    unsigned int vmax;
    unsigned int hpmrecord;     /* hpm record */    
    unsigned int div;           /* frequency dividing ratio */
    unsigned int temp;
}; 

struct avs_device
{
    unsigned int avs_enable;

    struct avs_ops *avs_ops;
    struct task_struct *avs_thread;    
    struct mutex avs_lock;

    unsigned int cur_volt_max;
    unsigned int cur_volt_min;

    unsigned int avs_temp_num;
    unsigned int temperature_index;
    int temperature;
    int avs_temp[10];

    unsigned int avs_prof_num;
    unsigned int avs_profile;

    struct hpm_opp *freq_hpm_table;
    unsigned int max_hpm_tale_index;

    unsigned int cur_freq;
    unsigned int cur_hpm;
    unsigned int div;
};

struct clk_info 
{
    struct clk *clk;
    char name[MAX_CLK_NAME_LEN];
};

struct regulator_config_data {
	struct regulator_init_data *reg_init;
	void __iomem *reg;
	int step_uV;
};

struct pm_regulator {
	struct device *dev;
    struct regulator *regulator;
	void __iomem *reg_base;
	struct regulator_dev *rdev;
	struct regulator_desc desc;
	struct regulator_config_data config_data;
};

struct pm_domain_device
{
    /* cpufreq clk */
    struct clk_info *manager_clk;
    struct clk_info *manager_clk_ex;
    struct clk_info *var_clks;
    struct cpufreq_frequency_table *freq_table;
    struct cpufreq_frequency_table *freq_table_ex;
    struct mutex freq_table_lock;
    struct mutex cpufreq_mutex;
	bool cpufreq_en;
    struct mutex dvfs_lock;
    int latency;
    domain_cpufreq_init *domain_cpufreq_init;
    domain_cpufreq_exit *domain_cpufreq_exit;

    /*regulator*/
    struct pm_regulator *regulator;
    domain_regulator_init *domain_regulator_init;
    domain_regulator_exit *domain_regulator_exit;
    int cur_volt;
    unsigned long cur_freq_volt;
    
    /* avs  */
    struct avs_device avs_dev;
    domain_avs_init *domain_avs_init;
    domain_avs_exit *domain_avs_exit;
    
    domain_freeze *domain_freeze;
    domain_restore * domain_restore;
};

struct pm_device 
{
    struct platform_device device;
    struct platform_driver driver;
    struct pm_domain_device *cpu_domain_device;
    struct pm_domain_device *media_domain_device;
    struct pm_domain_device *ddr_domain_device;
    struct pm_domain_device *core_domain_device;
};

#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif /* End of #ifdef __cplusplus */
    
#endif/* End of #ifndef __PM_H__*/
