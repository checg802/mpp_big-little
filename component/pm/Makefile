##########################################################################
#                   makefile for pm@l00165163.hisilicon                  #
##########################################################################
ifeq ($(PARAM_FILE), )
        PARAM_FILE:=../../Makefile.param
        include $(PARAM_FILE)
endif

include $(PWD)/Makefile.param

CODE_ROOT := $(SDK_PATH)/mpp/code
ARCH_ROOT := $(CODE_ROOT)/arch/$(ARCH_DIR)
			
CFG_INC += -I$(REL_INC)

EXTRA_CFLAGS += $(CFG_INC) 
EXTRA_CFLAGS += -Wall -D$(HIARCH) -DHICHIP=$(HICHIP) -D$(HIDBG) -D$(HI_FPGA)

ifeq ($(HIGDB), y)
     EXTRA_CFLAGS += -g
endif

include $(PWD)/pm/Makefile
include $(PWD)/avs/Makefile
include $(PWD)/dvfs/Makefile
include $(PWD)/regulator/Makefile
include $(PWD)/domain_cpu/Makefile
include $(PWD)/domain_media/Makefile
include $(PWD)/chip/$(HIARCH)/Makefile

ifeq ($(CPU_EN), y)
    EXTRA_CFLAGS += -DCPU_PM
    ifeq ($(CPU_VOLTAGE_MODE),PWM)
        EXTRA_CFLAGS += -DCPU_PWM
    endif
    
    ifeq ($(CPU_AVS_EN), y)
        EXTRA_CFLAGS += -DCPU_AVS
	include $(PWD)/avs/Makefile
    
        ifeq ($(CPU_AVS_HPM), y)
        EXTRA_CFLAGS += -DCPU_AVS_HPM
        endif

        ifeq ($(CPU_AVS_HW), y)
        EXTRA_CFLAGS += -DCPU_AVS_HW
        endif
   endif
endif

ifeq ($(MEDIA_EN), y)
    EXTRA_CFLAGS += -DMEDIA_PM
    ifeq ($(MEDIA_VOLTAGE_MODE), PWM)
        EXTRA_CFLAGS += -DMEDIA_PWM
    endif
    
    ifeq ($(MEDIA_AVS_EN), y)
	EXTRA_CFLAGS += -DMEDIA_AVS
        include $(PWD)/avs/Makefile
    
        ifeq ($(MEDIA_AVS_HPM), y)
        EXTRA_CFLAGS += -DMEDIA_AVS_HPM
        endif

        ifeq ($(MEDIA_AVS_HW), y)
        EXTRA_CFLAGS += -DMEDIA_AVS_HW
        endif
   endif
endif

ifeq ($(PM_DEBUG_ON), y)
	EXTRA_CFLAGS += -DPM_DEBUG
endif

EXTRA_CFLAGS += -I$(PWD)/include
EXTRA_CFLAGS += -I$(PWD)/avs/
EXTRA_CFLAGS += -I$(PWD)/dvfs
EXTRA_CFLAGS += -I$(PWD)/regulator
EXTRA_CFLAGS += -I$(PWD)/domain_cpu
EXTRA_CFLAGS += -I$(PWD)/domain_media
EXTRA_CFLAGS += -I$(PWD)/chip/$(HIARCH)/include
 
obj-m := $(HIARCH)_pm.o
$(HIARCH)_pm-objs := $(PM_SRC:%.c=%.o) 


default:
ifeq ($(CPU_TYPE),big-little)
	@make -C $(KERNEL_ROOT) M=$(PWD) modules
	cp -rf $(HIARCH)_pm.ko $(REL_KO)
else
	@echo -e "\n\n Only CPU_TYPE=big-little support pm module,now CPU_TYPE is $(CPU_TYPE) .\n\n"
endif
	
clean: 
	@make -C $(KERNEL_ROOT) M=$(PWD) clean
	rm -rf  $(REL_KO)/$(HIARCH)_pm.ko
