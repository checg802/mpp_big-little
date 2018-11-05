#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/prctl.h>

#include "hi_sceneauto_comm.h"
#include "hi_type.h"
#include "hi_comm_vpss.h"
#include "mpi_vpss.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

void V19yPrintNRs(const HI_SCENEAUTO_3DNR_S *pS )
{
  printf("\n\n                             sizeof( S ) = %d ", sizeof(*pS) );
  printf("\n*******************************************************************************");
  
  printf("\n   sf0_bright  ( %3d )      sfx_bright    ( %3d,%3d )      sfk_bright ( %3d )",  pS->sf0_bright,  pS->sf1_bright,    pS->sf2_bright, pS->sfk_bright  );    
  printf("\n   sf0_dark    ( %3d )      sfx_dark      ( %3d,%3d )      sfk_dark   ( %3d )",  pS->sf0_dark  ,  pS->sf1_dark  ,    pS->sf2_dark  , pS->sfk_dark    );       
  printf("\n  ----------------------+                                  sfk_pro    ( %3d )",                                                      pS->sfk_profile );
  printf("\n                        |   tfx_moving    ( %3d,%3d )      sfk_ratio  ( %3d )",                   pS->tf1_moving,    pS->tf2_moving, pS->sfk_ratio   ); 
  printf("\n   csf_strength( %3d )  |   tfx_md_thresh ( %3d,%3d )                        ",  pS->csf_strength,pS->tf1_md_thresh, pS->tf2_md_thresh                  ); 
  printf("\n   ctf_strength( %3d )  |   tfx_md_profile( %3d,%3d )    sf_ed_thresh ( %3d )",  pS->ctf_strength,pS->tf1_md_profile,pS->tf2_md_profile,pS->sf_ed_thresh); 
  printf("\n   ctf_range   ( %3d )  |   tfx_md_type   ( %3d,%3d )    sf_bsc_freq  ( %3d )",  pS->ctf_range,   pS->tf1_md_type,   pS->tf2_md_type,   pS->sf_bsc_freq ); 
  printf("\n                        |   tfx_still     ( %3d,%3d )    tf_profile   ( %3d )",                   pS->tf1_still,     pS->tf2_still,     pS->tf_profile  ); 
  
  printf("\n*******************************************************************************\n\n");  
}

void V19yPrintVpssNRs(const VPSS_GRP_NRS_PARAM_S *pS )
{
  printf("\n\n                             sizeof( S ) = %d ", sizeof(*pS) );
  printf("\n*******************************************************************************");
  
  printf("\n   sf0_bright  ( %3d )      sfx_bright    ( %3d,%3d )      sfk_bright ( %3d )",  pS->stNRSParam_V3.sf0_bright,  pS->stNRSParam_V3.sf1_bright,    pS->stNRSParam_V3.sf2_bright, pS->stNRSParam_V3.sfk_bright  );    
  printf("\n   sf0_dark    ( %3d )      sfx_dark      ( %3d,%3d )      sfk_dark   ( %3d )",  pS->stNRSParam_V3.sf0_dark  ,  pS->stNRSParam_V3.sf1_dark  ,    pS->stNRSParam_V3.sf2_dark  , pS->stNRSParam_V3.sfk_dark    );       
  printf("\n  ----------------------+                                  sfk_pro    ( %3d )",                                                      pS->stNRSParam_V3.sfk_profile );
  printf("\n                        |   tfx_moving    ( %3d,%3d )      sfk_ratio  ( %3d )",                   pS->stNRSParam_V3.tf1_moving,    pS->stNRSParam_V3.tf2_moving, pS->stNRSParam_V3.sfk_ratio   ); 
  printf("\n   csf_strength( %3d )  |   tfx_md_thresh ( %3d,%3d )                        ",  pS->stNRSParam_V3.csf_strength,pS->stNRSParam_V3.tf1_md_thresh, pS->stNRSParam_V3.tf2_md_thresh                  ); 
  printf("\n   ctf_strength( %3d )  |   tfx_md_profile( %3d,%3d )    sf_ed_thresh ( %3d )",  pS->stNRSParam_V3.ctf_strength,pS->stNRSParam_V3.tf1_md_profile,pS->stNRSParam_V3.tf2_md_profile,pS->stNRSParam_V3.sf_ed_thresh); 
  printf("\n   ctf_range   ( %3d )  |   tfx_md_type   ( %3d,%3d )    sf_bsc_freq  ( %3d )",  pS->stNRSParam_V3.ctf_range,   pS->stNRSParam_V3.tf1_md_type,   pS->stNRSParam_V3.tf2_md_type,   pS->stNRSParam_V3.sf_bsc_freq ); 
  printf("\n                        |   tfx_still     ( %3d,%3d )    tf_profile   ( %3d )",                   pS->stNRSParam_V3.tf1_still,     pS->stNRSParam_V3.tf2_still,     pS->stNRSParam_V3.tf_profile  ); 
  
  printf("\n*******************************************************************************\n\n");  
}

