//****************************************************************************** 
// Copyright     :  Copyright (C) 2014, Hisilicon Technologies Co., Ltd.
// File name     :  pmc.h
// Author        :  xxx
// Version       :  1.0 
// Date          :  2014-06-19
// Description   :  Define all registers/tables for xxx
// Others        :  Generated automatically by nManager V4.0 
// History       :  xxx 2014-06-19 Create file
//******************************************************************************

#ifndef __PM_HAL_H__
#define __PM_HAL_H__

/* Define the union U_PERI_PMC0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    core_pwm_period       : 16  ; /* [15..0]  */
        unsigned int    core_pwm_duty         : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_PERI_PMC0;

/* Define the union U_PERI_PMC1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    a17_pwm_period        : 16  ; /* [15..0]  */
        unsigned int    a17_pwm_duty          : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_PERI_PMC1;

/* Define the union U_PERI_PMC2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ddr_pwm_period        : 16  ; /* [15..0]  */
        unsigned int    ddr_pwm_duty          : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_PERI_PMC2;

/* Define the union U_PERI_PMC3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mda0_pwm_period        : 16  ; /* [15..0]  */
        unsigned int    mda0_pwm_duty          : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_PERI_PMC3;

/* Define the union U_PERI_PMC4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    core_pwm_enable       : 1   ; /* [0]  */
        unsigned int    core_pwm_inv          : 1   ; /* [1]  */
        unsigned int    a17_pwm_enable        : 1   ; /* [2]  */
        unsigned int    a17_pwm_inv           : 1   ; /* [3]  */
        unsigned int    ddr_pwm_enable        : 1   ; /* [4]  */
        unsigned int    ddr_pwm_inv           : 1   ; /* [5]  */
        unsigned int    mda0_pwm_enable        : 1   ; /* [6]  */
        unsigned int    mda0_pwm_inv           : 1   ; /* [7]  */
        unsigned int    pwm0_reuse_cfg        : 2   ; /* [9..8]  */
        unsigned int    pwm1_reuse_cfg        : 2   ; /* [11..10]  */
        unsigned int    pwm2_reuse_cfg        : 2   ; /* [13..12]  */
        unsigned int    pwm3_reuse_cfg        : 2   ; /* [15..14]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_PERI_PMC4;

/* Define the union U_PERI_PMC5 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    core_hpm_fsm_en       : 1   ; /* [0]  */
        unsigned int    core_avs_hpm_fsm_calc_mode : 2   ; /* [2..1]  */
        unsigned int    reserved_0            : 13  ; /* [15..3]  */
        unsigned int    core_avs_fsm_dec_pwm_step : 8   ; /* [23..16]  */
        unsigned int    core_avs_fsm_inc_pwm_step : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_PERI_PMC5;

/* Define the union U_PERI_PMC6 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    a17_hpm_fsm_en        : 1   ; /* [0]  */
        unsigned int    a17_avs_hpm_fsm_calc_mode : 2   ; /* [2..1]  */
        unsigned int    reserved_0            : 13  ; /* [15..3]  */
        unsigned int    a17_avs_fsm_dec_pwm_step : 8   ; /* [23..16]  */
        unsigned int    a17_avs_fsm_inc_pwm_step : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_PERI_PMC6;

/* Define the union U_PERI_PMC7 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ddr_hpm_fsm_en        : 1   ; /* [0]  */
        unsigned int    ddr_avs_hpm_fsm_calc_mode : 2   ; /* [2..1]  */
        unsigned int    reserved_0            : 13  ; /* [15..3]  */
        unsigned int    ddr_avs_fsm_dec_pwm_step : 8   ; /* [23..16]  */
        unsigned int    ddr_avs_fsm_inc_pwm_step : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_PERI_PMC7;

