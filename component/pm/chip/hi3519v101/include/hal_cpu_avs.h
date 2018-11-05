#ifndef  __HI_HAL_CPU_AVS_H__
#define  __HI_HAL_CPU_AVS_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* Begin of #ifdef __cplusplus */

#define CPU_AVS_VOLT_STEP 10000 /*mv*/
#define CPU_AVS_HPM_DELTA_MIN 1 
#define CPU_AVS_HPM_DELTA_MAX 10

void hal_cpu_set_hpm_div(unsigned int div);
void hal_cpu_hpm_init(void);
int hal_cpu_get_average_hpm(void);


#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif /* End of #ifdef __cplusplus */
    
#endif/* End of #ifndef __HI_HAL_CPU_AVS_H__*/


