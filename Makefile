# $(ROOT)/mpp/Makefile

ifeq ($(PARAM_FILE),) 
    PARAM_FILE:=./Makefile.param
    include $(PARAM_FILE)
endif

.PHONY:all prepare drv clean
 
all: prepare drv

prepare:
	@cd $(SDK_PATH)/osal/source/kernel; make

drv:
	@cd $(SDK_PATH)/drv/extdrv;   make
	@cd $(SDK_PATH)/drv/interdrv; make



clean:
	@cd $(SDK_PATH)/osal/source/kernel; make clean
	@cd $(SDK_PATH)/drv/interdrv;       make clean
	@cd $(SDK_PATH)/drv/extdrv;         make clean
	