/* Define the union U_PERI_PMC8 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mda0_hpm_fsm_en        : 1   ; /* [0]  */
        unsigned int    mda0_avs_hpm_fsm_calc_mode : 2   ; /* [2..1]  */
        unsigned int    reserved_0            : 13  ; /* [15..3]  */
        unsigned int    mda0_avs_fsm_dec_pwm_step : 8   ; /* [23..16]  */
        unsigned int    mda0_avs_fsm_inc_pwm_step : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_PERI_PMC8;

/* Define the union U_PERI_PMC9 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    core_avs_state_machine : 3   ; /* [2..0]  */
        unsigned int    reserved_0            : 1   ; /* [3]  */
        unsigned int    cpu_avs_state_machine : 3   ; /* [6..4]  */
        unsigned int    reserved_1            : 1   ; /* [7]  */
        unsigned int    ddr_avs_state_machine : 3   ; /* [10..8]  */
        unsigned int    reserved_2            : 1   ; /* [11]  */
        unsigned int    mda_avs_state_machine : 3   ; /* [14..12]  */
        unsigned int    reserved_3            : 17  ; /* [31..15]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_PERI_PMC9;

/* Define the union U_PERI_PMC10 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    core_hpm_div          : 6   ; /* [5..0]  */
        unsigned int    reserved_0            : 2   ; /* [7..6]  */
        unsigned int    core_hpm_shift        : 2   ; /* [9..8]  */
        unsigned int    reserved_1            : 2   ; /* [11..10]  */
        unsigned int    core_hpm_offset       : 10  ; /* [21..12]  */
        unsigned int    reserved_2            : 2   ; /* [23..22]  */
        unsigned int    core_hpm_en           : 1   ; /* [24]  */
        unsigned int    reserved              : 1   ; /* [25]  */
        unsigned int    core_hpm_monitor_en   : 1   ; /* [26]  */
        unsigned int    core_hpm_srst_req     : 1   ; /* [27]  */
        unsigned int    reserved_3            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_PERI_PMC10;

/* Define the union U_PERI_PMC11 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    core_hpm_pc_record0   : 10  ; /* [9..0]  */
        unsigned int    core_hpm_pc_valid     : 1   ; /* [10]  */
        unsigned int    reserved_0            : 1   ; /* [11]  */
        unsigned int    core_hpm_pc_record1   : 10  ; /* [21..12]  */
        unsigned int    reserved_1            : 2   ; /* [23..22]  */
        unsigned int    core_hpm_low_warning  : 1   ; /* [24]  */
        unsigned int    core_hpm_up_warning   : 1   ; /* [25]  */
        unsigned int    reserved_2            : 6   ; /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_PERI_PMC11;

/* Define the union U_PERI_PMC12 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    core_hpm_pc_record2   : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 2   ; /* [11..10]  */
        unsigned int    core_hpm_pc_record3   : 10  ; /* [21..12]  */
        unsigned int    reserved_1            : 2   ; /* [23..22]  */
        unsigned int    core_hpm_rcc          : 5   ; /* [28..24]  */
        unsigned int    reserved_2            : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_PERI_PMC12;

/* Define the union U_PERI_PMC13 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    core_hpm_uplimit      : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 2   ; /* [11..10]  */
        unsigned int    core_hpm_lowlimit     : 10  ; /* [21..12]  */
        unsigned int    reserved_1            : 2   ; /* [23..22]  */
        unsigned int    core_hpm_monitor_period : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_PERI_PMC13;

/* Define the union U_PERI_PMC14 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    a17_hpm_div           : 6   ; /* [5..0]  */
        unsigned int    reserved_0            : 2   ; /* [7..6]  */
        unsigned int    a17_hpm_shift         : 2   ; /* [9..8]  */
        unsigned int    reserved_1            : 2   ; /* [11..10]  */
        unsigned int    a17_hpm_offset        : 10  ; /* [21..12]  */
        unsigned int    reserved_2            : 2   ; /* [23..22]  */
        unsigned int    a17_hpm_en            : 1   ; /* [24]  */
        unsigned int    a17_hpm_bypass        : 1   ; /* [25]  */
        unsigned int    a17_hpm_monitor_en    : 1   ; /* [26]  */
        unsigned int    a17_hpm_srst_req      : 1   ; /* [27]  */
        unsigned int    reserved_3            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_PERI_PMC14;

