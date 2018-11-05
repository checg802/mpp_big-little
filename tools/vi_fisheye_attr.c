#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>


#include "hi_common.h"
#include "hi_comm_video.h"
#include "hi_comm_vi.h"
#include "mpi_vi.h"

static void usage(void)
{
    #ifdef __HuaweiLite__
    printf("Usage: vi_fisheye_attr para value. sample: vi_fisheye_attr ViExtChn ViewMode 1\n");
    #else
    printf("Usage: ./vi_fisheye_attr para value. sample: ./vi_fisheye_attr ViExtChn ViewMode 1\n");
    #endif
    
	printf("ViExtChn  	The number of vi extend channel[1, 8]\n");
	printf("para: \n");
	printf("bEnable  	[0, disable; 1,enable]\n");
	printf("bBgColor 	[0, disable; 1,enable]\n");
	printf("Color   	[Value of background color, RGB888 format,value:0~0xffffff]\n");
	printf("ViewMode	[Correction mode, 0,360 panorama; 1,180 panorama; 2,normal; 3,no_transformation]\n");
	printf("MountMode	[Mount mode, 0,desktop; 1,ceiling; 2,wall]\n");
    printf("FanStrength	[Strength of sector correction,value:-760~760]\n");
	printf("TrapeCoef	[Strength of trapezoidal correction,value:0~160]\n");
	printf("InRadius 	[Inside  radius, value:0~OutRadius-1]\n");
	printf("OutRadius	[Outside radius, value:1~2304]\n");
	printf("HorOffset 	[The horizontal offset of the lens center with respect to the SENSOR center, value:-127~127]\n");
	printf("VerOffset	[The vertical offset of the lens center with respect to the SENSOR center, value:-127~127]\n");
	printf("Pan		[Pan,value:0~360]\n");
	printf("Tilt    	[Tilt,value:0~360]\n");
	printf("HorZoom  	[Horizontal zoom, value:1~4095]\n");
	printf("VerZoom  	[Vertical   zoom, value:1~4095]\n");
    exit(1);
}


