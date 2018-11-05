# $(ROOT)/mpp/code/component/pci/Makefile

ifeq ($(PARAM_FILE), ) 
    PARAM_FILE:=../../Makefile.param
    include $(PARAM_FILE)
endif

DCI_HEADER := hi_defines.h
COM_HEADER := hi_comm_video.h hi_comm_pciv.h hi_comm_vb.h hi_comm_vgs.h hi_comm_sys.h hi_comm_region.h hi_comm_vo.h hi_type.h hi_common.h hi_math.h hi_errno.h hi_debug.h
MKP_HEADER := mkp_ioctl.h mkp_sys.h mkp_vb.h
EXT_HEADER := vgs_ext.h mm_ext.h mod_ext.h dev_ext.h pciv_fmwext.h proc_ext.h sys_ext.h vb_ext.h region_ext.h
VPSS_HEADER := vpss_ext.h mkp_vpss.h
VPSS_COMM_HEADER := hi_comm_vpss.h
OSAL_HEADER := hi_osal.h osal_list.h osal_ioctl.h osal_mmz.h

INC_DIR=$(MPP_PATH)/component/pci/include

.PHONY: default clean

default:
ifeq ($(MPP_BUILD), y)
	@-pushd $(MPP_PATH)/component/pci/include > /dev/null; rm -f $(COM_HEADER) rm -f $(MKP_HEADER); rm -f $(EXT_HEADER); popd > /dev/null
	@-pushd $(MPP_PATH)/code/mkp_inc > /dev/null;                      cp $(MKP_HEADER) $(INC_DIR); popd > /dev/null
	@-pushd $(MPP_PATH)/code/mkp/include > /dev/null;                  cp $(EXT_HEADER) $(INC_DIR); popd > /dev/null
	@-pushd $(MPP_PATH)/code/include > /dev/null;                      cp $(COM_HEADER) $(INC_DIR); popd > /dev/null
	@-pushd $(MPP_PATH)/code/arch/$(ARCH_DIR)/include > /dev/null;     cp $(DCI_HEADER) $(INC_DIR); popd > /dev/null
	@-pushd $(MPP_PATH)/code/shelf/vpss2/mkp/include > /dev/null;      cp $(VPSS_HEADER) $(INC_DIR); popd > /dev/null
	@-pushd $(MPP_PATH)/code/shelf/vpss2/product/IPC > /dev/null;      cp $(VPSS_COMM_HEADER) $(INC_DIR); popd > /dev/null
	@-pushd $(SDK_PATH)/osal/include > /dev/null;      		     	   cp $(OSAL_HEADER) $(INC_DIR); popd > /dev/null
endif
	@for x in `find ./ -maxdepth 2 -mindepth 2 -name "Makefile" `; \
	   do cd `dirname $$x`; if [ $$? ]; then make || exit 1; cd ../; fi; done

clean:
	@for x in `find ./ -maxdepth 2 -mindepth 2 -name "Makefile" `; \
	   do cd `dirname $$x`; if [ $$? ]; then make clean || exit 1; cd ../; fi; done

ifeq ($(MPP_BUILD), y)
	@-pushd $(MPP_PATH)/component/pci/include > /dev/null; popd > /dev/null
endif