/* Define the union U_PERI_PMC15 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    a17_hpm_pc_record0    : 10  ; /* [9..0]  */
        unsigned int    a17_hpm_pc_valid      : 1   ; /* [10]  */
        unsigned int    reserved_0            : 1   ; /* [11]  */
        unsigned int    a17_hpm_pc_record1    : 10  ; /* [21..12]  */
        unsigned int    reserved_1            : 2   ; /* [23..22]  */
        unsigned int    a17_hpm_low_warning   : 1   ; /* [24]  */
        unsigned int    a17_hpm_up_warning    : 1   ; /* [25]  */
        unsigned int    reserved_2            : 6   ; /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_PERI_PMC15;

/* Define the union U_PERI_PMC16 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    a17_hpm_pc_record2    : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 2   ; /* [11..10]  */
        unsigned int    a17_hpm_pc_record3    : 10  ; /* [21..12]  */
        unsigned int    reserved_1            : 2   ; /* [23..22]  */
        unsigned int    a17_hpm_rcc           : 5   ; /* [28..24]  */
        unsigned int    reserved_2            : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_PERI_PMC16;

/* Define the union U_PERI_PMC17 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    a17_hpm_uplimit       : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 2   ; /* [11..10]  */
        unsigned int    a17_hpm_lowlimit      : 10  ; /* [21..12]  */
        unsigned int    reserved_1            : 2   ; /* [23..22]  */
        unsigned int    a17_hpm_monitor_period : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_PERI_PMC17;

/* Define the union U_PERI_PMC18 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ddr_hpm_div           : 6   ; /* [5..0]  */
        unsigned int    reserved_0            : 2   ; /* [7..6]  */
        unsigned int    ddr_hpm_shift         : 2   ; /* [9..8]  */
        unsigned int    reserved_1            : 2   ; /* [11..10]  */
        unsigned int    ddr_hpm_offset        : 10  ; /* [21..12]  */
        unsigned int    reserved_2            : 2   ; /* [23..22]  */
        unsigned int    ddr_hpm_en            : 1   ; /* [24]  */
        unsigned int    reserved_3            : 1   ; /* [25]  */
        unsigned int    ddr_hpm_monitor_en    : 1   ; /* [26]  */
        unsigned int    ddr_hpm_srst_req      : 1   ; /* [27]  */
        unsigned int    reserved_4            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_PERI_PMC18;

/* Define the union U_PERI_PMC19 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ddr_hpm_pc_record0    : 10  ; /* [9..0]  */
        unsigned int    ddr_hpm_pc_valid      : 1   ; /* [10]  */
        unsigned int    reserved_0            : 1   ; /* [11]  */
        unsigned int    ddr_hpm_pc_record1    : 10  ; /* [21..12]  */
        unsigned int    reserved_1            : 2   ; /* [23..22]  */
        unsigned int    ddr_hpm_low_warning   : 1   ; /* [24]  */
        unsigned int    ddr_hpm_up_warning    : 1   ; /* [25]  */
        unsigned int    reserved_2            : 6   ; /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_PERI_PMC19;

/* Define the union U_PERI_PMC20 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ddr_hpm_pc_record2    : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 2   ; /* [11..10]  */
        unsigned int    ddr_hpm_pc_record3    : 10  ; /* [21..12]  */
        unsigned int    reserved_1            : 2   ; /* [23..22]  */
        unsigned int    ddr_hpm_rcc           : 5   ; /* [28..24]  */
        unsigned int    reserved_2            : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_PERI_PMC20;

