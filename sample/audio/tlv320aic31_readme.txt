1. Update the file: mpp/ko/pinmux_hi3519v101.sh,
   It need to use I2S interface, so open the pinmux setting as below. 
	i2c2_pin_mux;
	i2s_pin_mux;

2. update the file: mpp/ko/sysctl_hi3519v101.sh
   It need to use the external audio codec, please modify the script as below. 
	#himm 0x120300e0 0xd				# internal codec: AIO MCLK out, CODEC AIO TX MCLK 
	himm 0x120300e0 0xe				# external codec: AIC31 AIO MCLK out, CODEC AIO TX MCLK

3. update the file: mpp\ko\load3519v101
   It need to insmod tlv_320aic31's driver. 
	insmod extdrv/hi_tlv320aic31.ko

4. modify the makefile parameter: mpp/sample/Makefile.param. Set ACODEC_TYPE to  ACODEC_TYPE_TLV320AIC31. 
   It means use the external codec tlv_320aic31 sample code.
	################ select audio codec type for your sample ################
	#ACODEC_TYPE ?= ACODEC_TYPE_INNER
	#external acodec
	ACODEC_TYPE ?= ACODEC_TYPE_TLV320AIC31

5. Rebuild the sample and get the sample_audio.