#ifdef __HuaweiLite__
HI_S32 vi_fisheye_attr(int argc, char* argv[])
#else
HI_S32 main(int argc, char* argv[])
#endif
{
	HI_S32			s32Ret;
	VI_CHN 			ViExtChn = 1;
	FISHEYE_ATTR_S	stFisheyeAttr;

	char paraTemp[16];    
	HI_S32 s32Value	= 0;    
	
	const char *para = paraTemp;
	
    printf("\nNOTICE: This tool only can be used for TESTING !!!\n");
    printf("usage:  ./vi_fisheye_attr ViExtChn [ViewMode] value. sample: ./vi_fisheye_attr 2 ViewMode 1\n");
	
	if (argc < 4)
	{
		printf("input arg number is wrong!\n");
		usage();
	}

	memset(&stFisheyeAttr,0,sizeof(FISHEYE_ATTR_S));

	ViExtChn	= atoi(argv[1]);
	strncpy(paraTemp, argv[2], sizeof(paraTemp) - 1); 
	paraTemp[sizeof(paraTemp) - 1] = '\0';
	s32Value 	= atoi(argv[3]);    

	if (ViExtChn < 1 || ViExtChn > 8)
	{
		printf("input Vi extchn:%d is wrong!\n", ViExtChn);
		usage();
	}

	s32Ret = HI_MPI_VI_GetFisheyeAttr(ViExtChn, &stFisheyeAttr);
	if (HI_SUCCESS != s32Ret)
	{
		 printf("HI_MPI_VI_GetFisheyeAttr failed\n");
		return s32Ret;
	}
	
	if (0 == strcmp(para, "bEnable"))    
	{        
		stFisheyeAttr.bEnable = s32Value;    
		printf("bEnable:%d\n", stFisheyeAttr.bEnable);
	}  
	else if (0 == strcmp(para, "bBgColor"))    
	{        
		stFisheyeAttr.bBgColor = s32Value;    
		printf("bBgColor:%d\n", stFisheyeAttr.bBgColor);
	}
	else if (0 == strcmp(para, "Color"))    
	{        
		stFisheyeAttr.u32BgColor = s32Value;    
		printf("u32BgColor:0x%x\n", stFisheyeAttr.u32BgColor);
	}
	else if (0 == strcmp(para, "ViewMode"))    
	{        
		stFisheyeAttr.astFisheyeRegionAttr[0].enViewMode = (FISHEYE_VIEW_MODE_E)s32Value;    
		printf("enViewMode:%d\n", stFisheyeAttr.astFisheyeRegionAttr[0].enViewMode);
	}
	else if (0 == strcmp(para, "MountMode"))    
	{        
		stFisheyeAttr.enMountMode = (FISHEYE_MOUNT_MODE_E)s32Value;   
		printf("enMountMode:%d\n", stFisheyeAttr.enMountMode);
		
	}    
	else if (0 == strcmp(para, "FanStrength"))    
	{        
		stFisheyeAttr.s32FanStrength = s32Value;   
		printf("s32FanStrength:%d\n", stFisheyeAttr.s32FanStrength);
		
	} 
	else if (0 == strcmp(para, "TrapeCoef"))    
	{        
		stFisheyeAttr.u32TrapezoidCoef = s32Value;   
		printf("u32TrapezoidCoef:%d\n", stFisheyeAttr.u32TrapezoidCoef);
		
	} 
	else if (0 == strcmp(para, "InRadius"))    
	{        
		stFisheyeAttr.astFisheyeRegionAttr[0].u32InRadius = s32Value;    
		printf("u32InRadius:%d\n", stFisheyeAttr.astFisheyeRegionAttr[0].u32InRadius);
	}    
	else if (0 == strcmp(para, "OutRadius"))    
	{        
		stFisheyeAttr.astFisheyeRegionAttr[0].u32OutRadius = s32Value;    
		printf("u32OutRadius:%d\n", stFisheyeAttr.astFisheyeRegionAttr[0].u32OutRadius);  
	}    
	else if (0 == strcmp(para, "HorOffset"))    
	{        
		stFisheyeAttr.s32HorOffset = s32Value;    
		printf("s32HorOffset:%d\n", stFisheyeAttr.s32HorOffset);  
	}
	else if (0 == strcmp(para, "VerOffset"))    
	{        
		stFisheyeAttr.s32VerOffset = s32Value;    
		printf("s32VerOffset:%d\n", stFisheyeAttr.s32VerOffset);  
	}
	else if (0 == strcmp(para, "Pan"))    
	{        
		stFisheyeAttr.astFisheyeRegionAttr[0].u32Pan = s32Value;    
		printf("u32Pan:%d\n", stFisheyeAttr.astFisheyeRegionAttr[0].u32Pan); 
	}
	else if (0 == strcmp(para, "Tilt"))    
	{        
		stFisheyeAttr.astFisheyeRegionAttr[0].u32Tilt = s32Value;    
		printf("u32Tilt:%d\n", stFisheyeAttr.astFisheyeRegionAttr[0].u32Tilt);
	}
	else if (0 == strcmp(para, "HorZoom"))    
	{        
		stFisheyeAttr.astFisheyeRegionAttr[0].u32HorZoom = s32Value;    
		printf("u32HorZoom:%d\n", stFisheyeAttr.astFisheyeRegionAttr[0].u32HorZoom);
	}
	else if (0 == strcmp(para, "VerZoom"))    
	{        
		stFisheyeAttr.astFisheyeRegionAttr[0].u32VerZoom = s32Value;    
		printf("u32VerZoom:%d\n", stFisheyeAttr.astFisheyeRegionAttr[0].u32VerZoom);
	}
	else    
	{        
		printf("err para\n");        
		usage();  
	}

	//stFisheyeAttr.u32RegionNum = 1;

	s32Ret = HI_MPI_VI_SetFisheyeAttr(ViExtChn, &stFisheyeAttr);
	if (HI_SUCCESS != s32Ret)
	{
		 printf("HI_MPI_VI_SetFisheyeAttr failed\n");
		return s32Ret;
	}
	
    return HI_SUCCESS;
}