/* Define the union U_PERI_PMC21 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ddr_hpm_uplimit       : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 2   ; /* [11..10]  */
        unsigned int    ddr_hpm_lowlimit      : 10  ; /* [21..12]  */
        unsigned int    reserved_1            : 2   ; /* [23..22]  */
        unsigned int    ddr_hpm_monitor_period : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_PERI_PMC21;

/* Define the union U_PERI_PMC22 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mda0_hpm_div        : 6   ; /* [5..0]  */
        unsigned int    reserved_0          : 2   ; /* [7..6]  */
        unsigned int    mda0_hpm_shift      : 2   ; /* [9..8]  */
        unsigned int    reserved_1          : 2   ; /* [11..10]  */
        unsigned int    mda0_hpm_offset     : 10  ; /* [21..12]  */
        unsigned int    reserved_2          : 2   ; /* [23..22]  */
        unsigned int    mda0_hpm_en         : 1   ; /* [24]  */
        unsigned int    reserved_3          : 1   ; /* [25]  */
        unsigned int    mda0_hpm_monitor_en : 1   ; /* [26]  */
        unsigned int    mda0_hpm_srst_req   : 1   ; /* [27]  */
        unsigned int    reserved_4          : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_PERI_PMC22;

/* Define the union U_PERI_PMC23 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mda0_hpm_pc_record0     : 10  ; /* [9..0]  */
        unsigned int    mda0_hpm_pc_valid       : 1   ; /* [10]  */
        unsigned int    reserved_0              : 1   ; /* [11]  */
        unsigned int    mda0_hpm_pc_record1     : 10  ; /* [21..12]  */
        unsigned int    reserved_1              : 2   ; /* [23..22]  */
        unsigned int    mda0_hpm_low_warning    : 1   ; /* [24]  */
        unsigned int    mda0_hpm_up_warning     : 1   ; /* [25]  */
        unsigned int    reserved_2              : 6   ; /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_PERI_PMC23;

/* Define the union U_PERI_PMC24 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mda0_hpm_pc_record2     : 10  ; /* [9..0]  */
        unsigned int    reserved_0              : 2   ; /* [11..10]  */
        unsigned int    mda0_hpm_pc_record3     : 10  ; /* [21..12]  */
        unsigned int    reserved_1              : 2   ; /* [23..22]  */
        unsigned int    mda0_hpm_rcc            : 5   ; /* [28..24]  */
        unsigned int    reserved_2              : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_PERI_PMC24;

/* Define the union U_PERI_PMC25 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mda0_hpm_uplimit        : 10  ; /* [9..0]  */
        unsigned int    reserved_0              : 2   ; /* [11..10]  */
        unsigned int    mda0_hpm_lowlimit       : 10  ; /* [21..12]  */
        unsigned int    reserved_1              : 2   ; /* [23..22]  */
        unsigned int    mda0_hpm_monitor_period : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_PERI_PMC25;

/* Define the union U_PERI_PMC30 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0                      : 8   ; /* [7..0]  */
        unsigned int    mda0_hpm_monitor_period_fine    : 8   ; /* [15..8]  */
        unsigned int    ddr_hpm_monitor_period_fine     : 8   ; /* [23..16]  */
        unsigned int    a17_hpm_monitor_period_fine     : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_PERI_PMC30;

/* Define the union U_PERI_PMC31 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    core_hpm_monitor_period_fine    : 8   ; /* [7..0]  */
        unsigned int    reserved_0                      : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_PERI_PMC31;


/* Define the union U_PERI_PMC68 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0                      : 16   ;/* [15..0]  */
        unsigned int    tsensor_monitor_period          : 8   ; /* [23..16]  */
        unsigned int    reserved_1                      : 5   ; /* [28..24]  */
        unsigned int    tsensor_monitor_en              : 1   ; /* [29]  */
        unsigned int    tsensor_en                      : 1   ; /* [30]  */
        unsigned int    reserved_2                      : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_PERI_PMC68;