HI_S32 Sceneauto_SetNrS(VPSS_GRP VpssGrp, const HI_SCENEAUTO_3DNR_S *pst3Nrs)
{
    HI_S32 s32Ret = HI_SUCCESS;
	VPSS_GRP_NRS_PARAM_S stVpssGrpNrS , stVpssGrpNrSGet;

    stVpssGrpNrS.enNRVer = VPSS_NR_V3;
	s32Ret = HI_MPI_VPSS_GetGrpNRSParam(VpssGrp, &stVpssGrpNrS);
	if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VPSS_GetNRSParam failed\n");
        return HI_FAILURE;
    }

    stVpssGrpNrS.stNRSParam_V3.sf0_bright =  pst3Nrs->sf0_bright;
    stVpssGrpNrS.stNRSParam_V3.sf1_bright =  pst3Nrs->sf1_bright;
    stVpssGrpNrS.stNRSParam_V3.sf2_bright =  pst3Nrs->sf2_bright;
    stVpssGrpNrS.stNRSParam_V3.sfk_bright =  pst3Nrs->sfk_bright;

    stVpssGrpNrS.stNRSParam_V3.sf0_dark =  pst3Nrs->sf0_dark;
    stVpssGrpNrS.stNRSParam_V3.sf1_dark =  pst3Nrs->sf1_dark;
    stVpssGrpNrS.stNRSParam_V3.sf2_dark =  pst3Nrs->sf2_dark;
    stVpssGrpNrS.stNRSParam_V3.sfk_dark =  pst3Nrs->sfk_dark;

    stVpssGrpNrS.stNRSParam_V3.sf_ed_thresh =  pst3Nrs->sf_ed_thresh;
    stVpssGrpNrS.stNRSParam_V3.sfk_profile =  pst3Nrs->sfk_profile;
    stVpssGrpNrS.stNRSParam_V3.sfk_ratio =  pst3Nrs->sfk_ratio;
    stVpssGrpNrS.stNRSParam_V3.sf_bsc_freq =  pst3Nrs->sf_bsc_freq;

    stVpssGrpNrS.stNRSParam_V3.tf1_md_thresh =  pst3Nrs->tf1_md_thresh;
    stVpssGrpNrS.stNRSParam_V3.tf2_md_thresh =  pst3Nrs->tf2_md_thresh;
    stVpssGrpNrS.stNRSParam_V3.tf1_still =  pst3Nrs->tf1_still;
    stVpssGrpNrS.stNRSParam_V3.tf2_still =  pst3Nrs->tf2_still;
    stVpssGrpNrS.stNRSParam_V3.tf1_md_type =  pst3Nrs->tf1_md_type;
    stVpssGrpNrS.stNRSParam_V3.tf2_md_type =  pst3Nrs->tf2_md_type;
    stVpssGrpNrS.stNRSParam_V3.tf1_moving =  pst3Nrs->tf1_moving;
    stVpssGrpNrS.stNRSParam_V3.tf2_moving =  pst3Nrs->tf2_moving;
    stVpssGrpNrS.stNRSParam_V3.tf1_md_profile =  pst3Nrs->tf1_md_profile;
    stVpssGrpNrS.stNRSParam_V3.tf2_md_profile =  pst3Nrs->tf2_md_profile;
    stVpssGrpNrS.stNRSParam_V3.tf_profile =  pst3Nrs->tf_profile;

    stVpssGrpNrS.stNRSParam_V3.csf_strength =  pst3Nrs->csf_strength;
    stVpssGrpNrS.stNRSParam_V3.ctf_strength =  pst3Nrs->ctf_strength;
    stVpssGrpNrS.stNRSParam_V3.ctf_range =  pst3Nrs->ctf_range;
    
    //printf("\nVpssNR_Set S:\n");
    //V19yPrintVpssNRs(&stVpssGrpNrS);
    
    s32Ret = HI_MPI_VPSS_SetGrpNRSParam(VpssGrp, &stVpssGrpNrS);
	if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VPSS_SetNRSParam failed\n");
        return HI_FAILURE;
    }

    stVpssGrpNrSGet.enNRVer = VPSS_NR_V3;
    s32Ret = HI_MPI_VPSS_GetGrpNRSParam(VpssGrp, &stVpssGrpNrSGet);
    //printf("\nVpssNR_Get S:\n");
    //V19yPrintVpssNRs(&stVpssGrpNrSGet);

    return HI_SUCCESS;
}



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
