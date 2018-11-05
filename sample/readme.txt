Usage Descriptions of the HiSilicon Hi3519v101 Sample Programs

1. Sample Program File Structure
   sample            # MPP sample program
    |-- common       # Common function used by the sample program
    |-- vio          # Video input/output implementation demo
    |-- venc         # Video encoding implementation demo
    |-- region       # Region implementation demo
    |-- dis          # Digital Image Stabilization (DIS) implementation demo
    |-- audio        # Audio implementation demo
    |-- hifb         # Frame buffer (FB) implementation demo
    |-- tde          # TDE implementation demo
    |-- ......    


2. Compilation and Makefile
   1) Compilation dependency descriptions
   The compilation of the sample programs depends on MPP header files in /mpp/include and library files in mpp/lib.
   
   2) Makefile descriptions    
   A Makefile is available on each service sample demo module (such as the vio and venc). The Makefile quotes the Makefile.param file in the sample directory, and this file quotes the Makefile.param file in the mpp directory.
   mpp -- Makefile.param                 # Defines the variables required by the MPP compilation.
    |---sample---Makefile.param          # Defines the variables required by the sample compilation.
          |--- vio/venc/...---Makefile   # Sample compilation script

   Run the make command on each service sample demo module to compile the module; run the make clean command to delete the executable files and target files after compilation; run the make cleanstream command to delete stream files generated after some of the sample programs run. 

   Run the make command in the sample directory to compile various service sample demo programs; run the make clean command to delete the executable files and target files after all service sample demo programs are compiled; run the make cleanall command to delete stream files generated after all service sample demo programs run.

3. Running Descriptions of Sample Programs
   1) Sample program running depends on the media driver. Before running a sample demo program, execute the load3519v101 script in the mpp_big-little/ko to load a specified module.
   
   2) Note that the corresponding sensor library must be selected. The default sensor is Aptina imx274. If you want to select other sensors, modify the Makefile.param file. For example, if you select Sony imx290, you should modify the Makefile.param file as follows:
   		SNS=imx290_mipi

   3) The default VI and ISP clocks are both 300M HZ,these clocks can support most scenes,but some other scenes must use higher VI/ISP clock,please refer to /mpp/component/isp/sensor/readme_cn.txt or /mpp/component/isp/sensor/readme_en.txt.

   4) The default VEDU clocks are 500M HZ,this clock can support most scenes,but some other scenes must use higher VEDU clock;for example:if 
VEDU supports H.265/H.264 mutli-stream encoding with the performance of 
3840*2160@30fps+1080p@30fps, the higher VEDU clock(600M HZ) is needed. 

4. Change History
   1) 2016-07 This document was created.


5. Copyright
Copyright @ HiSilicon Technologies Co., Ltd. 2016. All rights reserved.
No part of this document may be reproduced or transmitted in any form or by any means without prior written consent of HiSiliconTechnologies Co., Ltd. The sample programs are only used as user guides. All statements, information, and recommendations in this document do not constitute a warranty of any kind, express or implied.


-------------------------------------------------------------------------------
HiSilicon Technologies Co., Ltd.
Address: Huawei Industrial Base
Bantian, Longgang
Shenzhen 518129
People's Republic of China
Website: http://www.hisilicon.comEmail: support@hisilicon.com