/* Define the union U_PERI_PMC69 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    tsensor_temp_lowlimit           : 10   ;/* [9..0]  */
        unsigned int    reserved_0                      : 6   ; /* [15..10]  */
        unsigned int    tsensor_temp_uplimit            : 10  ; /* [25..16]  */
        unsigned int    reserved_1                      : 6   ; /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_PERI_PMC69;

/* Define the union U_PERI_PMC70 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    tsensor_temp_code0              : 10   ;/* [9..0]  */
        unsigned int    reserved_0                      : 6   ; /* [15..10]  */
        unsigned int    tsensor_temp_code1              : 10  ; /* [25..16]  */
        unsigned int    reserved_1                      : 6   ; /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_PERI_PMC70;

/* Define the union U_PERI_PMC71 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    tsensor_temp_code2              : 10   ;/* [9..0]  */
        unsigned int    reserved_0                      : 6   ; /* [15..10]  */
        unsigned int    tsensor_temp_code3              : 10  ; /* [25..16]  */
        unsigned int    reserved_1                      : 6   ; /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_PERI_PMC71;


/* Define the union U_PERI_PMC72 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    tsensor_temp_code4              : 10   ;/* [9..0]  */
        unsigned int    reserved_0                      : 6   ; /* [15..10]  */
        unsigned int    tsensor_temp_code5              : 10  ; /* [25..16]  */
        unsigned int    reserved_1                      : 6   ; /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_PERI_PMC72;


/* Define the union U_PERI_PMC73 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    tsensor_temp_code6              : 10   ;/* [9..0]  */
        unsigned int    reserved_0                      : 6   ; /* [15..10]  */
        unsigned int    tsensor_temp_code7              : 10  ; /* [25..16]  */
        unsigned int    reserved_1                      : 6   ; /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_PERI_PMC73;


/* Define the union U_PERI_PMC74 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0                      : 30  ; /* [31..2]  */
        unsigned int    tsensor_low_warning             : 1   ; /* [1]  */
        unsigned int    tsensor_up_warning              : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_PERI_PMC74;


typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hw_profile            : 2   ; /* [1..0]   */
        unsigned int    reserved              : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned long    u32;

} U_PROFILE_REG;

