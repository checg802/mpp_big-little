#ifndef  __HI_HAL_MEDIA_AVS_H__
#define  __HI_HAL_MEDIA_AVS_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* Begin of #ifdef __cplusplus */

#define MEDIA_AVS_VOLT_STEP 10000 /*mv*/
#define MEDIA_AVS_HPM_DELTA_MIN 1 
#define MEDIA_AVS_HPM_DELTA_MAX 16

#define MEDIA_AVS_TEMP_DELTA_MIN 5 


void hal_media_set_hpm_div(unsigned int div);
void hal_media_hpm_init(void);
void hal_media_tsensor_init(void);
int hal_media_get_average_hpm(void);
int hal_media_get_average_temperature(void);
int hal_media_get_profile(void);


#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif /* End of #ifdef __cplusplus */
    
#endif/* End of #ifndef __HI_HAL_MEDIA_AVS_H__*/


