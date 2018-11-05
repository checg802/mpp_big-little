//****************************************************************************** 
// Copyright     :  Copyright (C) 2014, Hisilicon Technologies Co., Ltd.
// File name     :  pmc.c
// Author        :  xxx
// Version       :  1.0 
// Date          :  2014-06-19
// Description   :  Define all registers/tables for xxx
// Others        :  Generated automatically by nManager V4.0 
// History       :  xxx 2014-06-19 Create file
//******************************************************************************
#include <linux/io.h>
#include "pm_hal.h"
#include <linux/kernel.h>
#include <mach/io.h>/* for IO_ADDRESS */

/* The base address of the module PMC */
#define PMC_BASE_ADDR 0x120A0000
#define PROFILE_REG   0x1202015C
#define VEDU_HZ_REG   0x1201004C
#define DDR_HZ_REG    0x12010034
#define DPLL_REG      0x12010014

/* Define the struct pointor of the module PMC */
volatile S_PMC_REGS_TYPE *gopPMCAllReg = (S_PMC_REGS_TYPE *)IO_ADDRESS(PMC_BASE_ADDR);
volatile U_PROFILE_REG *profile_reg = (U_PROFILE_REG *)IO_ADDRESS(PROFILE_REG);
volatile unsigned int  *u32VEDUReg = (volatile unsigned int *)IO_ADDRESS(VEDU_HZ_REG);
volatile unsigned int  *u32DDRreg  = (volatile unsigned int *)IO_ADDRESS(DDR_HZ_REG);
volatile unsigned int  *u32DPLLreg = (volatile unsigned int *)IO_ADDRESS(DPLL_REG);

#define RTL_Verify_RegWrite(addr, p)