//==============================================================================
/* Define the global struct */
typedef struct
{
    volatile U_PERI_PMC0            PERI_PMC0                         ; /* 0x0 */
    volatile U_PERI_PMC1            PERI_PMC1                         ; /* 0x4 */
    volatile U_PERI_PMC2            PERI_PMC2                         ; /* 0x8 */
    volatile U_PERI_PMC3            PERI_PMC3                         ; /* 0xc */
    volatile U_PERI_PMC4            PERI_PMC4                         ; /* 0x10 */
    volatile U_PERI_PMC5            PERI_PMC5                         ; /* 0x14 */
    volatile U_PERI_PMC6            PERI_PMC6                         ; /* 0x18 */
    volatile U_PERI_PMC7            PERI_PMC7                         ; /* 0x1c */
    volatile U_PERI_PMC8            PERI_PMC8                         ; /* 0x20 */
    volatile unsigned int           RESERVED0                         ; /* 0x24 */
    volatile U_PERI_PMC10           PERI_PMC10                        ; /* 0x28 */
    volatile U_PERI_PMC11           PERI_PMC11                        ; /* 0x2c */
    volatile U_PERI_PMC12           PERI_PMC12                        ; /* 0x30 */
    volatile U_PERI_PMC13           PERI_PMC13                        ; /* 0x34 */
    volatile U_PERI_PMC14           PERI_PMC14                        ; /* 0x38 */
    volatile U_PERI_PMC15           PERI_PMC15                        ; /* 0x3c */
    volatile U_PERI_PMC16           PERI_PMC16                        ; /* 0x40 */
    volatile U_PERI_PMC17           PERI_PMC17                        ; /* 0x44 */
    volatile U_PERI_PMC18           PERI_PMC18                        ; /* 0x48 */
    volatile U_PERI_PMC19           PERI_PMC19                        ; /* 0x4c */
    volatile U_PERI_PMC20           PERI_PMC20                        ; /* 0x50 */
    volatile U_PERI_PMC21           PERI_PMC21                        ; /* 0x54 */
    volatile U_PERI_PMC22           PERI_PMC22                        ; /* 0x58 */
    volatile U_PERI_PMC23           PERI_PMC23                        ; /* 0x5c */
    volatile U_PERI_PMC24           PERI_PMC24                        ; /* 0x60 */
    volatile U_PERI_PMC25           PERI_PMC25                        ; /* 0x64 */
    volatile unsigned int           RESERVED1                         ; /* 0x68 */
    volatile unsigned int           RESERVED2                         ; /* 0x6c */
    volatile unsigned int           RESERVED3                         ; /* 0x70 */
    volatile unsigned int           RESERVED4                         ; /* 0x74 */
    volatile U_PERI_PMC30           PERI_PMC30                        ; /* 0x78 */
    volatile U_PERI_PMC31           PERI_PMC31                        ; /* 0x7c */
    volatile unsigned int           RESERVED5[36]                     ; /* 0x80~0x10c */
    volatile U_PERI_PMC68           PERI_PMC68                        ; /* 0x110 */
    volatile U_PERI_PMC69           PERI_PMC69                        ; /* 0x114 */
    volatile U_PERI_PMC70           PERI_PMC70                        ; /* 0x118 */
    volatile U_PERI_PMC71           PERI_PMC71                        ; /* 0x11c */
    volatile U_PERI_PMC72           PERI_PMC72                        ; /* 0x120 */
    volatile U_PERI_PMC73           PERI_PMC73                        ; /* 0x124 */
    volatile U_PERI_PMC74           PERI_PMC74                        ; /* 0x128 */
    
} S_PMC_REGS_TYPE;

/* Declare the struct pointor of the module PMC */
extern volatile S_PMC_REGS_TYPE *gopPMCAllReg;


