1.Before using the gyro sensor,the config of kernel and some files should be modified firstly.
run command :make ARCH=arm CROSS_COMPILE=arm-hisiv500-linux- menuconfig

1).Select Periodic timer ticks and exclude Old Idle dynticks config.
  a. > General setup > Timers subsystem > Timer tich handling
 (X) Periodic timer ticks (constant rate, no dynticks)

  b.exclude Old Idle dynticks config.
  
  After modified ,it shows as follow:
      Timer tick handling (Periodic timer ticks (constant rate, no dynticks))  ---> 
 [ ] Old Idle dynticks config                                                      
 [ ] High Resolution Timer Support                                                 

2).Modify the timer frequency  to 1000Hz.
 > Kernel Features>Timer frequency (1000 Hz)

3).Ensure the Hisilicon DMAC Controller support is selected.
   If not ,please select Hisilicon DMAC Controller support.
   Device Drivers--->
   <*> Hisilicon DMAC Controller support

4).Modify the clock-frequency of i2c_bus2 from 100000 to 400000.
  The clock-frequency of i2c_bus2 is in the file "linux-3.18.y/arch/arm/boot/dts/hisi-hi3519v101.dtsi".
   
i2c_bus2: i2c@12112000 {
      compatible = "hisilicon,hisi-i2c-v110";
      reg = <0x12112000 0x100>;
      clocks = <&clock HI3519_I2C_MUX>;
      clock-frequency = <400000>;
      status = "disabled";	

2.After insmod the ko of the gyro sensor,please enable the clock of DMAC. 
 Do as follow:
 himm 0x120100e0 0xaa