//******************************************************************************
//  Function    : pm_set_pmc0core_pwm_period
//  Description : Set the value of the member PERI_PMC0.core_pwm_period
//  Input       : unsigned int ucore_pwm_period: 16 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc0core_pwm_period(unsigned int ucore_pwm_period)
{
    U_PERI_PMC0 o_peri_pmc0;
    o_peri_pmc0.u32 = gopPMCAllReg->PERI_PMC0.u32;
    o_peri_pmc0.bits.core_pwm_period = ucore_pwm_period;
    gopPMCAllReg->PERI_PMC0.u32 = o_peri_pmc0.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC0.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc0core_pwm_duty
//  Description : Set the value of the member PERI_PMC0.core_pwm_duty
//  Input       : unsigned int ucore_pwm_duty: 16 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc0core_pwm_duty(unsigned int ucore_pwm_duty)
{
    U_PERI_PMC0 o_peri_pmc0;
    o_peri_pmc0.u32 = gopPMCAllReg->PERI_PMC0.u32;
    o_peri_pmc0.bits.core_pwm_duty = ucore_pwm_duty;
    gopPMCAllReg->PERI_PMC0.u32 = o_peri_pmc0.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC0.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc1cpu_pwm_period
//  Description : Set the value of the member PERI_PMC1.cpu_pwm_period
//  Input       : unsigned int ucpu_pwm_period: 16 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc1cpu_pwm_period(unsigned int ucpu_pwm_period)
{
    U_PERI_PMC1 o_peri_pmc1;
    o_peri_pmc1.u32 = gopPMCAllReg->PERI_PMC1.u32;
    o_peri_pmc1.bits.a17_pwm_period = ucpu_pwm_period;
    gopPMCAllReg->PERI_PMC1.u32 = o_peri_pmc1.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC1.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc1cpu_pwm_duty
//  Description : Set the value of the member PERI_PMC1.cpu_pwm_duty
//  Input       : unsigned int ucpu_pwm_duty: 16 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc1cpu_pwm_duty(unsigned int ucpu_pwm_duty)
{
    U_PERI_PMC1 o_peri_pmc1;
    o_peri_pmc1.u32 = gopPMCAllReg->PERI_PMC1.u32;
    o_peri_pmc1.bits.a17_pwm_duty = ucpu_pwm_duty;
    gopPMCAllReg->PERI_PMC1.u32 = o_peri_pmc1.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC1.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_get_pmc1cpu_pwm_period
//  Description : Set the value of the member PERI_PMC1.cpu_pwm_period
//  Input       : unsigned int ucpu_pwm_period: 16 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_get_pmc1cpu_pwm_period(void)
{
    U_PERI_PMC1 o_peri_pmc1;
    unsigned int ucpu_pwm_period;
    
    o_peri_pmc1.u32 = gopPMCAllReg->PERI_PMC1.u32;
    ucpu_pwm_period = o_peri_pmc1.bits.a17_pwm_period;
    return ucpu_pwm_period;
}

//******************************************************************************
//  Function    : pm_get_pmc1cpu_pwm_duty
//  Description : Set the value of the member PERI_PMC1.cpu_pwm_duty
//  Input       : unsigned int ucpu_pwm_duty: 16 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_get_pmc1cpu_pwm_duty(void)
{
    U_PERI_PMC1 o_peri_pmc1;
    unsigned int ucpu_pwm_duty;
    
    o_peri_pmc1.u32 = gopPMCAllReg->PERI_PMC1.u32;
    ucpu_pwm_duty = o_peri_pmc1.bits.a17_pwm_duty;
    
    return ucpu_pwm_duty;
}

//******************************************************************************
//  Function    : pm_set_pmc2ddr_pwm_period
//  Description : Set the value of the member PERI_PMC2.ddr_pwm_period
//  Input       : unsigned int uddr_pwm_period: 16 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc2ddr_pwm_period(unsigned int uddr_pwm_period)
{
    U_PERI_PMC2 o_peri_pmc2;
    o_peri_pmc2.u32 = gopPMCAllReg->PERI_PMC2.u32;
    o_peri_pmc2.bits.ddr_pwm_period = uddr_pwm_period;
    gopPMCAllReg->PERI_PMC2.u32 = o_peri_pmc2.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC2.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc2ddr_pwm_duty
//  Description : Set the value of the member PERI_PMC2.ddr_pwm_duty
//  Input       : unsigned int uddr_pwm_duty: 16 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc2ddr_pwm_duty(unsigned int uddr_pwm_duty)
{
    U_PERI_PMC2 o_peri_pmc2;
    o_peri_pmc2.u32 = gopPMCAllReg->PERI_PMC2.u32;
    o_peri_pmc2.bits.ddr_pwm_duty = uddr_pwm_duty;
    gopPMCAllReg->PERI_PMC2.u32 = o_peri_pmc2.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC2.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc3mda_pwm_period
//  Description : Set the value of the member PERI_PMC3.mda_pwm_period
//  Input       : unsigned int umda_pwm_period: 16 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc3mda0_pwm_period(unsigned int umda_pwm_period)
{
    U_PERI_PMC3 o_peri_pmc3;
    o_peri_pmc3.u32 = gopPMCAllReg->PERI_PMC3.u32;
    o_peri_pmc3.bits.mda0_pwm_period = umda_pwm_period;
    gopPMCAllReg->PERI_PMC3.u32 = o_peri_pmc3.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC3.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc3mda_pwm_duty
//  Description : Set the value of the member PERI_PMC3.mda_pwm_duty
//  Input       : unsigned int umda_pwm_duty: 16 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc3mda0_pwm_duty(unsigned int umda_pwm_duty)
{
    U_PERI_PMC3 o_peri_pmc3;
    o_peri_pmc3.u32 = gopPMCAllReg->PERI_PMC3.u32;
    o_peri_pmc3.bits.mda0_pwm_duty = umda_pwm_duty;
    gopPMCAllReg->PERI_PMC3.u32 = o_peri_pmc3.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC3.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_get_pmc1cpu_pwm_period
//  Description : Set the value of the member PERI_PMC1.cpu_pwm_period
//  Input       : unsigned int ucpu_pwm_period: 16 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_get_pmc3mda0_pwm_period(void)
{
    U_PERI_PMC1 o_peri_pmc1;
    unsigned int ucpu_pwm_period;
    
    o_peri_pmc1.u32 = gopPMCAllReg->PERI_PMC3.u32;
    ucpu_pwm_period = o_peri_pmc1.bits.a17_pwm_period;
    return ucpu_pwm_period;
}

//******************************************************************************
//  Function    : pm_get_pmc1cpu_pwm_duty
//  Description : Set the value of the member PERI_PMC1.cpu_pwm_duty
//  Input       : unsigned int ucpu_pwm_duty: 16 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_get_pmc3mda0_pwm_duty(void)
{
    U_PERI_PMC1 o_peri_pmc1;
    unsigned int ucpu_pwm_duty;
    
    o_peri_pmc1.u32 = gopPMCAllReg->PERI_PMC3.u32;
    ucpu_pwm_duty = o_peri_pmc1.bits.a17_pwm_duty;
    
    return ucpu_pwm_duty;
}

//******************************************************************************
//  Function    : pm_set_pmc4core_pwm_enable
//  Description : Set the value of the member PERI_PMC4.core_pwm_enable
//  Input       : unsigned int ucore_pwm_enable: 1 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc4core_pwm_enable(unsigned int ucore_pwm_enable)
{
    U_PERI_PMC4 o_peri_pmc4;
    o_peri_pmc4.u32 = gopPMCAllReg->PERI_PMC4.u32;
    o_peri_pmc4.bits.core_pwm_enable = ucore_pwm_enable;
    gopPMCAllReg->PERI_PMC4.u32 = o_peri_pmc4.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC4.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc4core_pwm_inv
//  Description : Set the value of the member PERI_PMC4.core_pwm_inv
//  Input       : unsigned int ucore_pwm_inv: 1 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc4core_pwm_inv(unsigned int ucore_pwm_inv)
{
    U_PERI_PMC4 o_peri_pmc4;
    o_peri_pmc4.u32 = gopPMCAllReg->PERI_PMC4.u32;
    o_peri_pmc4.bits.core_pwm_inv = ucore_pwm_inv;
    gopPMCAllReg->PERI_PMC4.u32 = o_peri_pmc4.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC4.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc4cpu_pwm_enable
//  Description : Set the value of the member PERI_PMC4.cpu_pwm_enable
//  Input       : unsigned int ucpu_pwm_enable: 1 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc4cpu_pwm_enable(unsigned int ucpu_pwm_enable)
{
    U_PERI_PMC4 o_peri_pmc4;
    o_peri_pmc4.u32 = gopPMCAllReg->PERI_PMC4.u32;
    o_peri_pmc4.bits.a17_pwm_enable = ucpu_pwm_enable;
    gopPMCAllReg->PERI_PMC4.u32 = o_peri_pmc4.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC4.u32));

    return 1;
}

int pm_get_pmc4cpu_pwm_enable(unsigned int *ucpu_pwm_enable)
{
    *ucpu_pwm_enable = gopPMCAllReg->PERI_PMC4.bits.a17_pwm_enable;

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc4cpu_pwm_inv
//  Description : Set the value of the member PERI_PMC4.cpu_pwm_inv
//  Input       : unsigned int ucpu_pwm_inv: 1 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc4cpu_pwm_inv(unsigned int ucpu_pwm_inv)
{
    U_PERI_PMC4 o_peri_pmc4;
    o_peri_pmc4.u32 = gopPMCAllReg->PERI_PMC4.u32;
    o_peri_pmc4.bits.a17_pwm_inv = ucpu_pwm_inv;
    gopPMCAllReg->PERI_PMC4.u32 = o_peri_pmc4.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC4.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc4ddr_pwm_enable
//  Description : Set the value of the member PERI_PMC4.ddr_pwm_enable
//  Input       : unsigned int uddr_pwm_enable: 1 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc4ddr_pwm_enable(unsigned int uddr_pwm_enable)
{
    U_PERI_PMC4 o_peri_pmc4;
    o_peri_pmc4.u32 = gopPMCAllReg->PERI_PMC4.u32;
    o_peri_pmc4.bits.ddr_pwm_enable = uddr_pwm_enable;
    gopPMCAllReg->PERI_PMC4.u32 = o_peri_pmc4.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC4.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc4ddr_pwm_inv
//  Description : Set the value of the member PERI_PMC4.ddr_pwm_inv
//  Input       : unsigned int uddr_pwm_inv: 1 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc4ddr_pwm_inv(unsigned int uddr_pwm_inv)
{
    U_PERI_PMC4 o_peri_pmc4;
    o_peri_pmc4.u32 = gopPMCAllReg->PERI_PMC4.u32;
    o_peri_pmc4.bits.ddr_pwm_inv = uddr_pwm_inv;
    gopPMCAllReg->PERI_PMC4.u32 = o_peri_pmc4.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC4.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc4mda_pwm_enable
//  Description : Set the value of the member PERI_PMC4.mda_pwm_enable
//  Input       : unsigned int umda_pwm_enable: 1 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc4mda0_pwm_enable(unsigned int umda_pwm_enable)
{
    U_PERI_PMC4 o_peri_pmc4;
    o_peri_pmc4.u32 = gopPMCAllReg->PERI_PMC4.u32;
    o_peri_pmc4.bits.mda0_pwm_enable = umda_pwm_enable;
    gopPMCAllReg->PERI_PMC4.u32 = o_peri_pmc4.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC4.u32));

    return 1;
}

int pm_get_pmc4mda0_pwm_enable(unsigned int *umda_pwm_enable)
{
    *umda_pwm_enable = gopPMCAllReg->PERI_PMC4.bits.mda0_pwm_enable;

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc4mda_pwm_inv
//  Description : Set the value of the member PERI_PMC4.mda_pwm_inv
//  Input       : unsigned int umda_pwm_inv: 1 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc4mda0_pwm_inv(unsigned int umda_pwm_inv)
{
    U_PERI_PMC4 o_peri_pmc4;
    o_peri_pmc4.u32 = gopPMCAllReg->PERI_PMC4.u32;
    o_peri_pmc4.bits.mda0_pwm_inv = umda_pwm_inv;
    gopPMCAllReg->PERI_PMC4.u32 = o_peri_pmc4.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC4.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc4pwm0_reuse_cfg
//  Description : Set the value of the member PERI_PMC4.pwm0_reuse_cfg
//  Input       : unsigned int upwm0_reuse_cfg: 2 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc4pwm0_reuse_cfg(unsigned int upwm0_reuse_cfg)
{
    U_PERI_PMC4 o_peri_pmc4;
    o_peri_pmc4.u32 = gopPMCAllReg->PERI_PMC4.u32;
    o_peri_pmc4.bits.pwm0_reuse_cfg = upwm0_reuse_cfg;
    gopPMCAllReg->PERI_PMC4.u32 = o_peri_pmc4.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC4.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc4pwm1_reuse_cfg
//  Description : Set the value of the member PERI_PMC4.pwm1_reuse_cfg
//  Input       : unsigned int upwm1_reuse_cfg: 2 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc4pwm1_reuse_cfg(unsigned int upwm1_reuse_cfg)
{
    U_PERI_PMC4 o_peri_pmc4;
    o_peri_pmc4.u32 = gopPMCAllReg->PERI_PMC4.u32;
    o_peri_pmc4.bits.pwm1_reuse_cfg = upwm1_reuse_cfg;
    gopPMCAllReg->PERI_PMC4.u32 = o_peri_pmc4.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC4.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc4pwm2_reuse_cfg
//  Description : Set the value of the member PERI_PMC4.pwm2_reuse_cfg
//  Input       : unsigned int upwm2_reuse_cfg: 2 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc4pwm2_reuse_cfg(unsigned int upwm2_reuse_cfg)
{
    U_PERI_PMC4 o_peri_pmc4;
    o_peri_pmc4.u32 = gopPMCAllReg->PERI_PMC4.u32;
    o_peri_pmc4.bits.pwm2_reuse_cfg = upwm2_reuse_cfg;
    gopPMCAllReg->PERI_PMC4.u32 = o_peri_pmc4.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC4.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc4pwm3_reuse_cfg
//  Description : Set the value of the member PERI_PMC4.pwm3_reuse_cfg
//  Input       : unsigned int upwm3_reuse_cfg: 2 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc4pwm3_reuse_cfg(unsigned int upwm3_reuse_cfg)
{
    U_PERI_PMC4 o_peri_pmc4;
    o_peri_pmc4.u32 = gopPMCAllReg->PERI_PMC4.u32;
    o_peri_pmc4.bits.pwm3_reuse_cfg = upwm3_reuse_cfg;
    gopPMCAllReg->PERI_PMC4.u32 = o_peri_pmc4.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC4.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc5core_hpm_fsm_en
//  Description : Set the value of the member PERI_PMC5.core_hpm_fsm_en
//  Input       : unsigned int ucore_hpm_fsm_en: 1 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc5core_hpm_fsm_en(unsigned int ucore_hpm_fsm_en)
{
    U_PERI_PMC5 o_peri_pmc5;
    o_peri_pmc5.u32 = gopPMCAllReg->PERI_PMC5.u32;
    o_peri_pmc5.bits.core_hpm_fsm_en = ucore_hpm_fsm_en;
    gopPMCAllReg->PERI_PMC5.u32 = o_peri_pmc5.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC5.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc5core_avs_hpm_fsm_calc_mode
//  Description : Set the value of the member PERI_PMC5.core_avs_hpm_fsm_calc_mode
//  Input       : unsigned int ucore_avs_hpm_fsm_calc_mode: 2 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc5core_avs_hpm_fsm_calc_mode(unsigned int ucore_avs_hpm_fsm_calc_mode)
{
    U_PERI_PMC5 o_peri_pmc5;
    o_peri_pmc5.u32 = gopPMCAllReg->PERI_PMC5.u32;
    o_peri_pmc5.bits.core_avs_hpm_fsm_calc_mode = ucore_avs_hpm_fsm_calc_mode;
    gopPMCAllReg->PERI_PMC5.u32 = o_peri_pmc5.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC5.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc5core_avs_fsm_dec_pwm_step
//  Description : Set the value of the member PERI_PMC5.core_avs_fsm_dec_pwm_step
//  Input       : unsigned int ucore_avs_fsm_dec_pwm_step: 8 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc5core_avs_fsm_dec_pwm_step(unsigned int ucore_avs_fsm_dec_pwm_step)
{
    U_PERI_PMC5 o_peri_pmc5;
    o_peri_pmc5.u32 = gopPMCAllReg->PERI_PMC5.u32;
    o_peri_pmc5.bits.core_avs_fsm_dec_pwm_step = ucore_avs_fsm_dec_pwm_step;
    gopPMCAllReg->PERI_PMC5.u32 = o_peri_pmc5.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC5.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc5core_avs_fsm_inc_pwm_step
//  Description : Set the value of the member PERI_PMC5.core_avs_fsm_inc_pwm_step
//  Input       : unsigned int ucore_avs_fsm_inc_pwm_step: 8 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc5core_avs_fsm_inc_pwm_step(unsigned int ucore_avs_fsm_inc_pwm_step)
{
    U_PERI_PMC5 o_peri_pmc5;
    o_peri_pmc5.u32 = gopPMCAllReg->PERI_PMC5.u32;
    o_peri_pmc5.bits.core_avs_fsm_inc_pwm_step = ucore_avs_fsm_inc_pwm_step;
    gopPMCAllReg->PERI_PMC5.u32 = o_peri_pmc5.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC5.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc6cpu_hpm_fsm_en
//  Description : Set the value of the member PERI_PMC6.cpu_hpm_fsm_en
//  Input       : unsigned int ucpu_hpm_fsm_en: 1 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc6cpu_hpm_fsm_en(unsigned int ucpu_hpm_fsm_en)
{
    U_PERI_PMC6 o_peri_pmc6;
    o_peri_pmc6.u32 = gopPMCAllReg->PERI_PMC6.u32;
    o_peri_pmc6.bits.a17_hpm_fsm_en = ucpu_hpm_fsm_en;
    gopPMCAllReg->PERI_PMC6.u32 = o_peri_pmc6.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC6.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc6cpu_avs_hpm_fsm_calc_mode
//  Description : Set the value of the member PERI_PMC6.cpu_avs_hpm_fsm_calc_mode
//  Input       : unsigned int ucpu_avs_hpm_fsm_calc_mode: 2 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc6cpu_avs_hpm_fsm_calc_mode(unsigned int ucpu_avs_hpm_fsm_calc_mode)
{
    U_PERI_PMC6 o_peri_pmc6;
    o_peri_pmc6.u32 = gopPMCAllReg->PERI_PMC6.u32;
    o_peri_pmc6.bits.a17_avs_hpm_fsm_calc_mode = ucpu_avs_hpm_fsm_calc_mode;
    gopPMCAllReg->PERI_PMC6.u32 = o_peri_pmc6.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC6.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc6cpu_avs_fsm_dec_pwm_step
//  Description : Set the value of the member PERI_PMC6.cpu_avs_fsm_dec_pwm_step
//  Input       : unsigned int ucpu_avs_fsm_dec_pwm_step: 8 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc6cpu_avs_fsm_dec_pwm_step(unsigned int ucpu_avs_fsm_dec_pwm_step)
{
    U_PERI_PMC6 o_peri_pmc6;
    o_peri_pmc6.u32 = gopPMCAllReg->PERI_PMC6.u32;
    o_peri_pmc6.bits.a17_avs_fsm_dec_pwm_step = ucpu_avs_fsm_dec_pwm_step;
    gopPMCAllReg->PERI_PMC6.u32 = o_peri_pmc6.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC6.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc6cpu_avs_fsm_inc_pwm_step
//  Description : Set the value of the member PERI_PMC6.cpu_avs_fsm_inc_pwm_step
//  Input       : unsigned int ucpu_avs_fsm_inc_pwm_step: 8 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc6cpu_avs_fsm_inc_pwm_step(unsigned int ucpu_avs_fsm_inc_pwm_step)
{
    U_PERI_PMC6 o_peri_pmc6;
    o_peri_pmc6.u32 = gopPMCAllReg->PERI_PMC6.u32;
    o_peri_pmc6.bits.a17_avs_fsm_inc_pwm_step = ucpu_avs_fsm_inc_pwm_step;
    gopPMCAllReg->PERI_PMC6.u32 = o_peri_pmc6.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC6.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc7ddr_hpm_fsm_en
//  Description : Set the value of the member PERI_PMC7.ddr_hpm_fsm_en
//  Input       : unsigned int uddr_hpm_fsm_en: 1 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc7ddr_hpm_fsm_en(unsigned int uddr_hpm_fsm_en)
{
    U_PERI_PMC7 o_peri_pmc7;
    o_peri_pmc7.u32 = gopPMCAllReg->PERI_PMC7.u32;
    o_peri_pmc7.bits.ddr_hpm_fsm_en = uddr_hpm_fsm_en;
    gopPMCAllReg->PERI_PMC7.u32 = o_peri_pmc7.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC7.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc7ddr_avs_hpm_fsm_calc_mode
//  Description : Set the value of the member PERI_PMC7.ddr_avs_hpm_fsm_calc_mode
//  Input       : unsigned int uddr_avs_hpm_fsm_calc_mode: 2 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc7ddr_avs_hpm_fsm_calc_mode(unsigned int uddr_avs_hpm_fsm_calc_mode)
{
    U_PERI_PMC7 o_peri_pmc7;
    o_peri_pmc7.u32 = gopPMCAllReg->PERI_PMC7.u32;
    o_peri_pmc7.bits.ddr_avs_hpm_fsm_calc_mode = uddr_avs_hpm_fsm_calc_mode;
    gopPMCAllReg->PERI_PMC7.u32 = o_peri_pmc7.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC7.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc7ddr_avs_fsm_dec_pwm_step
//  Description : Set the value of the member PERI_PMC7.ddr_avs_fsm_dec_pwm_step
//  Input       : unsigned int uddr_avs_fsm_dec_pwm_step: 8 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc7ddr_avs_fsm_dec_pwm_step(unsigned int uddr_avs_fsm_dec_pwm_step)
{
    U_PERI_PMC7 o_peri_pmc7;
    o_peri_pmc7.u32 = gopPMCAllReg->PERI_PMC7.u32;
    o_peri_pmc7.bits.ddr_avs_fsm_dec_pwm_step = uddr_avs_fsm_dec_pwm_step;
    gopPMCAllReg->PERI_PMC7.u32 = o_peri_pmc7.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC7.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc7ddr_avs_fsm_inc_pwm_step
//  Description : Set the value of the member PERI_PMC7.ddr_avs_fsm_inc_pwm_step
//  Input       : unsigned int uddr_avs_fsm_inc_pwm_step: 8 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc7ddr_avs_fsm_inc_pwm_step(unsigned int uddr_avs_fsm_inc_pwm_step)
{
    U_PERI_PMC7 o_peri_pmc7;
    o_peri_pmc7.u32 = gopPMCAllReg->PERI_PMC7.u32;
    o_peri_pmc7.bits.ddr_avs_fsm_inc_pwm_step = uddr_avs_fsm_inc_pwm_step;
    gopPMCAllReg->PERI_PMC7.u32 = o_peri_pmc7.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC7.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc8mda_hpm_fsm_en
//  Description : Set the value of the member PERI_PMC8.mda_hpm_fsm_en
//  Input       : unsigned int umda_hpm_fsm_en: 1 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc8mda_hpm_fsm_en(unsigned int umda_hpm_fsm_en)
{
    U_PERI_PMC8 o_peri_pmc8;
    o_peri_pmc8.u32 = gopPMCAllReg->PERI_PMC8.u32;
    o_peri_pmc8.bits.mda0_hpm_fsm_en = umda_hpm_fsm_en;
    gopPMCAllReg->PERI_PMC8.u32 = o_peri_pmc8.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC8.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc8mda_avs_hpm_fsm_calc_mode
//  Description : Set the value of the member PERI_PMC8.mda_avs_hpm_fsm_calc_mode
//  Input       : unsigned int umda_avs_hpm_fsm_calc_mode: 2 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc8mda_avs_hpm_fsm_calc_mode(unsigned int umda_avs_hpm_fsm_calc_mode)
{
    U_PERI_PMC8 o_peri_pmc8;
    o_peri_pmc8.u32 = gopPMCAllReg->PERI_PMC8.u32;
    o_peri_pmc8.bits.mda0_avs_hpm_fsm_calc_mode = umda_avs_hpm_fsm_calc_mode;
    gopPMCAllReg->PERI_PMC8.u32 = o_peri_pmc8.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC8.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc8mda_avs_fsm_dec_pwm_step
//  Description : Set the value of the member PERI_PMC8.mda_avs_fsm_dec_pwm_step
//  Input       : unsigned int umda_avs_fsm_dec_pwm_step: 8 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc8mda_avs_fsm_dec_pwm_step(unsigned int umda_avs_fsm_dec_pwm_step)
{
    U_PERI_PMC8 o_peri_pmc8;
    o_peri_pmc8.u32 = gopPMCAllReg->PERI_PMC8.u32;
    o_peri_pmc8.bits.mda0_avs_fsm_dec_pwm_step = umda_avs_fsm_dec_pwm_step;
    gopPMCAllReg->PERI_PMC8.u32 = o_peri_pmc8.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC8.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc8mda_avs_fsm_inc_pwm_step
//  Description : Set the value of the member PERI_PMC8.mda_avs_fsm_inc_pwm_step
//  Input       : unsigned int umda_avs_fsm_inc_pwm_step: 8 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc8mda_avs_fsm_inc_pwm_step(unsigned int umda_avs_fsm_inc_pwm_step)
{
    U_PERI_PMC8 o_peri_pmc8;
    o_peri_pmc8.u32 = gopPMCAllReg->PERI_PMC8.u32;
    o_peri_pmc8.bits.mda0_avs_fsm_inc_pwm_step = umda_avs_fsm_inc_pwm_step;
    gopPMCAllReg->PERI_PMC8.u32 = o_peri_pmc8.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC8.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc10core_hpm_div
//  Description : Set the value of the member PERI_PMC10.core_hpm_div
//  Input       : unsigned int ucore_hpm_div: 6 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc10core_hpm_div(unsigned int ucore_hpm_div)
{
    U_PERI_PMC10 o_peri_pmc10;
    o_peri_pmc10.u32 = gopPMCAllReg->PERI_PMC10.u32;
    o_peri_pmc10.bits.core_hpm_div = ucore_hpm_div;
    gopPMCAllReg->PERI_PMC10.u32 = o_peri_pmc10.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC10.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc10core_hpm_shift
//  Description : Set the value of the member PERI_PMC10.core_hpm_shift
//  Input       : unsigned int ucore_hpm_shift: 2 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc10core_hpm_shift(unsigned int ucore_hpm_shift)
{
    U_PERI_PMC10 o_peri_pmc10;
    o_peri_pmc10.u32 = gopPMCAllReg->PERI_PMC10.u32;
    o_peri_pmc10.bits.core_hpm_shift = ucore_hpm_shift;
    gopPMCAllReg->PERI_PMC10.u32 = o_peri_pmc10.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC10.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc10core_hpm_offset
//  Description : Set the value of the member PERI_PMC10.core_hpm_offset
//  Input       : unsigned int ucore_hpm_offset: 10 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc10core_hpm_offset(unsigned int ucore_hpm_offset)
{
    U_PERI_PMC10 o_peri_pmc10;
    o_peri_pmc10.u32 = gopPMCAllReg->PERI_PMC10.u32;
    o_peri_pmc10.bits.core_hpm_offset = ucore_hpm_offset;
    gopPMCAllReg->PERI_PMC10.u32 = o_peri_pmc10.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC10.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc10core_hpm_en
//  Description : Set the value of the member PERI_PMC10.core_hpm_en
//  Input       : unsigned int ucore_hpm_en: 1 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc10core_hpm_en(unsigned int ucore_hpm_en)
{
    U_PERI_PMC10 o_peri_pmc10;
    o_peri_pmc10.u32 = gopPMCAllReg->PERI_PMC10.u32;
    o_peri_pmc10.bits.core_hpm_en = ucore_hpm_en;
    gopPMCAllReg->PERI_PMC10.u32 = o_peri_pmc10.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC10.u32));

    return 1;
}


//******************************************************************************
//  Function    : pm_set_pmc10core_hpm_monitor_en
//  Description : Set the value of the member PERI_PMC10.core_hpm_monitor_en
//  Input       : unsigned int ucore_hpm_monitor_en: 1 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc10core_hpm_monitor_en(unsigned int ucore_hpm_monitor_en)
{
    U_PERI_PMC10 o_peri_pmc10;
    o_peri_pmc10.u32 = gopPMCAllReg->PERI_PMC10.u32;
    o_peri_pmc10.bits.core_hpm_monitor_en = ucore_hpm_monitor_en;
    gopPMCAllReg->PERI_PMC10.u32 = o_peri_pmc10.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC10.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc10core_hpm_srst_req
//  Description : Set the value of the member PERI_PMC10.core_hpm_srst_req
//  Input       : unsigned int ucore_hpm_srst_req: 1 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc10core_hpm_srst_req(unsigned int ucore_hpm_srst_req)
{
    U_PERI_PMC10 o_peri_pmc10;
    o_peri_pmc10.u32 = gopPMCAllReg->PERI_PMC10.u32;
    o_peri_pmc10.bits.core_hpm_srst_req = ucore_hpm_srst_req;
    gopPMCAllReg->PERI_PMC10.u32 = o_peri_pmc10.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC10.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc13core_hpm_uplimit
//  Description : Set the value of the member PERI_PMC13.core_hpm_uplimit
//  Input       : unsigned int ucore_hpm_uplimit: 10 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc13core_hpm_uplimit(unsigned int ucore_hpm_uplimit)
{
    U_PERI_PMC13 o_peri_pmc13;
    o_peri_pmc13.u32 = gopPMCAllReg->PERI_PMC13.u32;
    o_peri_pmc13.bits.core_hpm_uplimit = ucore_hpm_uplimit;
    gopPMCAllReg->PERI_PMC13.u32 = o_peri_pmc13.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC13.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc13core_hpm_lowlimit
//  Description : Set the value of the member PERI_PMC13.core_hpm_lowlimit
//  Input       : unsigned int ucore_hpm_lowlimit: 10 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc13core_hpm_lowlimit(unsigned int ucore_hpm_lowlimit)
{
    U_PERI_PMC13 o_peri_pmc13;
    o_peri_pmc13.u32 = gopPMCAllReg->PERI_PMC13.u32;
    o_peri_pmc13.bits.core_hpm_lowlimit = ucore_hpm_lowlimit;
    gopPMCAllReg->PERI_PMC13.u32 = o_peri_pmc13.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC13.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc13core_hpm_monitor_period
//  Description : Set the value of the member PERI_PMC13.core_hpm_monitor_period
//  Input       : unsigned int ucore_hpm_monitor_period: 8 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc13core_hpm_monitor_period(unsigned int ucore_hpm_monitor_period)
{
    U_PERI_PMC13 o_peri_pmc13;
    o_peri_pmc13.u32 = gopPMCAllReg->PERI_PMC13.u32;
    o_peri_pmc13.bits.core_hpm_monitor_period = ucore_hpm_monitor_period;
    gopPMCAllReg->PERI_PMC13.u32 = o_peri_pmc13.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC13.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc14cpu_hpm_div
//  Description : Set the value of the member PERI_PMC14.cpu_hpm_div
//  Input       : unsigned int ucpu_hpm_div: 6 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc14cpu_hpm_div(unsigned int ucpu_hpm_div)
{
    U_PERI_PMC14 o_peri_pmc14;
    o_peri_pmc14.u32 = gopPMCAllReg->PERI_PMC14.u32;
    o_peri_pmc14.bits.a17_hpm_div = ucpu_hpm_div;
    gopPMCAllReg->PERI_PMC14.u32 = o_peri_pmc14.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC14.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc14cpu_hpm_shift
//  Description : Set the value of the member PERI_PMC14.cpu_hpm_shift
//  Input       : unsigned int ucpu_hpm_shift: 2 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc14cpu_hpm_shift(unsigned int ucpu_hpm_shift)
{
    U_PERI_PMC14 o_peri_pmc14;
    o_peri_pmc14.u32 = gopPMCAllReg->PERI_PMC14.u32;
    o_peri_pmc14.bits.a17_hpm_shift = ucpu_hpm_shift;
    gopPMCAllReg->PERI_PMC14.u32 = o_peri_pmc14.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC14.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc14cpu_hpm_offset
//  Description : Set the value of the member PERI_PMC14.cpu_hpm_offset
//  Input       : unsigned int ucpu_hpm_offset: 10 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc14cpu_hpm_offset(unsigned int ucpu_hpm_offset)
{
    U_PERI_PMC14 o_peri_pmc14;
    o_peri_pmc14.u32 = gopPMCAllReg->PERI_PMC14.u32;
    o_peri_pmc14.bits.a17_hpm_offset = ucpu_hpm_offset;
    gopPMCAllReg->PERI_PMC14.u32 = o_peri_pmc14.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC14.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc14cpu_hpm_en
//  Description : Set the value of the member PERI_PMC14.cpu_hpm_en
//  Input       : unsigned int ucpu_hpm_en: 1 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc14cpu_hpm_en(unsigned int ucpu_hpm_en)
{
    U_PERI_PMC14 o_peri_pmc14;
    o_peri_pmc14.u32 = gopPMCAllReg->PERI_PMC14.u32;
    o_peri_pmc14.bits.a17_hpm_en = ucpu_hpm_en;
    gopPMCAllReg->PERI_PMC14.u32 = o_peri_pmc14.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC14.u32));

    return 1;
}


//******************************************************************************
//  Function    : pm_set_pmc14cpu_hpm_monitor_en
//  Description : Set the value of the member PERI_PMC14.cpu_hpm_monitor_en
//  Input       : unsigned int ucpu_hpm_monitor_en: 1 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc14cpu_hpm_monitor_en(unsigned int ucpu_hpm_monitor_en)
{
    U_PERI_PMC14 o_peri_pmc14;
    o_peri_pmc14.u32 = gopPMCAllReg->PERI_PMC14.u32;
    o_peri_pmc14.bits.a17_hpm_monitor_en = ucpu_hpm_monitor_en;
    gopPMCAllReg->PERI_PMC14.u32 = o_peri_pmc14.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC14.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc14cpu_hpm_srst_req
//  Description : Set the value of the member PERI_PMC14.cpu_hpm_srst_req
//  Input       : unsigned int ucpu_hpm_srst_req: 1 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc14cpu_hpm_srst_req(unsigned int ucpu_hpm_srst_req)
{
    U_PERI_PMC14 o_peri_pmc14;
    o_peri_pmc14.u32 = gopPMCAllReg->PERI_PMC14.u32;
    o_peri_pmc14.bits.a17_hpm_srst_req = ucpu_hpm_srst_req;
    gopPMCAllReg->PERI_PMC14.u32 = o_peri_pmc14.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC14.u32));

    return 1;
}

//******************************************************************************
//  Function    : PERI_PMC16.a17_hpm_pc_record2
//  Description : get the value of the member PERI_PMC15.a17_hpm_pc_record0 + PERI_PMC16.a17_hpm_pc_record1
//  Input       : void
//  Return      : void
//******************************************************************************
int pm_get_pmc15cpu_hpm_pc_records(void)
{
    return  gopPMCAllReg->PERI_PMC15.bits.a17_hpm_pc_record0 + gopPMCAllReg->PERI_PMC15.bits.a17_hpm_pc_record1;
}

//******************************************************************************
//  Function    : pm_get_pmc16cpu_hpm_pc_records
//  Description : get the value of the member PERI_PMC16.a17_hpm_pc_record2 + PERI_PMC16.a17_hpm_pc_record3
//  Input       : void
//  Return      : void
//******************************************************************************
int pm_get_pmc16cpu_hpm_pc_records(void)
{
    return gopPMCAllReg->PERI_PMC16.bits.a17_hpm_pc_record2 + gopPMCAllReg->PERI_PMC16.bits.a17_hpm_pc_record3;
}


//******************************************************************************
//  Function    : pm_set_pmc17cpu_hpm_uplimit
//  Description : Set the value of the member PERI_PMC17.cpu_hpm_uplimit
//  Input       : unsigned int ucpu_hpm_uplimit: 10 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc17cpu_hpm_uplimit(unsigned int ucpu_hpm_uplimit)
{
    U_PERI_PMC17 o_peri_pmc17;
    o_peri_pmc17.u32 = gopPMCAllReg->PERI_PMC17.u32;
    o_peri_pmc17.bits.a17_hpm_uplimit = ucpu_hpm_uplimit;
    gopPMCAllReg->PERI_PMC17.u32 = o_peri_pmc17.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC17.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc17cpu_hpm_lowlimit
//  Description : Set the value of the member PERI_PMC17.cpu_hpm_lowlimit
//  Input       : unsigned int ucpu_hpm_lowlimit: 10 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc17cpu_hpm_lowlimit(unsigned int ucpu_hpm_lowlimit)
{
    U_PERI_PMC17 o_peri_pmc17;
    o_peri_pmc17.u32 = gopPMCAllReg->PERI_PMC17.u32;
    o_peri_pmc17.bits.a17_hpm_lowlimit = ucpu_hpm_lowlimit;
    gopPMCAllReg->PERI_PMC17.u32 = o_peri_pmc17.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC17.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc17cpu_hpm_monitor_period
//  Description : Set the value of the member PERI_PMC17.cpu_hpm_monitor_period
//  Input       : unsigned int ucpu_hpm_monitor_period: 8 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc17cpu_hpm_monitor_period(unsigned int ucpu_hpm_monitor_period)
{     
    U_PERI_PMC17 o_peri_pmc17;
    o_peri_pmc17.u32 = gopPMCAllReg->PERI_PMC17.u32;
    o_peri_pmc17.bits.a17_hpm_monitor_period = ucpu_hpm_monitor_period;
    gopPMCAllReg->PERI_PMC17.u32 = o_peri_pmc17.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC17.u32));
    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc18ddr_hpm_div
//  Description : Set the value of the member PERI_PMC18.ddr_hpm_div
//  Input       : unsigned int uddr_hpm_div: 6 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc18ddr_hpm_div(unsigned int uddr_hpm_div)
{
    U_PERI_PMC18 o_peri_pmc18;
    o_peri_pmc18.u32 = gopPMCAllReg->PERI_PMC18.u32;
    o_peri_pmc18.bits.ddr_hpm_div = uddr_hpm_div;
    gopPMCAllReg->PERI_PMC18.u32 = o_peri_pmc18.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC18.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc18ddr_hpm_shift
//  Description : Set the value of the member PERI_PMC18.ddr_hpm_shift
//  Input       : unsigned int uddr_hpm_shift: 2 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc18ddr_hpm_shift(unsigned int uddr_hpm_shift)
{
    U_PERI_PMC18 o_peri_pmc18;
    o_peri_pmc18.u32 = gopPMCAllReg->PERI_PMC18.u32;
    o_peri_pmc18.bits.ddr_hpm_shift = uddr_hpm_shift;
    gopPMCAllReg->PERI_PMC18.u32 = o_peri_pmc18.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC18.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc18ddr_hpm_offset
//  Description : Set the value of the member PERI_PMC18.ddr_hpm_offset
//  Input       : unsigned int uddr_hpm_offset: 10 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc18ddr_hpm_offset(unsigned int uddr_hpm_offset)
{
    U_PERI_PMC18 o_peri_pmc18;
    o_peri_pmc18.u32 = gopPMCAllReg->PERI_PMC18.u32;
    o_peri_pmc18.bits.ddr_hpm_offset = uddr_hpm_offset;
    gopPMCAllReg->PERI_PMC18.u32 = o_peri_pmc18.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC18.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc18ddr_hpm_en
//  Description : Set the value of the member PERI_PMC18.ddr_hpm_en
//  Input       : unsigned int uddr_hpm_en: 1 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc18ddr_hpm_en(unsigned int uddr_hpm_en)
{
    U_PERI_PMC18 o_peri_pmc18;
    o_peri_pmc18.u32 = gopPMCAllReg->PERI_PMC18.u32;
    o_peri_pmc18.bits.ddr_hpm_en = uddr_hpm_en;
    gopPMCAllReg->PERI_PMC18.u32 = o_peri_pmc18.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC18.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc18ddr_hpm_monitor_en
//  Description : Set the value of the member PERI_PMC18.ddr_hpm_monitor_en
//  Input       : unsigned int uddr_hpm_monitor_en: 1 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc18ddr_hpm_monitor_en(unsigned int uddr_hpm_monitor_en)
{
    U_PERI_PMC18 o_peri_pmc18;
    o_peri_pmc18.u32 = gopPMCAllReg->PERI_PMC18.u32;
    o_peri_pmc18.bits.ddr_hpm_monitor_en = uddr_hpm_monitor_en;
    gopPMCAllReg->PERI_PMC18.u32 = o_peri_pmc18.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC18.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc18ddr_hpm_srst_req
//  Description : Set the value of the member PERI_PMC18.ddr_hpm_srst_req
//  Input       : unsigned int uddr_hpm_srst_req: 1 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc18ddr_hpm_srst_req(unsigned int uddr_hpm_srst_req)
{
    U_PERI_PMC18 o_peri_pmc18;
    o_peri_pmc18.u32 = gopPMCAllReg->PERI_PMC18.u32;
    o_peri_pmc18.bits.ddr_hpm_srst_req = uddr_hpm_srst_req;
    gopPMCAllReg->PERI_PMC18.u32 = o_peri_pmc18.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC18.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc21ddr_hpm_uplimit
//  Description : Set the value of the member PERI_PMC21.ddr_hpm_uplimit
//  Input       : unsigned int uddr_hpm_uplimit: 10 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc21ddr_hpm_uplimit(unsigned int uddr_hpm_uplimit)
{
    U_PERI_PMC21 o_peri_pmc21;
    o_peri_pmc21.u32 = gopPMCAllReg->PERI_PMC21.u32;
    o_peri_pmc21.bits.ddr_hpm_uplimit = uddr_hpm_uplimit;
    gopPMCAllReg->PERI_PMC21.u32 = o_peri_pmc21.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC21.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc21ddr_hpm_lowlimit
//  Description : Set the value of the member PERI_PMC21.ddr_hpm_lowlimit
//  Input       : unsigned int uddr_hpm_lowlimit: 10 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc21ddr_hpm_lowlimit(unsigned int uddr_hpm_lowlimit)
{
    U_PERI_PMC21 o_peri_pmc21;
    o_peri_pmc21.u32 = gopPMCAllReg->PERI_PMC21.u32;
    o_peri_pmc21.bits.ddr_hpm_lowlimit = uddr_hpm_lowlimit;
    gopPMCAllReg->PERI_PMC21.u32 = o_peri_pmc21.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC21.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc21ddr_hpm_monitor_period
//  Description : Set the value of the member PERI_PMC21.ddr_hpm_monitor_period
//  Input       : unsigned int uddr_hpm_monitor_period: 8 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc21ddr_hpm_monitor_period(unsigned int uddr_hpm_monitor_period)
{
    U_PERI_PMC21 o_peri_pmc21;
    o_peri_pmc21.u32 = gopPMCAllReg->PERI_PMC21.u32;
    o_peri_pmc21.bits.ddr_hpm_monitor_period = uddr_hpm_monitor_period;
    gopPMCAllReg->PERI_PMC21.u32 = o_peri_pmc21.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC21.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc22mda_top_hpm_div
//  Description : Set the value of the member PERI_PMC22.mda_top_hpm_div
//  Input       : unsigned int umda_top_hpm_div: 6 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc22mda0_hpm_div(unsigned int umda_hpm_div)
{
    U_PERI_PMC22 o_peri_pmc22;
    o_peri_pmc22.u32 = gopPMCAllReg->PERI_PMC22.u32;
    o_peri_pmc22.bits.mda0_hpm_div = umda_hpm_div;
    gopPMCAllReg->PERI_PMC22.u32 = o_peri_pmc22.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC22.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc22mda_top_hpm_shift
//  Description : Set the value of the member PERI_PMC22.mda_top_hpm_shift
//  Input       : unsigned int umda_top_hpm_shift: 2 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc22mda0_hpm_shift(unsigned int umda_hpm_shift)
{
    U_PERI_PMC22 o_peri_pmc22;
    o_peri_pmc22.u32 = gopPMCAllReg->PERI_PMC22.u32;
    o_peri_pmc22.bits.mda0_hpm_shift = umda_hpm_shift;
    gopPMCAllReg->PERI_PMC22.u32 = o_peri_pmc22.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC22.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc22mda_top_hpm_offset
//  Description : Set the value of the member PERI_PMC22.mda_top_hpm_offset
//  Input       : unsigned int umda_top_hpm_offset: 10 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc22mda0_hpm_offset(unsigned int umda_top_hpm_offset)
{
    U_PERI_PMC22 o_peri_pmc22;
    o_peri_pmc22.u32 = gopPMCAllReg->PERI_PMC22.u32;
    o_peri_pmc22.bits.mda0_hpm_offset = umda_top_hpm_offset;
    gopPMCAllReg->PERI_PMC22.u32 = o_peri_pmc22.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC22.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc22mda_top_hpm_en
//  Description : Set the value of the member PERI_PMC22.mda_top_hpm_en
//  Input       : unsigned int umda_top_hpm_en: 1 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc22mda0_hpm_en(unsigned int umda_hpm_en)
{
    U_PERI_PMC22 o_peri_pmc22;
    o_peri_pmc22.u32 = gopPMCAllReg->PERI_PMC22.u32;
    o_peri_pmc22.bits.mda0_hpm_en = umda_hpm_en;
    gopPMCAllReg->PERI_PMC22.u32 = o_peri_pmc22.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC22.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc22mda_top_hpm_monitor_en
//  Description : Set the value of the member PERI_PMC22.mda_top_hpm_monitor_en
//  Input       : unsigned int umda_top_hpm_monitor_en: 1 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc22mda0_hpm_monitor_en(unsigned int umda_hpm_monitor_en)
{
    U_PERI_PMC22 o_peri_pmc22;
    o_peri_pmc22.u32 = gopPMCAllReg->PERI_PMC22.u32;
    o_peri_pmc22.bits.mda0_hpm_monitor_en = umda_hpm_monitor_en;
    gopPMCAllReg->PERI_PMC22.u32 = o_peri_pmc22.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC22.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc22mda_top_hpm_srst_req
//  Description : Set the value of the member PERI_PMC22.mda_top_hpm_srst_req
//  Input       : unsigned int umda_top_hpm_srst_req: 1 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc22mda0_hpm_srst_req(unsigned int umda_hpm_srst_req)
{
    U_PERI_PMC22 o_peri_pmc22;
    o_peri_pmc22.u32 = gopPMCAllReg->PERI_PMC22.u32;
    o_peri_pmc22.bits.mda0_hpm_srst_req = umda_hpm_srst_req;
    gopPMCAllReg->PERI_PMC22.u32 = o_peri_pmc22.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC22.u32));

    return 1;
}
//******************************************************************************
//  Function    : pm_get_pmc23mda0_hpm_pc_records
//  Description : get the value of the member PERI_PMC23.bits.mda0_hpm_pc_record0 + PERI_PMC23.bits.mda0_hpm_pc_record1
//  Input       : void
//  Return      : void
//******************************************************************************
int pm_get_pmc23mda0_hpm_pc_records(void)
{
    return  gopPMCAllReg->PERI_PMC23.bits.mda0_hpm_pc_record0 + gopPMCAllReg->PERI_PMC23.bits.mda0_hpm_pc_record1;
}

//******************************************************************************
//  Function    : pm_get_pmc24mda0_hpm_pc_records
//  Description : get the value of the member PERI_PMC24.bits.mda0_hpm_pc_record2 + PERI_PMC24.bits.mda0_hpm_pc_record3
//  Input       : void
//  Return      : void
//******************************************************************************
int pm_get_pmc24mda0_hpm_pc_records(void)
{
    return  gopPMCAllReg->PERI_PMC24.bits.mda0_hpm_pc_record2 + gopPMCAllReg->PERI_PMC24.bits.mda0_hpm_pc_record3;
}

//******************************************************************************
//  Function    : pm_set_pmc25mda_top_hpm_uplimit
//  Description : Set the value of the member PERI_PMC25.mda_top_hpm_uplimit
//  Input       : unsigned int umda_top_hpm_uplimit: 10 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc25mda0_hpm_uplimit(unsigned int umda_hpm_uplimit)
{
    U_PERI_PMC25 o_peri_pmc25;
    o_peri_pmc25.u32 = gopPMCAllReg->PERI_PMC25.u32;
    o_peri_pmc25.bits.mda0_hpm_uplimit = umda_hpm_uplimit;
    gopPMCAllReg->PERI_PMC25.u32 = o_peri_pmc25.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC25.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc25mda_top_hpm_lowlimit
//  Description : Set the value of the member PERI_PMC25.mda_top_hpm_lowlimit
//  Input       : unsigned int umda_top_hpm_lowlimit: 10 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc25mda0_hpm_lowlimit(unsigned int umda_hpm_lowlimit)
{
    U_PERI_PMC25 o_peri_pmc25;
    o_peri_pmc25.u32 = gopPMCAllReg->PERI_PMC25.u32;
    o_peri_pmc25.bits.mda0_hpm_lowlimit = umda_hpm_lowlimit;
    gopPMCAllReg->PERI_PMC25.u32 = o_peri_pmc25.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC25.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc25mda_top_hpm_monitor_period
//  Description : Set the value of the member PERI_PMC25.mda_top_hpm_monitor_period
//  Input       : unsigned int umda_top_hpm_monitor_period: 8 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc25mda0_hpm_monitor_period(unsigned int umda_hpm_monitor_period)
{
    U_PERI_PMC25 o_peri_pmc25;
    o_peri_pmc25.u32 = gopPMCAllReg->PERI_PMC25.u32;
    o_peri_pmc25.bits.mda0_hpm_monitor_period = umda_hpm_monitor_period;
    gopPMCAllReg->PERI_PMC25.u32 = o_peri_pmc25.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC25.u32));

    return 1;
}


//******************************************************************************
//  Function    : pm_set_pmc30mda_top_hpm_monitor_period
//  Description : Set the value of the member PERI_PMC30.mda_top_hpm_monitor_period
//  Input       : unsigned int umda_top_hpm_monitor_period: 8 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc30mda0_hpm_monitor_period_fine(unsigned int umda_hpm_monitor_period_fine)
{
    U_PERI_PMC30 o_peri_pmc30;
    o_peri_pmc30.u32 = gopPMCAllReg->PERI_PMC30.u32;
    o_peri_pmc30.bits.mda0_hpm_monitor_period_fine = umda_hpm_monitor_period_fine;
    gopPMCAllReg->PERI_PMC30.u32 = o_peri_pmc30.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC30.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc30ddr_hpm_monitor_period
//  Description : Set the value of the member PERI_PMC30.ddr_hpm_monitor_period
//  Input       : unsigned int uddr_hpm_monitor_period: 8 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc30ddr_hpm_monitor_period_fine(unsigned int uddr_hpm_monitor_period_fine)
{
    U_PERI_PMC30 o_peri_pmc30;
    o_peri_pmc30.u32 = gopPMCAllReg->PERI_PMC30.u32;
    o_peri_pmc30.bits.ddr_hpm_monitor_period_fine = uddr_hpm_monitor_period_fine;
    gopPMCAllReg->PERI_PMC30.u32 = o_peri_pmc30.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC30.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc30cpu_hpm_monitor_period
//  Description : Set the value of the member PERI_PMC30.cpu_hpm_monitor_period
//  Input       : unsigned int ucpu_hpm_monitor_period: 8 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc30cpu_hpm_monitor_period_fine(unsigned int ucpu_hpm_monitor_period_fine)
{
    U_PERI_PMC30 o_peri_pmc30;
    
    o_peri_pmc30.u32 = gopPMCAllReg->PERI_PMC30.u32;
    o_peri_pmc30.bits.a17_hpm_monitor_period_fine = ucpu_hpm_monitor_period_fine;
    gopPMCAllReg->PERI_PMC30.u32 = o_peri_pmc30.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC30.u32));
    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc31core_hpm_monitor_period
//  Description : Set the value of the member PERI_PMC31.core_hpm_monitor_period
//  Input       : unsigned int ucore_hpm_monitor_period: 8 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc31core_hpm_monitor_period_fine(unsigned int ucore_hpm_monitor_period_fine)
{
    U_PERI_PMC31 o_peri_pmc31;
    o_peri_pmc31.u32 = gopPMCAllReg->PERI_PMC31.u32;
    o_peri_pmc31.bits.core_hpm_monitor_period_fine = ucore_hpm_monitor_period_fine;
    gopPMCAllReg->PERI_PMC31.u32 = o_peri_pmc31.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC31.u32));

    return 1;
}


//******************************************************************************
//  Function    : pm_set_pmc69tsensor_temp_uplimit
//  Description : Set the value of the member PERI_PMC69.tsensor_temp_uplimit
//  Input       : unsigned int utsensor_uplimit: 10 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc69tsensor_temp_uplimit(unsigned int utsensor_uplimit)
{
    U_PERI_PMC69 o_peri_pmc69;
    o_peri_pmc69.u32 = gopPMCAllReg->PERI_PMC69.u32;
    o_peri_pmc69.bits.tsensor_temp_uplimit = utsensor_uplimit;
    gopPMCAllReg->PERI_PMC69.u32 = o_peri_pmc69.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC69.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc69tsensor_temp_lowlimit
//  Description : Set the value of the member PERI_PMC69.tsensor_temp_lowlimit
//  Input       : unsigned int utsensor_hpm_lowlimit: 10 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc69tsensor_temp_lowlimit(unsigned int utsensor_hpm_lowlimit)
{
    U_PERI_PMC69 o_peri_pmc69;
    o_peri_pmc69.u32 = gopPMCAllReg->PERI_PMC69.u32;
    o_peri_pmc69.bits.tsensor_temp_lowlimit = utsensor_hpm_lowlimit;
    gopPMCAllReg->PERI_PMC69.u32 = o_peri_pmc69.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC69.u32));
    return 1;
}

//******************************************************************************
//  Function    : pm_set_pmc68tsensor_monitor_period
//  Description : Set the value of the member PERI_PMC68.tsensor_monitor_period
//  Input       : unsigned int utsensor_monitor_period: 8 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc68tsensor_monitor_period(unsigned int utsensor_monitor_period)
{
    U_PERI_PMC68 o_peri_pmc68;
    o_peri_pmc68.u32 = gopPMCAllReg->PERI_PMC68.u32;
    o_peri_pmc68.bits.tsensor_monitor_period = utsensor_monitor_period;
    gopPMCAllReg->PERI_PMC68.u32 = o_peri_pmc68.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC68.u32));

    return 1;
}


//******************************************************************************
//  Function    : pm_set_pmc68tsensor_monitor_en
//  Description : Set the value of the member PERI_PMC68.tsensor_monitor_en
//  Input       : unsigned int utsensor_monitor_en: 1 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc68tsensor_monitor_en(unsigned int utsensor_monitor_en)
{
    U_PERI_PMC68 o_peri_pmc68;
    o_peri_pmc68.u32 = gopPMCAllReg->PERI_PMC68.u32;
    o_peri_pmc68.bits.tsensor_monitor_en = utsensor_monitor_en;
    gopPMCAllReg->PERI_PMC68.u32 = o_peri_pmc68.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC68.u32));

    return 1;
}


//******************************************************************************
//  Function    : pm_set_pmc68tsensor_en
//  Description : Set the value of the member PERI_PMC68.tsensor_monitor_en
//  Input       : unsigned int utsensor_en: 1 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_set_pmc68tsensor_en(unsigned int utsensor_en)
{
    U_PERI_PMC68 o_peri_pmc68;
    o_peri_pmc68.u32 = gopPMCAllReg->PERI_PMC68.u32;
    o_peri_pmc68.bits.tsensor_en = utsensor_en;
    gopPMCAllReg->PERI_PMC68.u32 = o_peri_pmc68.u32;
    RTL_Verify_RegWrite(PMC_BASE_ADDR, (unsigned int *)&(gopPMCAllReg->PERI_PMC68.u32));

    return 1;
}

//******************************************************************************
//  Function    : pm_get_pmc70tsensor_temp_records
//  Description : get the value of the member PERI_PMC70.bits.tsensor_temp_code0 + PERI_PMC70.bits.tsensor_temp_code1
//  Input       : void
//  Return      : void
//******************************************************************************
int pm_get_pmc70tsensor_temp_records(void)
{
    return  gopPMCAllReg->PERI_PMC70.bits.tsensor_temp_code0 + gopPMCAllReg->PERI_PMC70.bits.tsensor_temp_code1;
}

//******************************************************************************
//  Function    : pm_get_pmc71tsensor_temp_records
//  Description : get the value of the member PERI_PMC70.bits.tsensor_temp_code0 + PERI_PMC71.bits.tsensor_temp_code1
//  Input       : void
//  Return      : void
//******************************************************************************
int pm_get_pmc71tsensor_temp_records(void)
{
    return  gopPMCAllReg->PERI_PMC71.bits.tsensor_temp_code2 + gopPMCAllReg->PERI_PMC71.bits.tsensor_temp_code3;
}

//******************************************************************************
//  Function    : pm_get_pmc72tsensor_temp_records
//  Description : get the value of the member PERI_PMC72.bits.tsensor_temp_code0 + PERI_PMC72.bits.tsensor_temp_code1
//  Input       : void
//  Return      : void
//******************************************************************************
int pm_get_pmc72tsensor_temp_records(void)
{
    return  gopPMCAllReg->PERI_PMC72.bits.tsensor_temp_code4 + gopPMCAllReg->PERI_PMC72.bits.tsensor_temp_code5;
}


//******************************************************************************
//  Function    : pm_get_pmc73tsensor_temp_records
//  Description : get the value of the member PERI_PMC73.bits.tsensor_temp_code0 + PERI_PMC73.bits.tsensor_temp_code1
//  Input       : void
//  Return      : void
//******************************************************************************
int pm_get_pmc73tsensor_temp_records(void)
{
    return  gopPMCAllReg->PERI_PMC73.bits.tsensor_temp_code6 + gopPMCAllReg->PERI_PMC73.bits.tsensor_temp_code7;
}





//******************************************************************************
//  Function    : pm_set_pmc31core_hpm_monitor_period
//  Description : Set the value of the member PERI_PMC31.core_hpm_monitor_period
//  Input       : unsigned int ucore_hpm_monitor_period: 8 bits
//  Return      : int: 0-Error, 1-Success
//******************************************************************************
int pm_get_profie_reg_profile(unsigned int *profile)
{
    *profile = profile_reg->bits.hw_profile;
    return 1;
}


