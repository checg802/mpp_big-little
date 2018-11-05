#include "hal_cpu_freq.h"

static struct clk *fixed_594m_clk;
static struct clk *fixed_792m_clk;


int hal_cpu_freq_init_fixed_clk(struct device *cpu_dev)
{
	fixed_594m_clk = clk_get(cpu_dev, "594m");
	fixed_792m_clk = clk_get(cpu_dev, "792m");
    
    if(IS_ERR(fixed_594m_clk) || IS_ERR(fixed_792m_clk))
    {
        printk("failed to init fixed clocks\n");
        return -ENOENT;
    } 
    return 0;
}

void hal_cpu_freq_release_fixed_clk(void)
{
    if(fixed_594m_clk)
        clk_put(fixed_594m_clk);
    
    if(fixed_792m_clk)
        clk_put(fixed_792m_clk);
}



struct clk *hal_cpu_freq_get_clk(unsigned int index, unsigned long rate)
{
	if (rate == 594000 && fixed_594m_clk)
		return fixed_594m_clk;
	else if (rate == 792000 && fixed_792m_clk)
		return fixed_792m_clk;
	else 
        return NULL;
}

struct clk *hal_cpu_freq_get_lowest_fixed_clk(void)
{
    return fixed_594m_clk;
}