int pm_set_pmc0core_pwm_period(unsigned int ucore_pwm_period);
int pm_set_pmc0core_pwm_duty(unsigned int ucore_pwm_duty);
int pm_set_pmc1cpu_pwm_period(unsigned int ucpu_pwm_period);
int pm_set_pmc1cpu_pwm_duty(unsigned int ucpu_pwm_duty);
int pm_get_pmc1cpu_pwm_period(void);
int pm_get_pmc1cpu_pwm_duty(void);
int pm_set_pmc2ddr_pwm_period(unsigned int uddr_pwm_period);
int pm_set_pmc2ddr_pwm_duty(unsigned int uddr_pwm_duty);
int pm_set_pmc3mda0_pwm_period(unsigned int umda_pwm_period);
int pm_set_pmc3mda0_pwm_duty(unsigned int umda_pwm_duty);
int pm_get_pmc3mda0_pwm_period(void);
int pm_get_pmc3mda0_pwm_duty(void);
int pm_set_pmc4core_pwm_enable(unsigned int ucore_pwm_enable);
int pm_set_pmc4core_pwm_inv(unsigned int ucore_pwm_inv);
int pm_set_pmc4cpu_pwm_enable(unsigned int ucpu_pwm_enable);
int pm_get_pmc4cpu_pwm_enable(unsigned int *ucpu_pwm_enable);
int pm_set_pmc4cpu_pwm_inv(unsigned int ucpu_pwm_inv);
int pm_set_pmc4ddr_pwm_enable(unsigned int uddr_pwm_enable);
int pm_set_pmc4ddr_pwm_inv(unsigned int uddr_pwm_inv);
int pm_set_pmc4mda0_pwm_enable(unsigned int umda_pwm_enable);
int pm_get_pmc4mda0_pwm_enable(unsigned int *umda_pwm_enable);
int pm_set_pmc4mda0_pwm_inv(unsigned int umda_pwm_inv);
int pm_set_pmc4pwm0_reuse_cfg(unsigned int upwm0_reuse_cfg);
int pm_set_pmc4pwm1_reuse_cfg(unsigned int upwm1_reuse_cfg);
int pm_set_pmc4pwm2_reuse_cfg(unsigned int upwm2_reuse_cfg);
int pm_set_pmc4pwm3_reuse_cfg(unsigned int upwm3_reuse_cfg);
int pm_set_pmc5core_hpm_fsm_en(unsigned int ucore_hpm_fsm_en);
int pm_set_pmc5core_avs_hpm_fsm_calc_mode(unsigned int ucore_avs_hpm_fsm_calc_mode);
int pm_set_pmc5core_avs_fsm_dec_pwm_step(unsigned int ucore_avs_fsm_dec_pwm_step);
int pm_set_pmc5core_avs_fsm_inc_pwm_step(unsigned int ucore_avs_fsm_inc_pwm_step);
int pm_set_pmc6cpu_hpm_fsm_en(unsigned int ucpu_hpm_fsm_en);
int pm_set_pmc6cpu_avs_hpm_fsm_calc_mode(unsigned int ucpu_avs_hpm_fsm_calc_mode);
int pm_set_pmc6cpu_avs_fsm_dec_pwm_step(unsigned int ucpu_avs_fsm_dec_pwm_step);
int pm_set_pmc6cpu_avs_fsm_inc_pwm_step(unsigned int ucpu_avs_fsm_inc_pwm_step);
int pm_set_pmc7ddr_hpm_fsm_en(unsigned int uddr_hpm_fsm_en);
int pm_set_pmc7ddr_avs_hpm_fsm_calc_mode(unsigned int uddr_avs_hpm_fsm_calc_mode);
int pm_set_pmc7ddr_avs_fsm_dec_pwm_step(unsigned int uddr_avs_fsm_dec_pwm_step);
int pm_set_pmc7ddr_avs_fsm_inc_pwm_step(unsigned int uddr_avs_fsm_inc_pwm_step);
int pm_set_pmc8mda0_hpm_fsm_en(unsigned int umda_hpm_fsm_en);
int pm_set_pmc8mda0_avs_hpm_fsm_calc_mode(unsigned int umda_avs_hpm_fsm_calc_mode);
int pm_set_pmc8mda0_avs_fsm_dec_pwm_step(unsigned int umda_avs_fsm_dec_pwm_step);
int pm_set_pmc8mda0_avs_fsm_inc_pwm_step(unsigned int umda_avs_fsm_inc_pwm_step);
int pm_set_pmc10core_hpm_div(unsigned int ucore_hpm_div);
int pm_set_pmc10core_hpm_shift(unsigned int ucore_hpm_shift);
int pm_set_pmc10core_hpm_offset(unsigned int ucore_hpm_offset);
int pm_set_pmc10core_hpm_en(unsigned int ucore_hpm_en);
int pm_set_pmc10core_hpm_monitor_en(unsigned int ucore_hpm_monitor_en);
int pm_set_pmc10core_hpm_srst_req(unsigned int ucore_hpm_srst_req);
int pm_set_pmc13core_hpm_uplimit(unsigned int ucore_hpm_uplimit);
int pm_set_pmc13core_hpm_lowlimit(unsigned int ucore_hpm_lowlimit);
int pm_set_pmc13core_hpm_monitor_period(unsigned int ucore_hpm_monitor_period);
int pm_set_pmc14cpu_hpm_div(unsigned int ucpu_hpm_div);
int pm_set_pmc14cpu_hpm_shift(unsigned int ucpu_hpm_shift);
int pm_set_pmc14cpu_hpm_offset(unsigned int ucpu_hpm_offset);
int pm_set_pmc14cpu_hpm_en(unsigned int ucpu_hpm_en);
int pm_set_pmc14cpu_hpm_monitor_en(unsigned int ucpu_hpm_monitor_en);
int pm_set_pmc14cpu_hpm_srst_req(unsigned int ucpu_hpm_srst_req);
int pm_get_pmc15cpu_hpm_pc_records(void);
int pm_get_pmc16cpu_hpm_pc_records(void);


