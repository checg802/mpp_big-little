cmd_/home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/component/isp/firmware/drv/../vreg/hi_vreg.o := arm-hisiv500-linux-gcc -Wp,-MD,/home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/component/isp/firmware/drv/../vreg/.hi_vreg.o.d  -nostdinc -isystem /opt/hisi-linux/x86-arm/arm-hisiv500-linux/bin/../lib/gcc/arm-hisiv500-linux-uclibcgnueabi/4.9.4/include -I./arch/arm/include -Iarch/arm/include/generated  -Iinclude -I./arch/arm/include/uapi -Iarch/arm/include/generated/uapi -I./include/uapi -Iinclude/generated/uapi -include ./include/linux/kconfig.h -D__KERNEL__ -mlittle-endian -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -Wno-format-security -std=gnu89 -fno-dwarf2-cfi-asm -fno-ipa-sra -mabi=aapcs-linux -mno-thumb-interwork -mfpu=vfp -funwind-tables -marm -D__LINUX_ARM_ARCH__=7 -march=armv7-a -msoft-float -Uarm -fno-delete-null-pointer-checks -O2 --param=allow-store-data-races=0 -Wframe-larger-than=1024 -fno-stack-protector -Wno-unused-but-set-variable -fomit-frame-pointer -fno-var-tracking-assignments -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fconserve-stack -Werror=implicit-int -Werror=strict-prototypes -Werror=date-time -DCC_HAVE_ASM_GOTO -Wall -Wno-error=implicit-function-declaration -DVER_X=1 -DVER_Y=0 -DVER_Z=0 -DVER_P=0 -DVER_B=10 -Wno-date-time -DENABLE_JPEGEDCF -I/home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/include -I/home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/code/mkp/include -I/home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/include -I/home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/code/arch/hi3519v101/include -I/home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/code/mkp_inc -I/home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/component/isp/firmware/vreg -I/home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/component/isp/firmware/vreg/arch/hi3519v101 -I/home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/component/isp/include -I/home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/drv/extdrv/linux/i2c -I/home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/drv/extdrv/linux/hi_i2c -I/home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/drv/extdrv/linux/ssp-sony -I/home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/osal/include -I/home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/osal/include -I/home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/drv/extdrv/linux/i2c -I/home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/drv/extdrv/linux/hi_i2c -D ENABLE_JPEGEDCF -DHI_XXXX -DHI_GPIO_XXX -DHI_RELEASE -D__linux_  -DMODULE  -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(hi_vreg)"  -c -o /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/component/isp/firmware/drv/../vreg/hi_vreg.o /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/component/isp/firmware/drv/../vreg/hi_vreg.c

source_/home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/component/isp/firmware/drv/../vreg/hi_vreg.o := /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/component/isp/firmware/drv/../vreg/hi_vreg.c

deps_/home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/component/isp/firmware/drv/../vreg/hi_vreg.o := \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/osal/include/hi_osal.h \
    $(wildcard include/config/hisi/snapshot/boot.h) \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/osal/include/osal_list.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/osal/include/osal_ioctl.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/include/hi_vreg.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/include/hi_type.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/include/hi_comm_isp.h \
    $(wildcard include/config/s.h) \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/include/hi_errno.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/include/hi_debug.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/include/hi_common.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/include/hi_math.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/include/hi_defines.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/include/hi_comm_video.h \
    $(wildcard include/config/info/s.h) \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/include/hi_isp_debug.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/include/hi_comm_isp.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/component/isp/firmware/drv/../vreg/hi_drv_vreg.h \
  /home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/include/hi_comm_3a.h \
    $(wildcard include/config/set.h) \
    $(wildcard include/config/get.h) \

/home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/component/isp/firmware/drv/../vreg/hi_vreg.o: $(deps_/home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/component/isp/firmware/drv/../vreg/hi_vreg.o)

$(deps_/home/davis/workspace/Hi3519V101_SDK_V1.0.3.0/mpp_big-little/component/isp/firmware/drv/../vreg/hi_vreg.o):
