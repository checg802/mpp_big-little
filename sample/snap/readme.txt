Usage Descriptions of the HiSilicon Hi3519V101 Sample Programs

1¡¢ In the double snap mode, please load the script: 

 ./load3519v101 -a -sensor0 imx274  -osmem 64 -offline -workmode double_pipe

2¡¢In the dual pipe snap mode, because the dev0 generates a timing sequence, so the dev0 and the isp0 clock frequency is set to be equal,as shown below£º
    himm 0x12010054 0x24043  