int pm_set_pmc17cpu_hpm_uplimit(unsigned int ucpu_hpm_uplimit);
int pm_set_pmc17cpu_hpm_lowlimit(unsigned int ucpu_hpm_lowlimit);
int pm_set_pmc17cpu_hpm_monitor_period(unsigned int ucpu_hpm_monitor_period);
int pm_set_pmc18ddr_hpm_div(unsigned int uddr_hpm_div);
int pm_set_pmc18ddr_hpm_shift(unsigned int uddr_hpm_shift);
int pm_set_pmc18ddr_hpm_offset(unsigned int uddr_hpm_offset);
int pm_set_pmc18ddr_hpm_en(unsigned int uddr_hpm_en);
int pm_set_pmc18ddr_hpm_monitor_en(unsigned int uddr_hpm_monitor_en);
int pm_set_pmc18ddr_hpm_srst_req(unsigned int uddr_hpm_srst_req);
int pm_set_pmc21ddr_hpm_uplimit(unsigned int uddr_hpm_uplimit);
int pm_set_pmc21ddr_hpm_lowlimit(unsigned int uddr_hpm_lowlimit);
int pm_set_pmc21ddr_hpm_monitor_period(unsigned int uddr_hpm_monitor_period);
int pm_set_pmc22mda0_hpm_div(unsigned int umda_hpm_div);
int pm_set_pmc22mda0_hpm_shift(unsigned int umda_hpm_shift);
int pm_set_pmc22mda0_hpm_offset(unsigned int umda_hpm_offset);
int pm_set_pmc22mda0_hpm_en(unsigned int umda_hpm_en);
int pm_set_pmc22mda0_hpm_monitor_en(unsigned int umda_hpm_monitor_en);
int pm_set_pmc22mda0_hpm_srst_req(unsigned int umda_hpm_srst_req);
int pm_get_pmc23mda0_hpm_pc_records(void);
int pm_get_pmc24mda0_hpm_pc_records(void);
int pm_set_pmc25mda0_hpm_uplimit(unsigned int umda_hpm_uplimit);
int pm_set_pmc25mda0_hpm_lowlimit(unsigned int umda_hpm_lowlimit);
int pm_set_pmc25mda0_hpm_monitor_period(unsigned int umda_hpm_monitor_period);
int pm_set_pmc30mda0_hpm_monitor_period_fine(unsigned int umda_hpm_monitor_period_fine);
int pm_set_pmc30ddr_hpm_monitor_period_fine(unsigned int uddr_hpm_monitor_period_fine);
int pm_set_pmc30cpu_hpm_monitor_period_fine(unsigned int ucpu_hpm_monitor_period_fine);
int pm_set_pmc31core_hpm_monitor_period_fine(unsigned int ucore_hpm_monitor_period_fine);
int pm_set_pmc69tsensor_temp_uplimit(unsigned int utsensor_uplimit);
int pm_set_pmc69tsensor_temp_lowlimit(unsigned int umda_hpm_lowlimit);
int pm_set_pmc68tsensor_monitor_period(unsigned int utsensor_monitor_period);
int pm_set_pmc68tsensor_monitor_en(unsigned int utsensor_monitor_en);
int pm_set_pmc68tsensor_en(unsigned int utsensor_en);
int pm_get_pmc70tsensor_temp_records(void);
int pm_get_pmc71tsensor_temp_records(void);
int pm_get_pmc72tsensor_temp_records(void);
int pm_get_pmc73tsensor_temp_records(void);

int pm_get_profie_reg_profile(unsigned int *profile);

#endif /* __PMC_H__ */


