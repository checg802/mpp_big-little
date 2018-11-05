cmd_/home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/component/isp/firmware/drv/isp.o := arm-hisiv500-linux-gcc -Wp,-MD,/home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/component/isp/firmware/drv/.isp.o.d  -nostdinc -isystem /opt/hisi-linux/x86-arm/arm-hisiv500-linux/bin/../lib/gcc/arm-hisiv500-linux-uclibcgnueabi/4.9.4/include -I./arch/arm/include -Iarch/arm/include/generated  -Iinclude -I./arch/arm/include/uapi -Iarch/arm/include/generated/uapi -I./include/uapi -Iinclude/generated/uapi -include ./include/linux/kconfig.h -D__KERNEL__ -mlittle-endian -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -Wno-format-security -std=gnu89 -fno-dwarf2-cfi-asm -fno-ipa-sra -mabi=aapcs-linux -mno-thumb-interwork -mfpu=vfp -funwind-tables -marm -D__LINUX_ARM_ARCH__=7 -march=armv7-a -msoft-float -Uarm -fno-delete-null-pointer-checks -O2 --param=allow-store-data-races=0 -Wframe-larger-than=1024 -fno-stack-protector -Wno-unused-but-set-variable -fomit-frame-pointer -fno-var-tracking-assignments -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fconserve-stack -Werror=implicit-int -Werror=strict-prototypes -Werror=date-time -DCC_HAVE_ASM_GOTO -Wall -Wno-error=implicit-function-declaration -DVER_X=1 -DVER_Y=0 -DVER_Z=0 -DVER_P=0 -DVER_B=10 -Wno-date-time -DENABLE_JPEGEDCF -I/home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/include -I/home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/code/mkp/include -I/home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/include -I/home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/code/arch/hi3519v101/include -I/home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/code/mkp_inc -I/home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/component/isp/firmware/vreg -I/home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/component/isp/firmware/vreg/arch/hi3519v101 -I/home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/component/isp/include -I/home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/drv/extdrv/linux/i2c -I/home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/drv/extdrv/linux/hi_i2c -I/home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/drv/extdrv/linux/ssp-sony -I/home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/osal/include -I/home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/osal/include -I/home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/drv/extdrv/linux/i2c -I/home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/drv/extdrv/linux/hi_i2c -D ENABLE_JPEGEDCF -DHI_XXXX -DHI_GPIO_XXX -DHI_RELEASE -D__linux_  -DMODULE  -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(isp)"  -c -o /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/component/isp/firmware/drv/isp.o /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/component/isp/firmware/drv/isp.c

source_/home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/component/isp/firmware/drv/isp.o := /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/component/isp/firmware/drv/isp.c

deps_/home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/component/isp/firmware/drv/isp.o := \
    $(wildcard include/config/info/s.h) \
    $(wildcard include/config/hisi/snapshot/boot.h) \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/osal/include/hi_osal.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/osal/include/osal_list.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/osal/include/osal_ioctl.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/include/hi_common.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/include/hi_type.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/include/hi_math.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/include/hi_defines.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/include/hi_comm_video.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/include/hi_comm_isp.h \
    $(wildcard include/config/s.h) \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/include/hi_errno.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/include/hi_debug.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/include/hi_common.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/include/hi_isp_debug.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/component/isp/firmware/drv/mkp_isp.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/include/hi_type.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/include/hi_comm_3a.h \
    $(wildcard include/config/set.h) \
    $(wildcard include/config/get.h) \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/include/hi_comm_isp.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/include/hi_defines.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/component/isp/firmware/drv/isp.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/component/isp/firmware/drv/isp_ext.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/component/isp/firmware/drv/mod_ext.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/include/hi_vreg.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/component/isp/firmware/vreg/hi_drv_vreg.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/component/isp/firmware/drv/mm_ext.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/osal/include/osal_mmz.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/osal/include/hi_osal.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/include/hi_comm_sys.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/component/isp/firmware/drv/proc_ext.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/component/isp/firmware/drv/sys_ext.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/component/isp/firmware/drv/mkp_sys.h \
    $(wildcard include/config/ctrl.h) \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/component/isp/firmware/drv/mkp_ioctl.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/component/isp/firmware/drv/dev_ext.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/component/isp/firmware/drv/acm_ext.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/component/isp/firmware/drv/vb_ext.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/include/hi_comm_vb.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/component/isp/firmware/drv/mkp_vb.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/component/isp/firmware/vreg/arch/hi3519v101/isp_metering_mem_config.h \
    $(wildcard include/config/h//.h) \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/component/isp/firmware/vreg/arch/hi3519v101/isp_histogram_mem_config.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/component/isp/firmware/vreg/arch/hi3519v101/isp_gamma_rgb_mem_config.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/component/isp/firmware/vreg/arch/hi3519v101/isp_config.h \
    $(wildcard include/config/buffer/mode/default.h) \
    $(wildcard include/config/buffer/mode/datasize.h) \
    $(wildcard include/config/buffer/mode/disabled/config/updates/immediately.h) \
    $(wildcard include/config/updates/immediately.h) \
    $(wildcard include/config/buffer/mode/blocked/config/never/updates.h) \
    $(wildcard include/config/never/updates.h) \
    $(wildcard include/config/buffer/mode/local/module/config/updates/during/local/vertical/blanking.h) \
    $(wildcard include/config/updates/during/local/vertical/blanking.h) \
    $(wildcard include/config/buffer/mode/global/all/module/config/updated/during/isp/vertical/blanking.h) \
    $(wildcard include/config/updated/during/isp/vertical/blanking.h) \
    $(wildcard include/config/default.h) \
    $(wildcard include/config/datasize.h) \
    $(wildcard include/config/normal/operation.h) \
    $(wildcard include/config/hold/previous/input/port/config/state.h) \
    $(wildcard include/config/state.h) \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/component/isp/firmware/vreg/arch/hi3519v101/isp_ext_config.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/include/hi_i2c.h \
  include/linux/i2c-dev.h \
  include/uapi/linux/i2c-dev.h \
  include/linux/types.h \
    $(wildcard include/config/uid16.h) \
    $(wildcard include/config/lbdaf.h) \
    $(wildcard include/config/arch/dma/addr/t/64bit.h) \
    $(wildcard include/config/phys/addr/t/64bit.h) \
    $(wildcard include/config/64bit.h) \
  include/uapi/linux/types.h \
  arch/arm/include/asm/types.h \
  include/asm-generic/int-ll64.h \
  include/uapi/asm-generic/int-ll64.h \
  arch/arm/include/generated/asm/bitsperlong.h \
  include/asm-generic/bitsperlong.h \
  include/uapi/asm-generic/bitsperlong.h \
  include/uapi/linux/posix_types.h \
  include/linux/stddef.h \
  include/uapi/linux/stddef.h \
  include/linux/compiler.h \
    $(wildcard include/config/sparse/rcu/pointer.h) \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/enable/warn/deprecated.h) \
    $(wildcard include/config/kprobes.h) \
  include/linux/compiler-gcc.h \
    $(wildcard include/config/arch/supports/optimized/inlining.h) \
    $(wildcard include/config/optimize/inlining.h) \
  include/linux/compiler-gcc4.h \
    $(wildcard include/config/arch/use/builtin/bswap.h) \
  arch/arm/include/uapi/asm/posix_types.h \
  include/uapi/asm-generic/posix_types.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/include/hi_spi.h \
  include/uapi/linux/spi/spidev.h \

/home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/component/isp/firmware/drv/isp.o: $(deps_/home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/component/isp/firmware/drv/isp.o)

$(deps_/home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/component/isp/firmware/drv/isp.o):
